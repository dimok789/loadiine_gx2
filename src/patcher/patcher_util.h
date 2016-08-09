#ifndef _PATCHER_UTIL_H
#define _PATCHER_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/common.h"
#include <stdio.h>
#include <string.h>

#define PATH_TYPE_IGNORE    0
#define PATH_TYPE_GAME      1
#define PATH_TYPE_SAVE      2
#define PATH_TYPE_AOC       3


/* Forward declarations */
#define MAX_CLIENT 32

struct bss_t {
    int global_sock;
    int socket_fs[MAX_CLIENT];
    void *pClient_fs[MAX_CLIENT];
    volatile int lock;
    char mount_base[255];
    char save_base[255];
	void* file_replacer;
	char update_path[50];
	char save_dir_common[7];
    char save_dir_user[9];
};

#define bss_ptr (*(struct bss_t **)0x100000e4)
#define bss (*bss_ptr)

extern game_paths_t gamePathStruct;

int client_num_alloc(void *pClient);
int client_num(void *pClient);
void client_num_free(int client);
int getPathType(const char *path);
void compute_new_path(char* new_path, const char* path, int len, int pathType);
int GetCurClient(void *pClient);
int getNewPathLen(int pathType);

#ifdef __cplusplus
}
#endif

#endif /* _PATCHER_UTIL_H */
