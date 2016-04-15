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
#ifndef _SETTINGS_CATEGORY_WINDOW_H_
#define _SETTINGS_CATEGORY_WINDOW_H_

#include "gui/Gui.h"
#include "settings/SettingsDefs.h"
#include "gui/Scrollbar.h"

class SettingsCategoryMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    SettingsCategoryMenu(int w, int h, const std::string & titleText, const SettingType * categorySettings, int settingsCount);
    virtual ~SettingsCategoryMenu();

     void update(GuiController *c);

    sigslot::signal1<GuiElement *> settingsBackClicked;
private:
    void OnSettingButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnBackButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsBackClicked(this);
    }

    void OnSubMenuCloseClicked(GuiElement *element);
    void OnSubMenuOpenEffectFinish(GuiElement *element);
    void OnSubMenuCloseEffectFinish(GuiElement *element);

    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);


    void OnKeyPadOkClicked(GuiElement *element, const std::string & newValue);
    void OnButtonChoiceOkClicked(GuiElement *element, int selectedButton);

    void OnScrollbarListChange(int selectItem, int pageIndex);

    const SettingType * categorySettings;
    const int categorySettingsCount;

    int currentYOffset;

    GuiFrame categoryFrame;

    Scrollbar scrollbar;

    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;
    GuiButton backButton;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;

    GuiImageData *settingImageData;
    GuiImageData *settingSelectedImageData;

    typedef struct
    {
        GuiImage *settingImage;
        GuiImage *settingImageSelected;
        GuiButton *settingButton;
        GuiText *settingLabel;
    } CategorySetting;

    std::vector<CategorySetting> settings;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;
    GuiTrigger buttonUpTrigger;
    GuiTrigger buttonDownTrigger;

    GuiButton DPADButtons;

    bool updateButtons = false;;

    int currentSettingsIdx;
	int selectedButtonDPAD;
};

#endif //_SETTINGS_WINDOW_H_
