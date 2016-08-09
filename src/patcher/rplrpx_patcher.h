#ifndef _RPLRPX_FUNCTION_PATCHER_H
#define _RPLRPX_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t            method_hooks_rplrpx[];
extern u32                      method_hooks_size_rplrpx;
extern volatile unsigned int    method_calls_rplrpx[];

extern int (* FSInit)(void);
extern int (* real_FSInit)(void);
extern int (* FSGetStat)(void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling);
extern int (* real_FSGetStat)(void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling);
extern int (* FSOpenFile)(void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling);
extern int (* real_FSOpenFile)(void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling);

#ifdef __cplusplus
}
#endif

#endif /* _RPLRPX_FUNCTION_PATCHER_H */
