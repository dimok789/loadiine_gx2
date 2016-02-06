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
    , okButton(okImage.getWidth(), okImage.getHeight())
    , okText("O.K.", 46, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f))
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , buttonImageData(Resources::GetImageData("choiceUncheckedSquare.png"))
    , buttonCheckedImageData(Resources::GetImageData("choiceCheckedSquare.png"))
    , buttonCheckedImage(buttonCheckedImageData)
{
    selectedButton = selected;

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &ButtonChoiceMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
    append(&backButton);

    okText.setPosition(10, -10);
    okButton.setLabel(&okText);
    okButton.setImage(&okImage);
    okButton.setAlignment(ALIGN_BOTTOM | ALIGN_RIGHT);
    okButton.clicked.connect(this, &ButtonChoiceMenu::OnOkButtonClick);
    okButton.setTrigger(&touchTrigger);
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

    for(u32 i = 0; i < buttonNames.size(); i++)
    {
        GuiText *text = new GuiText(buttonNames[i].c_str(), 42, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
        GuiImage *image = new GuiImage(buttonImageData);
        GuiButton *button = new GuiButton(image->getWidth() * imgScale, image->getHeight() * imgScale);

        text->setMaxWidth(image->getWidth() * imgScale - 20.0f, GuiText::WRAP);
        text->setPosition(0, 10);
        image->setScale(imgScale);
        button->setLabel(text);
        button->setSoundClick(buttonClickSound);

        if(selectedButton == (int)i)
            button->setImage(&buttonCheckedImage);
        else
            button->setImage(image);

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

        button->setPosition(posX, -50.0f + posY);
        button->setEffectGrow();
        button->setTrigger(&touchTrigger);
        button->clicked.connect(this, &ButtonChoiceMenu::OnChoiceButtonClick);
        append(button);

        choiceButtonText.push_back(text);
        choiceButtonImg.push_back(image);
        choiceButton.push_back(button);
    }


}

ButtonChoiceMenu::~ButtonChoiceMenu()
{
    for(u32 i = 0; i < choiceButton.size(); ++i)
    {
        delete choiceButton[i];
        delete choiceButtonImg[i];
        delete choiceButtonText[i];
    }
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(okImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(buttonImageData);
    Resources::RemoveImageData(buttonCheckedImageData);
    Resources::RemoveSound(buttonClickSound);
}

void ButtonChoiceMenu::OnChoiceButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < choiceButton.size(); i++)
    {
        if(choiceButton[i] == button)
        {
            selectedButton = i;
            choiceButton[i]->setImage(&buttonCheckedImage);
        }
        else
            choiceButton[i]->setImage(choiceButtonImg[i]);
    }
}

