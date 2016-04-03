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
#include "CreditsMenu.h"
#include "Application.h"

CreditsMenu::CreditsMenu(int w, int h, const std::string & title)
    : GuiFrame(w, h)
    , creditsMusic(Resources::GetSound("credits_music.ogg"))
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , backImageData(Resources::GetImageData("backButton.png"))
    , backImage(backImageData)
    , backButton(backImage.getWidth(), backImage.getHeight())
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
	, buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
{
    Application::instance()->getBgMusic()->Pause();

    creditsMusic->SetLoop(true);
    creditsMusic->Play();
    creditsMusic->SetVolume(50);

    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    titleText.setText(title.c_str());
    append(&titleImage);
    append(&titleText);

    titleText.setParent(&titleImage);
    titleImage.setAlignment(ALIGN_CENTER | ALIGN_TOP);

    backButton.setImage(&backImage);
    backButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    backButton.clicked.connect(this, &CreditsMenu::OnBackButtonClick);
    backButton.setTrigger(&touchTrigger);
    backButton.setTrigger(&wpadTouchTrigger);
	backButton.setTrigger(&buttonBTrigger);
    backButton.setSoundClick(buttonClickSound);
    backButton.setEffectGrow();
    append(&backButton);

    GuiText *text = NULL;

    f32 positionY = 230.0f;
    f32 positionX = 50.0f;
    f32 positionX2 = 350.0f;

    int fontSize = 40;
    glm::vec4 textColor = glm::vec4(1.0f);

    text = new GuiText(tr("Loadiine GX2"), 56, textColor);
    text->setPosition(0, positionY);
    text->setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 100;

    text = new GuiText(tr("Official Site:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText("https://gbatemp.net/threads/413823", fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Coding:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText("Dimok / dibas / Maschell / n1ghty", fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Design:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText(tr("Some guy who doesn't want to be named."), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Testing:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText(tr("Cyan / Maschell / n1ghty / OnionKnight and many more"), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Social Presence:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText("Cyan", fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Based on:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText(tr("Loadiine v4.0 by Golden45 and Dimok"), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Big thanks to:"), fontSize, textColor);
    text->setPosition(positionX, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);

    text = new GuiText(tr("lustar for GameTDB and hosting covers / disc images"), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("Marionumber1 for his kernel exploit"), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;

    text = new GuiText(tr("The whole libwiiu team and it's contributors."), fontSize, textColor);
    text->setPosition(positionX2, positionY);
    text->setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    creditsText.push_back(text);
    append(text);
    positionY -= 50;
}

CreditsMenu::~CreditsMenu()
{
    for(u32 i = 0; i < creditsText.size(); ++i)
    {
        delete creditsText[i];
    }
    Resources::RemoveImageData(backImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveSound(buttonClickSound);
    Resources::RemoveSound(creditsMusic);
    Application::instance()->getBgMusic()->Resume();
}
