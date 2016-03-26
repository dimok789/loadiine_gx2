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
#include "os_functions.h"
#include "vpad_functions.h"

unsigned int vpad_handle = 0;

EXPORT_DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error);
EXPORT_DECL(int, VPADGetLcdMode, int padnum, int *lcdmode);
EXPORT_DECL(int, VPADSetLcdMode, int padnum, int lcdmode);

void InitVPadFunctionPointers(void)
{
    unsigned int *funcPointer = 0;
    OSDynLoad_Acquire("vpad.rpl", &vpad_handle);

    OS_FIND_EXPORT(vpad_handle, VPADRead);
    OS_FIND_EXPORT(vpad_handle, VPADGetLcdMode);
    OS_FIND_EXPORT(vpad_handle, VPADSetLcdMode);
}

