/***************************************************************************
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
#ifndef GUI_TRIGGER_H_
#define GUI_TRIGGER_H_

#include "dynamic_libs/os_functions.h"


//!Menu input trigger management. Determine if action is neccessary based on input data by comparing controller input data to a specific trigger element.
class GuiTrigger
{
public:
    enum eChannels {
        CHANNEL_1       = 0x01,
        CHANNEL_2       = 0x02,
        CHANNEL_3       = 0x04,
        CHANNEL_4       = 0x08,
        CHANNEL_5       = 0x10,
        CHANNEL_ALL     = 0xFF
    };
    enum eButtons {
        BUTTON_NONE     = 0x0000,
        VPAD_TOUCH      = 0x80000000,
        BUTTON_Z        = 0x20000,
        BUTTON_C        = 0x10000,
        BUTTON_A        = 0x8000,
        BUTTON_B        = 0x4000,
        BUTTON_X        = 0x2000,
        BUTTON_Y        = 0x1000,
        BUTTON_1        = BUTTON_Y,
        BUTTON_2        = BUTTON_X,
        BUTTON_LEFT     = 0x0800,
        BUTTON_RIGHT    = 0x0400,
        BUTTON_UP       = 0x0200,
        BUTTON_DOWN     = 0x0100,
        BUTTON_ZL       = 0x0080,
        BUTTON_ZR       = 0x0040,
        BUTTON_L        = 0x0020,
        BUTTON_R        = 0x0010,
        BUTTON_PLUS     = 0x0008,
        BUTTON_MINUS    = 0x0004,
        BUTTON_HOME     = 0x0002,
        BUTTON_SYNC     = 0x0001,
        STICK_R_LEFT    = 0x04000000,
        STICK_R_RIGHT   = 0x02000000,
        STICK_R_UP      = 0x01000000,
        STICK_R_DOWN    = 0x00800000,
        STICK_L_LEFT    = 0x40000000,
        STICK_L_RIGHT   = 0x20000000,
        STICK_L_UP      = 0x10000000,
        STICK_L_DOWN    = 0x08000000
    };

    //!Constructor
    GuiTrigger();
    //!Constructor
    GuiTrigger(u32 ch, u32 btns, bool clickEverywhere = false, bool holdEverywhere = false, bool selectionClickEverywhere = false);
    //!Destructor
    virtual ~GuiTrigger();
    //!Sets a simple trigger. Requires: element is selected, and trigger button is pressed
    void setTrigger(u32 ch, u32 btns);

    void setClickEverywhere(bool b) { bClickEverywhere = b; }
    void setHoldOnly(bool b) { bHoldEverywhere = b; }
    void setSelectionClickEverywhere(bool b) { bSelectionClickEverywhere = b; }

    bool isClickEverywhere() const { return bClickEverywhere; }
    bool isHoldEverywhere() const { return bHoldEverywhere; }
    bool isSelectionClickEverywhere() const { return bSelectionClickEverywhere; }

    bool left(const GuiController *controller) const;
    bool right(const GuiController *controller) const;
    bool up(const GuiController *controller) const;
    bool down(const GuiController *controller) const;
    bool clicked(const GuiController *controller) const;
    bool held(const GuiController *controller) const;
    bool released(const GuiController *controller) const;
private:
    u32 chan;
    u32 btns;
    bool bClickEverywhere;
    bool bHoldEverywhere;
    bool bSelectionClickEverywhere;
    bool bLastTouched;
};

#endif
