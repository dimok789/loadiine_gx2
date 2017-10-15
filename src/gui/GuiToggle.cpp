/****************************************************************************
 * Copyright (C) 2016 Maschell
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
#include "GuiToggle.h"
/**
 * Constructor for the GuiToggle class.
 */

GuiToggle::GuiToggle(bool checked,f32 width,f32 height)
 : GuiButton(width,height)
{
    bChanged = false;
    selected = checked;
    clicked.connect(this,&GuiToggle::OnToggleClick);
}

/**
 * Destructor for the GuiButton class.
 */
GuiToggle::~GuiToggle()
{
    bChanged = false;
    selected = false;
}

void GuiToggle::OnToggleClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger){
    if(!isStateSet(STATE_DISABLED | STATE_HIDDEN | STATE_DISABLE_INPUT)){
        if(selected){
            setUnchecked();
        }else{
            setChecked();
        }
    }
}

void GuiToggle::update(GuiController * c){
    GuiButton::update(c);
}

