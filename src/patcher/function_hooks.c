#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "fs_logger.h"
#include "common/common.h"
#include "common/fs_defs.h"
#include "common/loader_defs.h"
#include "common/retain_vars.h"
#include "controller_patcher/cp_retain_vars.h"
#include "game/rpx_rpl_table.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/acp_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "kernel/kernel_functions.h"
#include "system/exception_handler.h"
#include "function_hooks.h"
#include "fs/fs_utils.h"
#include "utils/strings.h"
#include "utils/logger.h"
#include "settings/SettingsEnums.h"
#include "system/memory.h"
#include "controller_patcher/controller_patcher.h"

#include "cpp_to_c_util.h"

#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000
#define DEBUG_LOG_DYN                                   0

#define USE_EXTRA_LOG_FUNCTIONS   0

#define DECL(res, name, ...) \
        res (* real_ ## name)(__VA_ARGS__) __attribute__((section(".data"))); \
        res my_ ## name(__VA_ARGS__)

extern game_paths_t gamePathStruct;

/* Client functions */
static int client_num_alloc(void *pClient) {
    int i;

    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == pClient) {
            return i;
        }

    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == 0) {
            bss.pClient_fs[i] = pClient;
            return i;
        }
    return -1;
}

static int client_num(void *pClient) {
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == pClient)
            return i;
    return -1;
}

static void client_num_free(int client) {
    bss.pClient_fs[client] = 0;
}

static int is_gamefile(const char *path) {
    // In case the path starts by "//" and not "/" (some games do that ... ...)
    if (path[0] == '/' && path[1] == '/')
        path = &path[1];

    // In case the path does not start with "/" (some games do that too ...)
    int len = 0;
    char new_path[16];
    if(path[0] != '/') {
        new_path[0] = '/';
        len++;
    }

    while(*path && len < (int)sizeof(new_path)) {
        new_path[len++] = *path++;
    }

    /* Note : no need to check everything, it is faster this way */
    if (m_strncasecmp(new_path, "/vol/content", 12) == 0)
        return 1;

    return 0;
}
static int is_savefile(const char *path) {
    // In case the path starts by "//" and not "/" (some games do that ... ...)
    if (path[0] == '/' && path[1] == '/')
        path = &path[1];

    // In case the path does not start with "/" (some games do that too ...)
    int len = 0;
    char new_path[16];
    if(path[0] != '/') {
        new_path[0] = '/';
        len++;
    }

    while(*path && len < (int)sizeof(new_path)) {
        new_path[len++] = *path++;
    }

    if (m_strncasecmp(new_path, "/vol/save", 9) == 0)
        return 1;

    return 0;
}

static void compute_new_path(char* new_path, const char* path, int len, int is_save) {

    int i, n, path_offset = 0;

    // In case the path starts by "//" and not "/" (some games do that ... ...)
    if (path[0] == '/' && path[1] == '/')
        path = &path[1];

    // In case the path does not start with "/" set an offset for all the accesses
	if(path[0] != '/')
		path_offset = -1;

    // some games are doing /vol/content/./....
    if(path[13 + path_offset] == '.' && path[14 + path_offset] == '/') {
        path_offset += 2;
    }

    if (!is_save) {

		char * pathfoo = (char*)path + 13 + path_offset;
		if(path[13 + path_offset] == '/') pathfoo++; //Skip double slash

        n = m_strlcpy(new_path, bss.mount_base, sizeof(bss.mount_base));

		if(gSettingUseUpdatepath){
			if(replacement_FileReplacer_isFileExisting(bss.file_replacer,pathfoo) == 0){
                n -= (sizeof(CONTENT_PATH) -1); // Go back the content folder! (-1 to ignore \0)
                n += m_strlcpy(new_path+n, UPDATE_PATH, sizeof(UPDATE_PATH));
                new_path[n++] = '/';
                n += m_strlcpy(new_path+n, gamePathStruct.update_folder, sizeof(gamePathStruct.update_folder));
                n += m_strlcpy(new_path+n, CONTENT_PATH, sizeof(CONTENT_PATH));
			}
		}

        // copy the content file path with slash at the beginning
        for (i = 0; i < (len - 12 - path_offset); i++) {
            char cChar = path[12 + i + path_offset];
            // skip double slashes
            if((new_path[n-1] == '/') && (cChar == '/')) {
                continue;
            }
            new_path[n++] = cChar;
        }

        new_path[n++] = '\0';
    }
    else {
        n = m_strlcpy(new_path, bss.save_base, sizeof(bss.save_base));
        new_path[n++] = '/';

        if(gSettingUseUpdatepath){
            if(gamePathStruct.extraSave){
                n += m_strlcpy(new_path+n, gamePathStruct.update_folder, sizeof(gamePathStruct.update_folder));
                n += m_strlcpy(new_path+n, "/", 2);
            }
        }

        // Create path for common and user dirs
        if (path[10 + path_offset] == 'c') // common dir ("common")
        {
            n += m_strlcpy(&new_path[n], bss.save_dir_common, m_strlen(bss.save_dir_common) + 1);

            // copy the save game filename now with the slash at the beginning
            for (i = 0; i < (len - 16 - path_offset); i++) {
                char cChar = path[16 + path_offset + i];
                // skip double slashes
                if((new_path[n-1] == '/') && (cChar == '/')) {
                    continue;
                }
                new_path[n++] = cChar;
            }
        }
        else if (path[10 + path_offset] == '8') // user dir ("800000??") ?? = user permanent id
        {
            n += m_strlcpy(&new_path[n], bss.save_dir_user, m_strlen(bss.save_dir_user) + 1);

            // copy the save game filename now with the slash at the beginning
            for (i = 0; i < (len - 18 - path_offset); i++) {
               char cChar = path[18 + path_offset + i];
                // skip double slashes
                if((new_path[n-1] == '/') && (cChar == '/')) {
                    continue;
                }
                new_path[n++] = cChar;
            }
        }
        new_path[n++] = '\0';
    }
}

static int GetCurClient(void *pClient) {
    if ((int)bss_ptr != 0x0a000000) {
        int client = client_num(pClient);
        if (client >= 0) {
            return client;
        }
    }
    return -1;
}

static int getNewPathLen(int is_save){

    int len_base = 0;
    if(is_save){
        len_base += m_strlen(bss.save_base) + 15;
        if(gSettingUseUpdatepath){
            if(gamePathStruct.extraSave){
                len_base += (m_strlen(gamePathStruct.update_folder) + 2);
            }
        }
    }else{
        len_base +=  m_strlen(bss.mount_base);
        if(gSettingUseUpdatepath){
             len_base += sizeof(UPDATE_PATH);
             //len_base += sizeof(CONTENT_PATH); <-- Is already in the path!
             len_base += 1; // "/"
             len_base += sizeof(gamePathStruct.update_folder);
        }
    }

    return len_base;
}

DECL(int, FSInit, void)
{
    if ((int)bss_ptr == 0x0a000000)
    {
        if(gamePathStruct.os_game_path_base == 0)
            return real_FSInit();

        // allocate memory for our stuff
        void *mem_ptr = memalign(0x40, sizeof(struct bss_t));
        if(!mem_ptr)
            return real_FSInit();

        // copy pointer
        bss_ptr = mem_ptr;
        m_memset(bss_ptr, 0, sizeof(struct bss_t));

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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);

            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
            // log new path
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
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
        int is_save = 0;
        if (is_gamefile(path) || (is_save = is_savefile(path))) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(is_save);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, is_save);
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);
            return real_FSChangeDirAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSChangeDirAsync(pClient, pCmd, path, error, asyncParams);
}

// only for saves on sdcard
DECL(int, FSMakeDir, void *pClient, void *pCmd, const char *path, int error) {
    int client = GetCurClient(pClient);
    if (client != -1) {
        // log
        fs_log_string(bss.socket_fs[client], path, BYTE_MAKE_DIR);

        // change path if it is a save folder
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(oldPath)) {
            // old path
            int len_base = getNewPathLen(1);
            int len_old = m_strlen(oldPath);
            char new_old_path[len_old + len_base + 1];
            compute_new_path(new_old_path, oldPath, len_old, 1);

            // new path
            int len_new = m_strlen(newPath);
            char new_new_path[len_new + len_base + 1];
            compute_new_path(new_new_path, newPath, len_new, 1);

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
        if (is_savefile(oldPath)) {
            // old path
            int len_base = getNewPathLen(1);
            int len_old = m_strlen(oldPath);
            char new_old_path[len_old + len_base + 1];
            compute_new_path(new_old_path, oldPath, len_old, 1);

            // new path
            int len_new = m_strlen(newPath);
            char new_new_path[len_new + len_base + 1];
            compute_new_path(new_new_path, newPath, len_new, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

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
        if (is_savefile(path)) {
            int len = m_strlen(path);
            int len_base = getNewPathLen(1);
            char new_path[len + len_base + 1];
            compute_new_path(new_path, path, len, 1);

            // log new path
            fs_log_string(bss.socket_fs[client], new_path, BYTE_LOG_STR);

            return real_FSRollbackQuotaAsync(pClient, pCmd, new_path, error, asyncParams);
        }
    }
    return real_FSRollbackQuotaAsync(pClient, pCmd, path, error, asyncParams);
}

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
    my_FSInit();
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
    int path_len = m_strlen(gamePathStruct.os_game_path_base) + m_strlen(gamePathStruct.game_dir) + m_strlen(RPX_RPL_PATH) + m_strlen(rpl_entry->name) + 3;
	char *path_update_rpl = NULL;
    char *path_rpl = malloc(path_len);
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
		int path_len_update_rpl = m_strlen(gamePathStruct.os_game_path_base) + m_strlen(gamePathStruct.game_dir) + m_strlen(UPDATE_PATH) + m_strlen(gamePathStruct.update_folder) + m_strlen(RPX_RPL_PATH) + m_strlen(rpl_entry->name) + 4;
		path_update_rpl = malloc(path_len_update_rpl);
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
            // Copy in memory and save offset
            int copiedData = rpxRplCopyDataToMem(rpl_entry, rpl_size, dataBuf, ret);
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

        int len = m_strlen(rpl);
        int len2 = m_strlen(rpl_entry->name);
        if ((len != len2) && (len != (len2 - 4))) {
            continue;
        }

        // compare name string case insensitive and without ".rpl" extension
        if (m_strncasecmp(rpl_entry->name, rpl, len) == 0)
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
                int len = m_strnlen(filename, 0x1000);
                int len2 = m_strnlen(rpl_struct->name, 0x1000);
                if ((len != len2) && (len != (len2 - 4)))
                    continue;

                if(m_strncasecmp(filename, rpl_struct->name, len) != 0) {
                    found = 0;
                }
            }

            if (found)
            {
                unsigned int load_address = (sgBufferNumber == 1) ? 0xF6000000 : 0xF6400000;

                // set our game RPX loaded variable for use in FS system
                if(fileType == 0)
                    GAME_RPX_LOADED = 1;

                remaining_bytes = rpl_struct->size - sgFileOffset;
                if (remaining_bytes > 0x400000)
                    // truncate size
                    remaining_bytes = 0x400000;

                rpxRplCopyDataFromMem(rpl_struct, sgFileOffset, (unsigned char*)load_address, remaining_bytes);
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

DECL(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){

    if(gHIDCurrentDevice & HID_LIST_MOUSE && gHID_Mouse_Mode == HID_MOUSE_MODE_TOUCH) {
        draw_Cursor_at(gHID_Mouse.pad_data[0].data[0].X, gHID_Mouse.pad_data[0].data[0].Y);
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer,scan_target);
}

DECL(void, _Exit, void){
    draw_Cursor_destroy();
    real__Exit();
}
DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {

    int result = real_VPADRead(chan, buffer, buffer_size, error);

    if(gHIDAttached){
        setControllerDataFromHID(buffer,gHIDCurrentDevice);
    }

    if(gSettingPadconMode == SETTING_ON){
        if(buffer->btns_r&VPAD_BUTTON_STICK_R) {
            int mode;
            VPADGetLcdMode(0, &mode);       // Get current display mode
            if(mode != 1) {
                VPADSetLcdMode(0, 1);       // Turn it off
            }
            else {
                VPADSetLcdMode(0, 0xFF);    // Turn it on
            }
        }
    }
    return result;
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/
#define MAKE_MAGIC(x, lib,functionType) { (unsigned int) my_ ## x, (unsigned int) &real_ ## x, lib, # x,0,0,functionType,0}

static struct hooks_magic_t {
    const unsigned int replaceAddr;
    const unsigned int replaceCall;
    const unsigned int library;
    const char functionName[50];
    unsigned int realAddr;
    unsigned int restoreInstruction;
    unsigned char functionType;
    unsigned char alreadyPatched;
} method_hooks[] = {
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

    // LOADER function
    MAKE_MAGIC(LiWaitOneChunk,              LIB_LOADER,STATIC_FUNCTION),
    MAKE_MAGIC(LiBounceOneChunk,            LIB_LOADER,STATIC_FUNCTION),

    // Dynamic RPL loading functions
    MAKE_MAGIC(OSDynLoad_Acquire,           LIB_CORE_INIT,STATIC_FUNCTION),

    MAKE_MAGIC(VPADRead,                    LIB_VPAD,STATIC_FUNCTION),
    MAKE_MAGIC(GX2CopyColorBufferToScanBuffer,     LIB_GX2,STATIC_FUNCTION),
    MAKE_MAGIC(_Exit,     LIB_CORE_INIT,STATIC_FUNCTION),

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

//! buffer to store our 7 instructions needed for our replacements
//! the code will be placed in the address of that buffer - CODE_RW_BASE_OFFSET
//! avoid this buffer to be placed in BSS and reset on start up
volatile unsigned int dynamic_method_calls[sizeof(method_hooks) / sizeof(struct hooks_magic_t) * 7] __attribute__((section(".data")));

/*
*Patches a function that is loaded at the start of each application. Its not required to restore, at least when they are really dynamic.
* "normal" functions should be patch with the normal patcher.
*/
void PatchMethodHooks(void)
{
    /* Patch branches to it.  */
    volatile unsigned int *space = &dynamic_method_calls[0];

    int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);

    u32 skip_instr = 1;
    u32 my_instr_len = 6;
    u32 instr_len = my_instr_len + skip_instr;
    u32 flush_len = 4*instr_len;
    for(int i = 0; i < method_hooks_count; i++)
    {
        if(method_hooks[i].functionType == STATIC_FUNCTION && method_hooks[i].alreadyPatched == 1){
            if(isDynamicFunction((u32)OSEffectiveToPhysical((void*)method_hooks[i].realAddr))){
                log_printf("The function %s is a dynamic function. Please fix that <3\n", method_hooks[i].functionName);
                method_hooks[i].functionType = DYNAMIC_FUNCTION;
            }else{
                log_printf("Skipping %s, its already patched\n", method_hooks[i].functionName);
                space += instr_len;
                continue;
            }
        }

        u32 physical = 0;
        unsigned int repl_addr = (unsigned int)method_hooks[i].replaceAddr;
        unsigned int call_addr = (unsigned int)method_hooks[i].replaceCall;

        unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            log_printf("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            space += instr_len;
            continue;
        }

        if(DEBUG_LOG_DYN)log_printf("%s is located at %08X!\n", method_hooks[i].functionName,real_addr);

        physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
             log_printf("Something is wrong with the physical address\n");
             space += instr_len;
             continue;
        }

        if(DEBUG_LOG_DYN)log_printf("%s physical is located at %08X!\n", method_hooks[i].functionName,physical);

        bat_table_t my_dbat_table;
        if(DEBUG_LOG_DYN)log_printf("Setting up DBAT\n");
        KernelSetDBATsForDynamicFuction(&my_dbat_table,physical);

        //log_printf("Setting call_addr to %08X\n",(unsigned int)(space) - CODE_RW_BASE_OFFSET);
        *(volatile unsigned int *)(call_addr) = (unsigned int)(space) - CODE_RW_BASE_OFFSET;

        // copy instructions from real function.
        u32 offset_ptr = 0;
        for(offset_ptr = 0;offset_ptr<skip_instr*4;offset_ptr +=4){
             if(DEBUG_LOG_DYN)log_printf("(real_)%08X = %08X\n",space,*(volatile unsigned int*)(physical+offset_ptr));
            *space = *(volatile unsigned int*)(physical+offset_ptr);
            space++;
        }

        //Only works if skip_instr == 1
        if(skip_instr == 1){
            // fill the restore instruction section
            method_hooks[i].realAddr = real_addr;
            method_hooks[i].restoreInstruction = *(volatile unsigned int*)(physical);
        }else{
            log_printf("Can't save %s for restoring!\n", method_hooks[i].functionName);
        }

        //adding jump to real function
        /*
            90 61 ff e0     stw     r3,-32(r1)
            3c 60 12 34     lis     r3,4660
            60 63 56 78     ori     r3,r3,22136
            7c 69 03 a6     mtctr   r3
            80 61 ff e0     lwz     r3,-32(r1)
            4e 80 04 20     bctr*/
        *space = 0x9061FFE0;
        space++;
        *space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
        space++;
        *space = 0x60630000 |  ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
        space++;
        *space = 0x7C6903A6; // mtctr   r3
        space++;
        *space = 0x8061FFE0; // lwz     r3,-32(r1)
        space++;
        *space = 0x4E800420; // bctr
        space++;
        DCFlushRange((void*)(space - instr_len), flush_len);
        ICInvalidateRange((unsigned char*)(space - instr_len), flush_len);

        //setting jump back
        unsigned int replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
        *(volatile unsigned int *)(physical) = replace_instr;
        ICInvalidateRange((void*)(real_addr), 4);

        //restore my dbat stuff
        KernelRestoreDBATs(&my_dbat_table);

        method_hooks[i].alreadyPatched = 1;
    }
    log_print("Done with patching all functions!\n");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInstructions(void)
{
    bat_table_t table;
    log_printf("Restore functions!\n");
    int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);
    for(int i = 0; i < method_hooks_count; i++)
    {
        if(method_hooks[i].restoreInstruction == 0 || method_hooks[i].realAddr == 0){
            log_printf("I dont have the information for the restore =( skip\n");
            continue;
        }

        unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            log_printf("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            continue;
        }

        u32 physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
            log_printf("Something is wrong with the physical address\n");
            continue;
        }

        if(isDynamicFunction(physical)){
             log_printf("Its a dynamic function. We don't need to restore it! %s\n",method_hooks[i].functionName);
        }else{
            KernelSetDBATs(&table);

            *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr) = method_hooks[i].restoreInstruction;
            DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr), 4);
            ICInvalidateRange((void*)method_hooks[i].realAddr, 4);
            log_printf("Restored %s\n",method_hooks[i].functionName);
            KernelRestoreDBATs(&table);
        }
        method_hooks[i].alreadyPatched = 0; // In case a
    }
    KernelRestoreInstructions();
    gPatchSDKDone = 0;
    log_print("Done with restoring all functions!\n");
}

int isDynamicFunction(unsigned int physicalAddress){
    if((physicalAddress & 0x80000000) == 0x80000000){
        return 1;
    }
    return 0;
}

unsigned int GetAddressOfFunction(const char * functionName,unsigned int library){
    unsigned int real_addr = 0;

    if(strcmp(functionName, "OSDynLoad_Acquire") == 0)
    {
        memcpy(&real_addr, &OSDynLoad_Acquire, 4);
        return real_addr;
    }
    else if(strcmp(functionName, "LiWaitOneChunk") == 0)
    {
        real_addr = addr_LiWaitOneChunk;
        return real_addr;
    }
    else if(strcmp(functionName, "LiBounceOneChunk") == 0)
    {
        //! not required on firmwares above 3.1.0
        if(OS_FIRMWARE >= 400)
            return 0;

        unsigned int addr_LiBounceOneChunk = 0x010003A0;
        real_addr = addr_LiBounceOneChunk;
        return real_addr;
    }

    unsigned int rpl_handle = 0;
    if(library == LIB_CORE_INIT){
        log_printf("FindExport of %s! From LIB_CORE_INIT\n", functionName);
        if(coreinit_handle == 0){log_print("LIB_CORE_INIT not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_NSYSNET){
        log_printf("FindExport of %s! From LIB_NSYSNET\n", functionName);
        if(nsysnet_handle == 0){log_print("LIB_NSYSNET not aquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_GX2){
        log_printf("FindExport of %s! From LIB_GX2\n", functionName);
        if(gx2_handle == 0){log_print("LIB_GX2 not aquired\n"); return 0;}
        rpl_handle = gx2_handle;
    }
    else if(library == LIB_AOC){
        log_printf("FindExport of %s! From LIB_AOC\n", functionName);
        if(aoc_handle == 0){log_print("LIB_AOC not aquired\n"); return 0;}
        rpl_handle = aoc_handle;
    }
    else if(library == LIB_AX){
        log_printf("FindExport of %s! From LIB_AX\n", functionName);
        if(sound_handle == 0){log_print("LIB_AX not aquired\n"); return 0;}
        rpl_handle = sound_handle;
    }
    else if(library == LIB_FS){
        log_printf("FindExport of %s! From LIB_FS\n", functionName);
        if(coreinit_handle == 0){log_print("LIB_FS not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_OS){
        log_printf("FindExport of %s! From LIB_OS\n", functionName);
        if(coreinit_handle == 0){log_print("LIB_OS not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_PADSCORE){
        log_printf("FindExport of %s! From LIB_PADSCORE\n", functionName);
        if(padscore_handle == 0){log_print("LIB_PADSCORE not aquired\n"); return 0;}
        rpl_handle = padscore_handle;
    }
    else if(library == LIB_SOCKET){
        log_printf("FindExport of %s! From LIB_SOCKET\n", functionName);
        if(nsysnet_handle == 0){log_print("LIB_SOCKET not aquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_SYS){
        log_printf("FindExport of %s! From LIB_SYS\n", functionName);
        if(sysapp_handle == 0){log_print("LIB_SYS not aquired\n"); return 0;}
        rpl_handle = sysapp_handle;
    }
    else if(library == LIB_VPAD){
        log_printf("FindExport of %s! From LIB_VPAD\n", functionName);
        if(vpad_handle == 0){log_print("LIB_VPAD not aquired\n"); return 0;}
        rpl_handle = vpad_handle;
    }
    else if(library == LIB_NN_ACP){
        log_printf("FindExport of %s! From LIB_NN_ACP\n", functionName);
        if(acp_handle == 0){log_print("LIB_NN_ACP not aquired\n"); return 0;}
        rpl_handle = acp_handle;
    }
    else if(library == LIB_SYSHID){
        log_printf("FindExport of %s! From LIB_SYSHID\n", functionName);
        if(syshid_handle == 0){log_print("LIB_SYSHID not aquired\n"); return 0;}
        rpl_handle = syshid_handle;
    }
    else if(library == LIB_VPADBASE){
        log_printf("FindExport of %s! From LIB_VPADBASE\n", functionName);
        if(vpadbase_handle == 0){log_print("LIB_VPADBASE not aquired\n"); return 0;}
        rpl_handle = vpadbase_handle;
    }

    if(!rpl_handle){
        log_printf("Failed to find the RPL handle for %s\n", functionName);
        return 0;
    }

    OSDynLoad_FindExport(rpl_handle, 0, functionName, &real_addr);

    if(!real_addr){
        log_printf("OSDynLoad_FindExport failed for %s\n", functionName);
        return 0;
    }

    if((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000){
        real_addr += (u32)(*(volatile unsigned int*)(real_addr) & 0x0000FFFF);
        if((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000){
            return 0;
        }
    }

    return real_addr;
}

void PatchSDK(void){
    if(gPatchSDKDone) return;
    gPatchSDKDone = 1;
    bat_table_t table;
    KernelSetDBATs(&table);
    //! TODO: Not sure if this is still needed at all after changing the SDK version in the xml struct, check that
    if((OS_FIRMWARE == 532) || (OS_FIRMWARE == 540))
    {
        /* Patch to bypass SDK version tests */
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x010095b4)) = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01009658)) = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x010095b4), 4);
        ICInvalidateRange((void*)(0x010095b4), 4);
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01009658), 4);
        ICInvalidateRange((void*)(0x01009658), 4);
    }
    else if((OS_FIRMWARE == 500) || (OS_FIRMWARE == 510))
    {
        /* Patch to bypass SDK version tests */
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x010091CC)) = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01009270)) = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x010091CC), 4);
        ICInvalidateRange((void*)(0x010091CC), 4);
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01009270), 4);
        ICInvalidateRange((void*)(0x01009270), 4);
    }
    else if ((OS_FIRMWARE == 400) || (OS_FIRMWARE == 410))
    {
        /* Patch to bypass SDK version tests */
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01008DAC)) = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01008E50)) = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01008DAC), 4);
        ICInvalidateRange((void*)(0x01008DAC), 4);
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01008E50), 4);
        ICInvalidateRange((void*)(0x01008E50), 4);
    }
    else if (OS_FIRMWARE < 400)
    {
        /* Patch to bypass SDK version tests */
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x010067A8)) = 0x48000088;
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01006834)) = 0x480000b8;
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x010067A8), 4);
        ICInvalidateRange((void*)(0x010067A8), 4);
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01006834), 4);
        ICInvalidateRange((void*)(0x01006834), 4);
    }
	else if (OS_FIRMWARE == 550)
    {
       /* Patch to bypass SDK version tests */
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x010097AC)) = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)
        *((volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + 0x01009850)) = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x010097AC), 4);
        ICInvalidateRange((void*)(0x010097AC), 4);
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + 0x01009850), 4);
        ICInvalidateRange((void*)(0x01009850), 4);
    }
    KernelRestoreDBATs(&table);
}
