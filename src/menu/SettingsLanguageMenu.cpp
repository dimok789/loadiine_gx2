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
#include "SettingsLanguageMenu.h"
#include "utils/StringTools.h"
#include "settings/CSettingsLanguages.h"
#include "language/gettext.h"
#include "settings/CSettings.h"
#include "settings/SettingsEnums.h"
#include "settings/SettingsDefs.h"

#define MAX_SETTINGS_PER_PAGE 3

SettingsLanguageMenu::SettingsLanguageMenu(int w, int h, const std::string & title, const char *nameTitleImage)
    : GuiFrame(w, h)
	, scrollbar(h - 150)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("backButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , titleImageData(Resources::GetImageData(nameTitleImage))
    , titleImage(titleImageData)
	, buttonImageData(Resources::GetImageData("choiceUncheckedRectangle.png"))
    , buttonCheckedImageData(Resources::GetImageData("choiceCheckedRectangle.png"))
    , buttonHighlightedImageData(Resources::GetImageData("settingSelectedButton.png"))
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
    , buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT | GuiTrigger::STICK_L_LEFT, true)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT | GuiTrigger::STICK_L_RIGHT, true)
    , DPADButtons(w,h)
    , selectedButtonDPAD(-1)	
{
	currentYOffset = 0;
	
    backButton.setImage(&backImage);
	//backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.setPosition(-560, -302);
    backButton.clicked.connect(this, &SettingsLanguageMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setTrigger(&wpadTouchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
	
	titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(title.c_str());
    //titleImage.setAlignment(ALIGN_MIDDLE | ALIGN_TOP);
    titleImage.setPosition(0, 310);
	
	std::vector<std::string> buttonLanguageNames;
	
	buttonLanguageNames= CSettingsLanguages::getInstance()->LoadLanguages(&selectedButton);
	
	buttonCount = buttonLanguageNames.size();
		
	languageButtons.resize(buttonCount);
	
    for(int i = 0; i < buttonCount; i++)
    {      
        languageButtons[i].languageButtonImg = new GuiImage(buttonImageData);
        languageButtons[i].languageButtonCheckedImg = new GuiImage(buttonCheckedImageData);
        languageButtons[i].languageButtonHighlightedImg = new GuiImage(buttonHighlightedImageData);
        languageButtons[i].languageButton = new GuiButton(languageButtons[i].languageButtonImg->getWidth(), languageButtons[i].languageButtonImg->getHeight());
        languageButtons[i].languageButtonText = new GuiText(tr(buttonLanguageNames[i].c_str()), 42, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));

		
		languageButtons[i].languageButton->setIconOver(languageButtons[i].languageButtonHighlightedImg);
        languageButtons[i].languageButton->setLabel(languageButtons[i].languageButtonText);
        languageButtons[i].languageButton->setSoundClick(buttonClickSound);

        if(selectedButton == i)
            languageButtons[i].languageButton->setImage(languageButtons[i].languageButtonCheckedImg);
        else
            languageButtons[i].languageButton->setImage(languageButtons[i].languageButtonImg);

        languageButtons[i].languageButton->setPosition(0, 150 - (languageButtons[i].languageButtonImg->getHeight() + 30) * i);
        languageButtons[i].languageButton->setEffectGrow();
        languageButtons[i].languageButton->setTrigger(&touchTrigger);
        languageButtons[i].languageButton->setTrigger(&wpadTouchTrigger);
        languageButtons[i].languageButton->clicked.connect(this, &SettingsLanguageMenu::OnlanguageButtonClick);
        languageFrame.append(languageButtons[i].languageButton);
    }

	if(buttonCount > MAX_SETTINGS_PER_PAGE)
    {
		scrollbar.SetPageSize((buttonImageData->getHeight() + 30) * (buttonCount - MAX_SETTINGS_PER_PAGE));
        scrollbar.SetEntrieCount((buttonImageData->getHeight() + 30) * (buttonCount - MAX_SETTINGS_PER_PAGE));
        scrollbar.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
        scrollbar.setPosition(500, -30);
        scrollbar.listChanged.connect(this, &SettingsLanguageMenu::OnScrollbarListChange);
        languageFrame.append(&scrollbar);
    }
	
	// append on top
	languageFrame.append(&backButton);
	languageFrame.append(&titleImage);
    languageFrame.append(&titleText);
	titleText.setParent(&titleImage);
	
    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);
    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.clicked.connect(this, &SettingsLanguageMenu::OnDPADClick);
    append(&DPADButtons);
	
	languageFrame.append(&DPADButtons);
	
	append(&languageFrame);

}

SettingsLanguageMenu::~SettingsLanguageMenu()
{
    for(u32 i = 0; i < languageButtons.size(); ++i)
    {
        delete languageButtons[i].languageButtonImg;
        delete languageButtons[i].languageButtonCheckedImg;
        delete languageButtons[i].languageButtonHighlightedImg;
        delete languageButtons[i].languageButton;
        delete languageButtons[i].languageButtonText;
    }
   
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(buttonImageData);
    Resources::RemoveImageData(buttonCheckedImageData);
    Resources::RemoveSound(buttonClickSound);
	
	CSettingsLanguages::destroyInstance();
}

void SettingsLanguageMenu::UpdatelanguageButtons(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	for(int i = 0; i < buttonCount; i++)
    {
		if(i == selectedButtonDPAD){
			languageButtons[i].languageButton->setState(STATE_SELECTED);
		}else{
			languageButtons[i].languageButton->clearState(STATE_SELECTED);
		}
		
        if(i == selectedButton){   
			languageButtons[i].languageButton->setImage(languageButtons[i].languageButtonCheckedImg);
        }else{
			languageButtons[i].languageButton->setImage(languageButtons[i].languageButtonImg);
        }            
    }
  
}

void SettingsLanguageMenu::OnlanguageButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	
	for(u32 i = 0; i < languageButtons.size(); i++)
    {
        if(languageButtons[i].languageButton == button)
        {
            selectedButton = i;
            selectedButtonDPAD = i;
			
            UpdatelanguageButtons(button,controller,trigger);
			CSettingsLanguages::getInstance()->SetLanguage(i);
        }
    }

}

void SettingsLanguageMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(trigger == &buttonATrigger)
	{
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }
		if(selectedButtonDPAD >= 0 && selectedButtonDPAD <= buttonCount-1)
		{
			selectedButton = selectedButtonDPAD;
			
		}
		else if(selectedButtonDPAD == buttonCount || selectedButtonDPAD == -2)
		{
			OnlanguageButtonClick(button,controller,trigger);
		}            
	}
	else if(trigger == &buttonBTrigger)
	{
		OnBackButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonUpTrigger){
		selectedButtonDPAD--;
		if(selectedButtonDPAD < 0){
			selectedButtonDPAD = 0;
		}
	}
	else if(trigger == &buttonDownTrigger){
		selectedButtonDPAD++;
		if(selectedButtonDPAD >= buttonCount){
			selectedButtonDPAD = buttonCount;
		}
	}
	
	scrollbar.SetSelectedItem((buttonImageData->getHeight() + 30) * selectedButtonDPAD);
	UpdatelanguageButtons(button,controller,trigger);
}

void SettingsLanguageMenu::OnScrollbarListChange(int selectItem, int pageIndex)
{
    currentYOffset = selectItem + pageIndex;

    for(int i = 0; i < buttonCount; i++)
    {
        languageButtons[i].languageButton->setPosition(0, 150 - (languageButtons[i].languageButtonImg->getHeight() + 30) * i + currentYOffset);
    }
}

void SettingsLanguageMenu::OnSubMenuCloseClicked(GuiElement *element)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &SettingsLanguageMenu::OnSubMenuCloseEffectFinish);

    //! fade in category selection
    languageFrame.setEffect(EFFECT_FADE, 10, 255);
	
    remove(&languageFrame);
}

void SettingsLanguageMenu::OnSubMenuOpenEffectFinish(GuiElement *element)
{
    //! make element clickable again
    element->clearState(GuiElement::STATE_DISABLED);
    element->effectFinished.disconnect(this);
    //! remove category selection from settings
    remove(&languageFrame);

}

void SettingsLanguageMenu::OnSubMenuCloseEffectFinish(GuiElement *element)
{
    remove(element);
    AsyncDeleter::pushForDelete(element);

    //! enable all elements again
    languageFrame.clearState(GuiElement::STATE_DISABLED);
}
