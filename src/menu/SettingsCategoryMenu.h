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

#include "gui/gui.h"
#include "settings/SettingsDefs.h"

class SettingsCategoryMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    SettingsCategoryMenu(int w, int h, const std::string & titleText, const SettingType * categorySettings, int settingsCount);
    virtual ~SettingsCategoryMenu();

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
    void OnKeyPadOkClicked(GuiElement *element, const std::string & newValue);
    void OnButtonChoiceOkClicked(GuiElement *element, int selectedButton);

    const SettingType * categorySettings;
    const int categorySettingsCount;

    GuiFrame categoryFrame;
    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;
    GuiButton backButton;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;

    GuiImageData *settingImageData;

    typedef struct
    {
        GuiImage *settingImage;
        GuiButton *settingButton;
        GuiText *settingLabel;
    } CategorySetting;

    std::vector<CategorySetting> settings;

    GuiTrigger touchTrigger;
    int currentSettingsIdx;
};

#endif //_SETTINGS_WINDOW_H_
