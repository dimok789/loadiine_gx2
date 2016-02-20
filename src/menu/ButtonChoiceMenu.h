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
#ifndef _BUTTON_CHOICE_MENU_H_
#define _BUTTON_CHOICE_MENU_H_

#include "gui/Gui.h"
#include "settings/SettingsDefs.h"

class ButtonChoiceMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    ButtonChoiceMenu(int w, int h, const std::string & titleText, const std::vector<std::string> & buttonNames, int selected);
    virtual ~ButtonChoiceMenu();

    sigslot::signal2<GuiElement *, int> settingsOkClicked;
    sigslot::signal1<GuiElement *> settingsBackClicked;
private:
    void OnChoiceButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnBackButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsBackClicked(this);
    }

    void OnOkButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsOkClicked(this, selectedButton);
    }

    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void UpdateChoiceButtons(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;   
    GuiButton backButton;

    GuiImageData *okImageData;
    GuiImage okImage;
    GuiImageData *okSelectedImageData;
    GuiImage okSelectedImage;
    GuiButton okButton;
    GuiText okText;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;

    GuiTrigger buttonUpTrigger;
    GuiTrigger buttonDownTrigger;
    GuiTrigger buttonLeftTrigger;
    GuiTrigger buttonRightTrigger;

    GuiImageData *buttonImageData;
    GuiImageData *buttonCheckedImageData;
    GuiImageData *buttonHighlightedImageData;

    GuiButton DPADButtons;

    typedef struct
    {
        GuiImage *choiceButtonImg;
        GuiImage *choiceButtonCheckedImg;
        GuiImage *choiceButtonHighlightedImg;
        GuiButton *choiceButton;
        GuiText *choiceButtonText;
    } ChoiceButton;

    std::vector<ChoiceButton> choiceButtons;

    int buttonCount;
    int selectedButton;
    int selectedButtonDPAD;
   
};

#endif //_BUTTON_CHOICE_MENU_H_
