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

class SettingsMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    SettingsMenu(int w, int h);
    virtual ~SettingsMenu();

    void update(GuiController *c);

    sigslot::signal1<GuiElement *> settingsCloseClicked;
private:
    void OnSmallIconClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnCategoryClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnCloseButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsCloseClicked(this);
    }

    void OnSubMenuBackClicked(GuiElement *element);
    void OnSubMenuOpenEffectFinish(GuiElement *element);
    void OnSubMenuBackEffectFinish(GuiElement *element);

    void setTargetPosition(int selectedIdx);

    GuiFrame categorySelectionFrame;
    GuiParticleImage particleBgImage;
    GuiText versionText;

    GuiSound *buttonClickSound;

    GuiImageData *closeImageData;
    GuiImageData *categoryImageData;
    GuiImageData *categoryBgImageData;


    GuiImage closeImage;

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

    GuiButton closeButton;

    GuiTrigger touchTrigger;

    GuiImageData *leftArrowImageData;
    GuiImageData *rightArrowImageData;
    GuiImage leftArrowImage;
    GuiImage rightArrowImage;
    GuiButton leftArrowButton;
    GuiButton rightArrowButton;

    int selectedCategory;
    int currentPosition;
    int targetPosition;
    int animationSpeed;
    bool bUpdatePositions;

};

#endif //_SETTINGS_WINDOW_H_
