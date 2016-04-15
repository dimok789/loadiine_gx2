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
#include "language/gettext.h"

#define MAX_SETTINGS_PER_PAGE 3

SettingsCategoryMenu::SettingsCategoryMenu(int w, int h, const std::string & title, const SettingType * catSettings, int settingsCount)
    : GuiFrame(w, h)
    , categorySettings(catSettings)
    , categorySettingsCount(settingsCount)
    , categoryFrame(w, h)
    , scrollbar(h - 150)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("backButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , settingImageData(Resources::GetImageData("settingButton.png"))
    , settingSelectedImageData(Resources::GetImageData("settingSelectedButton.png"))
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
    , buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
    , DPADButtons(w,h)
    , updateButtons(false)
    , selectedButtonDPAD(-1)
{
    currentSettingsIdx = 0;
    currentYOffset = 0;

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &SettingsCategoryMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setTrigger(&wpadTouchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();

    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(title.c_str());

    titleImage.setAlignment(ALIGN_MIDDLE | ALIGN_TOP);

    settings.resize(categorySettingsCount);

    for(int i = 0; i < categorySettingsCount; i++)
    {
        settings[i].settingImage = new GuiImage(settingImageData);
        settings[i].settingImageSelected = new GuiImage(settingSelectedImageData);
        settings[i].settingButton = new GuiButton(settingImageData->getWidth(), settingImageData->getHeight());
        settings[i].settingLabel = new GuiText(tr(categorySettings[i].name), 46, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

        settings[i].settingButton->setIconOver(settings[i].settingImageSelected);
        settings[i].settingButton->setImage(settings[i].settingImage);
        settings[i].settingButton->setLabel(settings[i].settingLabel);

        settings[i].settingButton->setPosition(0, 150 - (settings[i].settingImage->getHeight() + 30) * i);
        settings[i].settingButton->setTrigger(&touchTrigger);
        settings[i].settingButton->setTrigger(&wpadTouchTrigger);
        settings[i].settingButton->setEffectGrow();
        settings[i].settingButton->setSoundClick(buttonClickSound);
        settings[i].settingButton->clicked.connect(this, &SettingsCategoryMenu::OnSettingButtonClick);
        categoryFrame.append(settings[i].settingButton);
    }

    if(categorySettingsCount > MAX_SETTINGS_PER_PAGE)
    {
        scrollbar.SetPageSize((settingImageData->getHeight() + 30) * (categorySettingsCount - MAX_SETTINGS_PER_PAGE));
        scrollbar.SetEntrieCount((settingImageData->getHeight() + 30) * (categorySettingsCount - MAX_SETTINGS_PER_PAGE));
        scrollbar.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
        scrollbar.setPosition(500, -30);
        scrollbar.listChanged.connect(this, &SettingsCategoryMenu::OnScrollbarListChange);
        categoryFrame.append(&scrollbar);
    }

    // append on top
    categoryFrame.append(&backButton);
    categoryFrame.append(&titleImage);
    categoryFrame.append(&titleText);
    titleText.setParent(&titleImage);

    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);
    DPADButtons.clicked.connect(this, &SettingsCategoryMenu::OnDPADClick);
    append(&DPADButtons);

    categoryFrame.append(&DPADButtons);

    append(&categoryFrame);
}

SettingsCategoryMenu::~SettingsCategoryMenu()
{
    for(u32 i = 0; i < settings.size(); ++i)
    {
        delete settings[i].settingImage;
        delete settings[i].settingImageSelected;
        delete settings[i].settingButton;
        delete settings[i].settingLabel;
    }
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(settingImageData);
    Resources::RemoveImageData(settingSelectedImageData);
    Resources::RemoveSound(buttonClickSound);
}

void SettingsCategoryMenu::OnSettingButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < settings.size(); i++) // Touch overrides selection
    {
        if(button == settings[i].settingButton)
        {
            currentSettingsIdx = i;
            break;
        }
    }
    u32 i = currentSettingsIdx;

    GuiFrame *menu = NULL;
    switch(categorySettings[i].type)
    {
        default:
            return;

        case TypeIP: {
            if(CSettings::getDataType(categorySettings[i].index) != CSettings::TypeString)
                return;

            KeyPadMenu *keyMenu = new KeyPadMenu(width, height, tr(categorySettings[i].name), CSettings::getValueAsString(categorySettings[i].index));
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
                buttonNames.push_back(tr(categorySettings[i].valueStrings[n].name));

            ButtonChoiceMenu *buttonMenu = new ButtonChoiceMenu(width, height, tr(categorySettings[i].name), buttonNames, buttonSelected);
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

void SettingsCategoryMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(trigger == &buttonATrigger)
    {
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }
        OnSettingButtonClick(button,controller,trigger);
    }
    else if(trigger == &buttonBTrigger)
    {
        OnBackButtonClick(button,controller,trigger);
    }
    else if(trigger == &buttonUpTrigger || trigger == &buttonDownTrigger)
    {
        if(selectedButtonDPAD == -1)
        {
            selectedButtonDPAD = currentSettingsIdx;
        }
        else
        {
            if(trigger == &buttonUpTrigger)
            {
                if(currentSettingsIdx > 0){
                    currentSettingsIdx--;
                }
            }
            else if(trigger == &buttonDownTrigger){
                if(currentSettingsIdx < categorySettingsCount-1){
                    currentSettingsIdx++;
                }
            }
            selectedButtonDPAD = currentSettingsIdx;
        }

        scrollbar.SetSelectedItem((settingImageData->getHeight() + 30) * selectedButtonDPAD);
        updateButtons = true;
    }
}

void SettingsCategoryMenu::OnScrollbarListChange(int selectItem, int pageIndex)
{
    currentYOffset = selectItem + pageIndex;

    for(int i = 0; i < categorySettingsCount; i++)
    {
        settings[i].settingButton->setPosition(0, 150 - (settings[i].settingImage->getHeight() + 30) * i + currentYOffset);
    }
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

void SettingsCategoryMenu::update(GuiController *c)
{
    if(updateButtons)
    {
        for(int i = 0; i < categorySettingsCount; i++)
        {
            if(i == selectedButtonDPAD) {
                settings[i].settingButton->setState(STATE_SELECTED);
            }
            else {
                settings[i].settingButton->clearState(STATE_SELECTED);
            }
        }
        updateButtons = false;
    }

    GuiFrame::update(c);
}
