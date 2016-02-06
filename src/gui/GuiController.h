/****************************************************************************
 * Copyright (C) 2015 Dimok
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
#ifndef GUI_CONTROLLER_H_
#define GUI_CONTROLLER_H_

#include <string.h>
#include "dynamic_libs/vpad_functions.h"

class GuiController
{
public:
    //!Constructor
    GuiController()
        : chan(0)
        , vpadError(0)
        , vpadErrorLast(0)
        , vpadDrcX(0), vpadDrcY(0)
        , vpadTvX(0), vpadTvY(0)
    {
        memset(&vpad, 0, sizeof(vpad));
        memset(&vpadLast, 0, sizeof(vpadLast));
    }

    //!Destructor
    virtual ~GuiController()  {}

    void update(int tvWidth, int tvHeight, int drcWidth, int drcHeight)
    {
        memcpy(&vpadLast, &vpad, sizeof(VPADData));
        chan = 1;
        VPADRead(0, &vpad, 1, &vpadError);
        //! calculate the screen offsets
        vpadDrcX = -(drcWidth >> 1) + (int)((vpad.tpdata1.x * drcWidth) >> 12);
        vpadDrcY = (drcHeight >> 1) - (int)(drcHeight - ((vpad.tpdata1.y * drcHeight) >> 12));
        vpadTvX = -(tvWidth >> 1) + (int)((vpad.tpdata1.x * tvWidth) >> 12);
        vpadTvY = (tvHeight >> 1) - (int)(tvHeight - ((vpad.tpdata1.y * tvHeight) >> 12));
    }

    int chan;
    int vpadError;
    int vpadErrorLast;
    int vpadDrcX;
    int vpadDrcY;
    int vpadTvX;
    int vpadTvY;
    VPADData vpad;
    VPADData vpadLast;
};

#endif
