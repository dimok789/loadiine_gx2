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
#ifndef _SETTINGS_WINDOW_H_
#define _SETTINGS_WINDOW_H_

#include "gui/Gui.h"
#include "gui/GuiParticleImage.h"
#include "settings/SettingsDefs.h"
#include "settings/SettingsEnums.h"
#include "common/common.h"

static const ValueString ValueGameSaveModes[] =
{
    { GAME_SAVES_SHARED, trNOOP("Shared Mode") },
    { GAME_SAVES_UNIQUE, trNOOP("Unique Mode") },
};

static const ValueString ValueLaunchMode[] =
{
    { LOADIINE_MODE_MII_MAKER, trNOOP("Mii Maker Mode") },
    { LOADIINE_MODE_SMASH_BROS, trNOOP("Smash Bros Mode") },
    { LOADIINE_MODE_KARAOKE, trNOOP("Karaoke Mode") },
    { LOADIINE_MODE_ART_ATELIER, trNOOP("Art Atelier Mode") }
};

class SettingsMenu : public GuiFrame, public sigslot::has_slots<>
{

public:
    SettingsMenu(int w, int h);
    virtual ~SettingsMenu();

    void update(GuiController *c);

    sigslot::signal1<GuiElement *> settingsQuitClicked;
private:
    void OnSmallIconClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnQuitButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsQuitClicked(this);
    }

    void OnSubMenuCloseClicked(GuiElement *element);
    void OnSubMenuOpenEffectFinish(GuiElement *element);
    void OnSubMenuCloseEffectFinish(GuiElement *element);

    void setTargetPosition(int selectedIdx);

    GuiFrame categorySelectionFrame;
    GuiParticleImage particleBgImage;
    GuiText versionText;

    GuiSound *buttonClickSound;

    GuiImageData *quitImageData;
    GuiImageData *categoryImageData;
    GuiImageData *categoryBgImageData;


    GuiImage quitImage;

    typedef struct
    {
        GuiImageData *categoryIconData;
        GuiImageData *categoryIconGlowData;
        GuiText *categoryLabel;
        GuiImage *categoryIcon;
        GuiImage *categoryIconGlow;
        GuiImage *categoryImages;
        GuiImage *categoryBgImage;
        GuiButton *categoryButton;
        std::vector<GuiText *> descriptions;
    } GuiSettingsCategory;

    std::vector<GuiSettingsCategory> settingsCategories;
    std::vector<GuiButton *> categoryButtons;

    std::vector<GuiButton *> categorySmallButtons;
    std::vector<GuiImage *> categorySmallImagesOver;
    std::vector<GuiImage *> categorySmallImages;

    GuiButton quitButton;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;
    GuiTrigger buttonLTrigger;
    GuiTrigger buttonRTrigger;
    GuiTrigger buttonLeftTrigger;
    GuiTrigger buttonRightTrigger;

    GuiImageData *leftArrowImageData;
    GuiImageData *rightArrowImageData;
    GuiImage leftArrowImage;
    GuiImage rightArrowImage;
    GuiButton leftArrowButton;
    GuiButton rightArrowButton;
    GuiButton DPADButtons;

    int selectedCategory;
    int currentPosition;
    int targetPosition;
    int animationSpeed;
    bool bUpdatePositions;
    bool moving;
};

#endif //_SETTINGS_WINDOW_H_
