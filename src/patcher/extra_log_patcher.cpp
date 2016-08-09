#include "extra_log_patcher.h"

#define USE_EXTRA_LOG_FUNCTIONS   0

#if (USE_EXTRA_LOG_FUNCTIONS == 1)
DECL(int, OSDynLoad_GetModuleName, unsigned int *handle, char *name_buffer, int *name_buffer_size) {

    int result = real_OSDynLoad_GetModuleName(handle, name_buffer, name_buffer_size);
    if ((int)bss_ptr != 0x0a000000)
    {
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "OSDynLoad_GetModuleName: %s result %i", (name_buffer && result == 0) ? name_buffer : "NULL", result);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
    }

    return result;
}

DECL(int, OSDynLoad_IsModuleLoaded, char* rpl, unsigned int *handle, int r5 __attribute__((unused))) {

    int result = real_OSDynLoad_IsModuleLoaded(rpl, handle, 1);
    if ((int)bss_ptr != 0x0a000000)
    {
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "OSDynLoad_IsModuleLoaded: %s result %i", rpl, result);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
    }

    return result;
}
#endif

/* *****************************************************************************
 * Log functions
 * ****************************************************************************/
#if (USE_EXTRA_LOG_FUNCTIONS == 1)
static void fs_log_byte_for_client(void *pClient, char byte) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        fs_log_byte(bss.socket_fs[client], byte);
    }
}

DECL(int, FSCloseFile_log, void *pClient, void *pCmd, int fd, int error) {
    fs_log_byte_for_client(pClient, BYTE_CLOSE_FILE);
    return real_FSCloseFile_log(pClient, pCmd, fd, error);
}
DECL(int, FSCloseDir_log, void *pClient, void *pCmd, int fd, int error) {
    fs_log_byte_for_client(pClient, BYTE_CLOSE_DIR);
    return real_FSCloseDir_log(pClient, pCmd, fd, error);
}
DECL(int, FSFlushFile_log, void *pClient, void *pCmd, int fd, int error) {
    fs_log_byte_for_client(pClient, BYTE_FLUSH_FILE);
    return real_FSFlushFile_log(pClient, pCmd, fd, error);
}
DECL(int, FSGetErrorCodeForViewer_log, void *pClient, void *pCmd) {
    fs_log_byte_for_client(pClient, BYTE_GET_ERROR_CODE_FOR_VIEWER);
    return real_FSGetErrorCodeForViewer_log(pClient, pCmd);
}
DECL(int, FSGetLastError_log, void *pClient) {
    fs_log_byte_for_client(pClient, BYTE_GET_LAST_ERROR);
    return real_FSGetLastError_log(pClient);
}
DECL(int, FSGetPosFile_log, void *pClient, void *pCmd, int fd, int *pos, int error) {
    fs_log_byte_for_client(pClient, BYTE_GET_POS_FILE);
    return real_FSGetPosFile_log(pClient, pCmd, fd, pos, error);
}
DECL(int, FSGetStatFile_log, void *pClient, void *pCmd, int fd, void *buffer, int error) {
    fs_log_byte_for_client(pClient, BYTE_GET_STAT_FILE);
    return real_FSGetStatFile_log(pClient, pCmd, fd, buffer, error);
}
DECL(int, FSIsEof_log, void *pClient, void *pCmd, int fd, int error) {
    fs_log_byte_for_client(pClient, BYTE_EOF);
    return real_FSIsEof_log(pClient, pCmd, fd, error);
}

DECL(int, FSReadFileWithPos_log, void *pClient, void *pCmd, void *buffer, int size, int count, int pos, int fd, int flag, int error) {
    fs_log_byte_for_client(pClient, BYTE_READ_FILE_WITH_POS);
    return real_FSReadFileWithPos_log(pClient, pCmd, buffer, size, count, pos, fd, flag, error);
}
DECL(int, FSSetPosFile_log, void *pClient, void *pCmd, int fd, int pos, int error) {
    fs_log_byte_for_client(pClient, BYTE_SET_POS_FILE);
    return real_FSSetPosFile_log(pClient, pCmd, fd, pos, error);
}
DECL(void, FSSetStateChangeNotification_log, void *pClient, int r4) {
    fs_log_byte_for_client(pClient, BYTE_SET_STATE_CHG_NOTIF);
    real_FSSetStateChangeNotification_log(pClient, r4);
}
DECL(int, FSTruncateFile_log, void *pClient, void *pCmd, int fd, int error) {
    fs_log_byte_for_client(pClient, BYTE_TRUNCATE_FILE);
    return real_FSTruncateFile_log(pClient, pCmd, fd, error);
}
DECL(int, FSWriteFile_log, void *pClient, void *pCmd, const void *source, int size, int count, int fd, u32 flag, int error) {
    fs_log_byte_for_client(pClient, BYTE_WRITE_FILE);
    return real_FSWriteFile_log(pClient, pCmd, source, size, count, fd, flag, error);
}
DECL(int, FSWriteFileWithPos_log, void *pClient, void *pCmd, const void *source, int size, int count, int pos, int fd, u32 flag, int error) {
    fs_log_byte_for_client(pClient, BYTE_WRITE_FILE_WITH_POS);
    return real_FSWriteFileWithPos_log(pClient, pCmd, source, size, count, pos, fd, flag, error);
}

DECL(int, FSGetVolumeState_log, void *pClient) {
    int result = real_FSGetVolumeState_log(pClient);

    if ((int)bss_ptr != 0x0a000000)
    {
        if (result > 1)
        {
            int error = real_FSGetLastError_log(pClient);

            char buffer[200];
            __os_snprintf(buffer, sizeof(buffer), "FSGetVolumeState: %i, last error = %i", result, error);
            fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);
        }
    }

    return result;
}

#endif

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_extra_log[] __attribute__((section(".data"))) = {
    #if (USE_EXTRA_LOG_FUNCTIONS == 1)
    MAKE_MAGIC(FSCloseFile_log,             LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSCloseDir_log,              LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSFlushFile_log,             LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetErrorCodeForViewer_log, LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetLastError_log,          LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetPosFile_log,            LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFile_log,           LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEof_log,                 LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSReadDir_log,               LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFile_log,              LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPos_log,       LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFile_log,            LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSSetStateChangeNotification_log, LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSTruncateFile_log,          LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSWriteFile_log,             LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSWriteFileWithPos_log,      LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetVolumeState_log,        LIB_CORE_INIT,STATIC_FUNCTION),
#endif
};


u32 method_hooks_size_extra_log __attribute__((section(".data"))) = sizeof(method_hooks_extra_log) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_extra_log[sizeof(method_hooks_extra_log) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
