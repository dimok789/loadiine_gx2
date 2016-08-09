#include "malloc.h"
#include "common/retain_vars.h"
#include "patcher/fs_patcher.h"
#include "common/loader_defs.h"
#include "rplrpx_patcher.h"
#include "patcher_util.h"
#include "fs_logger.h"
#include "game/rpx_rpl_table.h"

/* *****************************************************************************
 * Dynamic RPL loading to memory
 * ****************************************************************************/
static int LoadRPLToMemory(s_rpx_rpl *rpl_entry)
{
    if ((int)bss_ptr != 0x0a000000)
    {
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "CheckAndLoadRPL(%s) found and loading", rpl_entry->name);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
    }

    // initialize FS
    FSInit();
    void *pClient = malloc(FS_CLIENT_SIZE);
    if (!pClient)
        return 0;

    void *pCmd = malloc(FS_CMD_BLOCK_SIZE);
    if (!pCmd)
    {
        free(pClient);
        return 0;
    }

    // calculate path length for SD access of RPL
    int path_len = strlen(gamePathStruct.os_game_path_base) + strlen(gamePathStruct.game_dir) + strlen(RPX_RPL_PATH) + strlen(rpl_entry->name) + 3;
	char *path_update_rpl = NULL;
    char *path_rpl = (char *) malloc(path_len);
    if(!path_rpl) {
        free(pCmd);
        free(pClient);
        return 0;
    }

    // malloc mem for read file
    unsigned char* dataBuf = (unsigned char*)memalign(0x40, 0x10000);
    if(!dataBuf) {
        free(pCmd);
        free(pClient);
        free(path_rpl);
        return 0;
    }

	unsigned char * dataBufPhysical = (unsigned char*)OSEffectiveToPhysical(dataBuf);

    // do more initial FS stuff
    FSInitCmdBlock(pCmd);
    FSAddClientEx(pClient, 0, -1);

    // set RPL size to 0 to avoid wrong RPL being loaded when opening file fails
    rpl_entry->size = 0;
    rpl_entry->offset = 0;

	// create path
    __os_snprintf(path_rpl, path_len, "%s/%s%s/%s", gamePathStruct.os_game_path_base, gamePathStruct.game_dir, RPX_RPL_PATH, rpl_entry->name);

	if(gSettingUseUpdatepath){
        free(path_rpl);
		int path_len_update_rpl = strlen(gamePathStruct.os_game_path_base) + strlen(gamePathStruct.game_dir) + strlen(UPDATE_PATH) + strlen(gamePathStruct.update_folder) + strlen(RPX_RPL_PATH) + strlen(rpl_entry->name) + 4;
		path_update_rpl = (char *) malloc(path_len_update_rpl);
		if(!path_update_rpl) {
			free(pCmd);
			free(pClient);
			return 0;
		}
		__os_snprintf(path_update_rpl, path_len_update_rpl, "%s/%s%s/%s/%s", gamePathStruct.os_game_path_base, gamePathStruct.game_dir, UPDATE_PATH, gamePathStruct.update_folder, RPX_RPL_PATH, rpl_entry->name);
		fs_log_string(bss.global_sock, path_update_rpl, BYTE_LOG_STR);
		int ret = -2;
		FSStat stats;
		if((ret = real_FSGetStat(pClient, pCmd, path_update_rpl, &stats, FS_RET_ALL_ERROR)) == 0){
			fs_log_string(bss.global_sock, "Loading RPL from update path", BYTE_LOG_STR);
			path_rpl = path_update_rpl;
		}else{
			char error_message[50];
			__os_snprintf(error_message, 50, "FSGetStat Result: %d", ret);
			fs_log_string(bss.global_sock, error_message, BYTE_LOG_STR);
		}
	}

    int fd = 0;
    if (real_FSOpenFile(pClient, pCmd, path_rpl, "r", &fd, FS_RET_ALL_ERROR) == FS_STATUS_OK)
    {
        int ret;
        int rpl_size = 0;

        // Copy rpl in memory
        while ((ret = FSReadFile(pClient, pCmd, dataBuf, 0x1, 0x10000, fd, 0, FS_RET_ALL_ERROR)) > 0)
        {
			DCFlushRange((void *)dataBuf, ret);

            // Copy in memory and save offset
            int copiedData = rpxRplCopyDataToMem(rpl_entry, rpl_size, dataBufPhysical, ret);
            if(copiedData != ret)
            {
                char buffer[200];
                __os_snprintf(buffer, sizeof(buffer), "CheckAndLoadRPL(%s) failure on copying data to memory. Copied %i expected %i.", rpl_entry->name, copiedData, ret);
                fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
            }
            rpl_size += ret;
        }

        // Fill rpl entry
        rpl_entry->size = rpl_size;

        if ((int)bss_ptr != 0x0a000000)
        {
            char buffer[200];
            __os_snprintf(buffer, sizeof(buffer), "CheckAndLoadRPL(%s) file loaded 0x%08X %i", rpl_entry->name, rpl_entry->area->address, rpl_entry->size);
            fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
        }

        FSCloseFile(pClient, pCmd, fd, FS_RET_NO_ERROR);
    }

    FSDelClient(pClient);
    free(dataBuf);
    free(pCmd);
    free(pClient);
	if(!gSettingUseUpdatepath){
        free(path_rpl);
    }else{
        free(path_update_rpl);
    }
    return 1;
}

/* *****************************************************************************
 * Searching for RPL to load
 * ****************************************************************************/
static int CheckAndLoadRPL(const char *rpl) {
    // If we are in Smash Bros app
    if (GAME_LAUNCHED == 0)
        return 0;

    // Look for rpl name in our table
    s_rpx_rpl *rpl_entry = rpxRplTableGet();

    do
    {
        if(rpl_entry->is_rpx)
            continue;

        int len = strlen(rpl);
        int len2 = strlen(rpl_entry->name);
        if ((len != len2) && (len != (len2 - 4))) {
            continue;
        }

        // compare name string case insensitive and without ".rpl" extension
        if (strncasecmp(rpl_entry->name, rpl, len) == 0)
        {
            int result = LoadRPLToMemory(rpl_entry);
            return result;
        }
    }
    while((rpl_entry = rpl_entry->next) != 0);
    return 0;
}

DECL(int, OSDynLoad_Acquire, char* rpl, unsigned int *handle, int r5 __attribute__((unused))) {
    // Get only the filename (in case there is folders in the module name ... like zombiu)
    char *ptr = rpl;
    while(*ptr) {
        if (*ptr == '/') {
            rpl = ptr + 1;
        }
        ptr++;
    }

    // Look if RPL is in our table and load it from SD Card
    CheckAndLoadRPL(rpl);

    int result = real_OSDynLoad_Acquire(rpl, handle, 0);

    if ((int)bss_ptr != 0x0a000000)
    {
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "OSDynLoad_Acquire: %s result: %i", rpl, result);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
    }

    return result;
}


static struct {
    unsigned int fileType;
    unsigned int sgBufferNumber;
    unsigned int sgFileOffset;
} loaderGlobals3XX;

DECL(int, LiBounceOneChunk, const char * filename, int fileType, int procId, int * hunkBytes, int fileOffset, int bufferNumber, int * dst_address)
{
    loaderGlobals3XX.fileType = fileType;
    loaderGlobals3XX.sgBufferNumber = bufferNumber;
    loaderGlobals3XX.sgFileOffset = fileOffset;

    return real_LiBounceOneChunk(filename, fileType, procId, hunkBytes, fileOffset, bufferNumber, dst_address);
}

// This function is called every time after LiBounceOneChunk.
// It waits for the asynchronous call of LiLoadAsync for the IOSU to fill data to the RPX/RPL address
// and return the still remaining bytes to load.
// We override it and replace the loaded date from LiLoadAsync with our data and our remaining bytes to load.
DECL(int, LiWaitOneChunk, unsigned int * iRemainingBytes, const char *filename, int fileType)
{
    unsigned int result;
    register int core_id;
    int remaining_bytes = 0;

    int sgFileOffset;
    int sgBufferNumber;
    int *sgBounceError;
    int *sgGotBytes;
    int *sgTotalBytes;
    int *sgIsLoadingBuffer;
    int *sgFinishedLoadingBuffer;

    // get the current core
    asm volatile("mfspr %0, 0x3EF" : "=r" (core_id));

    // get the offset of per core global variable for dynload initialized (just a simple address + (core_id * 4))
    unsigned int gDynloadInitialized = *(volatile unsigned int*)(addr_gDynloadInitialized + (core_id << 2));

    // Comment (Dimok):
    // time measurement at this position for logger  -> we don't need it right now except maybe for debugging
    //unsigned long long systemTime1 = Loader_GetSystemTime();

	if(OS_FIRMWARE == 550)
    {
        // pointer to global variables of the loader
        loader_globals_550_t *loader_globals = (loader_globals_550_t*)(0xEFE19E80);

        sgBufferNumber = loader_globals->sgBufferNumber;
        sgFileOffset = loader_globals->sgFileOffset;
        sgBounceError = &loader_globals->sgBounceError;
        sgGotBytes = &loader_globals->sgGotBytes;
        sgTotalBytes = &loader_globals->sgTotalBytes;
        sgFinishedLoadingBuffer = &loader_globals->sgFinishedLoadingBuffer;
        // not available on 5.5.x
        sgIsLoadingBuffer = NULL;
    }
    else if(OS_FIRMWARE < 400)
    {
        sgBufferNumber = loaderGlobals3XX.sgBufferNumber;
        sgFileOffset = loaderGlobals3XX.sgFileOffset;
        fileType = loaderGlobals3XX.fileType;   // fileType is actually not passed to this function on < 400
        sgIsLoadingBuffer = (int *)addr_sgIsLoadingBuffer;

        // not available on < 400
        sgBounceError = NULL;
        sgGotBytes = NULL;
        sgTotalBytes = NULL;
        sgFinishedLoadingBuffer = NULL;
    }
    else
    {
        // pointer to global variables of the loader
        loader_globals_t *loader_globals = (loader_globals_t*)(addr_sgIsLoadingBuffer);

        sgBufferNumber = loader_globals->sgBufferNumber;
        sgFileOffset = loader_globals->sgFileOffset;
        sgBounceError = &loader_globals->sgBounceError;
        sgGotBytes = &loader_globals->sgGotBytes;
        sgIsLoadingBuffer = &loader_globals->sgIsLoadingBuffer;
        // not available on < 5.5.x
        sgTotalBytes = NULL;
        sgFinishedLoadingBuffer = NULL;
    }

    // the data loading was started in LiBounceOneChunk() and here it waits for IOSU to finish copy the data
    if(gDynloadInitialized != 0) {
        result = LiWaitIopCompleteWithInterrupts(0x2160EC0, &remaining_bytes);

    }
    else {
        result = LiWaitIopComplete(0x2160EC0, &remaining_bytes);
    }


    // Comment (Dimok):
    // time measurement at this position for logger -> we don't need it right now except maybe for debugging
    //unsigned long long systemTime2 = Loader_GetSystemTime();

    //------------------------------------------------------------------------------------------------------------------
    // Start of our function intrusion:
    // After IOSU is done writing the data into the 0xF6000000/0xF6400000 address,
    // we overwrite it with our data before setting the global flag for IsLoadingBuffer to 0
    // Do this only if we are in the game that was launched by our method
    if (*(volatile unsigned int*)0xEFE00000 == RPX_CHECK_NAME && (GAME_LAUNCHED == 1))
    {
        s_rpx_rpl *rpl_struct = rpxRplTableGet();

        do
        {
            // the argument fileType = 0 is RPX, fileType = 1 is RPL (inverse to our is_rpx)
            if((!rpl_struct->is_rpx) != fileType)
                continue;

            int found = 1;

            // if we load RPX then the filename can't be checked as it is the Mii Maker or Smash Bros RPX name
            // there skip the filename check for RPX
            if(fileType == 1)
            {
                int len = strnlen(filename, 0x1000);
                int len2 = strnlen(rpl_struct->name, 0x1000);
                if ((len != len2) && (len != (len2 - 4)))
                    continue;

                if(strncasecmp(filename, rpl_struct->name, len) != 0) {
                    found = 0;
                }
            }

            if (found)
            {
                unsigned int load_address = (sgBufferNumber == 1) ? 0xF6000000 : (0xF6000000 + 0x00400000);
                unsigned int load_addressPhys = (sgBufferNumber == 1) ? gLoaderPhysicalBufferAddr : (gLoaderPhysicalBufferAddr + 0x00400000); // virtual 0xF6000000 and 0xF6400000

                // set our game RPX loaded variable for use in FS system
                if(fileType == 0)
                    GAME_RPX_LOADED = 1;

                remaining_bytes = rpl_struct->size - sgFileOffset;
                if (remaining_bytes > 0x400000)
                    // truncate size
                    remaining_bytes = 0x400000;

                DCFlushRange((void *)load_address, remaining_bytes);

                rpxRplCopyDataFromMem(rpl_struct, sgFileOffset, (unsigned char*)load_addressPhys, remaining_bytes);

                DCInvalidateRange((void *) load_address, remaining_bytes);

                // set result to 0 -> "everything OK"
                result = 0;
                break;
            }
        }
        while((rpl_struct = rpl_struct->next) != 0);
    }
    // check if the game was left back to system menu after a game was launched by our method and reset our flags for game launch
    else if (*(volatile unsigned int*)0xEFE00000 != RPX_CHECK_NAME && (GAME_LAUNCHED == 1) && (GAME_RPX_LOADED == 1))
    {
        GAME_RPX_LOADED = 0;
        GAME_LAUNCHED = 0;
		gSettingUseUpdatepath = 0;
        RPX_CHECK_NAME = 0xDEADBEAF;
    }

    // end of our little intrusion into this function
    //------------------------------------------------------------------------------------------------------------------

    // set the result to the global bounce error variable
    if(sgBounceError) {
        *sgBounceError = result;
    }

    // disable global flag that buffer is still loaded by IOSU
	if(sgFinishedLoadingBuffer)
    {
        unsigned int zeroBitCount = 0;
        asm volatile("cntlzw %0, %0" : "=r" (zeroBitCount) : "r"(*sgFinishedLoadingBuffer));
        *sgFinishedLoadingBuffer = zeroBitCount >> 5;
    }
    else if(sgIsLoadingBuffer)
    {
        *sgIsLoadingBuffer = 0;
    }

    // check result for errors
    if(result == 0) {
        // the remaining size is set globally and in stack variable only
        // if a pointer was passed to this function
        if(iRemainingBytes) {
            if(sgGotBytes) {
                *sgGotBytes = remaining_bytes;
            }

            *iRemainingBytes = remaining_bytes;

            // on 5.5.x a new variable for total loaded bytes was added
            if(sgTotalBytes) {
                *sgTotalBytes += remaining_bytes;
            }
        }
        // Comment (Dimok):
        // calculate time difference and print it on logging how long the wait for asynchronous data load took
        // something like (systemTime2 - systemTime1) * constant / bus speed, did not look deeper into it as we don't need that crap
    }
    else {
        // Comment (Dimok):
        // a lot of error handling here. depending on error code sometimes calls Loader_Panic() -> we don't make errors so we can skip that part ;-P
    }
    return result;
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_rplrpx[] __attribute__((section(".data"))) = {
    // LOADER function
    MAKE_MAGIC(LiWaitOneChunk,              LIB_LOADER,STATIC_FUNCTION),
    MAKE_MAGIC(LiBounceOneChunk,            LIB_LOADER,STATIC_FUNCTION),

    // Dynamic RPL loading functions
    MAKE_MAGIC(OSDynLoad_Acquire,           LIB_CORE_INIT,STATIC_FUNCTION),

};


u32 method_hooks_size_rplrpx __attribute__((section(".data"))) = sizeof(method_hooks_rplrpx) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_rplrpx[sizeof(method_hooks_rplrpx) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

