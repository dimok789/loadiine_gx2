#ifndef __OS_DEFS_H_
#define __OS_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsSpecifics
{
    unsigned int addr_OSDynLoad_Acquire;
    unsigned int addr_OSDynLoad_FindExport;
    unsigned int addr_OSTitle_main_entry;

    unsigned int addr_LiWaitOneChunk;
    unsigned int addr_LiWaitIopComplete;
    unsigned int addr_LiWaitIopCompleteWithInterrupts;
} OsSpecifics;

#ifdef __cplusplus
}
#endif

#endif // __OS_DEFS_H_
