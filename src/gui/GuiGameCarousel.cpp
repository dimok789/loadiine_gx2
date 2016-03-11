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
#include <map>
#include <string.h>
#include <math.h>
#include <sstream>
#include <unistd.h>
#include "settings/CSettings.h"
#include "GuiGameCarousel.h"
#include "GuiImageAsync.h"
#include "common/common.h"
#include "game/GameList.h"
#include "resources/Resources.h"
#include "utils/utils.h"

#define MAX_PAGE_SIZE   11

#define DEG_OFFSET      8.0f
#define RADIUS	        1000.0f
#define IN_SPEED	    175
#define SHIFT_SPEED     75
#define SPEED_STEP      4
#define SPEED_LIMIT     250

/**
 * Constructor for the GuiGameCarousel class.
 */
GuiGameCarousel::GuiGameCarousel(int w, int h, int GameIndex)
    : GuiGameBrowser(w, h, GameIndex)
    , buttonClickSound(Resources::GetSound("button_click.mp3"))
    , noCover(Resources::GetFile("noCover.png"), Resources::GetFileSize("noCover.png"))
    , gameTitle((char*)NULL, 52, glm::vec4(1.0f))
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonLTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_L, true)
    , buttonRTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_R, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT | GuiTrigger::STICK_L_LEFT, true)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT | GuiTrigger::STICK_L_RIGHT, true)
    , touchButton(w, h)
    , DPADButtons(w, h)
    , particleBgImage(w, h, 100)
    , bgUsedImageDataAsync(NULL)
    , bgNewImageDataAsync(NULL)
    , bgFadingImageDataAsync(NULL)
{
    pagesize = GameList::instance()->size();//(GameList::instance()->size() < MAX_PAGE_SIZE) ? GameList::instance()->size() : MAX_PAGE_SIZE;
    listOffset = 0;
    refreshDrawMap = true;
    selectable = true;
    selectedGame = GameIndex;
    selectedGameOnDragStart = 0;
    bWasDragging = false;


    lastTouchDifference = 0;
    gameLaunchTimer = 0;
    touchClickDelay = 0;
    circleRotationSpeed = 0.0f;
    circleSpeedLimit = 1.8f;
    startRotationDistance = 0.0f;
    destDegree = 90.0f + selectedGame * DEG_OFFSET;
    currDegree = destDegree;
    speed = 0;

    this->append(&particleBgImage);

    game.resize(pagesize);
    drawOrder.resize(pagesize);
    coverImg.resize(pagesize);

    touchButton.setAlignment(ALIGN_LEFT | ALIGN_TOP);
    touchButton.setTrigger(&touchTrigger);
    touchButton.setTrigger(&wpadTouchTrigger);
    touchButton.setClickable(true);
    touchButton.setHoldable(true);
    touchButton.clicked.connect(this, &GuiGameCarousel::OnTouchClick);
    touchButton.held.connect(this, &GuiGameCarousel::OnTouchHold);
    touchButton.released.connect(this, &GuiGameCarousel::OnTouchRelease);
    this->append(&touchButton);

    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonLTrigger);
    DPADButtons.setTrigger(&buttonRTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.clicked.connect(this, &GuiGameCarousel::OnDPADClick);
    append(&DPADButtons);

    gameTitle.setPosition(0, -320);
    gameTitle.setBlurGlowColor(5.0f, glm::vec4(0.109804, 0.6549, 1.0f, 1.0f));
    gameTitle.setMaxWidth(900, GuiText::DOTTED);
    append(&gameTitle);

    refresh();
}

/**
 * Destructor for the GuiGameCarousel class.
 */
GuiGameCarousel::~GuiGameCarousel()
{
    AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
    AsyncDeleter::pushForDelete(bgUsedImageDataAsync);
    AsyncDeleter::pushForDelete(bgNewImageDataAsync);

    for (u32 i = 0; i < game.size(); ++i)
        delete coverImg[i];
    for (u32 i = 0; i < game.size(); ++i)
        delete game[i];

    Resources::RemoveSound(buttonClickSound);
}

void GuiGameCarousel::setSelectedGame(int idx)
{
    if(pagesize == 0)
        return;

    if(idx < 0)
        idx = 0;
    if(idx >= pagesize)
        idx = pagesize-1;

    selectedGame = idx;
    loadBgImage(selectedGame);

    //! normalize to 360°
    currDegree = ((int)(currDegree + 0.5f)) % 360;
    if(currDegree < 0.0f)
        currDegree += 360.0f;

    destDegree = 90.0f + selectedGame * DEG_OFFSET;
//
//    f32 angleDiff = (circleTargetPosition - currDegree);
//    if(angleDiff > 180.0f)
//        circleTargetPosition -= 360.0f;
//    else if(angleDiff < -180.0f)
//        circleTargetPosition += 360.0f;

    touchClickDelay = 20;
    circleSpeedLimit = 1.8f;
    refreshDrawMap = true;
}

int GuiGameCarousel::getSelectedGame()
{
    return selectedGame;
}

void GuiGameCarousel::refresh()
{
    for (int i = 0; i < pagesize; i++)
    {
        //------------------------
        // Index
        //------------------------
        //gameIndex[i] = GetGameIndex( i, listOffset, GameList::instance()->size() );

        //------------------------
        // Image
        //------------------------
        delete coverImg[i];

        std::string filepath = CSettings::getValueAsString(CSettings::GameCover3DPath) + "/" + GameList::instance()->at(i)->id + ".png";

        coverImg[i] = new GuiImageAsync(filepath, &noCover);

        //------------------------
        // GameButton
        //------------------------
        delete game[i];
        game[i] = new GuiButton(coverImg[i]->getWidth(), coverImg[i]->getHeight());
        game[i]->setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
        game[i]->setImage(coverImg[i]);
        game[i]->setParent(this);
        game[i]->setTrigger(&touchTrigger);
        game[i]->setTrigger(&wpadTouchTrigger);
        game[i]->setSoundClick(buttonClickSound);
        game[i]->setEffectGrow();
        game[i]->clicked.connect(this, &GuiGameCarousel::OnGameButtonClick);

        drawOrder[i] = i;
    }

//    currDegree = 270.0f + pagesize * 0.5f * DEG_OFFSET - 0.5f * DEG_OFFSET;
//    destDegree = 90.0f + pagesize * 0.5f * DEG_OFFSET - 0.5f * DEG_OFFSET;
//    selectedGame = (int)((pagesize * 0.5f) + 0.5f); // round to nearest

    loadBgImage(selectedGame);
    setSelectedGame(selectedGame);
}

void GuiGameCarousel::OnGameButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < game.size(); i++)
    {
        if(button == game[i])
        {
            if(selectedGame ==  (int)i)
            {
                if(gameLaunchTimer < 30)
                    OnLaunchClick(button, controller, trigger);

                gameLaunchTimer = 0;
            }

            setSelectedGame(i);
            gameSelectionChanged(this, selectedGame);
            break;
        }
    }
}

void GuiGameCarousel::OnTouchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->data.validPointer)
        return;

    bWasDragging = false;
    selectedGameOnDragStart = getSelectedGame();
    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;
}

void GuiGameCarousel::OnTouchHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->data.validPointer)
        return;

    lastTouchDifference = (controller->data.x - lastPosition.x);
    f32 degreeAdd = lastTouchDifference * 0.096f;
    if(touchClickDelay || fabs(degreeAdd) < 0.5f) {
        return;
    }

    currDegree -= degreeAdd;
    destDegree = currDegree;

    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;
    refreshDrawMap = true;
    bWasDragging = true;
}

void GuiGameCarousel::OnTouchRelease(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->lastData.validPointer)
        return;

    f32 degreeAdd = lastTouchDifference * 0.128f;
    if(touchClickDelay || fabsf(degreeAdd) < 2.0f)
    {
        updateDrawMap();

        if(bWasDragging && selectedGameOnDragStart != selectedGame)
        {
            setSelectedGame(selectedGame);
            gameSelectionChanged(this, selectedGame);
        }
        return;
    }

    currDegree = ((int)(currDegree + 0.5f)) % 360;
    if(currDegree < 0.0f)
        currDegree += 360.0f;

    f32 partDegree = DEG_OFFSET;

    destDegree = currDegree - degreeAdd * partDegree;

    //! round to nearest game position at the target position
    destDegree = ((int)(destDegree / partDegree + 0.5f)) * partDegree;
    circleSpeedLimit = 10.0f;


    int iMin = 0;
    int iDegreeMin = destDegree;

    for(int i = 0; i < pagesize; i++)
    {
        f32 setDegree = (destDegree - DEG_OFFSET * i);
        int degree = labs(((int)setDegree - 90) % 360);
        if(degree < iDegreeMin)
        {
            iDegreeMin = degree;
            iMin = i;
        }
    }

    selectedGame = iMin;
    gameSelectionChanged(this, selectedGame);

    loadBgImage(selectedGame);
    refreshDrawMap = true;
}

void GuiGameCarousel::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(trigger == &buttonATrigger)
    {
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }

        OnLaunchClick(button,controller,trigger);
    }
    else if(trigger == &buttonLTrigger){
        OnLeftSkipClick(button,controller,trigger);
    }
    else if(trigger == &buttonRTrigger){
        OnRightSkipClick(button,controller,trigger);
    }
    else if(trigger == &buttonLeftTrigger){
        OnLeftClick(button,controller,trigger);
    }
    else if(trigger == &buttonRightTrigger){
        OnRightClick(button,controller,trigger);
    }
}

void GuiGameCarousel::OnLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    int sel = getSelectedGame() - 1;
    if(sel < 0 && (GameList::instance()->size() > 1))
        sel = GameList::instance()->size() - 1;

    setSelectedGame(sel);
    gameSelectionChanged(this, sel);
}

void GuiGameCarousel::OnRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    int sel = getSelectedGame() + 1;
    if(sel >= (int)GameList::instance()->size())
        sel = 0;

    setSelectedGame(sel);
    gameSelectionChanged(this, sel);
}

void GuiGameCarousel::OnRightSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if((int)GameList::instance()->size() > 5){
        int sel = getSelectedGame() + 5;
        if(sel >= (int)GameList::instance()->size())
            sel -= (int)GameList::instance()->size();

        setSelectedGame(sel);
        gameSelectionChanged(this, sel);
    }
}

void GuiGameCarousel::OnLeftSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if((int)GameList::instance()->size() > 5){
        int sel = getSelectedGame() - 5;
        if(sel < 0 && (GameList::instance()->size() > 5))
            sel += GameList::instance()->size();

        setSelectedGame(sel);
        gameSelectionChanged(this, sel);
    }
}

void GuiGameCarousel::loadBgImage(int idx)
{
    if(idx < 0 || idx >= GameList::instance()->size())
        return;

    std::string filepath = GameList::instance()->at(idx)->gamepath + META_PATH + "/bootTvTex.tga";

    //! remove image that is possibly still loading
    //! TODO: fix (state != STATE_DISABLED) its a cheap trick to make the thread not create new images when fading out because it causes issues
    if(bgNewImageDataAsync && !isStateSet(STATE_DISABLED))
    {
        GuiImageAsync::removeFromQueue(bgNewImageDataAsync);
        AsyncDeleter::pushForDelete(bgNewImageDataAsync);
        bgNewImageDataAsync = new GuiImageAsync(filepath, NULL);
    }
    else
    {
        delete bgNewImageDataAsync;
        bgNewImageDataAsync = new GuiImageAsync(filepath, NULL);
    }
}

void GuiGameCarousel::OnBgEffectFinished(GuiElement *element)
{
    if(element == bgFadingImageDataAsync)
    {
        remove(bgFadingImageDataAsync);
        AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
        bgFadingImageDataAsync = NULL;
    }
}

void GuiGameCarousel::updateDrawMap(void)
{
    const int carousel_x = 0;
    const int carousel_y = -RADIUS + 80;

    std::multimap<float, int> drawMap;
    std::multimap<float, int>::iterator itr;

    for(int i = 0; i < pagesize; i++)
    {
        float setDegree = (currDegree - DEG_OFFSET * i);
        float rotationAngle = 90.0f - setDegree;
        float posX = (RADIUS * cosf(DegToRad(setDegree)) + 0.5f);
        float posY = (RADIUS * sinf(DegToRad(setDegree)) + 0.5f);
        game[i]->setPosition(carousel_x + posX, carousel_y + posY);
        game[i]->setAngle(rotationAngle);

        drawMap.insert(std::pair<float, int>(posY, i));
    }

    int n = 0;
    for(itr = drawMap.begin(); itr != drawMap.end(); itr++)
    {
        drawOrder[n++] = itr->second;
        coverImg[itr->second]->setColorIntensity(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
    }

    if(drawOrder.size())
    {
        int lastIdx = drawOrder[ drawOrder.size() - 1 ];
        gameTitle.setText(GameList::instance()->at(lastIdx)->name.c_str());
        coverImg[lastIdx]->setColorIntensity(glm::vec4(1.0f));
        selectedGame = lastIdx;
    }
}

/**
 * Draw the button on screen
 */
void GuiGameCarousel::draw(CVideo *v)
{
    if(bgNewImageDataAsync && bgNewImageDataAsync->getImageData() && !bgFadingImageDataAsync)
    {
        if(bgUsedImageDataAsync)
        {
            bgFadingImageDataAsync = bgUsedImageDataAsync;
            bgFadingImageDataAsync->setEffect(EFFECT_FADE, -10, 0);
            bgFadingImageDataAsync->effectFinished.connect(this, &GuiGameCarousel::OnBgEffectFinished);
        }

        bgUsedImageDataAsync = bgNewImageDataAsync;
        bgNewImageDataAsync = NULL;
        bgUsedImageDataAsync->setColorIntensity(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        bgUsedImageDataAsync->setParent(this);
        bgUsedImageDataAsync->setEffect(EFFECT_FADE, 5, 255);
        insert(bgUsedImageDataAsync, 0);
    }

    if((currDegree - 0.5f) > destDegree)
    {
        if(startRotationDistance == 0.0f)
            startRotationDistance = fabsf(destDegree - currDegree);

        currDegree -= circleRotationSpeed;

        f32 angleDistance = fabsf(destDegree - currDegree);
        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed > circleSpeedLimit)
            circleRotationSpeed = circleSpeedLimit;

        if(angleDistance < circleRotationSpeed)
            currDegree = destDegree;

        refreshDrawMap = true;
    }
    else if((currDegree + 0.5f) < destDegree)
    {
        if(startRotationDistance == 0.0f)
            startRotationDistance = fabsf(destDegree - currDegree);

        currDegree += circleRotationSpeed;

        f32 angleDistance = fabsf(destDegree - currDegree);
        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed > circleSpeedLimit)
            circleRotationSpeed = circleSpeedLimit;

        if(angleDistance < circleRotationSpeed)
            currDegree = destDegree;

        refreshDrawMap = true;
    }
    else
    {
        startRotationDistance = 0.0f;
    }


    if(refreshDrawMap)
    {
        refreshDrawMap = false;
        updateDrawMap();
    }

    GuiGameBrowser::draw(v);

    for(u32 i = 0; i < drawOrder.size(); i++)
    {
        int idx = drawOrder[i];
        game[idx]->draw(v);
    }
}

void GuiGameCarousel::update(GuiController * c)
{
    if (isStateSet(STATE_DISABLED) || !pagesize)
        return;

    GuiGameBrowser::update(c);

    for(u32 i = 0; i < drawOrder.size(); i++)
    {
        int idx = drawOrder[i];
        game[idx]->update(c);
    }
}


void GuiGameCarousel::updateEffects()
{
    gameLaunchTimer++;

    if(touchClickDelay)
        touchClickDelay--;

    GuiGameBrowser::updateEffects();

    for(u32 i = 0; i < drawOrder.size(); i++)
    {
        int idx = drawOrder[i];
        game[idx]->updateEffects();
    }
}
