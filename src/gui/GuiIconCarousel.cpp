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
#include "GuiIconCarousel.h"
#include "GuiController.h"
#include "common/common.h"
#include "Application.h"
#include "video/CVideo.h"
#include "game/GameList.h"
#include "resources/Resources.h"

static const float RADIUS = 0.9f;
static const float radiusScale = 1.0f; // 1:1 game
static const float Yoffset = 0.04f;
static const float cam_X_rot = 25.0f;
static const float fIconRgbDrop = 0.395f;
static const float fOpacy = 1.0f;

GuiIconCarousel::GuiIconCarousel(int w, int h, int GameIndex)
    : GuiGameBrowser(w, h, GameIndex)
    , buttonClickSound(Resources::GetSound("button_click.mp3"))
    , bgGridData(Resources::GetFile("bgGridTile.png"), Resources::GetFileSize("bgGridTile.png"), GX2_TEX_CLAMP_WRAP)
    , bgGrid(&bgGridData)
    , noIcon(Resources::GetFile("noGameIcon.png"), Resources::GetFileSize("noGameIcon.png"), GX2_TEX_CLAMP_MIRROR)
    , bgUsedImageDataAsync(NULL)
    , bgNewImageDataAsync(NULL)
    , bgFadingImageDataAsync(NULL)
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
{
    m_modelView = glm::mat4(1.0f);
    circlePosition = 0.0f;
    bUpdateMap = true;
    gameLaunchTimer = 60;
    touchClickDelay = 0;
    selectedGame = GameIndex;
    selectedGameOnDragStart = 0;
    bWasDragging = false;
    circleRadius = RADIUS * GameList::instance()->size() / 12.0f;

    circlePosition = 0.0f;
    circleTargetPosition = 0.0f;
    circleRotationSpeed = 0.0f;
    lastTouchDifference = 0.0f;
    startRotationDistance = 0.0f;
    circleSpeedLimit = 1.8f;

    bgGrid.setScale(0.25f);
    append(&bgGrid);

    touchButton.setClickable(true);
    touchButton.setHoldable(true);
    touchButton.setTrigger(&touchTrigger);
    touchButton.setTrigger(&wpadTouchTrigger);
    touchButton.clicked.connect(this, &GuiIconCarousel::OnTouchClick);
    touchButton.held.connect(this, &GuiIconCarousel::OnTouchHold);
    touchButton.released.connect(this, &GuiIconCarousel::OnTouchRelease);
    append(&touchButton);

    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonLTrigger);
    DPADButtons.setTrigger(&buttonRTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.clicked.connect(this, &GuiIconCarousel::OnDPADClick);
    append(&DPADButtons);

    for(int i = 0; i < GameList::instance()->size(); i++)
    {
		std::string filepath = GameList::instance()->at(i)->gamepath + META_PATH + "/iconTex.tga";

        GameIcon *icon = new GameIcon(filepath, &noIcon);
        icon->setParent(this);

        gameIcons.push_back(icon);
        drawOrder.push_back(i);
        append(icon);
    }

    gameTitle.setPosition(0, -320);
    gameTitle.setBlurGlowColor(16.0f, glm::vec4(0.109804, 0.6549, 1.0f, 1.0f));
    gameTitle.setMaxWidth(900, GuiText::DOTTED);
    append(&gameTitle);
    
    circleTargetPosition = 360.0f - GameIndex * 360.0f / (radiusScale * gameIcons.size());
    circlePosition = circleTargetPosition;
    setSelectedGame(selectedGame);
}

GuiIconCarousel::~GuiIconCarousel()
{
    AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
    AsyncDeleter::pushForDelete(bgUsedImageDataAsync);
    AsyncDeleter::pushForDelete(bgNewImageDataAsync);

    for(size_t i = 0; i < gameIcons.size(); i++)
    {
        delete gameIcons[i];
    }

    Resources::RemoveSound(buttonClickSound);
}

void GuiIconCarousel::OnBgEffectFinished(GuiElement *element)
{
    if(element == bgFadingImageDataAsync)
    {
        remove(bgFadingImageDataAsync);
        AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
        bgFadingImageDataAsync = NULL;
    }
}

int GuiIconCarousel::getSelectedGame()
{
    return selectedGame;
}

void GuiIconCarousel::setSelectedGame(int selectedIdx)
{
    if(selectedIdx < 0 || selectedIdx >= (int)drawOrder.size())
        return;

    //! normalize to 360°
    circlePosition = ((int)(circlePosition + 0.5f)) % 360;
    if(circlePosition < 0.0f)
        circlePosition += 360.0f;

    circleTargetPosition = 360.0f - selectedIdx * 360.0f / (radiusScale * gameIcons.size());

    f32 angleDiff = (circleTargetPosition - circlePosition);
    if(angleDiff > 180.0f)
        circleTargetPosition -= 360.0f;
    else if(angleDiff < -180.0f)
        circleTargetPosition += 360.0f;

    bUpdateMap = true;

    touchClickDelay = 20;
    circleSpeedLimit = 1.8f;
}

void GuiIconCarousel::OnTouchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->data.validPointer)
        return;

    bWasDragging = false;
    selectedGameOnDragStart = getSelectedGame();
    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;

    //! calculate ray origin and direction
    glm::vec3 rayOrigin;
    glm::vec3 rayDir;

    CVideo *video = Application::instance()->getVideo();
    video->screenPosToWorldRay(controller->data.x, controller->data.y, rayOrigin, rayDir);

    glm::vec3 rayDirFrac((rayDir.x != 0.0f) ? (1.0f / rayDir.x) : 0.0f, (rayDir.y != 0.0f) ? (1.0f / rayDir.y) : 0.0f, (rayDir.z != 0.0f) ? (1.0f / rayDir.z) : 0.0f);

    for(u32 i = 0; i < drawOrder.size(); ++i)
    {
        int idx = drawOrder[i];

        if(gameIcons[idx]->checkRayIntersection(rayOrigin, rayDirFrac))
        {
            if(buttonClickSound)
                buttonClickSound->Play();

            setSelectedGame(idx);
            gameSelectionChanged(this, idx);

            //! TODO: change this to a button assigned image
            gameIcons[idx]->setState(STATE_CLICKED);
            gameIcons[idx]->setEffect(EFFECT_SCALE, 4, 125);

            if(selectedGame == idx)
            {
                if(gameLaunchTimer < 30)
                    OnLaunchClick(button, controller, trigger);

                gameLaunchTimer = 0;
            }
        }
    }
}


void GuiIconCarousel::OnTouchHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->data.validPointer)
        return;

    lastTouchDifference = ((int)controller->data.x - (int)lastPosition.x);
    f32 degreeAdd = lastTouchDifference * 0.128f;
    if(touchClickDelay || fabsf(degreeAdd) < 0.5f) {
        return;
    }

    circlePosition -= degreeAdd;
    circleTargetPosition = circlePosition;

    lastPosition.x = controller->data.x;
    lastPosition.y = controller->data.y;
    bUpdateMap = true;
    bWasDragging = true;
}

void GuiIconCarousel::OnTouchRelease(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(!controller->lastData.validPointer)
        return;

    for(size_t i = 0; i < gameIcons.size(); ++i)
    {
        if(gameIcons[i]->isStateSet(STATE_CLICKED))
        {
            gameIcons[i]->setEffect(EFFECT_SCALE, -4, 100);
            gameIcons[i]->clearState(STATE_CLICKED);
        }
    }

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

    circlePosition = ((int)(circlePosition + 0.5f)) % 360;
    if(circlePosition < 0.0f)
        circlePosition += 360.0f;

    f32 partDegree = 360.0f / (radiusScale * gameIcons.size());

    circleTargetPosition = circlePosition - 0.5f * degreeAdd * partDegree;

    //! round to nearest game position at the target position
    circleTargetPosition = ((int)(circleTargetPosition / partDegree + 0.5f)) * partDegree;
    circleSpeedLimit = 10.0f;

    int iItem = 0;
    f32 zMax = -9999.9f;

    for(u32 i = 0; i < gameIcons.size(); i++)
    {
        float currDegree = DegToRad(360.0f / (radiusScale * gameIcons.size()) * i + circleTargetPosition + 90.0f);
        float posZ = radiusScale * circleRadius * sinf(currDegree) + RADIUS - gameIcons.size() * (RADIUS / 12.0f);

        if(zMax < posZ)
        {
            iItem = i;
            zMax = posZ;
        }
    }

    selectedGame = iItem;
    gameSelectionChanged(this, selectedGame);
}

void GuiIconCarousel::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(trigger == &buttonATrigger){
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

void GuiIconCarousel::OnLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    int sel = getSelectedGame() + 1;
    if(sel >= (int)GameList::instance()->size())
        sel = 0;

    setSelectedGame(sel);
    gameSelectionChanged(this, sel);
}

void GuiIconCarousel::OnRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    int sel = getSelectedGame() - 1;
    if(sel < 0 && (GameList::instance()->size() > 1))
        sel = GameList::instance()->size() - 1;

    setSelectedGame(sel);
    gameSelectionChanged(this, sel);
}

void GuiIconCarousel::OnLeftSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if((int)GameList::instance()->size() > 5){
		int sel = getSelectedGame() + 5;
		if(sel >= (int)GameList::instance()->size())
			sel -= (int)GameList::instance()->size();

		setSelectedGame(sel);
		gameSelectionChanged(this, sel);
	}
}

void GuiIconCarousel::OnRightSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if((int)GameList::instance()->size() > 5){
		int sel = getSelectedGame() - 5;
		if(sel < 0 && (GameList::instance()->size() > 5))
			sel += GameList::instance()->size();

		setSelectedGame(sel);
		gameSelectionChanged(this, sel);
	}
}

void GuiIconCarousel::updateDrawMap(void)
{
    //! create a multimap of Z position which will sort the items by z coordinate
    std::multimap< float, std::pair<float, int> > drawMap;
    std::multimap< float, std::pair<float, int> >::const_iterator itr;
    size_t i;

    for(i = 0; i < gameIcons.size(); i++)
    {
        int idx = i;

        float currDegree = DegToRad(360.0f / (radiusScale * gameIcons.size()) * i + circlePosition + 90.0f);
        float posX = radiusScale * circleRadius * cosf(currDegree);
        float posZ = radiusScale * circleRadius * sinf(currDegree) + RADIUS - gameIcons.size() * (RADIUS / 12.0f);
        drawMap.insert(std::pair<float, std::pair<float, int> >(posZ, std::pair<float, int>(posX, i)));

        float rgbReduction = std::min((circleRadius + posZ / 2.0f + fIconRgbDrop) / (2.0f * circleRadius), 1.0f);
        if(rgbReduction < 0.0f)
            rgbReduction = 0.0f;

        float alphaReduction = std::min((circleRadius + posZ + fOpacy * (circleRadius * 2.0f)) / (2.0f * circleRadius), 1.0f);
        if(alphaReduction < 0.0f)
            alphaReduction = 0.0f;

        gameIcons[idx]->setColorIntensity(glm::vec4(rgbReduction, rgbReduction, rgbReduction, 1.0f));
        gameIcons[idx]->setAlpha(alphaReduction);
        gameIcons[idx]->setRenderReflection(true);
        gameIcons[idx]->setRotationX(-cam_X_rot);
        gameIcons[idx]->setPosition(posX * width * 0.5f, Yoffset * height * 0.5f, posZ * width * 0.5f);
    }

    for(itr = drawMap.begin(), i = 0; itr != drawMap.end(); i++, itr++)
    {
        int idx = itr->second.second;

        itr++;
        bool bSelected = (itr == drawMap.end());
        itr--;

        if(!bSelected)
        {
            float rgbReduction = 0.8f;
            glm::vec4 intensity = gameIcons[idx]->getColorIntensity();
            gameIcons[idx]->setColorIntensity(glm::vec4(rgbReduction * intensity[0], rgbReduction * intensity[1], rgbReduction * intensity[2], intensity[3]));
        }
        else
        {
            gameTitle.setText(GameList::instance()->at(idx)->name.c_str());

            if(selectedGame != idx || !bgUsedImageDataAsync)
            {
                selectedGame = idx;
                std::string filepath = GameList::instance()->at(idx)->gamepath + META_PATH + "/bootTvTex.tga";

                //! remove image that is possibly still loading
                //! TODO: fix (state != STATE_DISABLED) its a cheap trick to make the thread not create new images when fading out because it causes issues
                //! TODO: this is probably not needed anymore -> verify
                if(bgNewImageDataAsync && !isStateSet(STATE_DISABLED))
                {
                    GuiImageAsync::removeFromQueue(bgNewImageDataAsync);
                    AsyncDeleter::pushForDelete(bgNewImageDataAsync);
                    bgNewImageDataAsync = new GameBgImage(filepath, NULL);
                }
                else
                {
                    delete bgNewImageDataAsync;
                    bgNewImageDataAsync = new GameBgImage(filepath, NULL);
                }

            }
            glm::vec4 intensity = gameIcons[idx]->getColorIntensity();
            gameIcons[idx]->setColorIntensity(glm::vec4(intensity[0], intensity[1], intensity[2], 1.2f * intensity[3]));
        }

        drawOrder[i] = idx;
        gameIcons[idx]->setSelected(bSelected);
    }
}

void GuiIconCarousel::draw(CVideo *pVideo, const glm::mat4 & modelView)
{
    if(!this->isVisible())
        return;

    if(bUpdateMap)
    {
        bUpdateMap = false;
        updateDrawMap();
    }

    if(bgNewImageDataAsync && bgNewImageDataAsync->getImageData() && !bgFadingImageDataAsync)
    {
        if(bgUsedImageDataAsync)
        {
            bgFadingImageDataAsync = bgUsedImageDataAsync;
            bgFadingImageDataAsync->setEffect(EFFECT_FADE, -10, 0);
            bgFadingImageDataAsync->effectFinished.connect(this, &GuiIconCarousel::OnBgEffectFinished);
        }

        bgUsedImageDataAsync = bgNewImageDataAsync;
        bgNewImageDataAsync = NULL;
        bgUsedImageDataAsync->setEffect(EFFECT_FADE, 5, 255);
        append(bgUsedImageDataAsync);
    }

    pVideo->setStencilRender(true);

    if(bgUsedImageDataAsync)
        bgUsedImageDataAsync->draw(pVideo);
    if(bgFadingImageDataAsync)
        bgFadingImageDataAsync->draw(pVideo);

    bgGrid.draw(pVideo, modelView);

    pVideo->setStencilRender(false);

    for(size_t i = 0; i < drawOrder.size(); ++i)
    {
        size_t idx = drawOrder[i];
        gameIcons[idx]->draw(pVideo, pVideo->getProjectionMtx(), pVideo->getViewMtx(), modelView);
    }

    gameTitle.draw(pVideo);

    if(touchClickDelay)
    {
        touchClickDelay--;
    }

    gameLaunchTimer++;
}

void GuiIconCarousel::updateEffects()
{
    if((circlePosition - 0.5f) > circleTargetPosition)
    {
        if(startRotationDistance == 0.0f)
            startRotationDistance = fabsf(circleTargetPosition - circlePosition);

        circlePosition -= circleRotationSpeed;

        f32 angleDistance = fabsf(circleTargetPosition - circlePosition);
        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed > circleSpeedLimit)
            circleRotationSpeed = circleSpeedLimit;

        if(angleDistance < circleRotationSpeed)
            circlePosition = circleTargetPosition;

        bUpdateMap = true;
    }
    else if((circlePosition + 0.5f) < circleTargetPosition)
    {
        if(startRotationDistance == 0.0f)
            startRotationDistance = fabsf(circleTargetPosition - circlePosition);

        circlePosition += circleRotationSpeed;

        f32 angleDistance = fabsf(circleTargetPosition - circlePosition);
        circleRotationSpeed = 8.0f * angleDistance / startRotationDistance;

        if(circleRotationSpeed > circleSpeedLimit)
            circleRotationSpeed = circleSpeedLimit;

        if(angleDistance < circleRotationSpeed)
            circlePosition = circleTargetPosition;

        bUpdateMap = true;
    }
    else
    {
        startRotationDistance = 0.0f;
    }

    GuiGameBrowser::updateEffects();
}
