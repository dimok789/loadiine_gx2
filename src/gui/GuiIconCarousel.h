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
#ifndef _GUI_ICON_CAROUSEL_H_
#define _GUI_ICON_CAROUSEL_H_

#include <map>
#include "GuiFrame.h"
#include "GuiButton.h"
#include "GuiGameBrowser.h"
#include "GameIcon.h"
#include "GameBgImage.h"
#include "GridBackground.h"

class GuiIconCarousel : public GuiGameBrowser, public sigslot::has_slots<>
{
public:
    GuiIconCarousel(int w, int h, int GameIndex);
    virtual ~GuiIconCarousel();

    void setSelectedGame(int idx);
    int getSelectedGame(void);

    void draw(CVideo *pVideo)
    {
        draw(pVideo, m_modelView);
    }

    void draw(CVideo *pVideo, const glm::mat4 & modelView);

    void updateEffects();
private:
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

    bool bUpdateMap;

    glm::mat4 m_modelView;

    POINT lastPosition;
    f32 circleRadius;
    f32 circlePosition;
    f32 circleTargetPosition;
    f32 circleRotationSpeed;
    f32 circleSpeedLimit;
    f32 startRotationDistance;
    f32 lastTouchDifference;

    GuiSound *buttonClickSound;
    GuiImageData bgGridData;
    GridBackground bgGrid;

    GuiImageData noIcon;

    GameBgImage *bgUsedImageDataAsync;
    GameBgImage *bgNewImageDataAsync;
    GameBgImage *bgFadingImageDataAsync;

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

    std::vector<GameIcon *> gameIcons;
    std::vector<int> drawOrder;
    int touchClickDelay;
    int selectedGame;
    int selectedGameOnDragStart;
    bool bWasDragging;
    u32 gameLaunchTimer;
};

#endif // _GUI_ICON_CAROUSEL_H_
