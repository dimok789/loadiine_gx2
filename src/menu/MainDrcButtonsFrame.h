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
#ifndef _MAIN_DRC_BUTTONS_FRAME_H_
#define _MAIN_DRC_BUTTONS_FRAME_H_

#include "gui/gui.h"
#include "resources/Resources.h"

class MainDrcButtonsFrame : public GuiFrame, public sigslot::has_slots<>
{
public:
    MainDrcButtonsFrame(int w, int h)
        : GuiFrame(w, h)
        , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
        , screenSwitchSound(Resources::GetSound("screenSwitchSound.mp3"))
        , switchIconData(Resources::GetImageData("layoutSwitchButton.png"))
        , settingsIconData(Resources::GetImageData("settingsButton.png"))
        , switchIcon(switchIconData)
        , settingsIcon(settingsIconData)
        , switchLayoutButton(switchIcon.getWidth(), switchIcon.getHeight())
        , settingsButton(settingsIcon.getWidth(), settingsIcon.getHeight())
        , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    {
        settingsButton.setClickable(true);
        settingsButton.setImage(&settingsIcon);
        settingsButton.setTrigger(&touchTrigger);
        settingsButton.setAlignment(ALIGN_LEFT | ALIGN_BOTTOM);
        settingsButton.setSoundClick(buttonClickSound);
        settingsButton.setEffectGrow();
        settingsButton.clicked.connect(this, &MainDrcButtonsFrame::OnSettingsButtonClick);
        append(&settingsButton);

        switchLayoutButton.setClickable(true);
        switchLayoutButton.setImage(&switchIcon);
        switchLayoutButton.setTrigger(&touchTrigger);
        switchLayoutButton.setAlignment(ALIGN_RIGHT | ALIGN_BOTTOM);
        switchLayoutButton.setSoundClick(screenSwitchSound);
        switchLayoutButton.setEffectGrow();
        switchLayoutButton.clicked.connect(this, &MainDrcButtonsFrame::OnLayoutSwithClick);
        append(&switchLayoutButton);
    }
    virtual ~MainDrcButtonsFrame()
    {
        Resources::RemoveImageData(switchIconData);
        Resources::RemoveImageData(settingsIconData);
        Resources::RemoveSound(buttonClickSound);
        Resources::RemoveSound(screenSwitchSound);
    }

    sigslot::signal1<GuiElement *> settingsButtonClicked;
    sigslot::signal1<GuiElement *> layoutSwitchClicked;
private:
    void OnSettingsButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *) {
        settingsButtonClicked(this);
    }
    void OnLayoutSwithClick(GuiButton *button, const GuiController *controller, GuiTrigger *) {
        layoutSwitchClicked(this);
    }

    GuiSound *buttonClickSound;
    GuiSound *screenSwitchSound;
    GuiImageData *switchIconData;
    GuiImageData *settingsIconData;
    GuiImage switchIcon;
    GuiImage settingsIcon;

    GuiButton switchLayoutButton;
    GuiButton settingsButton;

    GuiTrigger touchTrigger;
};

#endif //_SETTINGS_WINDOW_H_
