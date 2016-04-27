#ifndef __KERNEL_FUNCTIONS_H_
#define __KERNEL_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/kernel_defs.h"
#include "syscalls.h"

void SetupKernelCallback(void);

void KernelRestoreDBATs(bat_table_t * table);
void KernelSetDBATs(bat_table_t * table);
void KernelSetDBATsForDynamicFuction(bat_table_t * table, unsigned int physical_address);
void KernelSetDBATsInternal(bat_table_t * table, unsigned int high_address, unsigned int low_address);
#ifdef __cplusplus
}
#endif

#endif // __KERNEL_FUNCTIONS_H_
