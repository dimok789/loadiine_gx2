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
#ifndef GUICONFIGURATIONSCREEN_H_
#define GUICONFIGURATIONSCREEN_H_

#include "Gui.h"
#include "sigslot.h"

class GuiConfigurationScreen : public GuiFrame
{
public:
    GuiConfigurationScreen(s32 w, s32 h) : GuiFrame(w, h) {}
    virtual ~GuiConfigurationScreen() {}

    sigslot::signal2<GuiConfigurationScreen *, s32> gameLaunchClicked;
    sigslot::signal2<GuiConfigurationScreen *, s32> gameSelectionChanged;
};

#endif /* GUICONFIGURATIONSCREEN_H_ */
