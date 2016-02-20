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
#ifndef VPAD_CONTROLLER_H_
#define VPAD_CONTROLLER_H_

#include "GuiController.h"
#include "dynamic_libs/vpad_functions.h"

class VPadController : public GuiController
{
public:
    //!Constructor
    VPadController(int channel)
        : GuiController(channel)
    {
        memset(&vpad, 0, sizeof(vpad));
    }

    //!Destructor
    virtual ~VPadController()  {}

    bool update(int width, int height)
    {
        lastData = data;

        int vpadError = -1;
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0)
        {
            data.buttons_r = vpad.btns_r;
            data.buttons_h = vpad.btns_h;
            data.buttons_d = vpad.btns_d;
            data.validPointer = !vpad.tpdata.invalid;
            data.touched = vpad.tpdata.touched;
            //! calculate the screen offsets
            data.x = -(width >> 1) + (int)((vpad.tpdata1.x * width) >> 12);
            data.y = (height >> 1) - (int)(height - ((vpad.tpdata1.y * height) >> 12));
            return true;
        }
        return false;
    }

private:
    VPADData vpad;
};

#endif
