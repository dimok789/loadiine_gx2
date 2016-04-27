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
#ifndef GUI_TOGGLE_H_
#define GUI_TOGGLE_H_

#include "GuiButton.h"
#include "GuiFrame.h"

//!A simple CheckBox
class GuiToggle : public GuiButton, public sigslot::has_slots<>
{
	public:
		//!Constructor
		//!\param checked Checked
		GuiToggle(bool checked,f32 width,f32 height);
		//!Destructor
		virtual ~GuiToggle();
        void setValue(bool checked){
            if(selected != checked){
                selected = checked;
                bChanged=true;
                valueChanged(this,selected);
            }
        }
        void setChecked(){
            setValue(true);

        }
        void setUnchecked(){
            setValue(false);
        }
        bool getValue(){
            return selected;
        }
        sigslot::signal2<GuiToggle *, bool> valueChanged;
        void OnToggleClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
	protected:

       bool selected;
       bool bChanged;

       void update(GuiController * c);
};

#endif
