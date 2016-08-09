#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "fs_logger.h"
#include "common/common.h"
#include "common/loader_defs.h"
#include "common/retain_vars.h"

#include "game/rpx_rpl_table.h"
#include "kernel/kernel_functions.h"
#include "system/exception_handler.h"
#include "utils/function_patcher.h"
#include "fs/fs_utils.h"

#include "utils/logger.h"
#include "system/memory.h"

#include "patcher/fs_patcher.h"
#include "patcher/patcher_util.h"

#include "cpp_to_c_util.h"

DECL(int, FSInit, void)
{
    if(strlen(gServerIP) > 0){
        log_init(gServerIP);
    }

    if ((int)bss_ptr == 0x0a000000)
    {
        log_printf("FSINIT\n");
        if(gamePathStruct.os_game_path_base == 0)
            return real_FSInit();

        // allocate memory for our stuff
        void *mem_ptr = memalign(0x40, sizeof(struct bss_t));
        if(!mem_ptr)
            return real_FSInit();

        // copy pointer
        bss_ptr = (bss_t*)mem_ptr;
        memset(bss_ptr, 0, sizeof(struct bss_t));

        // first thing is connect to logger
        fs_logger_connect(&bss.global_sock);

        // create game mount path prefix
        __os_snprintf(bss.mount_base, sizeof(bss.mount_base), "%s/%s%s", gamePathStruct.os_game_path_base, gamePathStruct.game_dir, CONTENT_PATH);
		  // create game save path prefix
        __os_snprintf(bss.save_base, sizeof(bss.save_base), "%s/%s", gamePathStruct.os_save_path_base, gamePathStruct.game_dir);
        // copy save dirs
        __os_snprintf(bss.save_dir_common, sizeof(bss.save_dir_common), "%s", gamePathStruct.save_dir_common);
        __os_snprintf(bss.save_dir_user, sizeof(bss.save_dir_user), "%s", gamePathStruct.save_dir_user);

        // setup exceptions, has to be done once per core
        setup_os_exceptions();

        // Call real FSInit
        int result = real_FSInit();

        void* pClient = malloc(FS_CLIENT_SIZE);
        void* pCmd = malloc(FS_CMD_BLOCK_SIZE);

        if(pClient && pCmd)
        {
            FSInitCmdBlock(pCmd);
            FSAddClientEx(pClient, 0, -1);

            int result = MountFS(pClient, pCmd, 0);

			if(result == 0 && gSettingUseUpdatepath && GAME_LAUNCHED){
				char filepath[255];
				__os_snprintf(filepath, sizeof(filepath), "%s/%s%s/%s", gamePathStruct.os_game_path_base, gamePathStruct.game_dir,UPDATE_PATH, gamePathStruct.update_folder);
				bss.file_replacer = (void*)replacement_FileReplacer_initWithFile(filepath,CONTENT_PATH,FILELIST_NAME,pClient,pCmd);
			}

            fs_log_byte(bss.global_sock, (result == 0) ? BYTE_MOUNT_SD_OK : BYTE_MOUNT_SD_BAD);

            FSDelClient(pClient);
        }


        if(pClient)
            free(pClient);
        if(pCmd)
            free(pCmd);

        return result;
    }
    return real_FSInit();
}

DECL(int, FSShutdown, void) {
    if ((int)bss_ptr != 0x0a000000) {
        fs_logger_disconnect(bss.global_sock);
        bss.global_sock = -1;
		replacement_FileReplacer_destroy(bss.file_replacer);
    }
    return real_FSShutdown();
}

DECL(int, FSAddClientEx, void *pClient, int unk_param, int errHandling)
{
    int res = real_FSAddClientEx(pClient, unk_param, errHandling);

    if ((int)bss_ptr != 0x0a000000 && res >= 0)
    {
        if (GAME_RPX_LOADED != 0) {
            int client = client_num_alloc(pClient);
            if (client >= 0) {
                if (fs_logger_connect(&bss.socket_fs[client]) != 0)
                    client_num_free(client);
            }

        }
    }

    return res;
}

DECL(int, FSDelClient, void *pClient) {
    if ((int)bss_ptr != 0x0a000000) {
        int client = client_num(pClient);
        if (client >= 0) {
            fs_logger_disconnect(bss.socket_fs[client]);
            client_num_free(client);
        }
    }

    return real_FSDelClient(pClient);
}

// TODO: make new_path dynamically allocated from heap and not on stack to avoid stack overflow on too long names
// int len = m_strlen(path) + (is_save ? (m_strlen(bss.save_base) + 15) : m_strlen(bss.mount_base));
/* *****************************************************************************
 * Replacement functions
 * ****************************************************************************/
DECL(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_STAT);
        // change path if it is a game file
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            // return function with new_path if path exists
            return real_FSGetStat(pClient, pCmd, new_path, stats, error);
        }
    }
    return real_FSGetStat(pClient, pCmd, path, stats, error);
}

DECL(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, void *stats, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_STAT_ASYNC);

        // change path if it is a game/save file
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSGetStatAsync(pClient, pCmd, new_path, stats, error, asyncParams);
        }
    }
    return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);
}

DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error) {
/*
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_OPEN_FILE);

        // change path if it is a game file
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSOpenFile(pClient, pCmd, new_path, mode, handle, error);
        }
    }
*/
    return real_FSOpenFile(pClient, pCmd, path, mode, handle, error);
}

DECL(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, const void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_OPEN_FILE_ASYNC);

        // change path if it is a game file
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSOpenFileAsync(pClient, pCmd, new_path, mode, handle, error, asyncParams);
        }
    }
    return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error, asyncParams);
}

DECL(int, FSOpenDir, void *pClient, void* pCmd, const char *path, int *handle, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_OPEN_DIR);

        // change path if it is a game folder
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSOpenDir(pClient, pCmd, new_path, handle, error);
        }
    }
    return real_FSOpenDir(pClient, pCmd, path, handle, error);
}

DECL(int, FSOpenDirAsync, void *pClient, void* pCmd, const char *path, int *handle, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_OPEN_DIR_ASYNC);

        // change path if it is a game folder
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSOpenDirAsync(pClient, pCmd, new_path, handle, error, asyncParams);
        }
    }
    return real_FSOpenDirAsync(pClient, pCmd, path, handle, error, asyncParams);
}

DECL(int, FSChangeDir, void *pClient, void *pCmd, const char *path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_CHANGE_DIR);

        // change path if it is a game folder
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSChangeDir(pClient, pCmd, new_path, error);
        }
    }
    return real_FSChangeDir(pClient, pCmd, path, error);
}

DECL(int, FSChangeDirAsync, void *pClient, void *pCmd, const char *path, int error, void *asyncParams) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_CHANGE_DIR_ASYNC);

        // change path if it is a game folder
        int pathType = getPathType(path);
        if (pathType) {
            int len = strlen(path);
            int len_base = getNewPathLen(pathType);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, pathType);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSChangeDirAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSChangeDirAsync(pClient, pCmd, path, error, asyncParams);
}



/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
     // Common FS functions
    MAKE_MAGIC(FSInit,                      LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSShutdown,                  LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSAddClientEx,               LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSDelClient,                 LIB_CORE_INIT,STATIC_FUNCTION),

    // Replacement functions
    MAKE_MAGIC(FSGetStat,                   LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,              LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFile,                  LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,             LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDir,                   LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDirAsync,              LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDir,                 LIB_CORE_INIT,STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDirAsync,            LIB_CORE_INIT,STATIC_FUNCTION),

};


u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
