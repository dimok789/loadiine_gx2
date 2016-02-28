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
#ifndef _GUI_ICON_GRID_H_
#define _GUI_ICON_GRID_H_

#include <map>
#include "GuiFrame.h"
#include "GuiButton.h"
#include "GameIcon.h"
#include "GuiGameBrowser.h"
#include "GameBgImage.h"
#include "GridBackground.h"
#include "GuiParticleImage.h"
#include "GuiText.h"

class GuiIconGrid : public GuiGameBrowser, public sigslot::has_slots<>
{
public:
    GuiIconGrid(int w, int h, int GameIndex);
    virtual ~GuiIconGrid();

    void setSelectedGame(int idx);
    int getSelectedGame(void);

    void update(GuiController * t);
    void draw(CVideo *pVideo);
private:
    void OnLeftArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnRightArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnGameButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnDownClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnUpClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnLaunchClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void updateButtonPositions();

    static const int MAX_ROWS = 3;
    static const int MAX_COLS = 5;

    GuiSound *buttonClickSound;
    GuiImageData noIcon;
    GuiImageData emptyIcon;

    GuiParticleImage particleBgImage;

    GuiText gameTitle;
    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger leftTrigger;
    GuiTrigger rightTrigger;
    GuiTrigger downTrigger;
    GuiTrigger upTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonLTrigger;
    GuiTrigger buttonRTrigger;
    GuiButton leftButton;
    GuiButton rightButton;
    GuiButton downButton;
    GuiButton upButton;
    GuiButton launchButton;

    GuiImageData* arrowRightImageData;
    GuiImageData* arrowLeftImageData;
    GuiImage arrowRightImage;
    GuiImage arrowLeftImage;
    GuiButton arrowRightButton;
    GuiButton arrowLeftButton;

    std::vector<GameIcon *> gameIcons;
    std::vector<GuiButton *> gameButtons;
    int listOffset;
    int selectedGame;
    int currentLeftPosition;
    int targetLeftPosition;
    u32 gameLaunchTimer;
};

#endif // _GUI_ICON_GRID_H_
