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
#ifndef GUI_SWTICH_H_
#define GUI_SWTICH_H_

#include "GuiToggle.h"
#include "GuiImage.h"
#include "GuiImageData.h"

//!A simple switch
class GuiSwitch : public GuiToggle
{
	public:
		//!Constructor
		//!\param checked Checked
		GuiSwitch(bool checked,f32 switchscale = 1.0f);
		//!Destructor
		virtual ~GuiSwitch();

	protected:

       GuiImageData * switchbase_imgdata;
       GuiImage switchbase_img;

       GuiImageData * switchbase_highlighted_imgdata;
       GuiImage switchbase_highlighted_img;

       GuiImageData * switchOn_imgdata;
       GuiImage switchOn_img;

       GuiImageData * switchOff_imgdata;
       GuiImage switchOff_img;

       void draw(CVideo * v);
};

#endif
