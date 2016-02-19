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
#include "SettingsCategoryMenu.h"
#include "Application.h"
#include "settings/CSettings.h"
#include "utils/StringTools.h"
#include "KeyPadMenu.h"
#include "ButtonChoiceMenu.h"

SettingsCategoryMenu::SettingsCategoryMenu(int w, int h, const std::string & title, const SettingType * catSettings, int settingsCount)
    : GuiFrame(w, h)
    , categorySettings(catSettings)
    , categorySettingsCount(settingsCount)
    , categoryFrame(w, h)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("backButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , settingImageData(Resources::GetImageData("settingButton.png"))
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
{
    currentSettingsIdx = 0;

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &SettingsCategoryMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
    categoryFrame.append(&backButton);

    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(title.c_str());
    categoryFrame.append(&titleImage);
    categoryFrame.append(&titleText);

    titleText.setParent(&titleImage);
    titleImage.setAlignment(ALIGN_MIDDLE | ALIGN_TOP);

    settings.resize(categorySettingsCount);

    for(int i = 0; i < categorySettingsCount; i++)
    {
        settings[i].settingImage = new GuiImage(settingImageData);
        settings[i].settingButton = new GuiButton(settingImageData->getWidth(), settingImageData->getHeight());
        settings[i].settingLabel = new GuiText(categorySettings[i].name, 46, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

        settings[i].settingButton->setImage(settings[i].settingImage);
        settings[i].settingButton->setLabel(settings[i].settingLabel);
        settings[i].settingButton->setPosition(0, 150 - (settings[i].settingImage->getHeight() + 30) * i);
        settings[i].settingButton->setTrigger(&touchTrigger);
        settings[i].settingButton->setEffectGrow();
        settings[i].settingButton->setSoundClick(buttonClickSound);
        settings[i].settingButton->clicked.connect(this, &SettingsCategoryMenu::OnSettingButtonClick);
        categoryFrame.append(settings[i].settingButton);
    }

    append(&categoryFrame);
}

SettingsCategoryMenu::~SettingsCategoryMenu()
{
    for(u32 i = 0; i < settings.size(); ++i)
    {
        delete settings[i].settingImage;
        delete settings[i].settingButton;
        delete settings[i].settingLabel;
    }
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(settingImageData);
    Resources::RemoveSound(buttonClickSound);
}

void SettingsCategoryMenu::OnSettingButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < settings.size(); i++)
    {
        if(button == settings[i].settingButton)
        {
            GuiFrame *menu = NULL;
            currentSettingsIdx = i;

            switch(categorySettings[i].type)
            {
                default:
                    return;

                case TypeIP: {
                    if(CSettings::getDataType(categorySettings[i].index) != CSettings::TypeString)
                        return;

                    KeyPadMenu *keyMenu = new KeyPadMenu(width, height, categorySettings[i].name, CSettings::getValueAsString(categorySettings[i].index));
                    keyMenu->settingsBackClicked.connect(this, &SettingsCategoryMenu::OnSubMenuCloseClicked);
                    keyMenu->settingsOkClicked.connect(this, &SettingsCategoryMenu::OnKeyPadOkClicked);
                    menu = keyMenu;
                    break;
                }
                case Type2Buttons:
                case Type3Buttons:
                case Type4Buttons: {
                    int buttonCount = Type2Buttons + (categorySettings[i].type - Type2Buttons + 1);
                    int buttonSelected = 0;

                    switch(CSettings::getDataType(categorySettings[i].index))
                    {
                    default:
                        return;

                    case CSettings::TypeBool:
                        buttonSelected = CSettings::getValueAsBool(categorySettings[i].index);
                        break;
                    case CSettings::TypeS8:
                        buttonSelected = CSettings::getValueAsS8(categorySettings[i].index);
                        break;
                    case CSettings::TypeU8:
                        buttonSelected = CSettings::getValueAsU8(categorySettings[i].index);
                        break;
                    case CSettings::TypeS16:
                        buttonSelected = CSettings::getValueAsS16(categorySettings[i].index);
                        break;
                    case CSettings::TypeU16:
                        buttonSelected = CSettings::getValueAsU16(categorySettings[i].index);
                        break;
                    case CSettings::TypeS32:
                        buttonSelected = CSettings::getValueAsS32(categorySettings[i].index);
                        break;
                    case CSettings::TypeU32:
                        buttonSelected = CSettings::getValueAsU32(categorySettings[i].index);
                        break;
                    }

                    std::vector<std::string> buttonNames;
                    for(int n = 0; n < buttonCount; n++)
                        buttonNames.push_back(categorySettings[i].valueStrings[n].name);

                    ButtonChoiceMenu *buttonMenu = new ButtonChoiceMenu(width, height, categorySettings[i].name, buttonNames, buttonSelected);
                    buttonMenu->settingsBackClicked.connect(this, &SettingsCategoryMenu::OnSubMenuCloseClicked);
                    buttonMenu->settingsOkClicked.connect(this, &SettingsCategoryMenu::OnButtonChoiceOkClicked);
                    menu = buttonMenu;
                    break;
                }
            }

            menu->setEffect(EFFECT_FADE, 10, 255);
            menu->setState(STATE_DISABLED);
            menu->effectFinished.connect(this, &SettingsCategoryMenu::OnSubMenuOpenEffectFinish);
            append(menu);

            //! disable all current elements and fade them out with fading in new menu
            categoryFrame.setState(STATE_DISABLED);
            categoryFrame.setEffect(EFFECT_FADE, -10, 0);
            break;
        }
    }
}

void SettingsCategoryMenu::OnButtonChoiceOkClicked(GuiElement *element, int selectedButton)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &SettingsCategoryMenu::OnSubMenuCloseEffectFinish);

    if(selectedButton >= 0)
    {
        int value = categorySettings[currentSettingsIdx].valueStrings[selectedButton].value;

        switch(CSettings::getDataType(categorySettings[currentSettingsIdx].index))
        {
        default:
            return;

        case CSettings::TypeBool:
            CSettings::setValueAsBool(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeS8:
            CSettings::setValueAsS8(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeU8:
            CSettings::setValueAsU8(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeS16:
            CSettings::setValueAsS16(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeU16:
            CSettings::setValueAsU16(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeS32:
            CSettings::setValueAsS32(categorySettings[currentSettingsIdx].index, value);
            break;
        case CSettings::TypeU32:
            CSettings::setValueAsU32(categorySettings[currentSettingsIdx].index, value);
            break;
        }
    }

    //! fade in category selection
    categoryFrame.setEffect(EFFECT_FADE, 10, 255);
    append(&categoryFrame);
}

void SettingsCategoryMenu::OnKeyPadOkClicked(GuiElement *element, const std::string & newValue)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &SettingsCategoryMenu::OnSubMenuCloseEffectFinish);


    CSettings::setValueAsString(categorySettings[currentSettingsIdx].index, newValue);

    //! fade in category selection
    categoryFrame.setEffect(EFFECT_FADE, 10, 255);
    append(&categoryFrame);
}

void SettingsCategoryMenu::OnSubMenuCloseClicked(GuiElement *element)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &SettingsCategoryMenu::OnSubMenuCloseEffectFinish);

    //! fade in category selection
    categoryFrame.setEffect(EFFECT_FADE, 10, 255);
    append(&categoryFrame);
}

void SettingsCategoryMenu::OnSubMenuOpenEffectFinish(GuiElement *element)
{
    //! make element clickable again
    element->clearState(GuiElement::STATE_DISABLED);
    element->effectFinished.disconnect(this);
    //! remove category selection from settings
    remove(&categoryFrame);
}

void SettingsCategoryMenu::OnSubMenuCloseEffectFinish(GuiElement *element)
{
    remove(element);
    AsyncDeleter::pushForDelete(element);

    //! enable all elements again
    categoryFrame.clearState(GuiElement::STATE_DISABLED);
}
