#include "types.h"
#include "elf_abi.h"
#include "kexploit.h"
#include "structs.h"
#include "../src/common/common.h"
#include "../src/common/os_defs.h"
#include "../../libwiiu/src/coreinit.h"

//! this shouldnt depend on OS
#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0xC0000000
#define DATA_RW_BASE_OFFSET                             0

#if VER == 532
    #define ADDRESS_OSTitle_main_entry_ptr              0x1005d180
    #define ADDRESS_main_entry_hook                     0x0101c55c
    #define ADDRESS_LiWaitOneChunk                      0x010007EC
    #define ADDRESS_LiWaitIopComplete                   0x0100FFA4
    #define ADDRESS_LiWaitIopCompleteWithInterrupts     0x0100FE90
#endif // VER

/* Install functions */
static void InstallMain(private_data_t *private_data);
static void InstallPatches(private_data_t *private_data);
static void ExitFailure(private_data_t *private_data, const char *failure);

static int show_install_menu(unsigned int coreinit_handle, unsigned int *ip_address);
static void curl_thread_callback(int argc, void *argv);

static void SetupKernelSyscall(unsigned int addr);

/* assembly functions */
extern void Syscall_0x36(void);
extern void KernelPatches(void);

/* ****************************************************************** */
/*                               ENTRY POINT                          */
/* ****************************************************************** */
void __main(void)
{
    /* Get coreinit handle and keep it in memory */
    unsigned int coreinit_handle;
    OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);

    /* Get our memory functions */
    unsigned int* functionPointer;
    void* (*memset)(void * dest, unsigned int value, unsigned int bytes);
    OSDynLoad_FindExport(coreinit_handle, 0, "memset", &memset);

    private_data_t private_data;
    memset(&private_data, 0, sizeof(private_data_t));

    private_data.coreinit_handle = coreinit_handle;
    private_data.memset = memset;
    private_data.data_elf = (unsigned char *) 0xC0E00000; // use this address as temporary to load the elf

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPointer);
    private_data.MEMAllocFromDefaultHeapEx = (void*(*)(unsigned int, unsigned int))*functionPointer;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPointer);
    private_data.MEMFreeToDefaultHeap = (void (*)(void *))*functionPointer;

    OSDynLoad_FindExport(coreinit_handle, 0, "memcpy", &private_data.memcpy);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSEffectiveToPhysical", &private_data.OSEffectiveToPhysical);
    OSDynLoad_FindExport(coreinit_handle, 0, "DCFlushRange", &private_data.DCFlushRange);
    OSDynLoad_FindExport(coreinit_handle, 0, "ICInvalidateRange", &private_data.ICInvalidateRange);
    OSDynLoad_FindExport(coreinit_handle, 0, "_Exit", &private_data._Exit);

    if (private_data.OSEffectiveToPhysical((void *)0xa0000000) != (void *)0x10000000)
    {
        run_kexploit(&private_data);
        return;
    }

    /* Get functions to send restart signal */
    int(*IM_Open)();
    int(*IM_Close)(int fd);
    int(*IM_SetDeviceState)(int fd, void *mem, int state, int a, int b);
    void*(*OSAllocFromSystem)(unsigned int size, int align);
    void(*OSFreeToSystem)(void *ptr);
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_Open", &IM_Open);
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_Close", &IM_Close);
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_SetDeviceState", &IM_SetDeviceState);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSAllocFromSystem", &OSAllocFromSystem);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSFreeToSystem", &OSFreeToSystem);

    /* Send restart signal to get rid of uneeded threads */
    /* Cause the other browser threads to exit */
    int fd = IM_Open();
    void *mem = OSAllocFromSystem(0x100, 64);
    if(!mem)
        ExitFailure(&private_data, "Not enough memory. Exit and re-enter browser.");

    private_data.memset(mem, 0, 0x100);

    /* Sets wanted flag */
    IM_SetDeviceState(fd, mem, 3, 0, 0);
    IM_Close(fd);
    OSFreeToSystem(mem);

    /* Waits for thread exits */
    unsigned int t1 = 0x1FFFFFFF;
    while(t1--) ;

    /* Prepare for thread startups */
    int (*OSCreateThread)(void *thread, void *entry, int argc, void *args, unsigned int stack, unsigned int stack_size, int priority, unsigned short attr);
    int (*OSResumeThread)(void *thread);
    int (*OSIsThreadTerminated)(void *thread);

    OSDynLoad_FindExport(coreinit_handle, 0, "OSCreateThread", &OSCreateThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSResumeThread", &OSResumeThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSIsThreadTerminated", &OSIsThreadTerminated);

    /* Allocate a stack for the thread */
    /* IMPORTANT: libcurl uses around 0x1000 internally so make
        sure to allocate more for the stack to avoid crashes */
    void *stack = private_data.MEMAllocFromDefaultHeapEx(0x4000, 0x20);
    /* Create the thread variable */
    void *thread = private_data.MEMAllocFromDefaultHeapEx(0x1000, 8);
    if(!thread || !stack)
        ExitFailure(&private_data, "Thread memory allocation failed. Exit and re-enter browser.");

    // the thread stack is too small on current thread, switch to an own created thread
    // create a detached thread with priority 0 and use core 1
    int ret = OSCreateThread(thread, curl_thread_callback, 1, (void*)&private_data, (unsigned int)stack+0x2000, 0x2000, 0, 0x1A);
    if (ret == 0)
        ExitFailure(&private_data, "Failed to create thread. Exit and re-enter browser.");

    /* Schedule it for execution */
    OSResumeThread(thread);

    // Keep this main thread around for ELF loading
    // Can not use OSJoinThread, which hangs for some reason, so we use a detached one and wait for it to terminate
    while(OSIsThreadTerminated(thread) == 0)
    {
        asm volatile (
        "    nop\n"
        "    nop\n"
        "    nop\n"
        "    nop\n"
        "    nop\n"
        "    nop\n"
        "    nop\n"
        "    nop\n"
        );
    }

    /* Install our code now */
    InstallMain(&private_data);

    /* setup our own syscall and call it */
    SetupKernelSyscall((unsigned int)KernelPatches);
    Syscall_0x36();

    /* Patch functions and our code for usage */
    InstallPatches(&private_data);

    /* Free thread memory and stack */
    private_data.MEMFreeToDefaultHeap(thread);
    private_data.MEMFreeToDefaultHeap(stack);

    //! we are done -> exit browser now
    private_data._Exit();
}

void ExitFailure(private_data_t *private_data, const char *failure)
{
    /************************************************************************/
    // Prepare screen
    void (*OSScreenInit)();
    unsigned int (*OSScreenGetBufferSizeEx)(unsigned int bufferNum);
    unsigned int (*OSScreenSetBufferEx)(unsigned int bufferNum, void * addr);
    unsigned int (*OSScreenClearBufferEx)(unsigned int bufferNum, unsigned int temp);
    unsigned int (*OSScreenFlipBuffersEx)(unsigned int bufferNum);
    unsigned int (*OSScreenPutFontEx)(unsigned int bufferNum, unsigned int posX, unsigned int posY, const char * buffer);

    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenInit", &OSScreenInit);
    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenGetBufferSizeEx", &OSScreenGetBufferSizeEx);
    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenSetBufferEx", &OSScreenSetBufferEx);
    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenClearBufferEx", &OSScreenClearBufferEx);
    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenFlipBuffersEx", &OSScreenFlipBuffersEx);
    OSDynLoad_FindExport(private_data->coreinit_handle, 0, "OSScreenPutFontEx", &OSScreenPutFontEx);

    // Prepare screen
    int screen_buf0_size = 0;
    int screen_buf1_size = 0;
    unsigned int screen_color = 0; // (r << 24) | (g << 16) | (b << 8) | a;

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    screen_buf1_size = OSScreenGetBufferSizeEx(1);
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)0xF4000000 + screen_buf0_size);

    // Clear screens
    OSScreenClearBufferEx(0, screen_color);
    OSScreenClearBufferEx(1, screen_color);

    // Flush the cache
    private_data->DCFlushRange((void *)0xF4000000, screen_buf0_size);
    private_data->DCFlushRange((void *)0xF4000000 + screen_buf0_size, screen_buf1_size);

    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    OSScreenPutFontEx(1, 0, 0, failure);

    OSScreenFlipBuffersEx(1);
    OSScreenClearBufferEx(1, 0);

    unsigned int t1 = 0x3FFFFFFF;
    while(t1--) asm volatile("nop");

    private_data->_Exit();
}

/* *****************************************************************************
 * Base functions
 * ****************************************************************************/
#define KERN_SYSCALL_TBL_5          0xFFEAA0E0 // works with browser

static void SetupKernelSyscall(unsigned int address)
{
    // Add syscall #0x36
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x36 * 4)), address);
}

/* libcurl data write callback */
static int curl_write_data_callback(void *buffer, int size, int nmemb, void *userp)
{
    file_struct_t *file = (file_struct_t *)userp;
    int insize = size*nmemb;
    file->memcpy(file->data + file->len, buffer, insize);
    file->len += insize;
    return insize;
}

/* The downloader thread */
#define CURLOPT_WRITEDATA 10001
#define CURLOPT_URL 10002
#define CURLOPT_ERRORBUFFER 10010
#define CURLOPT_WRITEFUNCTION 20011
#define CURLINFO_RESPONSE_CODE 0x200002
#define 	CURL_ERROR_SIZE   256

static int curl_download_file(private_data_t *private_data, void * curl, const char *url)
{
    char errbuf[CURL_ERROR_SIZE+1];
    errbuf[CURL_ERROR_SIZE] = 0;
    file_struct_t file;
    file.memcpy = private_data->memcpy;
    file.data = private_data->data_elf;
    file.len = 0;
    file.alloc_size = 0;
    private_data->curl_easy_setopt(curl, CURLOPT_URL, url);
    private_data->curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data_callback);
    private_data->curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    private_data->curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    /* Download file */
    int ret = private_data->curl_easy_perform(curl);
    if(ret)
        ExitFailure(private_data, errbuf);

    /* Do error checks */
    if(!file.len) {
        ExitFailure(private_data, "file length is 0");
    }

    int resp = 404;
    private_data->curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp);
    if(resp != 200) {
        ExitFailure(private_data, "response != 200");
    }

    return file.len;
}

static void curl_thread_callback(int argc, void *argv)
{
    private_data_t *private_data = (private_data_t *)argv;
    char *leaddr = (char*)0;
    unsigned char *str;

    char buf[128];
    /* find address left in ram */
    for(str = (unsigned char*)0x1A000000; str < (unsigned char*)0x20000000; str++)
    { /* Search for /payload which indicates the current address */
        if(*(unsigned int*)str == 0x2F706179 && *(unsigned int*)(str+4) == 0x6C6F6164)
        {
            leaddr = (char*)str;
            while(*leaddr)
            {
                if(*(unsigned int*)leaddr == 0x68747470)
                    break;
                leaddr--;
            }
            /* If string starts with http its likely to be correct */
            if(*(unsigned int*)leaddr == 0x68747470)
                break;
            leaddr = (char*)0;
        }
    }
    if(leaddr == (char*)0)
        ExitFailure(private_data, "URL not found");


    unsigned int nn_ac_handle;
    unsigned int nn_startupid;
    int(*ACInitialize)();
    int(*ACGetStartupId) (unsigned int *id);
    int(*ACConnectWithConfigId) (unsigned int id);

    OSDynLoad_Acquire("nn_ac.rpl", &nn_ac_handle);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACInitialize", &ACInitialize);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACGetStartupId", &ACGetStartupId);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACConnectWithConfigId",&ACConnectWithConfigId);

    ACInitialize();
    ACGetStartupId(&nn_startupid);
    ACConnectWithConfigId(nn_startupid);

    unsigned int nsysnet_handle;
    int(*socket_lib_init)();
    OSDynLoad_Acquire("nsysnet.rpl", &nsysnet_handle);
    OSDynLoad_FindExport(nsysnet_handle, 0, "socket_lib_init", &socket_lib_init);
    socket_lib_init();

    /* Acquire and setup libcurl */
    unsigned int libcurl_handle;
    OSDynLoad_Acquire("nlibcurl", &libcurl_handle);

    int(*curl_global_init)(int opts);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_global_init", &curl_global_init);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_easy_init", &private_data->curl_easy_init);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_easy_setopt", &private_data->curl_easy_setopt);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_easy_perform", &private_data->curl_easy_perform);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_easy_getinfo", &private_data->curl_easy_getinfo);
    OSDynLoad_FindExport(libcurl_handle, 0, "curl_easy_cleanup", &private_data->curl_easy_cleanup);

    curl_global_init(0);

    void *curl = private_data->curl_easy_init();
    if(!curl) {
        ExitFailure(private_data, "cURL init failed");
    }

    /* Generate the url address */
    char *src_ptr = leaddr;
    char *ptr = buf;
    while(*src_ptr) {
        *ptr++ = *src_ptr++;
    }
    *ptr = 0;
    // go back to last /
    while(*ptr != 0x2F)
        ptr--;

#ifdef USE_SD_LOADER
    private_data->memcpy(ptr+1, "sd_loader.elf", 14);
#else
    private_data->memcpy(ptr+1, "loadiine_gx2.elf", 17);
#endif
    curl_download_file(private_data, curl, buf);

    /* Cleanup to gain back memory */
    private_data->curl_easy_cleanup(curl);

    /* Pre-load the Mii Studio to be executed on _Exit - thanks to wj444 for sharing it */
    unsigned int sysapp_handle;
    void (*_SYSLaunchMiiStudio)(void) = 0;
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
    OSDynLoad_FindExport(sysapp_handle, 0, "_SYSLaunchMiiStudio", &_SYSLaunchMiiStudio);

    _SYSLaunchMiiStudio();
}

static int strcmp(const char *s1, const char *s2)
{
    while(*s1 && *s2)
    {
        if(*s1 != *s2) {
            return -1;
        }
        s1++;
        s2++;
    }

    if(*s1 != *s2) {
        return -1;
    }
    return 0;
}

static unsigned int get_section(private_data_t *private_data, unsigned char *data, const char *name, unsigned int * size, unsigned int * addr, int fail_on_not_found)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) data;

    if (   !data
        || !IS_ELF (*ehdr)
        || (ehdr->e_type != ET_EXEC)
        || (ehdr->e_machine != EM_PPC))
    {
        ExitFailure(private_data, "Invalid elf file");
    }

    Elf32_Shdr *shdr = (Elf32_Shdr *) (data + ehdr->e_shoff);
    int i;
    for(i = 0; i < ehdr->e_shnum; i++)
    {
        const char *section_name = ((const char*)data) + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name;
        if(strcmp(section_name, name) == 0)
        {
            if(addr)
                *addr = shdr[i].sh_addr;
            if(size)
                *size = shdr[i].sh_size;
            return shdr[i].sh_offset;
        }
    }

    if(fail_on_not_found)
        ExitFailure(private_data, (char*)name);

    return 0;
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
static void RestoreInstructions(private_data_t *private_data)
{
    restore_instructions_t * restore = (restore_instructions_t *)(RESTORE_INSTR_ADDR);
    if(restore->magic == RESTORE_INSTR_MAGIC)
    {
        for(unsigned int i = 0; i < restore->instr_count; i++)
        {
            *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + restore->data[i].addr) = restore->data[i].instr;
            private_data->DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + restore->data[i].addr), 4);
            private_data->ICInvalidateRange((void*)restore->data[i].addr, 4);
        }

    }

    restore->magic = 0;
    restore->instr_count = 0;
}

/* ****************************************************************** */
/*                         INSTALL MAIN CODE                          */
/* ****************************************************************** */
static void InstallMain(private_data_t *private_data)
{
    //! restore original instructions
    RestoreInstructions(private_data);

    // get .text section
    unsigned int main_text_addr = 0;
    unsigned int main_text_len = 0;
    unsigned int section_offset = get_section(private_data, private_data->data_elf, ".text", &main_text_len, &main_text_addr, 1);
    unsigned char *main_text = private_data->data_elf + section_offset;
    /* Copy main .text to memory */
    if(section_offset > 0)
    {
        private_data->memcpy((void*)(CODE_RW_BASE_OFFSET + main_text_addr), main_text, main_text_len);
        private_data->DCFlushRange((void*)(CODE_RW_BASE_OFFSET + main_text_addr), main_text_len);
        private_data->ICInvalidateRange((void*)(main_text_addr), main_text_len);
    }

    // get the .rodata section
    unsigned int main_rodata_addr = 0;
    unsigned int main_rodata_len = 0;
    section_offset = get_section(private_data, private_data->data_elf, ".rodata", &main_rodata_len, &main_rodata_addr, 0);
    if(section_offset > 0)
    {
        unsigned char *main_rodata = private_data->data_elf + section_offset;
        /* Copy main rodata to memory */
        private_data->memcpy((void*)(DATA_RW_BASE_OFFSET + main_rodata_addr), main_rodata, main_rodata_len);
        private_data->DCFlushRange((void*)(DATA_RW_BASE_OFFSET + main_rodata_addr), main_rodata_len);
    }

    // get the .data section
    unsigned int main_data_addr = 0;
    unsigned int main_data_len = 0;
    section_offset = get_section(private_data, private_data->data_elf, ".data", &main_data_len, &main_data_addr, 0);
    if(section_offset > 0)
    {
        unsigned char *main_data = private_data->data_elf + section_offset;
        /* Copy main data to memory */
        private_data->memcpy((void*)(DATA_RW_BASE_OFFSET + main_data_addr), main_data, main_data_len);
        private_data->DCFlushRange((void*)(DATA_RW_BASE_OFFSET + main_data_addr), main_data_len);
    }

    // get the .bss section
    unsigned int main_bss_addr = 0;
    unsigned int main_bss_len = 0;
    section_offset = get_section(private_data, private_data->data_elf, ".bss", &main_bss_len, &main_bss_addr, 0);
    if(section_offset > 0)
    {
        /* Copy main data to memory */
        private_data->memset((void*)(DATA_RW_BASE_OFFSET + main_bss_addr), 0, main_bss_len);
        private_data->DCFlushRange((void*)(DATA_RW_BASE_OFFSET + main_bss_addr), main_bss_len);
    }
}

/* ****************************************************************** */
/*                         INSTALL PATCHES                            */
/* All OS specific stuff is done here                                 */
/* ****************************************************************** */
static void InstallPatches(private_data_t *private_data)
{
    OsSpecifics * osSpecificFunctions = OS_SPECIFICS;
    private_data->memset(osSpecificFunctions, 0, sizeof(OsSpecifics));

    /* Pre-setup a few options to defined values */
    OS_FIRMWARE = VER;
    GAME_LAUNCHED = 0;
    GAME_RPX_LOADED = 0;
    RPX_CHECK_NAME = 0xDEADBEAF;
    MAIN_ENTRY_ADDR = 0xDEADC0DE;
    PREP_TITLE_CALLBACK = 0;
    LOADIINE_MODE = LOADIINE_MODE_SMASH_BROS;

    game_paths_t *game_paths = (game_paths_t *)GAME_PATH_STRUCT;
    private_data->memset(game_paths, 0, sizeof(game_paths_t));

    unsigned int jump_main_hook = 0;
    osSpecificFunctions->addr_OSDynLoad_Acquire = (unsigned int)OSDynLoad_Acquire;
    osSpecificFunctions->addr_OSDynLoad_FindExport = (unsigned int)OSDynLoad_FindExport;

    osSpecificFunctions->addr_LiWaitOneChunk = ADDRESS_LiWaitOneChunk;
    osSpecificFunctions->addr_LiWaitIopComplete = ADDRESS_LiWaitIopComplete;
    osSpecificFunctions->addr_LiWaitIopCompleteWithInterrupts = ADDRESS_LiWaitIopCompleteWithInterrupts;

    //! pointer to main entry point of a title
    osSpecificFunctions->addr_OSTitle_main_entry = ADDRESS_OSTitle_main_entry_ptr;

    //! at this point we dont need to check header and stuff as it is sure to be OK
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) private_data->data_elf;
    unsigned int mainEntryPoint = ehdr->e_entry;

    //! Install out entry point hook
    unsigned int repl_addr = ADDRESS_main_entry_hook;
    unsigned int jump_addr = mainEntryPoint & 0x03fffffc;
    *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + repl_addr)) = 0x48000003 | jump_addr;
    // flush caches and invalidate instruction cache
    private_data->DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + repl_addr), 4);
    private_data->ICInvalidateRange((void*)(repl_addr), 4);

    //! TODO: Not sure if this is still needed at all after changing the SDK version in the xml struct, check that
#if ((VER == 532) || (VER == 540))
    /* Patch to bypass SDK version tests */
    *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x010095b4)) = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)
    *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01009658)) = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
    private_data->DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x010095b4), 4);
    private_data->ICInvalidateRange((void*)(0x010095b4), 4);
    private_data->DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01009658), 4);
    private_data->ICInvalidateRange((void*)(0x01009658), 4);
#else
    #ERROR  Please define an SDK check address.
#endif
}
