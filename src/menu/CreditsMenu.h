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
#ifndef _CREDITS_MENU_H_
#define _CREDITS_MENU_H_

#include "gui/Gui.h"

class CreditsMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    CreditsMenu(int w, int h, const std::string & titleText);
    virtual ~CreditsMenu();

    sigslot::signal1<GuiElement *> settingsBackClicked;
private:
    void OnBackButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsBackClicked(this);
    }

    std::vector<GuiText *> creditsText;

    GuiSound *creditsMusic;
    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;
    GuiButton backButton;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonBTrigger;
};

#endif //_SETTINGS_WINDOW_H_
