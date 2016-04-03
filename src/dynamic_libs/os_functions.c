/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "common/common.h"
#include "os_functions.h"

unsigned int coreinit_handle = 0;

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Lib handle functions
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(int, OSDynLoad_Acquire, const char* rpl, u32 *handle);
EXPORT_DECL(int, OSDynLoad_FindExport, u32 handle, int isdata, const char *symbol, void *address);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Thread functions
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(int, OSCreateThread, void *thread, s32 (*callback)(s32, void*), s32 argc, void *args, u32 stack, u32 stack_size, s32 priority, u32 attr);
EXPORT_DECL(int, OSResumeThread, void *thread);
EXPORT_DECL(int, OSSuspendThread, void *thread);
EXPORT_DECL(int, OSIsThreadTerminated, void *thread);
EXPORT_DECL(int, OSIsThreadSuspended, void *thread);
EXPORT_DECL(int, OSSetThreadPriority, void * thread, int priority);
EXPORT_DECL(int, OSJoinThread, void * thread, int * ret_val);
EXPORT_DECL(void, OSDetachThread, void * thread);
EXPORT_DECL(void, OSSleepTicks, u64 ticks);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Mutex functions
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(void, OSInitMutex, void* mutex);
EXPORT_DECL(void, OSLockMutex, void* mutex);
EXPORT_DECL(void, OSUnlockMutex, void* mutex);
EXPORT_DECL(int, OSTryLockMutex, void* mutex);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! System functions
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(u64, OSGetTitleID, void);
EXPORT_DECL(void, __Exit, void);
EXPORT_DECL(void, OSFatal, const char* msg);
EXPORT_DECL(void, OSSetExceptionCallback, u8 exceptionType, exception_callback newCallback);
EXPORT_DECL(void, DCFlushRange, const void *addr, u32 length);
EXPORT_DECL(void, ICInvalidateRange, const void *addr, u32 length);
EXPORT_DECL(void*, OSEffectiveToPhysical, const void*);
EXPORT_DECL(int, __os_snprintf, char* s, int n, const char * format, ...);
EXPORT_DECL(int *, __gh_errno_ptr, void);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Memory functions
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_VAR(unsigned int *, pMEMAllocFromDefaultHeapEx);
EXPORT_VAR(unsigned int *, pMEMAllocFromDefaultHeap);
EXPORT_VAR(unsigned int *, pMEMFreeToDefaultHeap);

EXPORT_DECL(int, MEMGetBaseHeapHandle, int mem_arena);
EXPORT_DECL(unsigned int, MEMGetAllocatableSizeForFrmHeapEx, int heap, int align);
EXPORT_DECL(void *, MEMAllocFromFrmHeapEx, int heap, unsigned int size, int align);
EXPORT_DECL(void, MEMFreeToFrmHeap, int heap, int mode);
EXPORT_DECL(void *, MEMAllocFromExpHeapEx, int heap, unsigned int size, int align);
EXPORT_DECL(int , MEMCreateExpHeapEx, void* address, unsigned int size, unsigned short flags);
EXPORT_DECL(void *, MEMDestroyExpHeap, int heap);
EXPORT_DECL(void, MEMFreeToExpHeap, int heap, void* ptr);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Loader functions (not real rpl)
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(int, LiWaitIopComplete, int unknown_syscall_arg_r3, int * remaining_bytes);
EXPORT_DECL(int, LiWaitIopCompleteWithInterrupts, int unknown_syscall_arg_r3, int * remaining_bytes);
EXPORT_DECL(void, addr_LiWaitOneChunk, void);
EXPORT_DECL(void, addr_sgIsLoadingBuffer, void);
EXPORT_DECL(void, addr_gDynloadInitialized, void);

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Kernel function addresses
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
EXPORT_DECL(void, addr_PrepareTitle_hook, void);

void InitOSFunctionPointers(void)
{
    unsigned int *funcPointer = 0;
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! Lib handle functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    EXPORT_FUNC_WRITE(OSDynLoad_Acquire, (int (*)(const char*, unsigned *))OS_SPECIFICS->addr_OSDynLoad_Acquire);
    EXPORT_FUNC_WRITE(OSDynLoad_FindExport, (int (*)(u32, int, const char *, void *))OS_SPECIFICS->addr_OSDynLoad_FindExport);

    OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);

    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! System functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    OS_FIND_EXPORT(coreinit_handle, OSFatal);
    OS_FIND_EXPORT(coreinit_handle, OSGetTitleID);
    OS_FIND_EXPORT(coreinit_handle, OSSetExceptionCallback);
    OS_FIND_EXPORT(coreinit_handle, DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, OSEffectiveToPhysical);
    OS_FIND_EXPORT(coreinit_handle, __os_snprintf);
    OS_FIND_EXPORT(coreinit_handle, __gh_errno_ptr);
    OSDynLoad_FindExport(coreinit_handle, 0, "_Exit", &__Exit);
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! Thread functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    OS_FIND_EXPORT(coreinit_handle, OSCreateThread);
    OS_FIND_EXPORT(coreinit_handle, OSResumeThread);
    OS_FIND_EXPORT(coreinit_handle, OSSuspendThread);
    OS_FIND_EXPORT(coreinit_handle, OSIsThreadTerminated);
    OS_FIND_EXPORT(coreinit_handle, OSIsThreadSuspended);
    OS_FIND_EXPORT(coreinit_handle, OSJoinThread);
    OS_FIND_EXPORT(coreinit_handle, OSSetThreadPriority);
    OS_FIND_EXPORT(coreinit_handle, OSDetachThread);
    OS_FIND_EXPORT(coreinit_handle, OSSleepTicks);
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! Mutex functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    OS_FIND_EXPORT(coreinit_handle, OSInitMutex);
    OS_FIND_EXPORT(coreinit_handle, OSLockMutex);
    OS_FIND_EXPORT(coreinit_handle, OSUnlockMutex);
    OS_FIND_EXPORT(coreinit_handle, OSTryLockMutex);

    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! Memory functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &pMEMAllocFromDefaultHeapEx);
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeap", &pMEMAllocFromDefaultHeap);
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &pMEMFreeToDefaultHeap);

    OS_FIND_EXPORT(coreinit_handle, MEMGetBaseHeapHandle);
    OS_FIND_EXPORT(coreinit_handle, MEMGetAllocatableSizeForFrmHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMAllocFromFrmHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMFreeToFrmHeap);
    OS_FIND_EXPORT(coreinit_handle, MEMAllocFromExpHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMCreateExpHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMDestroyExpHeap);
    OS_FIND_EXPORT(coreinit_handle, MEMFreeToExpHeap);

    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //! Special non library functions
    //!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    if(OS_FIRMWARE == 532 || OS_FIRMWARE == 540)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100FFA4);                // loader.elf
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100FE90);  // loader.elf
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010007EC);              // loader.elf
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF18558);           // kernel.elf

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19D00);           // loader.elf
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE13C3C);         // loader.elf
    }
    else if(OS_FIRMWARE == 500 || OS_FIRMWARE == 510)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100FBC4);
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100FAB0);
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010007EC);
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF18534);

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19D00);
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE13C3C);
    }
    else if(OS_FIRMWARE == 410)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100F78C);                // ok
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100F678);  // ok
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010007F8);              // ok
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF166DC);           // ok

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19CC0);           // ok
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE13BFC);         // ok
    }
    else if(OS_FIRMWARE == 400) // same for 4.0.2 and 4.0.3
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100F78C);
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100F678);
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010007F8);
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF15E70);

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19CC0);
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE13BFC);
    }
    else if(OS_FIRMWARE == 310)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100C4E4);                // ok
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100C3D4);  // ok
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010004D8);              // ok
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF15A0C);           // ok

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19340);           // ok
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE1329C);         // ok
    }
    else if(OS_FIRMWARE == 300 || OS_FIRMWARE == 301)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x0100C4E4);                // ok
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0100C3D4);  // ok
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x010004D8);              // ok
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF15974);           // ok

        EXPORT_FUNC_WRITE(addr_sgIsLoadingBuffer, (int (*)(int, int *))0xEFE19340);           // ok
        EXPORT_FUNC_WRITE(addr_gDynloadInitialized, (int (*)(int, int *))0xEFE1329C);         // ok
    }
	else if(OS_FIRMWARE == 550)
    {
        EXPORT_FUNC_WRITE(LiWaitIopComplete, (int (*)(int, int *))0x01010180);
        EXPORT_FUNC_WRITE(LiWaitIopCompleteWithInterrupts, (int (*)(int, int *))0x0101006C);
        EXPORT_FUNC_WRITE(addr_LiWaitOneChunk, (int (*)(int, int *))0x0100080C);
        EXPORT_FUNC_WRITE(addr_PrepareTitle_hook, (int (*)(int, int *))0xFFF184E4);
    }
    else
    {
        OSFatal("Missing all OS specific addresses.");
    }
}
