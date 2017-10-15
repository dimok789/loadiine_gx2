#ifndef _RPLRPX_FUNCTION_PATCHER_H
#define _RPLRPX_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t            method_hooks_rplrpx[];
extern u32                      method_hooks_size_rplrpx;
extern volatile u32             method_calls_rplrpx[];

extern s32 (* FSInit)(void);
extern s32 (* real_FSInit)(void);
extern s32 (* FSGetStat)(void *pClient, void *pCmd, const char *path, FSStat *stats, s32 errHandling);
extern s32 (* real_FSGetStat)(void *pClient, void *pCmd, const char *path, FSStat *stats, s32 errHandling);
extern s32 (* FSOpenFile)(void *pClient, void *pCmd, const char *path, const char *mode, s32 *fd, s32 errHandling);
extern s32 (* real_FSOpenFile)(void *pClient, void *pCmd, const char *path, const char *mode, s32 *fd, s32 errHandling);

#ifdef __cplusplus
}
#endif

#endif /* _RPLRPX_FUNCTION_PATCHER_H */
