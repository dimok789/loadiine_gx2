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
#ifndef _GUIGAMECAROUSEL_H_
#define _GUIGAMECAROUSEL_H_

#include <vector>
#include "GuiImageAsync.h"
#include "GuiGameBrowser.h"
#include "GuiImage.h"
#include "GuiText.h"
#include "GuiParticleImage.h"

class GuiGameCarousel : public GuiGameBrowser, public sigslot::has_slots<>
{
public:
    GuiGameCarousel(int w, int h, int GameIndex);
    virtual ~GuiGameCarousel();

    void setSelectedGame(int idx);
    int getSelectedGame();
    void refresh();
    void draw(CVideo *v);
    void update(GuiController * c);
    void updateEffects();
protected:
    void OnGameButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnTouchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnTouchHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnTouchRelease(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnLeftSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnRightSkipClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnLaunchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger) {
        gameLaunchClicked(this, getSelectedGame());
    }

    void OnBgEffectFinished(GuiElement *element);

    void updateDrawMap(void);

    void loadBgImage(int idx);

    int getGameIndex(int listOffset, int idx);

    bool refreshDrawMap;
    bool bAnimating;
    bool bWasDragging;
    bool bWasSwapped;
    bool bFullCircleMode;
    bool bPageSizeEven;
    int rotateDirection;
    int forceRotateDirection;
    int selectedGame;
    int selectedGameOnDragStart;
    int listOffset;
    int pagesize;
    POINT lastPosition;
    int lastTouchDifference;
    int touchClickDelay;
    u32 gameLaunchTimer;

    f32 circleCenterDegree;
    f32 circleStartDegree;
    f32 circleRotationSpeed;
    f32 circleSpeedMin;
    f32 circleSpeedMax;
    f32 startRotationDistance;

    GuiSound *buttonClickSound;
    GuiImageData noCover;
    std::vector<GuiButton *> game;
    std::vector<GuiImage *> coverImg;
    std::vector<int> drawOrder;

    f32 currDegree;
    f32 destDegree;

    GuiText gameTitle;
    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonLTrigger;
    GuiTrigger buttonRTrigger;
    GuiTrigger buttonLeftTrigger;
    GuiTrigger buttonRightTrigger;
    GuiButton touchButton;
    GuiButton DPADButtons;

    GuiParticleImage particleBgImage;

    GuiImageAsync *bgUsedImageDataAsync;
    GuiImageAsync *bgNewImageDataAsync;
    GuiImageAsync *bgFadingImageDataAsync;
};
#endif
