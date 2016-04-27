/****************************************************************************
 * Copyright (C) 2016 Maschell
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
#include <vector>
#include <string>
#include "GuiSelectBox.h"
#include "GuiImage.h"
#include "GuiTrigger.h"
#include "GuiImageData.h"
#include "utils/StringTools.h"
/**
 * Constructor for the GuiCheckBox class.
 */

GuiSelectBox::GuiSelectBox(std::string caption,GuiFrame *parent)
 : GuiFrame(300,300,parent)
 ,selected(0)
 ,captionText(caption)
 ,topValueImageData(Resources::GetImageData("gameSettingsButton.png"))
 ,topValueImage(topValueImageData)
 ,topValueImageSelectedData(Resources::GetImageData("gameSettingsButtonSelected.png"))
 ,topValueImageSelected(topValueImageSelectedData)
 ,topValueButton(topValueImage.getWidth(),topValueImage.getHeight())
 ,valueImageData(Resources::GetImageData("gameSettingsButtonEx.png"))
 ,valueSelectedImageData(Resources::GetImageData("gameSettingsButtonExSelected.png"))
 ,valueHighlightedImageData(Resources::GetImageData("gameSettingsButtonExHighlighted.png"))
 ,touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
 ,wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
 ,buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
 ,buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
 ,buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
 ,buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
 ,DPADButtons(5,5)
 ,buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
 {
    showValues = false;
    bChanged = false;
    bSelectedChanged = false;
    opened = false;
    topValueText.setFontSize(32);
    topValueText.setAlignment(ALIGN_LEFT);
    topValueText.setPosition(10,-7);
    topValueButton.setLabel(&topValueText);
    topValueButton.setImage(&topValueImage);
    topValueButton.setIconOver(&topValueImageSelected);
    topValueButton.setTrigger(&touchTrigger);
    topValueButton.setTrigger(&wpadTouchTrigger);
    topValueButton.setSoundClick(buttonClickSound);
    topValueButton.clicked.connect(this, &GuiSelectBox::OnTopValueClicked);

    valuesFrame.setState(STATE_HIDDEN);

    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);
    DPADButtons.clicked.connect(this, &GuiSelectBox::OnDPADClick);
    DPADButtons.setState(STATE_DISABLE_INPUT);

    append(&DPADButtons);
    append(&valuesFrame);
    append(&topValueButton);

    showValues = false;
    bChanged = true;
}

void GuiSelectBox::OnValueClicked(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
  for(u32 i = 0; i < valueButtons.size(); ++i){
    if(valueButtons[i].valueButton == button){
        selected = i;
        SelectValue(i);
        break;
    }
  }
}

void GuiSelectBox::SelectValue(u32 value){
    if(value < valueButtons.size()){
        const wchar_t* w_text = valueButtons[value].valueButtonText->getText();
        std::wstring ws(w_text);
        std::string text(ws.begin(), ws.end());
        topValueText.setText(getCaptionWithValue(text).c_str());

        std::string real_value = buttonToValue[valueButtons[value].valueButton];
        if(real_value.compare(std::string()) == 0) real_value = "<error>";

        valueChanged(this,real_value);
        ShowHideValues(false);
    }
}
std::string GuiSelectBox::getCaptionWithValue(std::string value){
    u32 pad = (38 - captionText.size() -2);
    if(pad > value.size())
    	value.insert(0, pad - value.size(), ' ');
    return strfmt("%s: %s",captionText.c_str(),value.c_str());
}

void GuiSelectBox::OnTopValueClicked(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    ShowHideValues(!showValues);
}

void GuiSelectBox::ShowHideValues(bool showhide)
{
    showValues = showhide;
    bChanged = true;
}

void GuiSelectBox::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(opened == true){
        if(trigger == &buttonATrigger)
        {
            //! do not auto launch when wiimote is pointing to screen and presses A
            if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
            {
                return;
            }
            SelectValue(selected);
        }
        else if(trigger == &buttonBTrigger)
        {
            if(button == &DPADButtons){
                ShowHideValues(false);
            }else{
             }
        }else if(trigger == &buttonUpTrigger){
            if(selected > 0 ) selected--;
            bSelectedChanged = true;
        }
        else if(trigger == &buttonDownTrigger){
            selected++;
            if(selected >= valueButtons.size()) selected = valueButtons.size() - 1;
            bSelectedChanged = true;
        }
    }
}

void GuiSelectBox::Init(std::map<std::string,std::string> values, int valueID)
{
    if((u32)valueID >= values.size()){
        valueID = 0;
    }

    selected = valueID;
    bSelectedChanged = true;

    DeleteValueData();

    valueButtons.resize(values.size());

    int i = 0;
    f32 imgScale = 1.0f;
    std::map<std::string, std::string>::iterator itr;
    for(itr = values.begin(); itr != values.end(); itr++) {
        if(i == valueID){
            topValueText.setText(getCaptionWithValue(itr->first).c_str());
        }

        valueButtons[i].valueButtonImg = new GuiImage(valueImageData);

        valueButtons[i].valueButtonCheckedImg = new GuiImage(valueSelectedImageData);
        valueButtons[i].valueButtonHighlightedImg = new GuiImage(valueHighlightedImageData);
        valueButtons[i].valueButton = new GuiButton(valueButtons[i].valueButtonImg->getWidth() * imgScale, valueButtons[i].valueButtonImg->getHeight() * imgScale);
        valueButtons[i].valueButtonText = new GuiText(itr->first.c_str(),32,glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        valueButtons[i].valueButtonText->setMaxWidth(valueButtons[i].valueButtonImg->getWidth() * imgScale - 20.0f, GuiText::WRAP);
        valueButtons[i].valueButtonText->setPosition(0, 0);

        valueButtons[i].valueButtonImg->setScale(imgScale);
        valueButtons[i].valueButtonCheckedImg->setScale(imgScale);

        valueButtons[i].valueButton->setImage(valueButtons[i].valueButtonImg);
        valueButtons[i].valueButton->setIconOver(valueButtons[i].valueButtonHighlightedImg);
        valueButtons[i].valueButton->setTrigger(&touchTrigger);
        valueButtons[i].valueButton->setTrigger(&wpadTouchTrigger);
        valueButtons[i].valueButton->clicked.connect(this,&GuiSelectBox::OnValueClicked);
        valueButtons[i].valueButton->setSoundClick(buttonClickSound);
        valueButtons[i].valueButton->setLabel(valueButtons[i].valueButtonText);

        //valueButtons[i].valueButton->setState(STATE_HIDDEN); //Wont get disabled soon enough

        buttonToValue[valueButtons[i].valueButton] = itr->second;

        valueButtons[i].valueButton->setPosition(0, (((valueButtons[i].valueButtonImg->getHeight()*getScale()) * (i))+ (topValueImage.getHeight()-5)*getScale())*-1.0f);
        valuesFrame.append(valueButtons[i].valueButton);

        i++;
    }
    //Collapse the thing!
    showValues = false;
    bChanged = true;
}

void GuiSelectBox::DeleteValueData()
{
    for(u32 i = 0; i < valueButtons.size(); ++i)
    {
        valuesFrame.remove(valueButtons[i].valueButton);
        delete valueButtons[i].valueButtonImg;
        delete valueButtons[i].valueButtonCheckedImg;
        delete valueButtons[i].valueButtonHighlightedImg;
        delete valueButtons[i].valueButton;
        delete valueButtons[i].valueButtonText;
    }
    buttonToValue.clear();
    valueButtons.clear();
}

/**
 * Destructor for the GuiButton class.
 */
GuiSelectBox::~GuiSelectBox()
{
    DeleteValueData();
    bChanged = false;
    selected = 0;
    showValues = false;
    Resources::RemoveSound(buttonClickSound);
    Resources::RemoveImageData(topValueImageData);
    Resources::RemoveImageData(topValueImageSelectedData);
    Resources::RemoveImageData(valueImageData);
    Resources::RemoveImageData(valueHighlightedImageData);
    Resources::RemoveImageData(valueSelectedImageData);
}


void GuiSelectBox::setState(int s, int c)
{
	GuiElement::setState(s, c);
}

void GuiSelectBox::OnValueCloseEffectFinish(GuiElement *element)
{
    valuesFrame.effectFinished.disconnect(this);
}

f32 GuiSelectBox::getTopValueHeight() {
    return topValueImage.getHeight();
}

f32 GuiSelectBox::getTopValueWidth() {
    return topValueImage.getWidth();
}

void GuiSelectBox::OnValueOpenEffectFinish(GuiElement *element)
{
    valuesFrame.effectFinished.disconnect(this);
    opened = true;
}

void GuiSelectBox::update(GuiController * c){
    if(bChanged){
        showhide(this,showValues);
        if(showValues){
            for(u32 i = 0; i < valueButtons.size(); ++i){ //TODO: only set when it really changed
                if(i == selected){
                    valueButtons[i].valueButton->setImage(valueButtons[i].valueButtonCheckedImg);
                }else{
                     valueButtons[i].valueButton->setImage(valueButtons[i].valueButtonImg);
                }
            }
            valuesFrame.clearState(STATE_HIDDEN);
            DPADButtons.clearState(STATE_DISABLE_INPUT);
            valuesFrame.setEffect(EFFECT_FADE, 10, 255);
            valuesFrame.effectFinished.connect(this, &GuiSelectBox::OnValueCloseEffectFinish);
        }else{
            opened = false;
            valuesFrame.setState(STATE_HIDDEN);
            DPADButtons.setState(STATE_DISABLE_INPUT);
            valuesFrame.setEffect(EFFECT_FADE, -10, 0);
            valuesFrame.effectFinished.connect(this, &GuiSelectBox::OnValueOpenEffectFinish);
        }

        bChanged = false;
    }
    if(bSelectedChanged){
        for(u32 i = 0; i < valueButtons.size(); ++i){
            if(i == selected){
                 valueButtons[i].valueButton->setState(STATE_SELECTED);
            }else{
                 valueButtons[i].valueButton->clearState(STATE_SELECTED);
            }
        }
    }
    topValueButton.setState(getState());
    GuiFrame::update(c);
}
