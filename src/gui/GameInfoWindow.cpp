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
#include "GameInfoWindow.h"
#include "common/common.h"
#include "game/GameList.h"
#include "utils/xml.h"

GameInfoWindow::GameInfoWindow(GuiGameBrowser* parent, const int idx)
    : GuiFrame(0, 0)
    , buttonClickSound(Resources::GetSound("button_click.mp3"))
    , backgroundImgData(Resources::GetImageData("launchMenuBox.png"))
    , backgroundImg(backgroundImgData)
    , buttonImgData(Resources::GetImageData("button.png"))
    , titleText((char*)NULL, 38, glm::vec4(1.0f))
    , idText((char*)NULL, 34, glm::vec4(1.0f))
    , gameInfoLabels((char*)NULL, 28, glm::vec4(1.0f))
    , gameInfoValues((char*)NULL, 28, glm::vec4(1.0f))
    , loadBtnLabel("Load", 34, glm::vec4(1.0f))
    , loadImg(buttonImgData)
    , loadBtn(loadImg.getWidth(), loadImg.getHeight())
    , backBtnLabel("Back", 34, glm::vec4(1.0f))
    , backImg(buttonImgData)
    , backBtn(backImg.getWidth(), backImg.getHeight())
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , parent(parent)
    , idx(idx)
{
    width = backgroundImg.getWidth();
    height = backgroundImg.getHeight();
    append(&backgroundImg);

    int xOffset = 500;
    int yOffset = height * 0.5f - 75.0f;

    titleText.setText(GameList::instance()->at(idx)->name.c_str());
    titleText.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    titleText.setPosition(0, yOffset);
    titleText.setMaxWidth(width - 100, GuiText::DOTTED);
    append(&titleText);

    yOffset -= 50;

    idText.setTextf(GameList::instance()->at(idx)->id.c_str());
    idText.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    idText.setPosition(width - xOffset, yOffset);
    append(&idText);
    
    ReducedCosAppXmlInfo cosAppXmlInfoStruct;
    LoadXmlParameters(&cosAppXmlInfoStruct, "", (GameList::instance()->at(idx)->gamepath + RPX_RPL_PATH).c_str());

    gameInfoLabels.setTextf("os_version:\n" 
                           "title_id:\n"
                           "app_type:\n"
                           "sdk_version:\n"
                           "title_version:\n"
                           );
    gameInfoValues.setTextf("%08X%08X\n" 
                             "%08X%08X\n"
                             "%08X\n"
                             "%i\n"
                             "v%i (%08X)\n"
                            , (unsigned int)(cosAppXmlInfoStruct.title_id >> 32), (unsigned int)(cosAppXmlInfoStruct.title_id & 0xFFFFFFFF)
                            , (unsigned int)(cosAppXmlInfoStruct.os_version >> 32), (unsigned int)(cosAppXmlInfoStruct.os_version & 0xFFFFFFFF)
                            , cosAppXmlInfoStruct.app_type
                            , cosAppXmlInfoStruct.sdk_version
                            , cosAppXmlInfoStruct.title_version, cosAppXmlInfoStruct.title_version
                            );

    gameInfoLabels.setAlignment(ALIGN_RIGHT | ALIGN_TOP);
    gameInfoLabels.setPosition(240 - width, -250);
    gameInfoLabels.setMaxWidth(200, GuiText::WRAP);
    append(&gameInfoLabels);
    gameInfoValues.setAlignment(ALIGN_LEFT | ALIGN_TOP);
    gameInfoValues.setPosition(250, -250);
    gameInfoValues.setMaxWidth(width - 200, GuiText::WRAP);
    append(&gameInfoValues);

    float scaleFactor = 1.0f;
    loadImg.setScale(scaleFactor);
    loadBtn.setSize(scaleFactor * loadImg.getWidth(), scaleFactor * loadImg.getHeight());
    loadBtn.setImage(&loadImg);
    loadBtn.setLabel(&loadBtnLabel);
    loadBtn.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    loadBtn.setPosition(-200, -310);
    loadBtn.setTrigger(&touchTrigger);
    loadBtn.setTrigger(&wpadTouchTrigger);
    loadBtn.setEffectGrow();
    loadBtn.setSoundClick(buttonClickSound);
    loadBtn.clicked.connect(this, &GameInfoWindow::OnLoadButtonClick);
    append(&loadBtn);

    backImg.setScale(scaleFactor);
    backBtn.setSize(scaleFactor * backImg.getWidth(), scaleFactor * backImg.getHeight());
    backBtn.setImage(&backImg);
    backBtn.setLabel(&backBtnLabel);
    backBtn.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
    backBtn.setPosition(200, -310);
    backBtn.setTrigger(&touchTrigger);
    backBtn.setTrigger(&wpadTouchTrigger);
    backBtn.setEffectGrow();
    backBtn.setSoundClick(buttonClickSound);
    backBtn.clicked.connect(this, &GameInfoWindow::OnBackButtonClick);
    append(&backBtn);
}

GameInfoWindow::~GameInfoWindow()
{
    Resources::RemoveSound(buttonClickSound);
    Resources::RemoveImageData(backgroundImgData);
    Resources::RemoveImageData(buttonImgData);
}

void GameInfoWindow::OnOpenEffectFinish(GuiElement *element)
{
    //! once the menu is open reset its state and allow it to be "clicked/hold"
    element->effectFinished.disconnect(this);
    element->clearState(GuiElement::STATE_DISABLED);
}

void GameInfoWindow::OnCloseEffectFinish(GuiElement *element)
{
    //! remove element from draw list and push to delete queue
    remove(element);
    AsyncDeleter::pushForDelete(element);

    backBtn.clearState(GuiElement::STATE_DISABLED);
    loadBtn.clearState(GuiElement::STATE_DISABLED);
}

void GameInfoWindow::OnLoadButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    backBtn.setState(GuiElement::STATE_DISABLED);
    loadBtn.setState(GuiElement::STATE_DISABLED);
    
    parent->gameLaunchClicked(parent, idx);
}
