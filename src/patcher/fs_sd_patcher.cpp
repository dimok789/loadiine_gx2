#include "malloc.h"
#include "common/retain_vars.h"
#include "fs_sd_patcher.h"
#include "patcher_util.h"
#include "fs_logger.h"

// only for saves on sdcard
DECL(int, FSMakeDir, void *pClient, void *pCmd, const char *path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_MAKE_DIR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSMakeDir(pClient, pCmd, new_path, error);
        }
    }
    return real_FSMakeDir(pClient, pCmd, path, error);
}

// only for saves on sdcard
DECL(int, FSMakeDirAsync, void *pClient, void *pCmd, const char *path, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_MAKE_DIR_ASYNC);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSMakeDirAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSMakeDirAsync(pClient, pCmd, path, error, asyncParams);
}

// only for saves on sdcard
DECL(int, FSRename, void *pClient, void *pCmd, const char *oldPath, const char *newPath, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], oldPath, BYTE_RENAME);
        fs_log_string(bss.socket_fs[client], newPath, BYTE_RENAME);

        // change path if it is a save folder
        int pathType = getPathType(oldPath);
        if (pathType == PATH_TYPE_SAVE) {
            // old path
            int len_base = getNewPathLen(pathType);
            int len_old = strlen(oldPath);
            char new_old_path[len_old + len_base + 1];
            compute_new_path(new_old_path, oldPath, len_old, pathType);

            // new path
            int len_new = strlen(newPath);
            char new_new_path[len_new + len_base + 1];
            compute_new_path(new_new_path, newPath, len_new, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_old_path, BYTE_LOG_STR);
            fs_log_string(bss.socket_fs[client], new_new_path, BYTE_LOG_STR);

            return real_FSRename(pClient, pCmd, new_old_path, new_new_path, error);
        }
    }
    return real_FSRename(pClient, pCmd, oldPath, newPath, error);
}

// only for saves on sdcard
DECL(int, FSRenameAsync, void *pClient, void *pCmd, const char *oldPath, const char *newPath, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], oldPath, BYTE_RENAME);
        fs_log_string(bss.socket_fs[client], newPath, BYTE_RENAME);

        // change path if it is a save folder
        int pathType = getPathType(oldPath);
        if (pathType == PATH_TYPE_SAVE) {
            // old path
            int len_base = getNewPathLen(pathType);
            int len_old = strlen(oldPath);
            char new_old_path[len_old + len_base + 1];
            compute_new_path(new_old_path, oldPath, len_old, pathType);

            // new path
            int len_new = strlen(newPath);
            char new_new_path[len_new + len_base + 1];
            compute_new_path(new_new_path, newPath, len_new, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_old_path, BYTE_LOG_STR);
            fs_log_string(bss.socket_fs[client], new_new_path, BYTE_LOG_STR);

            return real_FSRenameAsync(pClient, pCmd, new_old_path, new_new_path, error, asyncParams);
        }
    }
    return real_FSRenameAsync(pClient, pCmd, oldPath, newPath, error, asyncParams);
}

// only for saves on sdcard
DECL(int, FSRemove, void *pClient, void *pCmd, const char *path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSRemove(pClient, pCmd, new_path, error);
        }
    }
    return real_FSRemove(pClient, pCmd, path, error);
}

// only for saves on sdcard
DECL(int, FSRemoveAsync, void *pClient, void *pCmd, const char *path, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSRemoveAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSRemoveAsync(pClient, pCmd, path, error, asyncParams);
}

DECL(int, FSFlushQuota, void *pClient, void *pCmd, const char* path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        //fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSFlushQuota: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSFlushQuota(pClient, pCmd, new_path, error);
        }
    }
    return real_FSFlushQuota(pClient, pCmd, path, error);
}
DECL(int, FSFlushQuotaAsync, void *pClient, void *pCmd, const char *path, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        //fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSFlushQuotaAsync: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSFlushQuotaAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSFlushQuotaAsync(pClient, pCmd, path, error, asyncParams);
}

DECL(int, FSGetFreeSpaceSize, void *pClient, void *pCmd, const char *path, uint64_t *returnedFreeSize, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        //fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSGetFreeSpaceSize: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSGetFreeSpaceSize(pClient, pCmd, new_path, returnedFreeSize, error);
        }
    }
    return real_FSGetFreeSpaceSize(pClient, pCmd, path, returnedFreeSize, error);
}
DECL(int, FSGetFreeSpaceSizeAsync, void *pClient, void *pCmd, const char *path, uint64_t *returnedFreeSize, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        //fs_log_string(bss.socket_fs[client], path, BYTE_REMOVE);
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSGetFreeSpaceSizeAsync: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSGetFreeSpaceSizeAsync(pClient, pCmd, new_path, returnedFreeSize, error, asyncParams);
        }
    }
    return real_FSGetFreeSpaceSizeAsync(pClient, pCmd, path, returnedFreeSize, error, asyncParams);
}

DECL(int, FSRollbackQuota, void *pClient, void *pCmd, const char *path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSRollbackQuota: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSRollbackQuota(pClient, pCmd, new_path, error);
        }
    }
    return real_FSRollbackQuota(pClient, pCmd, path, error);
}
DECL(int, FSRollbackQuotaAsync, void *pClient, void *pCmd, const char *path, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        char buffer[200];
        __os_snprintf(buffer, sizeof(buffer), "FSRollbackQuotaAsync: %s", path);
        fs_log_string(bss.global_sock, buffer, BYTE_LOG_STR);

        // change path if it is a save folder
        int pathType = getPathType(path);
        if (pathType == PATH_TYPE_SAVE) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSRollbackQuotaAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSRollbackQuotaAsync(pClient, pCmd, path, error, asyncParams);
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_fs_sd[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSMakeDir,                   LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSMakeDirAsync,              LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRename,                    LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRenameAsync,               LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRemove,                    LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRemoveAsync,               LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSFlushQuota,                LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSFlushQuotaAsync,           LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetFreeSpaceSize,          LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetFreeSpaceSizeAsync,     LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRollbackQuota,             LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSRollbackQuotaAsync,        LIB_CORE_INIT,STATIC_FUNCTION),
};


u32 method_hooks_size_fs_sd __attribute__((section(".data"))) = sizeof(method_hooks_fs_sd) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs_sd[sizeof(method_hooks_fs_sd) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

