/****************************************************************************
 * Copyright (C) 2016 Maschell
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
#ifndef _GAMELAUNCHERMENU_WINDOW_H_
#define _GAMELAUNCHERMENU_WINDOW_H_

#include "system/CThread.h"
#include "gui/Gui.h"
#include "gui/GuiParticleImage.h"
#include "gui/GuiCheckBox.h"
#include "gui/GuiSelectBox.h"
#include "gui/GuiImageAsync.h"
#include "gui/GuiSwitch.h"
#include "game/GameList.h"
#include "ProgressWindow.h"
#include "settings/CSettingsGame.h"
#include "settings/SettingsEnums.h"

class GameLauncherMenu : public GuiFrame, public sigslot::has_slots<>
{

public:
    GameLauncherMenu(int gameIdx);
    virtual ~GameLauncherMenu();

    sigslot::signal3<GuiElement *,const discHeader *, bool> gameLauncherMenuQuitClicked;
    sigslot::signal3<GuiElement *,int,int> gameLauncherGetHeaderClicked;

    void draw(CVideo *v);
    void update(GuiController *c);

     enum GamelaunchermenuFocus
    {
        INVALID = -1,
       	OK,
        SaveMethod,
        LaunchMethod,
        ExtraSave,
        UpdatePath,
       	EnableDLC,
       	Quit,
       	MAX_VALUE
    };

private:
    float windowScale = 0.75f;
    bool gamesettingsChanged = false;
    void setHeader(const discHeader *);
    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnGetNextHeaderClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        gameLauncherGetHeaderClicked(this,gameIdx,1);
    }

    void OnGetPreviousHeaderClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        gameLauncherGetHeaderClicked(this,gameIdx,-1);
    }

    void ReturnFromSaving(CThread * thread, int result);
    static void SaveGameSettings(CThread *thread, void *arg);

    void OnOptionButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnAButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnOKButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    void OnQuitButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        gameLauncherMenuQuitClicked(this,header,false);
    }

    static void SaveSettingsAsync(GameLauncherMenu * menu);

    void OnSelectBoxValueChanged(GuiSelectBox * selectbox,std::string value);
    void OnExtraSaveValueChanged(GuiToggle * toggle,bool value);
    void OnDLCEnableValueChanged(GuiToggle * toggle,bool value);

    void OnGotHeaderFromMain(GuiElement *button, int gameIdx);
    void OnLeftArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnRightArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnSelectBoxShowHide(GuiSelectBox * selectBox,bool value);

    void loadBgImage();
    void OnBgLoadedFinished(GuiImageAsync *element);
    void OnCoverLoadedFinished(GuiImageAsync *element);
    void OnBgEffectFinished(GuiElement *element);

    GuiFrame gameLauncherMenuFrame;
    int gameIdx;
    GuiImage bgImage;
    GuiImage bgBlur;
    GuiImageData noCover;
    GuiImageAsync * coverImg = NULL;
    GuiSound *buttonClickSound;

    GuiImageData *quitImageData;
    GuiImage quitImage;
    GuiImageData *quitSelectedImageData;
    GuiImage quitSelectedImage;
    GuiButton quitButton;

    GuiImageData *okImageData;
    GuiImage okImage;
    GuiImageData *okSelectedImageData;
    GuiImage okSelectedImage;
    GuiButton okButton;
    GuiText okText;

    GuiText titleText;
    GuiImageData *titleImageData;
    GuiImage titleImage;

    GuiText extraSaveText;
    GuiText dlcEnableText;

    GuiImageData *frameImageData;
    GuiImage frameImage;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;
    GuiTrigger buttonLTrigger;
    GuiTrigger buttonRTrigger;
    GuiTrigger buttonLeftTrigger;
    GuiTrigger buttonRightTrigger;
    GuiTrigger buttonUpTrigger;
    GuiTrigger buttonDownTrigger;

    GuiImageData *leftArrowImageData;
    GuiImageData *rightArrowImageData;
    GuiImage leftArrowImage;
    GuiImage rightArrowImage;
    GuiButton leftArrowButton;
    GuiButton rightArrowButton;

    GuiButton DPADButtons;

    GuiSwitch extraSaveBox;
    GuiSwitch dlcEnableBox;

    const discHeader *header;
    GameSettings gamesettings;

    ProgressWindow progresswindow;

    std::map<std::string,std::string> updatePaths;
    std::map<std::string,std::string> saveModeNames;
    std::map<std::string,std::string> launchModeNames;

    int savemode_size = 0;
    int launchmode_size = 0;
    GuiSelectBox pathSelectBox;
    GuiSelectBox saveModeSelectBox;
    GuiSelectBox launchModeSelectBox;

    GuiImageAsync *bgUsedImageDataAsync;
    GuiImageAsync *bgNewImageDataAsync;
    GuiImageAsync *bgFadingImageDataAsync;


    std::vector<GuiSelectBox *> selectBoxes;

    std::map<int,GuiElement *> focusElements;

    static CThread *pThread;

    int gamelauncherelementfocus;
    bool bFocusChanged = false;
    bool bChanged = false;

};



#endif //_GAMELAUNCHERMENU_WINDOW_H_
