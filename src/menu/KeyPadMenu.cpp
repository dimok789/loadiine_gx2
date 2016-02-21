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
#include "KeyPadMenu.h"
#include "video/CVideo.h"
#include "utils/StringTools.h"

#define MAX_COLS    3
#define MAX_ROWS    4
#define MAX_FIELDS  15

static const char cpKeyPadButtons[] =
{
    "123456789.0"
};

KeyPadMenu::KeyPadMenu(int w, int h, const std::string & strTitle, const std::string & prefil)
    : GuiFrame(w, h)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("keyPadBackButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , okImageData(Resources::GetImageData("keyPadOkButton.png"))
    , okImage(okImageData)
    , okButton(okImage.getWidth(), okImage.getHeight())
    , okText("O.K.", 46, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , keyPadBgImageData(Resources::GetImageData("keyPadBg.png"))
    , keyPadButtonImgData(Resources::GetImageData("keyPadButton.png"))
    , keyPadButtonClickImgData(Resources::GetImageData("keyPadButtonClicked.png"))
    , deleteButtonImgData(Resources::GetImageData("keyPadDeleteButton.png"))
    , deleteButtonClickImgData(Resources::GetImageData("keyPadDeleteClicked.png"))
    , fieldImageData(Resources::GetImageData("keyPadField.png"))
    , fieldBlinkerImageData(Resources::GetImageData("keyPadFieldBlinker.png"))
    , bgImage(keyPadBgImageData)
    , fieldBlinkerImg(fieldBlinkerImageData)
    , deleteButtonImg(deleteButtonImgData)
    , deleteButtonImgClick(deleteButtonClickImgData)
    , deleteButton(deleteButtonImgData->getWidth(), deleteButtonImgData->getHeight())
    , DPADButtons(w,h)
{
    lastFrameCount = 0;
    currentText = prefil;
    if(currentText.size() > MAX_FIELDS)
        currentText.resize(MAX_FIELDS);

    textPosition = currentText.size();

    bgImage.setAlignment(ALIGN_CENTER | ALIGN_BOTTOM);
    append(&bgImage);

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &KeyPadMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setTrigger(&wpadTouchTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
    append(&backButton);

    okText.setPosition(0, -10);
    okButton.setLabel(&okText);
    okButton.setImage(&okImage);
    okButton.setAlignment(ALIGN_BOTTOM | ALIGN_RIGHT);
    okButton.clicked.connect(this, &KeyPadMenu::OnOkButtonClick);
    okButton.setTrigger(&touchTrigger);
    okButton.setTrigger(&wpadTouchTrigger);
    okButton.setSoundClick(buttonClickSound);
    okButton.setEffectGrow();
    append(&okButton);

    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 230);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(strTitle.c_str());
    append(&titleText);

    deleteButton.setImage(&deleteButtonImg);
    deleteButton.setImageOver(&deleteButtonImgClick);
    deleteButton.setTrigger(&touchTrigger);
    deleteButton.setTrigger(&wpadTouchTrigger);
    deleteButton.setSoundClick(buttonClickSound);
    deleteButton.setPosition(-(keyPadButtonImgData->getWidth() + 5) * (MAX_COLS - 1) * 0.5f + (keyPadButtonImgData->getWidth() + 5) * MAX_COLS, -60);
    deleteButton.setEffectGrow();
    deleteButton.clicked.connect(this, &KeyPadMenu::OnDeleteButtonClick);
    append(&deleteButton);


    for(int i = 0; i < MAX_FIELDS; i++)
    {
        char fieldTxt[2];
        fieldTxt[0] = (i < (int)currentText.size()) ? currentText[i] : 0;
        fieldTxt[1] = 0;

        GuiText *text = new GuiText(fieldTxt, 46, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        GuiImage *image = new GuiImage(fieldImageData);
        GuiButton *button = new GuiButton(image->getWidth(), image->getHeight());
        button->setImage(image);
        button->setLabel(text);
        button->setPosition(-(image->getWidth() + 8) * (MAX_FIELDS - 1) * 0.5f + (image->getWidth() + 8) * i, 120);
        button->setTrigger(&touchTrigger);
        button->setTrigger(&wpadTouchTrigger);
        button->setSoundClick(buttonClickSound);
        button->clicked.connect(this, &KeyPadMenu::OnTextPositionChange);
        append(button);

        textFieldText.push_back(text);
        textFieldImg.push_back(image);
        textFieldBtn.push_back(button);
    }

    fieldBlinkerImg.setAlignment(ALIGN_LEFT | ALIGN_LEFT);
    fieldBlinkerImg.setPosition(5, 0);

    if(textPosition < MAX_FIELDS)
        textFieldBtn[textPosition]->setIcon(&fieldBlinkerImg);

    int row = 0, column = 0;

    for(int i = 0; cpKeyPadButtons[i]; i++)
    {
        char buttonTxt[2];
        buttonTxt[0] = cpKeyPadButtons[i];
        buttonTxt[1] = 0;


        GuiImage *image = new GuiImage(keyPadButtonImgData);
        GuiImage *imageClick = new GuiImage(keyPadButtonClickImgData);
        GuiButton *button = new GuiButton(image->getWidth(), image->getHeight());
        GuiText *text = new GuiText(buttonTxt, 46, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        text->setTextBlur(5.0f);


        button->setImage(image);
        button->setImageOver(imageClick);
        button->setLabel(text);
        button->setPosition(-(image->getWidth() + 5) * (MAX_COLS - 1) * 0.5f + (image->getWidth() + 5) * column, -60 - (image->getHeight() + 5) * row);
        button->setTrigger(&touchTrigger);
        button->setTrigger(&wpadTouchTrigger);
        button->setSoundClick(buttonClickSound);
        button->setEffectGrow();
        button->clicked.connect(this, &KeyPadMenu::OnKeyPadButtonClick);
        append(button);


        keyText.push_back(text);
        keyButton.push_back(button);
        keyImg.push_back(image);
        keyImgOver.push_back(imageClick);

        column++;
        if(column >= MAX_COLS)
        {
            column = 0;
            row++;
        }
    }

    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.clicked.connect(this, &KeyPadMenu::OnDPADClick);
    append(&DPADButtons);

    UpdateTextFields();
}

KeyPadMenu::~KeyPadMenu()
{
    for(u32 i = 0; i < textFieldImg.size(); ++i)
    {
        delete textFieldText[i];
        delete textFieldImg[i];
        delete textFieldBtn[i];
    }

    for(u32 i = 0; i < keyButton.size(); ++i)
    {
        delete keyButton[i];
        delete keyText[i];
        delete keyImg[i];
        delete keyImgOver[i];
    }
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(okImageData);
    Resources::RemoveImageData(keyPadBgImageData);
    Resources::RemoveImageData(keyPadButtonImgData);
    Resources::RemoveImageData(keyPadButtonClickImgData);
    Resources::RemoveImageData(deleteButtonImgData);
    Resources::RemoveImageData(deleteButtonClickImgData);
    Resources::RemoveImageData(fieldImageData);
    Resources::RemoveImageData(fieldBlinkerImageData);
    Resources::RemoveSound(buttonClickSound);
}

void KeyPadMenu::UpdateTextFields()
{
    for(u32 i = 0; i < textFieldText.size(); i++)
    {
        char text[2];
        text[0] = (i < currentText.size()) ? currentText[i] : 0;
        text[1] = 0;

        textFieldText[i]->setText(text);
    }
}

void KeyPadMenu::OnKeyPadButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < keyButton.size(); i++)
    {
        if(button == keyButton[i] && textPosition < MAX_FIELDS)
        {
            char text[2];
            text[0] = cpKeyPadButtons[i];
            text[1] = 0;

            currentText.insert(textPosition, text);
            UpdateTextFields();

            textFieldBtn[textPosition]->setIcon(NULL);
            textPosition++;
            if(textPosition < MAX_FIELDS)
                textFieldBtn[textPosition]->setIcon(&fieldBlinkerImg);
            break;
        }
    }
}

void KeyPadMenu::OnTextPositionChange(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < textFieldBtn.size(); i++)
    {
        if(textFieldBtn[i] == button)
        {
            if(textPosition < MAX_FIELDS)
                textFieldBtn[textPosition]->setIcon(NULL);

            textPosition = i;
            if(textPosition > (int)currentText.size())
                textPosition = currentText.size();

            textFieldBtn[textPosition]->setIcon(&fieldBlinkerImg);
        }
    }
}

void KeyPadMenu::OnDeleteButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(textPosition > 0)
    {
        if(textPosition < MAX_FIELDS)
            textFieldBtn[textPosition]->setIcon(NULL);

        textPosition--;
        currentText.erase(textPosition, 1);
        UpdateTextFields();

        textFieldBtn[textPosition]->setIcon(&fieldBlinkerImg);
    }
}

void KeyPadMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(trigger == &buttonATrigger)
	{
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }
		OnOkButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonBTrigger)
	{
		OnBackButtonClick(button,controller,trigger);
	}
}

void KeyPadMenu::draw(CVideo *video)
{
    if((video->getFrameCount() - lastFrameCount) >= 30)
    {
        lastFrameCount = video->getFrameCount();

        bool blinkerVisible = fieldBlinkerImg.isVisible();
        fieldBlinkerImg.setVisible(!blinkerVisible);
    }

    GuiFrame::draw(video);
}
