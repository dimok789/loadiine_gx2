#ifndef _AOC_FUNCTION_PATCHER_H
#define _AOC_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"
#include "common/kernel_defs.h"

extern hooks_magic_t            method_hooks_aoc[];
extern u32                      method_hooks_size_aoc;
extern volatile unsigned int    method_calls_aoc[];

extern ReducedCosAppXmlInfo cosAppXmlInfoStruct;

#ifdef __cplusplus
}
#endif

#endif /* _AOC_FUNCTION_PATCHER_H */
