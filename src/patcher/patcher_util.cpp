#include "patcher_util.h"
#include "common/retain_vars.h"
#include "cpp_to_c_util.h"

/* Client functions */
int client_num_alloc(void *pClient) {
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

int client_num(void *pClient) {
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == pClient)
            return i;
    return -1;
}

void client_num_free(int client) {
    bss.pClient_fs[client] = 0;
}

 int getPathType(const char *path) {
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

    while(*path && len < (int)(sizeof(new_path) - 1)) {
        new_path[len++] = *path++;
    }
    new_path[len++] = 0;

    /* Note : no need to check everything, it is faster this way */
    if (strncasecmp(new_path, "/vol/content", 12) == 0)
        return PATH_TYPE_GAME;

    if (strncasecmp(new_path, "/vol/save", 9) == 0)
        return PATH_TYPE_SAVE;

    if (strncasecmp(new_path, "/vol/aoc", 8) == 0)
        return PATH_TYPE_AOC;

    return 0;
}

void compute_new_path(char* new_path, const char* path, int len, int pathType) {

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

    if(pathType == PATH_TYPE_GAME)
    {
		char * pathfoo = (char*)path + 13 + path_offset;
		if(path[13 + path_offset] == '/') pathfoo++; //Skip double slash

        n = strlcpy(new_path, bss.mount_base, sizeof(bss.mount_base));

		if(gSettingUseUpdatepath){
			if(replacement_FileReplacer_isFileExisting(bss.file_replacer,pathfoo) == 0){
                n -= (sizeof(CONTENT_PATH) -1); // Go back the content folder! (-1 to ignore \0)
                n += strlcpy(new_path+n, UPDATE_PATH, sizeof(UPDATE_PATH));
                new_path[n++] = '/';
                n += strlcpy(new_path+n, gamePathStruct.update_folder, sizeof(gamePathStruct.update_folder));
                n += strlcpy(new_path+n, CONTENT_PATH, sizeof(CONTENT_PATH));
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
    else if(pathType == PATH_TYPE_SAVE)
    {
        n = strlcpy(new_path, bss.save_base, sizeof(bss.save_base));
        new_path[n++] = '/';

        if(gSettingUseUpdatepath){
            if(gamePathStruct.extraSave){
                n += strlcpy(new_path+n, gamePathStruct.update_folder, sizeof(gamePathStruct.update_folder));
                n += strlcpy(new_path+n, "/", 2);
            }
        }

        // Create path for common and user dirs
        if (path[10 + path_offset] == 'c') // common dir ("common")
        {
            n += strlcpy(&new_path[n], bss.save_dir_common, strlen(bss.save_dir_common) + 1);

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
            n += strlcpy(&new_path[n], bss.save_dir_user, strlen(bss.save_dir_user) + 1);

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
    else if(pathType == PATH_TYPE_AOC)
    {
        char * pathfoo = (char*)path + 4 + path_offset;
        if(pathfoo[0] == '/') pathfoo++; //Skip double slash

        n = strlcpy(new_path, bss.mount_base, sizeof(bss.mount_base)) - strlen(CONTENT_PATH);
        n += strlcpy(new_path + n, "/", sizeof(bss.mount_base) - n);
        n += strlcpy(new_path + n, pathfoo, sizeof(bss.mount_base) - n);

        new_path[n++] = '\0';
    }
}

int GetCurClient(void *pClient) {
    if ((int)bss_ptr != 0x0a000000) {
        int client = client_num(pClient);
        if (client >= 0) {
            return client;
        }
    }
    return -1;
}

int getNewPathLen(int pathType){

    int len_base = 0;
    if(pathType == PATH_TYPE_SAVE)
    {
        len_base += strlen(bss.save_base) + 15;
        if(gSettingUseUpdatepath){
            if(gamePathStruct.extraSave){
                len_base += (strlen(gamePathStruct.update_folder) + 2);
            }
        }
    }
    else if(pathType == PATH_TYPE_AOC)
    {
        len_base += strlen(bss.mount_base) - strlen(CONTENT_PATH) + 23;
    }
    else
    {
        len_base += strlen(bss.mount_base);
        if(gSettingUseUpdatepath){
             len_base += sizeof(UPDATE_PATH);
             //len_base += sizeof(CONTENT_PATH); <-- Is already in the path!
             len_base += 1; // "/"
             len_base += sizeof(gamePathStruct.update_folder);
        }
    }

    return len_base;
}
