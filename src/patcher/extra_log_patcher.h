#ifndef _EXTRA_LOG_FUNCTION_PATCHER_H
#define _EXTRA_LOG_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t            method_hooks_extra_log[];
extern u32                      method_hooks_size_extra_log;
extern volatile unsigned int    method_calls_extra_log[];

#ifdef __cplusplus
}
#endif

#endif /* _EXTRA_LOG_FUNCTION_PATCHER_H */
