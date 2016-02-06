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
#include "GuiElement.h"
#include "GuiController.h"
#include "GuiTrigger.h"

/**
 * Constructor for the GuiTrigger class.
 */
GuiTrigger::GuiTrigger()
    : chan(CHANNEL_ALL)
    , btns(BUTTON_NONE)
    , bClickEverywhere(false)
    , bHoldEverywhere(false)
    , bSelectionClickEverywhere(false)
    , bLastTouched(false)
{
}

GuiTrigger::GuiTrigger(u32 ch, u32 btn, bool clickEverywhere, bool holdEverywhere, bool selectionClickEverywhere)
    : chan(ch)
    , btns(btn)
    , bClickEverywhere(clickEverywhere)
    , bHoldEverywhere(holdEverywhere)
    , bSelectionClickEverywhere(selectionClickEverywhere)
    , bLastTouched(false)
{
}

/**
 * Destructor for the GuiTrigger class.
 */
GuiTrigger::~GuiTrigger()
{
}

/**
 * Sets a simple trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed
 */
void GuiTrigger::setTrigger(u32 ch, u32 btn)
{
	chan = ch;
	btns = btn;
}

bool GuiTrigger::left(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }
	if((controller->vpad.btns_h | controller->vpad.btns_d) & VPAD_BUTTON_LEFT)
	{
	    return true;
	}
	return false;
}

bool GuiTrigger::right(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }
	if((controller->vpad.btns_h | controller->vpad.btns_d) & VPAD_BUTTON_RIGHT)
	{
	    return true;
	}
	return false;
}

bool GuiTrigger::up(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }
	if((controller->vpad.btns_h | controller->vpad.btns_d) & VPAD_BUTTON_UP)
	{
	    return true;
	}
	return false;
}

bool GuiTrigger::down(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }
	if((controller->vpad.btns_h | controller->vpad.btns_d) & VPAD_BUTTON_DOWN)
	{
	    return true;
	}
	return false;
}

bool GuiTrigger::clicked(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }

    bool bResult = false;

    if(controller->vpad.tpdata.touched && (btns & VPAD_TOUCH) && !controller->vpad.tpdata.invalid && !controller->vpadLast.tpdata.touched && !controller->vpad.tpdata.invalid)
    {
        bResult = true;
    }

	if(controller->vpad.btns_d & btns)
	{
	    bResult = true;
	}
	return bResult;
}

bool GuiTrigger::held(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }

    bool bResult = false;

    if(controller->vpad.tpdata.touched && (btns & VPAD_TOUCH) && !controller->vpad.tpdata.invalid && controller->vpadLast.tpdata.touched && !controller->vpadLast.tpdata.invalid)
    {
        bResult = true;
    }

	if(controller->vpad.btns_h & btns)
	{
	    bResult = true;
	}

	return bResult;
}

bool GuiTrigger::released(const GuiController *controller) const
{
    if((controller->chan & chan) == 0) {
        return false;
    }

    if(clicked(controller) || held(controller))
        return false;

    bool bResult = false;

    if(!controller->vpad.tpdata.touched && (btns & VPAD_TOUCH) && controller->vpadLast.tpdata.touched && !controller->vpadLast.tpdata.invalid)
    {
        bResult = true;
    }

	if(controller->vpad.btns_r & btns)
	{
	    bResult = true;
	}

	return bResult;
}

