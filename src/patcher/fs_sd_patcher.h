#ifndef _FS_SD_FUNCTION_PATCHER_H
#define _FS_SD_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t            method_hooks_fs_sd[];
extern u32                      method_hooks_size_fs_sd;
extern volatile unsigned int    method_calls_fs_sd[];

#ifdef __cplusplus
}
#endif

#endif /* _FS_SD_FUNCTION_PATCHER_H */
