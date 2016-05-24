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
#ifndef _SETTINGS_LANGUAGE_WINDOW_H_
#define _SETTINGS_LANGUAGE_WINDOW_H_

#include "gui/Gui.h"
#include "settings/SettingsDefs.h"
#include "gui/Scrollbar.h"

class SettingsLanguageMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    SettingsLanguageMenu(int w, int h, const std::string & title, const char *nameTitleImage);
    virtual ~SettingsLanguageMenu();
	
    sigslot::signal1<GuiElement *> settingsBackClicked;
private:
    void OnBackButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsBackClicked(this);
    }
	void OnlanguageButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
	void OnSubMenuCloseClicked(GuiElement *element);
    void OnSubMenuOpenEffectFinish(GuiElement *element);
    void OnSubMenuCloseEffectFinish(GuiElement *element);
	void OnOpenEffectFinish(GuiElement *element);
	
    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);	
	void UpdatelanguageButtons(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
	
	void OnScrollbarListChange(int selectItem, int pageIndex);
	
	GuiFrame languageFrame;
	
	Scrollbar scrollbar;
	
    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;   
    GuiButton backButton;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;
	
	GuiImageData *buttonImageData;
    GuiImageData *buttonCheckedImageData;
    GuiImageData *buttonHighlightedImageData;
	
    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;

    GuiTrigger buttonUpTrigger;
    GuiTrigger buttonDownTrigger;
    GuiTrigger buttonLeftTrigger;
    GuiTrigger buttonRightTrigger;

    GuiButton DPADButtons;
	
	int selectedButtonDPAD;
	int currentYOffset;
	int buttonCount;
	
    typedef struct
    {
        GuiImage *languageButtonImg;
        GuiImage *languageButtonCheckedImg;
        GuiImage *languageButtonHighlightedImg;
        GuiButton *languageButton;
        GuiText *languageButtonText;
    } languageButton;

    std::vector<languageButton> languageButtons;
	
	int selectedButton;	
};

#endif //_SETTINGS_LANGUAGE_WINDOW_H_
