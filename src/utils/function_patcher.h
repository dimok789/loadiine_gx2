/****************************************************************************
 * Copyright (C) 2016 Maschell
 * With code from chadderz and dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#ifndef _FUNCTION_HOOKS_H_
#define _FUNCTION_HOOKS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>
#include "common/common.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/acp_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/proc_ui_functions.h"
#include "dynamic_libs/ntag_functions.h"
#include "dynamic_libs/nfp_functions.h"
#include "dynamic_libs/nn_save_functions.h"
#include "dynamic_libs/nn_act_functions.h"
#include "dynamic_libs/nn_nim_functions.h"

//Orignal code by Chadderz.
#define DECL(res, name, ...) \
        res (* real_ ## name)(__VA_ARGS__) __attribute__((section(".data"))); \
        res my_ ## name(__VA_ARGS__)

#define FUNCTION_PATCHER_METHOD_STORE_SIZE  7

typedef struct {
    const u32 replaceAddr;
    const u32 replaceCall;
    const u32 library;
    const char functionName[50];
    u32 realAddr;
    u32 restoreInstruction;
    u8 functionType;
    u8 alreadyPatched;
} hooks_magic_t;

void PatchInvidualMethodHooks(hooks_magic_t hook_information[],s32 hook_information_size, volatile u32 dynamic_method_calls[]);
void RestoreInvidualInstructions(hooks_magic_t hook_information[],s32 hook_information_size);
u32 GetAddressOfFunction(const char * functionName,u32 library);
s32 isDynamicFunction(u32 physicalAddress);

void PatchSDK(void);

//Orignal code by Chadderz.
#define MAKE_MAGIC(x, lib,functionType) { (u32) my_ ## x, (u32) &real_ ## x, lib, # x,0,0,functionType,0}
#define MAKE_MAGIC_NAME(x,y, lib,functionType) { (u32) my_ ## x, (u32) &real_ ## x, lib, # y,0,0,functionType,0}

#ifdef __cplusplus
}
#endif

#endif /* _FS_H */
