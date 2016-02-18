#include <gctypes.h>
#include "elf_abi.h"
#include "../../src/common/common.h"
#include "../../src/common/fs_defs.h"
#include "../../src/common/os_defs.h"

#define CODE_RW_BASE_OFFSET                             0
#define DATA_RW_BASE_OFFSET                             0

#define EXPORT_DECL(res, func, ...)                     res (* func)(__VA_ARGS__) __attribute__((section(".data"))) = 0;

#define OS_FIND_EXPORT(handle, func)                    OSDynLoad_FindExport(handle, 0, # func, &func)

EXPORT_DECL(int, OSDynLoad_Acquire, const char* rpl, u32 *handle);
EXPORT_DECL(int, OSDynLoad_FindExport, u32 handle, int isdata, const char *symbol, void *address);

EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);
EXPORT_DECL(void, OSFatal, const char* msg);
EXPORT_DECL(void, DCFlushRange, const void *addr, u32 length);
EXPORT_DECL(void, ICInvalidateRange, const void *addr, u32 length);
EXPORT_DECL(int, __os_snprintf, char* s, int n, const char * format, ...);

EXPORT_DECL(int, FSInit, void);
EXPORT_DECL(int, FSAddClientEx, void *pClient, int unk_zero_param, int errHandling);
EXPORT_DECL(int, FSDelClient, void *pClient);
EXPORT_DECL(void, FSInitCmdBlock, void *pCmd);
EXPORT_DECL(int, FSGetMountSource, void *pClient, void *pCmd, int type, void *source, int errHandling);
EXPORT_DECL(int, FSMount, void *pClient, void *pCmd, void *source, const char *target, uint32_t bytes, int errHandling);
EXPORT_DECL(int, FSUnmount, void *pClient, void *pCmd, const char *target, int errHandling);
EXPORT_DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling);
EXPORT_DECL(int, FSGetStatFile, void *pClient, void *pCmd, int fd, void *buffer, int error);
EXPORT_DECL(int, FSReadFile, void *pClient, void *pCmd, void *buffer, int size, int count, int fd, int flag, int errHandling);
EXPORT_DECL(int, FSCloseFile, void *pClient, void *pCmd, int fd, int errHandling);

static int LoadFileToMem(const char *filepath, unsigned char **fileOut, unsigned int * sizeOut)
{
    int iFd = -1;
    void *pClient = MEMAllocFromDefaultHeapEx(FS_CLIENT_SIZE, 4);
    if(!pClient)
        return 0;

    void *pCmd = MEMAllocFromDefaultHeapEx(FS_CMD_BLOCK_SIZE, 4);
    if(!pCmd)
    {
        MEMFreeToDefaultHeap(pClient);
        return 0;
    }

    int success = 0;
    FSInit();
    FSInitCmdBlock(pCmd);
    FSAddClientEx(pClient, 0, -1);

    do
    {
        char tempPath[FS_MOUNT_SOURCE_SIZE];
        char mountPath[FS_MAX_MOUNTPATH_SIZE];

        int status = FSGetMountSource(pClient, pCmd, 0, tempPath, -1);
        if (status != 0) {
            OSFatal("FSGetMountSource failed.");
            break;
        }
        status = FSMount(pClient, pCmd, tempPath, mountPath, FS_MAX_MOUNTPATH_SIZE, -1);
        if(status != 0) {
            OSFatal("SD mount failed.");
            break;
        }

        status = FSOpenFile(pClient, pCmd, filepath, "r", &iFd, -1);
        if(status != 0)
        {
            FSUnmount(pClient, pCmd, mountPath, -1);
            break;
        }

        FSStat stat;
        stat.size = 0;

        void *pBuffer = NULL;

        FSGetStatFile(pClient, pCmd, iFd, &stat, -1);

        if(stat.size > 0)
            pBuffer = MEMAllocFromDefaultHeapEx((stat.size + 0x3F) & ~0x3F, 0x40);

        if(!pBuffer)
            OSFatal("Not enough memory for ELF file.");

        unsigned int done = 0;

        while(done < stat.size)
        {
            int readBytes = FSReadFile(pClient, pCmd, pBuffer + done, 1, stat.size - done, iFd, 0, -1);
            if(readBytes <= 0) {
                break;
            }
            done += readBytes;
        }

        if(done != stat.size)
        {
            MEMFreeToDefaultHeap(pBuffer);
        }
        else
        {
            *fileOut = (unsigned char*)pBuffer;
            *sizeOut = stat.size;
            success = 1;
        }

        FSCloseFile(pClient, pCmd, iFd, -1);
        FSUnmount(pClient, pCmd, mountPath, -1);
    }
    while(0);

    FSDelClient(pClient);
    MEMFreeToDefaultHeap(pClient);
    MEMFreeToDefaultHeap(pCmd);
    return success;
}

unsigned int load_elf_image (unsigned char *elfstart)
{
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdrs;
	unsigned char *image;
	int i;

	ehdr = (Elf32_Ehdr *) elfstart;

	if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0)
		return 0;

	if(ehdr->e_phentsize != sizeof(Elf32_Phdr))
		return 0;

	phdrs = (Elf32_Phdr*)(elfstart + ehdr->e_phoff);

	for(i = 0; i < ehdr->e_phnum; i++)
    {
		if(phdrs[i].p_type != PT_LOAD)
			continue;

		if(phdrs[i].p_filesz > phdrs[i].p_memsz)
			return 0;

		if(!phdrs[i].p_filesz)
			continue;

        unsigned int p_paddr = phdrs[i].p_paddr;

        // use 0xC1000000 offset fore coreinit
        //if(p_paddr >= 0x01000000 && p_paddr <= 0x02000000)
        //    p_paddr += 0xC1000000;

        // use correct offset address for executables and data access
		if(phdrs[i].p_flags & PF_X)
			p_paddr += CODE_RW_BASE_OFFSET;
        else
			p_paddr += DATA_RW_BASE_OFFSET;

		image = (unsigned char *) (elfstart + phdrs[i].p_offset);
		memcpy ((void *) p_paddr, image, phdrs[i].p_filesz);
        DCFlushRange((void*)p_paddr, phdrs[i].p_filesz);

		if(phdrs[i].p_flags & PF_X)
			ICInvalidateRange ((void *) phdrs[i].p_paddr, phdrs[i].p_memsz);
	}

	return ehdr->e_entry;
}

int _start(int argc, char **argv)
{
    if(MAIN_ENTRY_ADDR == 0xDEADC0DE)
    {
        unsigned int coreinit_handle = 0;

        OSDynLoad_Acquire = (int (*)(const char*, u32 *))OS_SPECIFICS->addr_OSDynLoad_Acquire;
        OSDynLoad_FindExport = (int (*)(u32, int, const char *, void *))OS_SPECIFICS->addr_OSDynLoad_FindExport;

        OSDynLoad_Acquire("coreinit", &coreinit_handle);

        unsigned int *functionPtr = 0;

        OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
        MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
        OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
        MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

        OS_FIND_EXPORT(coreinit_handle, memcpy);
        OS_FIND_EXPORT(coreinit_handle, memset);
        OS_FIND_EXPORT(coreinit_handle, OSFatal);
        OS_FIND_EXPORT(coreinit_handle, DCFlushRange);
        OS_FIND_EXPORT(coreinit_handle, ICInvalidateRange);
        OS_FIND_EXPORT(coreinit_handle, __os_snprintf);

        OS_FIND_EXPORT(coreinit_handle, FSInit);
        OS_FIND_EXPORT(coreinit_handle, FSAddClientEx);
        OS_FIND_EXPORT(coreinit_handle, FSDelClient);
        OS_FIND_EXPORT(coreinit_handle, FSInitCmdBlock);
        OS_FIND_EXPORT(coreinit_handle, FSGetMountSource);
        OS_FIND_EXPORT(coreinit_handle, FSMount);
        OS_FIND_EXPORT(coreinit_handle, FSUnmount);
        OS_FIND_EXPORT(coreinit_handle, FSOpenFile);
        OS_FIND_EXPORT(coreinit_handle, FSGetStatFile);
        OS_FIND_EXPORT(coreinit_handle, FSReadFile);
        OS_FIND_EXPORT(coreinit_handle, FSCloseFile);

        unsigned char *pElfBuffer = NULL;
        unsigned int uiElfSize = 0;

        LoadFileToMem(CAFE_OS_SD_PATH SD_LOADIINE_PATH "/apps/loadiine_gx2/loadiine_gx2.elf", &pElfBuffer, &uiElfSize);

        if(!pElfBuffer)
        {
            OSFatal("Could not load file " SD_LOADIINE_PATH "/apps/loadiine_gx2/loadiine_gx2.elf");
        }
        else
        {
            MAIN_ENTRY_ADDR = load_elf_image(pElfBuffer);
            MEMFreeToDefaultHeap(pElfBuffer);
        }
    }

    return ((int (*)(int, char **))MAIN_ENTRY_ADDR)(argc, argv);
}
