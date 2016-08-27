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
#ifndef _PROGRESS_WINDOW_H_
#define _PROGRESS_WINDOW_H_

#include "gui/Gui.h"

class ProgressWindow : public GuiFrame, public sigslot::has_slots<>
{
public:
    ProgressWindow(const std::string & titleText);
    virtual ~ProgressWindow();

    void setProgress(f32 percent);
    void setInfo(const std::string & info);
private:

	GuiText infoText;
    GuiImageData *bgImageData;
    GuiImage bgImage;
	GuiImage bgBlur;
    GuiImage progressImageBlack;
    GuiImage progressImageColored;
    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
};

#endif //_PROGRESS_WINDOW_H_
