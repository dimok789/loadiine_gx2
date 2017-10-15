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

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <gctypes.h>
#include "function_patcher.h"
#include "common/retain_vars.h"
#include "utils/logger.h"
#include "common/kernel_defs.h"
#include "kernel/kernel_functions.h"

#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000
#define DEBUG_LOG_DYN                                   0


/*
* Patches a function that is loaded at the start of each application. Its not required to restore, at least when they are really dynamic.
* "normal" functions should be patch with the normal patcher. Current Code by Maschell with the help of dimok. Orignal code by Chadderz.
*/
void PatchInvidualMethodHooks(hooks_magic_t method_hooks[],s32 hook_information_size, volatile u32 dynamic_method_calls[])
{
    DEBUG_FUNCTION_LINE("Patching %d given functions\n",hook_information_size);
    /* Patch branches to it.  */
    volatile u32 *space = &dynamic_method_calls[0];

    s32 method_hooks_count = hook_information_size;

    u32 skip_instr = 1;
    u32 my_instr_len = 6;
    u32 instr_len = my_instr_len + skip_instr;
    u32 flush_len = 4*instr_len;
    for(s32 i = 0; i < method_hooks_count; i++)
    {
        DEBUG_FUNCTION_LINE("Patching %s ...",method_hooks[i].functionName);
        if(method_hooks[i].functionType == STATIC_FUNCTION && method_hooks[i].alreadyPatched == 1){
            if(isDynamicFunction((u32)OSEffectiveToPhysical((void*)method_hooks[i].realAddr))){
                log_printf("The function %s is a dynamic function. Please fix that <3\n", method_hooks[i].functionName);
                method_hooks[i].functionType = DYNAMIC_FUNCTION;
            }else{
                log_printf("Skipping %s, its already patched\n", method_hooks[i].functionName);
                space += instr_len;
                continue;
            }
        }

        u32 physical = 0;
        u32 repl_addr = (u32)method_hooks[i].replaceAddr;
        u32 call_addr = (u32)method_hooks[i].replaceCall;

        u32 real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            log_printf("\n");
            DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            space += instr_len;
            continue;
        }

        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("%s is located at %08X!\n", method_hooks[i].functionName,real_addr);}

        physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
             log_printf("Error. Something is wrong with the physical address\n");
             space += instr_len;
             continue;
        }

        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("%s physical is located at %08X!\n", method_hooks[i].functionName,physical);}

        *(volatile u32 *)(call_addr) = (u32)(space) - CODE_RW_BASE_OFFSET;


        SC0x25_KernelCopyData((u32)space, physical, 4);
        space++;

        //Only works if skip_instr == 1
        if(skip_instr == 1){
            // fill the restore instruction section
            method_hooks[i].realAddr = real_addr;
            method_hooks[i].restoreInstruction = *(space-1);
             if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("method_hooks[i].realAddr = %08X!\n", method_hooks[i].realAddr);}
             if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("method_hooks[i].restoreInstruction = %08X!\n",method_hooks[i].restoreInstruction) ;}
        }
        else{
            log_printf("Error. Can't save %s for restoring!\n", method_hooks[i].functionName);
        }

        //adding jump to real function thx @ dimok for the assembler code
        /*
            90 61 ff e0     stw     r3,-32(r1)
            3c 60 12 34     lis     r3,4660
            60 63 56 78     ori     r3,r3,22136
            7c 69 03 a6     mtctr   r3
            80 61 ff e0     lwz     r3,-32(r1)
            4e 80 04 20     bctr*/
        *space = 0x9061FFE0;
        space++;
        *space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
        space++;
        *space = 0x60630000 |  ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
        space++;
        *space = 0x7C6903A6; // mtctr   r3
        space++;
        *space = 0x8061FFE0; // lwz     r3,-32(r1)
        space++;
        *space = 0x4E800420; // bctr
        space++;
        DCFlushRange((void*)(space - instr_len), flush_len);
        ICInvalidateRange((unsigned char*)(space - instr_len), flush_len);

        //setting jump back
        u32 replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
        DCFlushRange(&replace_instr, 4);

        SC0x25_KernelCopyData(physical, (u32)OSEffectiveToPhysical(&replace_instr), 4);
        ICInvalidateRange((void*)(real_addr), 4);

        method_hooks[i].alreadyPatched = 1;
        log_printf("done!\n");

    }
    DEBUG_FUNCTION_LINE("Done with patching given functions!\n");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInvidualInstructions(hooks_magic_t method_hooks[],s32 hook_information_size)
{
    DEBUG_FUNCTION_LINE("Restoring given functions!\n");
    s32 method_hooks_count = hook_information_size;
    for(s32 i = 0; i < method_hooks_count; i++)
    {
        DEBUG_FUNCTION_LINE("Restoring %s... ",method_hooks[i].functionName);
        if(method_hooks[i].restoreInstruction == 0 || method_hooks[i].realAddr == 0){
            log_printf("I dont have the information for the restore =( skip\n");
            continue;
        }

        u32 real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            log_printf("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            continue;
        }

        u32 physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
            log_printf("Something is wrong with the physical address\n");
            continue;
        }

        if(isDynamicFunction(physical))
        {
             log_printf("Its a dynamic function. We don't need to restore it!\n",method_hooks[i].functionName);
        }
        else
        {
            physical = (u32)OSEffectiveToPhysical((void*)method_hooks[i].realAddr); //When its an static function, we need to use the old location
            if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("Restoring %08X to %08X\n",(u32)method_hooks[i].restoreInstruction,physical);}
            SC0x25_KernelCopyData(physical,(u32)&method_hooks[i].restoreInstruction , 4);
            if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("ICInvalidateRange %08X\n",(void*)method_hooks[i].realAddr);}
            ICInvalidateRange((void*)method_hooks[i].realAddr, 4);
            log_printf("done\n");
        }
        method_hooks[i].alreadyPatched = 0; // In case a
    }

    DEBUG_FUNCTION_LINE("Done with restoring given functions!\n");
}

s32 isDynamicFunction(u32 physicalAddress){
    if((physicalAddress & 0x80000000) == 0x80000000){
        return 1;
    }
    return 0;
}

u32 GetAddressOfFunction(const char * functionName,u32 library){
    u32 real_addr = 0;

    if(strcmp(functionName, "OSDynLoad_Acquire") == 0)
    {
        memcpy(&real_addr, &OSDynLoad_Acquire, 4);
        return real_addr;
    }
    else if(strcmp(functionName, "LiWaitOneChunk") == 0)
    {
        real_addr = (u32)addr_LiWaitOneChunk;
        return real_addr;
    }
    else if(strcmp(functionName, "LiBounceOneChunk") == 0)
    {
        //! not required on firmwares above 3.1.0
        if(OS_FIRMWARE >= 400)
            return 0;

        u32 addr_LiBounceOneChunk = 0x010003A0;
        real_addr = (u32)addr_LiBounceOneChunk;
        return real_addr;
    }

    u32 rpl_handle = 0;
    if(library == LIB_CORE_INIT){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_CORE_INIT\n", functionName);}
        if(coreinit_handle == 0){DEBUG_FUNCTION_LINE("LIB_CORE_INIT not acquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_NSYSNET){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NSYSNET\n", functionName);}
        if(nsysnet_handle == 0){DEBUG_FUNCTION_LINE("LIB_NSYSNET not acquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_GX2){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_GX2\n", functionName);}
        if(gx2_handle == 0){DEBUG_FUNCTION_LINE("LIB_GX2 not acquired\n"); return 0;}
        rpl_handle = gx2_handle;
    }
    else if(library == LIB_AOC){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AOC\n", functionName);}
        if(aoc_handle == 0){DEBUG_FUNCTION_LINE("LIB_AOC not acquired\n"); return 0;}
        rpl_handle = aoc_handle;
    }
    else if(library == LIB_AX){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AX\n", functionName);}
        if(sound_handle == 0){DEBUG_FUNCTION_LINE("LIB_AX not acquired\n"); return 0;}
        rpl_handle = sound_handle;
    }
    else if(library == LIB_AX_OLD){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AX_OLD\n", functionName);}
        if(sound_handle_old == 0){DEBUG_FUNCTION_LINE("LIB_AX_OLD not acquired\n"); return 0;}
        rpl_handle = sound_handle_old;
    }
    else if(library == LIB_FS){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_FS\n", functionName);}
        if(coreinit_handle == 0){DEBUG_FUNCTION_LINE("LIB_FS not acquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_OS){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_OS\n", functionName);}
        if(coreinit_handle == 0){DEBUG_FUNCTION_LINE("LIB_OS not acquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_PADSCORE){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_PADSCORE\n", functionName);}
        if(padscore_handle == 0){DEBUG_FUNCTION_LINE("LIB_PADSCORE not acquired\n"); return 0;}
        rpl_handle = padscore_handle;
    }
    else if(library == LIB_SOCKET){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SOCKET\n", functionName);}
        if(nsysnet_handle == 0){DEBUG_FUNCTION_LINE("LIB_SOCKET not acquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_SYS){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SYS\n", functionName);}
        if(sysapp_handle == 0){DEBUG_FUNCTION_LINE("LIB_SYS not acquired\n"); return 0;}
        rpl_handle = sysapp_handle;
    }
    else if(library == LIB_VPAD){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_VPAD\n", functionName);}
        if(vpad_handle == 0){DEBUG_FUNCTION_LINE("LIB_VPAD not acquired\n"); return 0;}
        rpl_handle = vpad_handle;
    }
    else if(library == LIB_NN_ACP){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NN_ACP\n", functionName);}
        if(acp_handle == 0){DEBUG_FUNCTION_LINE("LIB_NN_ACP not acquired\n"); return 0;}
        rpl_handle = acp_handle;
    }
    else if(library == LIB_SYSHID){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SYSHID\n", functionName);}
        if(syshid_handle == 0){DEBUG_FUNCTION_LINE("LIB_SYSHID not acquired\n"); return 0;}
        rpl_handle = syshid_handle;
    }
    else if(library == LIB_VPADBASE){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_VPADBASE\n", functionName);}
        if(vpadbase_handle == 0){DEBUG_FUNCTION_LINE("LIB_VPADBASE not acquired\n"); return 0;}
        rpl_handle = vpadbase_handle;
    }
    else if(library == LIB_PROC_UI){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_PROC_UI\n", functionName);}
        if(proc_ui_handle == 0){DEBUG_FUNCTION_LINE("LIB_PROC_UI not acquired\n"); return 0;}
        rpl_handle = proc_ui_handle;
    }
    else if(library == LIB_NTAG){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NTAG\n", functionName);}
        if(ntag_handle == 0){DEBUG_FUNCTION_LINE("LIB_NTAG not acquired\n"); return 0;}
        rpl_handle = proc_ui_handle;
    }
    else if(library == LIB_NFP){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NFP\n", functionName);}
        if(nfp_handle == 0){DEBUG_FUNCTION_LINE("LIB_NFP not acquired\n"); return 0;}
        rpl_handle = nfp_handle;
    }
    else if(library == LIB_SAVE){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SAVE\n", functionName);}
        if(nn_save_handle == 0){DEBUG_FUNCTION_LINE("LIB_SAVE not acquired\n"); return 0;}
        rpl_handle = nn_save_handle;
    }
    else if(library == LIB_ACT){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_ACT\n", functionName);}
        if(nn_act_handle == 0){DEBUG_FUNCTION_LINE("LIB_ACT not acquired\n"); return 0;}
        rpl_handle = nn_act_handle;
    }
    else if(library == LIB_NIM){
        if(DEBUG_LOG_DYN){DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NIM\n", functionName);}
        if(nn_nim_handle == 0){DEBUG_FUNCTION_LINE("LIB_NIM not acquired\n"); return 0;}
        rpl_handle = nn_nim_handle;
    }

    if(!rpl_handle){
        DEBUG_FUNCTION_LINE("Failed to find the RPL handle for %s\n", functionName);
        return 0;
    }

    OSDynLoad_FindExport(rpl_handle, 0, functionName, &real_addr);

    if(!real_addr){
        DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s\n", functionName);
        return 0;
    }

    if((library == LIB_NN_ACP) && (u32)(*(volatile u32*)(real_addr) & 0x48000002) == 0x48000000)
    {
        u32 address_diff = (u32)(*(volatile u32*)(real_addr) & 0x03FFFFFC);
        if((address_diff & 0x03000000) == 0x03000000) {
            address_diff |=  0xFC000000;
        }
        real_addr += (s32)address_diff;
        if((u32)(*(volatile u32*)(real_addr) & 0x48000002) == 0x48000000){
            return 0;
        }
    }

    return real_addr;
}

void PatchSDK(void)
{
    if(gPatchSDKDone)
        return;

    gPatchSDKDone = 1;

    log_printf("Patching SDK\n");

    // this only needs
    gLoaderPhysicalBufferAddr = (u32)OSEffectiveToPhysical((void*)0xF6000000);
    if(gLoaderPhysicalBufferAddr == 0)
        gLoaderPhysicalBufferAddr = 0x1B000000; // this is just in case and probably never needed

    u32 sdkLeAddr = 0;
    u32 sdkGtAddr = 0;

    u32 sdkLePatch = 0;
    u32 sdkGtPatch = 0;

    /* Patch to bypass SDK version tests */
    if((OS_FIRMWARE == 532) || (OS_FIRMWARE == 540))
    {
        /* Patch to bypass SDK version tests */
        sdkLeAddr = 0x010095b4;
        sdkLePatch = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)

        sdkGtAddr = 0x01009658;
        sdkGtPatch = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
    }
    else if((OS_FIRMWARE == 500) || (OS_FIRMWARE == 510))
    {
        /* Patch to bypass SDK version tests */
        sdkLeAddr = 0x010091CC;
        sdkLePatch = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)

        sdkGtAddr = 0x01009270;
        sdkGtPatch = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
    }
    else if ((OS_FIRMWARE == 400) || (OS_FIRMWARE == 410))
    {
        /* Patch to bypass SDK version tests */
        sdkLeAddr = 0x01008DAC;
        sdkLePatch = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)

        sdkGtAddr = 0x01008E50;
        sdkGtPatch = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
    }
    else if (OS_FIRMWARE < 400)
    {
        /* Patch to bypass SDK version tests */
        sdkLeAddr = 0x010067A8;
        sdkLePatch = 0x48000088;

        sdkGtAddr = 0x01006834;
        sdkGtPatch = 0x480000b8;
    }
	else if (OS_FIRMWARE == 550)
    {
        /* Patch to bypass SDK version tests */
        sdkLeAddr = 0x010097AC;
        sdkLePatch = 0x480000a0; // ble loc_1009654    (0x408100a0) => b loc_1009654      (0x480000a0)

        sdkGtAddr = 0x01009850;
        sdkGtPatch = 0x480000e8; // bge loc_1009740    (0x408100a0) => b loc_1009740      (0x480000e8)
    }

    u32 sdkLePhysAddr = (u32)OSEffectiveToPhysical((void*)sdkLeAddr);
    u32 sdkGtPhysAddr = (u32)OSEffectiveToPhysical((void*)sdkGtAddr);

    if(sdkLePhysAddr != 0 && sdkGtPhysAddr != 0)
    {
        DCFlushRange(&sdkLePatch, sizeof(sdkLePatch));
        DCFlushRange(&sdkGtPatch, sizeof(sdkGtPatch));

        SC0x25_KernelCopyData(sdkLePhysAddr, (u32)OSEffectiveToPhysical(&sdkLePatch), 4);
        SC0x25_KernelCopyData(sdkGtPhysAddr, (u32)OSEffectiveToPhysical(&sdkGtPatch), 4);

        ICInvalidateRange((void*)(sdkLeAddr), 4);
        ICInvalidateRange((void*)(sdkGtPhysAddr), 4);
    }
}
