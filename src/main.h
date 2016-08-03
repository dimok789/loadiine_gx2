#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_defs.h"

/* General defines */
#define MAX_GAME_COUNT      255
#define MAX_GAME_ON_PAGE    11

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for out C++ functions
int Menu_Main(void);

#ifdef __cplusplus
}
#endif

#endif
