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
#include "ButtonChoiceMenu.h"
#include "utils/StringTools.h"
#include "KeyPadMenu.h"

ButtonChoiceMenu::ButtonChoiceMenu(int w, int h, const std::string & title, const std::vector<std::string> & buttonNames, int selected)
    : GuiFrame(w, h)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("backButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , okImageData(Resources::GetImageData("emptyRoundButton.png"))
    , okImage(okImageData)
    , okSelectedImageData(Resources::GetImageData("emptyRoundButtonSelected.png"))
    , okSelectedImage(okSelectedImageData)
    , okButton(okImage.getWidth(), okImage.getHeight())
    , okText("O.K.", 46, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f))
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
    , buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT | GuiTrigger::STICK_L_LEFT, true)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT | GuiTrigger::STICK_L_RIGHT, true)
    , buttonImageData(Resources::GetImageData("choiceUncheckedSquare.png"))
    , buttonCheckedImageData(Resources::GetImageData("choiceCheckedSquare.png"))
    , buttonHighlightedImageData(Resources::GetImageData("choiceHighlightedSquare.png"))
    , DPADButtons(w,h)   
{
    selectedButton = selected;
    selectedButtonDPAD = -1;

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &ButtonChoiceMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setTrigger(&wpadTouchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
    append(&backButton);

    okText.setPosition(10, -10);
    okButton.setLabel(&okText);
    okButton.setImage(&okImage);
	okButton.setIconOver(&okSelectedImage);
    okButton.setAlignment(ALIGN_BOTTOM | ALIGN_RIGHT);
    okButton.clicked.connect(this, &ButtonChoiceMenu::OnOkButtonClick);
    okButton.setTrigger(&touchTrigger);
    okButton.setTrigger(&wpadTouchTrigger);
    okButton.setSoundClick(buttonClickSound);
    okButton.setEffectGrow();
    append(&okButton);

    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(title.c_str());
    append(&titleImage);
    append(&titleText);

    titleText.setParent(&titleImage);
    titleImage.setAlignment(ALIGN_MIDDLE | ALIGN_TOP);

    f32 posX;
    f32 posY;
    f32 imgScale = 1.0f;

    buttonCount = buttonNames.size();

    choiceButtons.resize(buttonCount);

    for(u32 i = 0; i < buttonNames.size(); i++)
    {      
        choiceButtons[i].choiceButtonImg = new GuiImage(buttonImageData);
        choiceButtons[i].choiceButtonCheckedImg = new GuiImage(buttonCheckedImageData);
        choiceButtons[i].choiceButtonHighlightedImg = new GuiImage(buttonHighlightedImageData);
        choiceButtons[i].choiceButton = new GuiButton(choiceButtons[i].choiceButtonImg->getWidth() * imgScale, choiceButtons[i].choiceButtonImg->getHeight() * imgScale);
        choiceButtons[i].choiceButtonText = new GuiText(buttonNames[i].c_str(), 42, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));


        choiceButtons[i].choiceButtonText->setMaxWidth(choiceButtons[i].choiceButtonImg->getWidth() * imgScale - 20.0f, GuiText::WRAP);
        choiceButtons[i].choiceButtonText->setPosition(0, 10);

        choiceButtons[i].choiceButtonImg->setScale(imgScale);
        choiceButtons[i].choiceButtonCheckedImg->setScale(imgScale);
		
		choiceButtons[i].choiceButton->setIconOver(choiceButtons[i].choiceButtonHighlightedImg);
		
        choiceButtons[i].choiceButton->setLabel(choiceButtons[i].choiceButtonText);
        choiceButtons[i].choiceButton->setSoundClick(buttonClickSound);

        if(selectedButton == (int)i)
            choiceButtons[i].choiceButton->setImage(choiceButtons[i].choiceButtonCheckedImg);
        else
            choiceButtons[i].choiceButton->setImage(choiceButtons[i].choiceButtonImg);

        if(!(i & 0x01))
        {
            //! put last one in the middle if it is alone
            if(i == (buttonNames.size() - 1))
                posX = 0;
            else
                posX = -(buttonImageData->getWidth() * imgScale * 0.5f + 50);
        }
        else
        {
            posX = (buttonImageData->getWidth() * imgScale * 0.5f + 50);
        }

        if(buttonNames.size() < 3)
        {
            posY = 0.0f;
        }
        else if(!(i & 0x02))
        {
            posY = (buttonImageData->getHeight() * imgScale * 0.5f + 20.0f);
        }
        else
        {
            posY = -(buttonImageData->getHeight() * imgScale * 0.5f + 20.0f);
        }

        choiceButtons[i].choiceButton->setPosition(posX, -50.0f + posY);
        choiceButtons[i].choiceButton->setEffectGrow();
        choiceButtons[i].choiceButton->setTrigger(&touchTrigger);
        choiceButtons[i].choiceButton->setTrigger(&wpadTouchTrigger);
        choiceButtons[i].choiceButton->clicked.connect(this, &ButtonChoiceMenu::OnChoiceButtonClick);
        append(choiceButtons[i].choiceButton);
    }

    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);
    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.clicked.connect(this, &ButtonChoiceMenu::OnDPADClick);
    append(&DPADButtons);

}

ButtonChoiceMenu::~ButtonChoiceMenu()
{
    for(u32 i = 0; i < choiceButtons.size(); ++i)
    {
        delete choiceButtons[i].choiceButtonImg;
        delete choiceButtons[i].choiceButtonCheckedImg;
        delete choiceButtons[i].choiceButtonHighlightedImg;
        delete choiceButtons[i].choiceButton;
        delete choiceButtons[i].choiceButtonText;
    }
   
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(okImageData);
    Resources::RemoveImageData(okSelectedImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(buttonImageData);
    Resources::RemoveImageData(buttonCheckedImageData);
    Resources::RemoveSound(buttonClickSound);
}

void ButtonChoiceMenu::UpdateChoiceButtons(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(int i = 0; i < buttonCount; i++)
    {
		if(i == selectedButtonDPAD){
			choiceButtons[i].choiceButton->setState(STATE_SELECTED);
		}else{
			choiceButtons[i].choiceButton->clearState(STATE_SELECTED);
		}
		
        if(i == selectedButton){   
			choiceButtons[i].choiceButton->setImage(choiceButtons[i].choiceButtonCheckedImg);
        }else{
			choiceButtons[i].choiceButton->setImage(choiceButtons[i].choiceButtonImg);
        }            
    }
  
    if(selectedButtonDPAD == buttonCount){ // OK
        okButton.setState(STATE_SELECTED);
    }else{
        okButton.clearState(STATE_SELECTED);
    }
}



void ButtonChoiceMenu::OnChoiceButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < choiceButtons.size(); i++)
    {
        if(choiceButtons[i].choiceButton == button)
        {
            selectedButton = i;
            selectedButtonDPAD = i;
            UpdateChoiceButtons(button,controller,trigger);
        }
    }
}

void ButtonChoiceMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
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
			OnOkButtonClick(button,controller,trigger);
		}            
	}
	else if(trigger == &buttonBTrigger)
	{
		OnBackButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonUpTrigger)
	{
		if(buttonCount > 2){
			selectedButtonDPAD -= 2;
			if(selectedButtonDPAD < 0){
				selectedButtonDPAD = 0;
			}
		}

	}
	else if(trigger == &buttonDownTrigger){
		if(buttonCount > 2 || selectedButtonDPAD != buttonCount){
			selectedButtonDPAD += 2;
			if(selectedButtonDPAD >= buttonCount){
				selectedButtonDPAD = buttonCount-1;
			}
		}
	}
	else if(trigger == &buttonRightTrigger){
		selectedButtonDPAD++;
		if(selectedButtonDPAD >= buttonCount){
			selectedButtonDPAD = buttonCount;
		}
	}
	else if(trigger == &buttonLeftTrigger){
		selectedButtonDPAD--;
		if(selectedButtonDPAD < 0){
			selectedButtonDPAD = 0;
		}
	}
	UpdateChoiceButtons(button,controller,trigger);
}

