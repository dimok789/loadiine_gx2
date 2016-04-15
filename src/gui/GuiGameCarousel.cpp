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

#define DEG_OFFSET          8.0f
#define COVERS_CENTER_DEG   (bPageSizeEven ? (pagesize * 0.5f * DEG_OFFSET) : (pagesize * 0.5f * DEG_OFFSET - 0.5f * DEG_OFFSET))
#define RADIUS	            1000.0f
#define IN_SPEED	        175
#define SHIFT_SPEED         75
#define SPEED_STEP          4
#define SPEED_LIMIT         250

#define MAX_PAGE_SIZE       14

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
    pagesize = (GameList::instance()->size() < MAX_PAGE_SIZE) ? GameList::instance()->size() : MAX_PAGE_SIZE;
    bFullCircleMode = (GameList::instance()->size() > MAX_PAGE_SIZE);
    refreshDrawMap = true;
    selectable = true;
    selectedGame = GameIndex;
    listOffset = selectedGame;
    selectedGameOnDragStart = 0;
    forceRotateDirection = 0;
    bWasDragging = false;
    bAnimating = false;
    bPageSizeEven = ((pagesize >> 1) << 1) == pagesize;

    rotateDirection = 0;
    lastTouchDifference = 0;
    gameLaunchTimer = 0;
    touchClickDelay = 0;
    circleRotationSpeed = 0.0f;
    circleSpeedMin = 0.3f;
    circleSpeedMax = 1.8f;
    startRotationDistance = 0.0f;
    circleCenterDegree = 90.0f;
    circleStartDegree = circleCenterDegree + COVERS_CENTER_DEG;
    destDegree = bFullCircleMode ? circleStartDegree : (circleCenterDegree + listOffset * DEG_OFFSET);
    currDegree = 0.0f;

    this->append(&particleBgImage);

    game.resize(pagesize);
    drawOrder.resize(pagesize);
    coverImg.resize(GameList::instance()->size());

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

    for (u32 i = 0; i < coverImg.size(); ++i)
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
    if(idx >= GameList::instance()->size())
        idx = GameList::instance()->size()-1;

    selectedGame = idx;
    loadBgImage(idx);

    touchClickDelay = 20;
    circleSpeedMax = 1.8f;
    refreshDrawMap = true;
}

int GuiGameCarousel::getSelectedGame()
{
    return selectedGame;
}

void GuiGameCarousel::refresh()
{
    // since we got more than enough memory we can pre-cache all covers
    // once there are more than 1000 games for the WiiU we can do a load on demand
    for (int i = 0; i < GameList::instance()->size(); i++)
    {
        //------------------------
        // Image
        //------------------------
        delete coverImg[i];

        std::string filepath = CSettings::getValueAsString(CSettings::GameCover3DPath) + "/" + GameList::instance()->at(i)->id + ".png";

        coverImg[i] = new GuiImageAsync(filepath, &noCover);
    }

    for (int i = 0; i < pagesize; i++)
    {
        //------------------------
        // Index
        //------------------------
        int gameIndex = getGameIndex(listOffset, i);

        //------------------------
        // GameButton
        //------------------------
        delete game[i];
        game[i] = new GuiButton(coverImg[gameIndex]->getWidth(), coverImg[gameIndex]->getHeight());
        game[i]->setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
        game[i]->setImage(coverImg[gameIndex]);
        game[i]->setParent(this);
        game[i]->setTrigger(&touchTrigger);
        game[i]->setTrigger(&wpadTouchTrigger);
        game[i]->setSoundClick(buttonClickSound);
        game[i]->setEffectGrow();
        game[i]->clicked.connect(this, &GuiGameCarousel::OnGameButtonClick);

        drawOrder[i] = i;
    }

    setSelectedGame(selectedGame);
}

void GuiGameCarousel::OnGameButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < game.size(); i++)
    {
        if(button == game[i])
        {
            int gameIndex = getGameIndex(listOffset, i);
            if(selectedGame == (int)gameIndex)
            {
                if(gameLaunchTimer < 30)
                    OnLaunchClick(button, controller, trigger);

                gameLaunchTimer = 0;
            }

            setSelectedGame(gameIndex);
            gameSelectionChanged(this, gameIndex);
            break;
        }
    }
}

void GuiGameCarousel::OnTouchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if((pagesize == 0) || !controller->data.validPointer)
        return;

    bWasDragging = false;
    selectedGameOnDragStart = getGameIndex(listOffset, drawOrder[ drawOrder.size() - 1 ]);
    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;
}

void GuiGameCarousel::OnTouchHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(touchClickDelay || (pagesize == 0) || !controller->data.validPointer) {
        bWasDragging = false;
        return;
    }

    lastTouchDifference = (controller->data.x - lastPosition.x);
    f32 degreeAdd = lastTouchDifference * 0.096f;
    if(fabs(degreeAdd) < 0.5f)
        return;

    currDegree -= degreeAdd;

    if(!bFullCircleMode)
    {
        f32 leftLimit = (circleStartDegree - COVERS_CENTER_DEG);
        f32 rightLimit = (circleStartDegree + COVERS_CENTER_DEG) - (bPageSizeEven ? DEG_OFFSET : 0.0f); // TODO: find out why the 2nd part is required

        if(currDegree < leftLimit)
        {
            currDegree = leftLimit;
        }
        else if(currDegree > rightLimit)
        {
            currDegree = rightLimit;
        }
    }

    destDegree = currDegree;

    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;
    refreshDrawMap = true;
    bWasDragging = true;
}

void GuiGameCarousel::OnTouchRelease(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(touchClickDelay || (pagesize == 0) || !controller->lastData.validPointer) {
        bWasDragging = false;
        return;
    }

    f32 degreeAdd = lastTouchDifference * 0.128f;
    if(fabsf(degreeAdd) < 2.0f)
    {
        selectedGame = listOffset;

        if(bWasDragging && selectedGameOnDragStart != selectedGame)
        {
            setSelectedGame(selectedGame);
            gameSelectionChanged(this, selectedGame);
        }
        // if we ever want that the circle is always centered after dragging, comment this in
        // destDegree = circleStartDegree;
        bWasDragging = false;
        return;
    }

    selectedGame = selectedGame - (degreeAdd + 0.5f);

    if(bFullCircleMode)
    {
        while(selectedGame < 0)
            selectedGame += GameList::instance()->size();
        while(selectedGame >= GameList::instance()->size())
            selectedGame -= GameList::instance()->size();
    }
    else
    {
        if(selectedGame >= GameList::instance()->size())
            selectedGame = GameList::instance()->size() - 1;
        if(selectedGame < 0)
            selectedGame = 0;
    }

    if(selectedGame != selectedGameOnDragStart) {
        forceRotateDirection = (lastTouchDifference > 0) ? -1 : 1;
        setSelectedGame(selectedGame);
        gameSelectionChanged(this, selectedGame);
    }
    else {
        forceRotateDirection = 0;
    }

    circleSpeedMax = 10.0f;
    refreshDrawMap = true;
    bWasDragging = false;
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
    if(pagesize == 0)
        return;

    int sel = getSelectedGame() - 1;
    if(sel < 0)
    {
        if(bFullCircleMode)
            sel = GameList::instance()->size() - 1;
        else
            sel = 0;
    }

    if(sel != getSelectedGame())
    {
        setSelectedGame(sel);
        gameSelectionChanged(this, sel);
    }
}

void GuiGameCarousel::OnRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(pagesize == 0)
        return;

    int sel = getSelectedGame() + 1;
    if(sel >= (int)GameList::instance()->size())
        sel = bFullCircleMode ? 0 : (GameList::instance()->size() - 1);

    if(sel != getSelectedGame())
    {
        setSelectedGame(sel);
        gameSelectionChanged(this, sel);
    }
}

void GuiGameCarousel::OnRightSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(pagesize == 0)
        return;

    int sel = getSelectedGame() + 5;

    if(sel >= GameList::instance()->size())
    {
        if(bFullCircleMode)
            sel -= GameList::instance()->size();
        else
            sel = GameList::instance()->size() - 1;
    }

    if(sel != getSelectedGame())
    {
        setSelectedGame(sel);
        gameSelectionChanged(this, sel);
    }
}

void GuiGameCarousel::OnLeftSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(pagesize == 0)
        return;

    int sel = getSelectedGame() - 5;

    if(sel < 0)
    {
        if(bFullCircleMode)
            sel += GameList::instance()->size();
        else
            sel = 0;
    }

    if(sel != getSelectedGame())
    {
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

        int gameIndex = getGameIndex(listOffset, itr->second);
        if(gameIndex >= GameList::instance()->size())
            gameIndex -= GameList::instance()->size();

        coverImg[gameIndex]->setColorIntensity(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
    }

    if(drawOrder.size())
    {
        int lastIdx = drawOrder[ drawOrder.size() - 1 ];

        int gameIndex = getGameIndex(listOffset, lastIdx);
        coverImg[gameIndex]->setColorIntensity(glm::vec4(1.0f));
        gameTitle.setText(GameList::instance()->at(gameIndex)->name.c_str());
    }
}

int GuiGameCarousel::getGameIndex(int listOffset, int idx)
{
    if(pagesize == 0)
        return 0;

    int gameIndex = idx;
    if(bFullCircleMode) {
        gameIndex += listOffset - (pagesize >> 1);
    }

    while(gameIndex < 0)
        gameIndex += GameList::instance()->size();
    while(gameIndex >= GameList::instance()->size())
        gameIndex -= GameList::instance()->size();

    return gameIndex;
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

    bool bAnimationFinished = false;
    bool bUpdateButtonPositions = false;

    if((currDegree - 0.5f) > destDegree)
    {
        f32 gameDistance = fabsf(selectedGame - listOffset);
        f32 dynamicDegreeDistance = (DEG_OFFSET - fabsf(destDegree - currDegree));

        if(gameDistance > (GameList::instance()->size() >> 1))
            gameDistance = GameList::instance()->size() - gameDistance;

        f32 angleDistance = gameDistance * DEG_OFFSET - dynamicDegreeDistance;

        if(startRotationDistance == 0.0f)
            startRotationDistance = angleDistance;

        currDegree -= circleRotationSpeed;

        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed < circleSpeedMin)
            circleRotationSpeed = circleSpeedMin;
        if(circleRotationSpeed > circleSpeedMax)
            circleRotationSpeed = circleSpeedMax;

        if(currDegree < destDegree)
            currDegree = destDegree;

        refreshDrawMap = true;
    }
    else if((currDegree + 0.5f) < destDegree)
    {
        f32 gameDistance = fabsf(selectedGame - listOffset);
        f32 dynamicDegreeDistance = (DEG_OFFSET - fabsf(destDegree - currDegree));

        if(gameDistance > (GameList::instance()->size() >> 1))
            gameDistance = GameList::instance()->size() - gameDistance;

        f32 angleDistance = gameDistance * DEG_OFFSET - dynamicDegreeDistance;

        if(startRotationDistance == 0.0f)
            startRotationDistance = angleDistance;

        currDegree += circleRotationSpeed;

        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed < circleSpeedMin)
            circleRotationSpeed = circleSpeedMin;
        if(circleRotationSpeed > circleSpeedMax)
            circleRotationSpeed = circleSpeedMax;

        if(currDegree > destDegree)
            currDegree = destDegree;

        refreshDrawMap = true;
    }
    else
    {
        bAnimationFinished = bAnimating;
        bAnimating = false;
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

    if(bWasDragging)
    {
        if(bFullCircleMode)
        {
            if(currDegree > (circleStartDegree + DEG_OFFSET * 0.5f))
            {
                currDegree -= DEG_OFFSET;
                destDegree -= DEG_OFFSET;
                selectedGame++;
                if(selectedGame >= GameList::instance()->size()) {
                    selectedGame -= GameList::instance()->size();
                }

                listOffset = selectedGame;
                bUpdateButtonPositions = true;
                rotateDirection = 0;
            }
            else if(currDegree < (circleStartDegree - DEG_OFFSET * 0.5f))
            {
                currDegree += DEG_OFFSET;
                destDegree += DEG_OFFSET;
                selectedGame--;
                if(selectedGame < 0) {
                    selectedGame += GameList::instance()->size();
                }

                listOffset = selectedGame;
                bUpdateButtonPositions = true;
                rotateDirection = 0;
            }
        }
        else
        {
            selectedGame = (currDegree - circleCenterDegree) / DEG_OFFSET + 0.5f;
            listOffset = selectedGame;
        }

    }

    if(!bAnimating && (listOffset != selectedGame || bAnimationFinished || bUpdateButtonPositions))
    {
        if(bUpdateButtonPositions || bAnimationFinished)
        {
            if(rotateDirection > 0)
            {
                listOffset++;
            }
            else if(rotateDirection < 0)
            {
                listOffset--;
            }

            if(listOffset < 0) {
                listOffset += GameList::instance()->size();
            }
            if(listOffset >= GameList::instance()->size()) {
                listOffset -= GameList::instance()->size();
            }

            for(int i = 0; i < pagesize; i++)
            {
                int idx = getGameIndex(listOffset, i);

                if(idx < 0) {
                    idx += GameList::instance()->size();
                }
                if(idx >= GameList::instance()->size()) {
                    idx -= GameList::instance()->size();
                }

                game[i]->setImage(coverImg[idx]);
            }

            if(bAnimationFinished)
            {
                destDegree = bFullCircleMode ? circleStartDegree : (circleCenterDegree + listOffset * DEG_OFFSET);
                currDegree = destDegree;
            }
            refreshDrawMap = true;
        }

        rotateDirection = 0;

        if(listOffset < selectedGame)
        {
            if(forceRotateDirection)
                rotateDirection = forceRotateDirection;
            else if(!bFullCircleMode)
                rotateDirection = 1;
            else
                rotateDirection = ((selectedGame - listOffset) > (GameList::instance()->size() >> 1)) ? -1 : 1;
        }
        else if(listOffset > selectedGame)
        {
            if(forceRotateDirection)
                rotateDirection = forceRotateDirection;
            else if(!bFullCircleMode)
                rotateDirection = -1;
            else
                rotateDirection = ((listOffset - selectedGame) > (GameList::instance()->size() >> 1)) ? 1 : -1;
        }

        if(rotateDirection > 0)
        {
            destDegree += DEG_OFFSET;
            bAnimating = true;
            rotateDirection = 1;
        }
        else if(rotateDirection < 0)
        {
            destDegree -= DEG_OFFSET;
            bAnimating = true;
            rotateDirection = -1;
        }
        else
        {
            startRotationDistance = 0.0f;
            forceRotateDirection = 0;
        }

        bUpdateButtonPositions = false;
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
