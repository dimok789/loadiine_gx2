#ifndef __LOADER_DEFS_H_
#define __LOADER_DEFS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// struct holding the globals of the loader (there are actually more but we don't need others)
typedef struct _loader_globals_t
{
    int sgIsLoadingBuffer;
    int sgFileType;
    int sgProcId;
    int sgGotBytes;
    int sgFileOffset;
    int sgBufferNumber;
    int sgBounceError;
    char sgLoadName[0x1000];
} __attribute__((packed)) loader_globals_t;

typedef struct _loader_globals_550_t
{
    int sgFinishedLoadingBuffer;
    int sgFileType;
    int sgProcId;
    int sgGotBytes;
    int sgTotalBytes;
    int sgFileOffset;
    int sgBufferNumber;
    int sgBounceError;
    char sgLoadName[0x1000];
} __attribute__((packed)) loader_globals_550_t;

#ifdef __cplusplus
}
#endif

#endif // __LOADER_DEFS_H_
