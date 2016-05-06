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
#include "GameLauncherMenu.h"
#include "game/GameList.h"
#include "Application.h"
#include "settings/SettingsGameDefs.h"
#include "settings/CSettingsGame.h"
#include "fs/DirList.h"
#include "menu/ProgressWindow.h"
#include "gui/GuiSelectBox.h"
#include "menu/SettingsMenu.h"
#include "gui/GuiElement.h"
#include "utils/StringTools.h"
#include "settings/CSettings.h"

CThread * GameLauncherMenu::pThread = NULL;

GameLauncherMenu::GameLauncherMenu(int gameIdx)
    : GuiFrame(0,0)
    , gameIdx(gameIdx)
    , bgImage(500, 500, (GX2Color){0, 0, 0, 255})
    , bgBlur(1280, 720, (GX2Color){0, 0, 0, 255})
    , noCover(Resources::GetFile("noCover.png"), Resources::GetFileSize("noCover.png"))
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , quitImageData(Resources::GetImageData("quitButton.png"))
    , quitImage(quitImageData)
    , quitSelectedImageData(Resources::GetImageData("quitButtonSelected.png"))
    , quitSelectedImage(quitSelectedImageData)
    , quitButton(quitImage.getWidth(), quitImage.getHeight())
    , okImageData(Resources::GetImageData("emptyRoundButton.png"))
    , okImage(okImageData)
    , okSelectedImageData(Resources::GetImageData("emptyRoundButtonSelected.png"))
    , okSelectedImage(okSelectedImageData)
    , okButton(okImage.getWidth(), okImage.getHeight())
    , okText("O.K.", 46, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f))
    , titleImageData(Resources::GetImageData("settingsTitle.png"))
    , titleImage(titleImageData)
    , extraSaveText(tr("Extra Save:"), 40, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , dlcEnableText(tr("Enable DLC Support:"), 40, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , frameImageData(Resources::GetImageData("gameSettingsFrame.png"))
    , frameImage(frameImageData)
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , buttonLTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_L, true)
    , buttonRTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_R, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT | GuiTrigger::STICK_L_LEFT, true)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT | GuiTrigger::STICK_L_RIGHT, true)
    , buttonUpTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_UP | GuiTrigger::STICK_L_UP, true)
    , buttonDownTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_DOWN | GuiTrigger::STICK_L_DOWN, true)
    , leftArrowImageData(Resources::GetImageData("leftArrow.png"))
    , rightArrowImageData(Resources::GetImageData("rightArrow.png"))
    , leftArrowImage(leftArrowImageData)
    , rightArrowImage(rightArrowImageData)
    , leftArrowButton(leftArrowImage.getWidth(), leftArrowImage.getHeight())
    , rightArrowButton(rightArrowImage.getWidth(), rightArrowImage.getHeight())
    , DPADButtons(0,0)
    , extraSaveBox(false)
    , dlcEnableBox(false)
    , progresswindow("")
    , pathSelectBox(tr("Update Folder"),NULL)
    , saveModeSelectBox(tr("Save Mode"),NULL)
    , launchModeSelectBox(tr("Launch Mode"),NULL)
    , bgUsedImageDataAsync(NULL)
    , bgNewImageDataAsync(NULL)
    , bgFadingImageDataAsync(NULL)
{
    bFocusChanged = true;
    gamelauncherelementfocus = GamelaunchermenuFocus::OK;

    //Settings up the values for the selectboxes that don't change
    savemode_size = sizeof(ValueGameSaveModes) / sizeof(ValueGameSaveModes[0]);

    saveModeNames[tr("<Settings Default>")] = strfmt("%d", GAME_SAVES_DEFAULT);
    for(int i = 0; i < savemode_size; i++){
        saveModeNames[ValueGameSaveModes[i].name] = strfmt("%d",ValueGameSaveModes[i].value);
    }
    launchmode_size = sizeof(ValueLaunchMode) / sizeof(ValueLaunchMode[0]);
    launchModeNames[tr("<Settings Default>")] = strfmt("%d", LOADIINE_MODE_DEFAULT);
    for(int i = 0; i < launchmode_size; i++){
        launchModeNames[ValueLaunchMode[i].name] = strfmt("%d",ValueLaunchMode[i].value);
    }

    progresswindow.setVisible(false);

    Application::instance()->getMainWindow()->gameLauncherMenuNextClicked.connect(this,&GameLauncherMenu::OnGotHeaderFromMain);

    CVideo * video = Application::instance()->getVideo();
    width = video->getTvWidth()*windowScale;
    height = video->getTvHeight()*windowScale;
    gameLauncherMenuFrame = GuiFrame(width, height);

    bgImage.setSize(width,height);
    frameImage.setScale(windowScale);

    bgBlur.setAlpha(0.85f);
    append(&bgBlur);
    append(&bgImage);
    append(&frameImage);

    titleImage.setScale(windowScale);
    titleText.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleText.setFontSize(46);
    titleText.setPosition(0, 10);
    titleText.setBlurGlowColor(5.0f, glm::vec4(0.0, 0.0, 0.0f, 1.0f));
    append(&titleImage);
    append(&titleText);

    titleText.setParent(&titleImage);
    titleImage.setAlignment(ALIGN_MIDDLE | ALIGN_TOP);
    quitButton.setImage(&quitImage);
	quitButton.setIconOver(&quitSelectedImage);
    quitButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    quitButton.clicked.connect(this, &GameLauncherMenu::OnQuitButtonClick);
    quitButton.setTrigger(&touchTrigger);
    quitButton.setTrigger(&wpadTouchTrigger);
    quitButton.setEffectGrow();
    quitButton.setSoundClick(buttonClickSound);
    quitButton.setScale(windowScale*0.8);
    quitButton.setPosition((1.0/30.0)*width,(1.0/30.0)*width);
    gameLauncherMenuFrame.append(&quitButton);

    okText.setPosition(10, -10);
    okButton.setLabel(&okText);
    okButton.setImage(&okImage);
	okButton.setIconOver(&okSelectedImage);
    okButton.setAlignment(ALIGN_BOTTOM | ALIGN_RIGHT);
    okButton.clicked.connect(this, &GameLauncherMenu::OnOKButtonClick);
    okButton.setTrigger(&touchTrigger);
    okButton.setTrigger(&wpadTouchTrigger);
    okButton.setSoundClick(buttonClickSound);
    okButton.setEffectGrow();
    okButton.setScale(windowScale*0.8);
    okButton.setPosition(-(1.0/30.0)*width,(1.0/30.0)*width);
    gameLauncherMenuFrame.append(&okButton);

    leftArrowButton.setImage(&leftArrowImage);
    leftArrowButton.setEffectGrow();
    leftArrowButton.setPosition(-120, 0);
    leftArrowButton.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    leftArrowButton.setTrigger(&touchTrigger);
    leftArrowButton.setTrigger(&wpadTouchTrigger);

    leftArrowButton.setSoundClick(buttonClickSound);
    leftArrowButton.clicked.connect(this, &GameLauncherMenu::OnLeftArrowClick);
    gameLauncherMenuFrame.append(&leftArrowButton);

    rightArrowButton.setImage(&rightArrowImage);
    rightArrowButton.setEffectGrow();
    rightArrowButton.setPosition(120, 0);
    rightArrowButton.setAlignment(ALIGN_RIGHT | ALIGN_MIDDLE);
    rightArrowButton.setTrigger(&touchTrigger);
    rightArrowButton.setTrigger(&wpadTouchTrigger);
    rightArrowButton.setSoundClick(buttonClickSound);
    rightArrowButton.clicked.connect(this, &GameLauncherMenu::OnRightArrowClick);
    gameLauncherMenuFrame.append(&rightArrowButton);

    DPADButtons.setTrigger(&buttonDownTrigger);
    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonLTrigger);
    DPADButtons.setTrigger(&buttonRTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.setTrigger(&buttonUpTrigger);

    DPADButtons.clicked.connect(this, &GameLauncherMenu::OnDPADClick);
    gameLauncherMenuFrame.append(&DPADButtons);

    f32 buttonscale = 3.8f/3.0f;

    extraSaveBox.setTrigger(&touchTrigger);
    extraSaveBox.setTrigger(&wpadTouchTrigger);
    extraSaveBox.setSoundClick(buttonClickSound);
    extraSaveBox.valueChanged.connect(this, &GameLauncherMenu::OnExtraSaveValueChanged);

    gameLauncherMenuFrame.append(&extraSaveBox);

    dlcEnableBox.setTrigger(&touchTrigger);
    dlcEnableBox.setTrigger(&wpadTouchTrigger);
    dlcEnableBox.setSoundClick(buttonClickSound);
    dlcEnableBox.valueChanged.connect(this, &GameLauncherMenu::OnDLCEnableValueChanged);

    gameLauncherMenuFrame.append(&dlcEnableBox);

    f32 xpos = 0.11f;
    f32 yOffset = -(0.3 * height);

    dlcEnableBox.setScale(buttonscale*windowScale);
    saveModeSelectBox.setScale(buttonscale*windowScale);
    launchModeSelectBox.setScale(buttonscale*windowScale);
    extraSaveBox.setScale(buttonscale*windowScale);
    extraSaveText.setScale(buttonscale*windowScale);
    dlcEnableText.setScale(buttonscale*windowScale);
    pathSelectBox.setScale(buttonscale* windowScale);

    dlcEnableBox.setPosition(xpos*width + (saveModeSelectBox.getTopValueWidth()*saveModeSelectBox.getScale() /2.0) - (dlcEnableBox.getWidth()*dlcEnableBox.getScale()/2.0), yOffset);
    dlcEnableText.setPosition(xpos*width - (saveModeSelectBox.getTopValueWidth()*saveModeSelectBox.getScale() /2.0)+ (dlcEnableText.getTextWidth()/2.0), yOffset);
    yOffset += saveModeSelectBox.getTopValueHeight() * saveModeSelectBox.getScale();

    saveModeSelectBox.setPosition(xpos*width, yOffset);
    yOffset += saveModeSelectBox.getTopValueHeight() * saveModeSelectBox.getScale();

    launchModeSelectBox.setPosition(xpos*width, yOffset);
    yOffset += saveModeSelectBox.getTopValueHeight() * saveModeSelectBox.getScale() * 1.2f;

    extraSaveBox.setPosition(xpos*width + (saveModeSelectBox.getTopValueWidth()*saveModeSelectBox.getScale() /2.0) - (extraSaveBox.getWidth()*extraSaveBox.getScale()/2.0), yOffset);

    extraSaveText.setPosition(xpos*width - (saveModeSelectBox.getTopValueWidth()*saveModeSelectBox.getScale() /2.0)+ (extraSaveText.getTextWidth()/2.0), yOffset);
    yOffset += saveModeSelectBox.getTopValueHeight() * saveModeSelectBox.getScale() * 1.2f;

    pathSelectBox.setPosition(xpos*width, yOffset);
    yOffset += saveModeSelectBox.getTopValueHeight() * saveModeSelectBox.getScale() * 1.2f;

    pathSelectBox.showhide.connect(this, &GameLauncherMenu::OnSelectBoxShowHide);
    launchModeSelectBox.showhide.connect(this, &GameLauncherMenu::OnSelectBoxShowHide),
    saveModeSelectBox.showhide.connect(this, &GameLauncherMenu::OnSelectBoxShowHide);

    pathSelectBox.valueChanged.connect(this, &GameLauncherMenu::OnSelectBoxValueChanged);
    launchModeSelectBox.valueChanged.connect(this, &GameLauncherMenu::OnSelectBoxValueChanged),
    saveModeSelectBox.valueChanged.connect(this, &GameLauncherMenu::OnSelectBoxValueChanged);

    gameLauncherMenuFrame.append(&dlcEnableText);
    gameLauncherMenuFrame.append(&extraSaveText);
    gameLauncherMenuFrame.append(&saveModeSelectBox);
    gameLauncherMenuFrame.append(&pathSelectBox);
    gameLauncherMenuFrame.append(&launchModeSelectBox);

    selectBoxes.push_back(&pathSelectBox);
    selectBoxes.push_back(&launchModeSelectBox);
    selectBoxes.push_back(&saveModeSelectBox);

    append(&gameLauncherMenuFrame);
    append(&progresswindow);

    focusElements[GamelaunchermenuFocus::ExtraSave] = &extraSaveBox;
    focusElements[GamelaunchermenuFocus::EnableDLC] = &dlcEnableBox;
    focusElements[GamelaunchermenuFocus::Quit] = &quitButton;
    focusElements[GamelaunchermenuFocus::UpdatePath] = &pathSelectBox;
    focusElements[GamelaunchermenuFocus::SaveMethod] = &saveModeSelectBox;
    focusElements[GamelaunchermenuFocus::LaunchMethod] = &launchModeSelectBox;
    focusElements[GamelaunchermenuFocus::OK] = &okButton;

    setHeader(GameList::instance()->at(gameIdx));
}

GameLauncherMenu::~GameLauncherMenu()
{
    Resources::RemoveImageData(quitImageData);
    Resources::RemoveImageData(leftArrowImageData);
    Resources::RemoveImageData(rightArrowImageData);
    Resources::RemoveImageData(okImageData);
    Resources::RemoveImageData(okSelectedImageData);
    Resources::RemoveImageData(quitSelectedImageData);
    Resources::RemoveImageData(titleImageData);
    Resources::RemoveImageData(frameImageData);

    Resources::RemoveSound(buttonClickSound);

    if(bgFadingImageDataAsync)
    {
        bgFadingImageDataAsync->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
    }
    if(bgUsedImageDataAsync)
    {
        bgUsedImageDataAsync->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(bgUsedImageDataAsync);
    }
    if(bgNewImageDataAsync)
    {
        bgNewImageDataAsync->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(bgNewImageDataAsync);
    }
    if(coverImg)
    {
        coverImg->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(coverImg);
    }

    if(GameLauncherMenu::pThread != NULL){
        delete GameLauncherMenu::pThread;
    }
}

void GameLauncherMenu::OnSelectBoxShowHide(GuiSelectBox * selectBox,bool value){
    //Disable other selectboxes while one is open!
    if(value){
        DPADButtons.setState(STATE_DISABLED);
        for(u32 i = 0; i < selectBoxes.size(); i++){
            selectBoxes[i]->setState(STATE_DISABLED);
            if(selectBoxes[i] == selectBox){
                bringToFront(selectBoxes[i]);
                selectBoxes[i]->clearState(STATE_DISABLED);

                std::map<int, GuiElement *>::iterator itr;
                for(itr = focusElements.begin(); itr != focusElements.end(); itr++) {
                    if(itr->second == selectBox){
                        gamelauncherelementfocus = itr->first;
                        bFocusChanged = true;
                    }
                }
            }
        }
    }else{
        DPADButtons.clearState(STATE_DISABLED);
        for(u32 i = 0; i < selectBoxes.size(); i++){
            selectBoxes[i]->clearState(STATE_DISABLED);
        }
    }
}

void GameLauncherMenu::OnSelectBoxValueChanged(GuiSelectBox * selectBox, std::string value){
    if(selectBox == &pathSelectBox){
        log_printf("Setting update path to %s\n",value.c_str());
        gamesettings.updateFolder = value;
    }else if(selectBox == &saveModeSelectBox){
        log_printf("Setting savemode to %s\n",value.c_str());
        gamesettings.save_method = atoi(value.c_str());
    }else if(selectBox == &launchModeSelectBox){
        log_printf("Setting launchmode to %s\n",value.c_str());
        gamesettings.launch_method = atoi(value.c_str());
    }else{
        return;
    }
    bChanged = true;
    gamesettingsChanged = true;
}
void GameLauncherMenu::OnExtraSaveValueChanged(GuiToggle * toggle,bool value){
    gamesettings.extraSave = value;
    gamesettingsChanged = true;
    bChanged = true;
}

void GameLauncherMenu::OnDLCEnableValueChanged(GuiToggle * toggle,bool value){
    gamesettings.EnableDLC = value;
    gamesettingsChanged = true;
    bChanged = true;
}

void GameLauncherMenu::OnGotHeaderFromMain(GuiElement *button, int gameIdx)
{
    this->gameIdx = gameIdx;
    setHeader(GameList::instance()->at(gameIdx));
}

void GameLauncherMenu::setHeader(const discHeader * header)
{
    this->header =  header;
    gameLauncherMenuFrame.remove(coverImg);

    std::string filepath = CSettings::getValueAsString(CSettings::GameCover3DPath) + "/" + header->id + ".png";

    if(coverImg)
    {
        coverImg->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(coverImg);
        coverImg = NULL;
    }

    coverImg = new GuiImageAsync(filepath, &noCover);
    coverImg->setAlignment(ALIGN_LEFT);
    coverImg->setPosition(50,0);
    coverImg->setScale((5.0/3.0) * windowScale);
    coverImg->imageLoaded.connect(this, &GameLauncherMenu::OnCoverLoadedFinished);
    gameLauncherMenuFrame.append(coverImg);
    loadBgImage();

    //Set game title
    std::string gamename = header->name;
    if(gamename.length() > 40){
        gamename = gamename.substr(0,38) + "...";
    }

    titleText.setText(gamename.c_str());

    DirList updatefolder(header->gamepath + UPDATE_PATH,NULL,DirList::Dirs);
    log_printf("Found %d update folders for %s:\n",updatefolder.GetFilecount(),header->name.c_str());
    updatePaths.clear();

    updatePaths[COMMON_UPDATE_PATH] = COMMON_UPDATE_PATH;
    for(int i = 0; i < updatefolder.GetFilecount(); i++)
    {
        updatePaths[updatefolder.GetFilename(i)] = updatefolder.GetFilename(i);
        log_printf("%s\n",updatefolder.GetFilename(i));
    }

    //gameTitle.setText(gamename.c_str());
    bool result = CSettingsGame::getInstance()->LoadGameSettings(header->id,gamesettings);
    if(result){
        log_print("Found ");
    }

    log_printf("Game Setting for: %s\n\n",header->id.c_str());
    log_printf("Update Folder: \"%s\"\n",gamesettings.updateFolder.c_str());
    log_printf("Extra Save: %d\n",gamesettings.extraSave);
    log_printf("Launch Method: %d\n",gamesettings.launch_method);
    log_printf("Save Method: %d\n",gamesettings.save_method);
    log_print("--------\n");

    //getting selected Items for selectboxes
    std::map<std::string, std::string>::iterator itr;
    int i = 0;

    int updatePathID  = 0;
    for(itr = updatePaths.begin(); itr != updatePaths.end(); itr++) {
        if(itr->second.compare(gamesettings.updateFolder) == 0){
            updatePathID = i;
            break;
        }
        i++;
    }

    int savemodeID  = 0;
    i=0;
    for(itr = saveModeNames.begin(); itr != saveModeNames.end(); itr++) {
        if(atoi(itr->second.c_str()) == gamesettings.save_method){
            savemodeID = i;
        }
        i++;
    }
    int launchmodeID  = 0;
    i = 0;
    for(itr = launchModeNames.begin(); itr != launchModeNames.end(); itr++) {
         if(atoi(itr->second.c_str()) == gamesettings.launch_method){
            launchmodeID = i;
        }
        i++;
    }


    pathSelectBox.Init(updatePaths,updatePathID);
    //TODO: change to set Value
    saveModeSelectBox.Init(saveModeNames,savemodeID);
    launchModeSelectBox.Init(launchModeNames,launchmodeID);
    extraSaveBox.setValue(gamesettings.extraSave);
    dlcEnableBox.setValue(gamesettings.EnableDLC);

    gamesettingsChanged = false;
    bChanged = true;
}

void GameLauncherMenu::OnLeftArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    OnGetPreviousHeaderClick(button,controller,trigger);
}

void GameLauncherMenu::OnRightArrowClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    OnGetNextHeaderClick(button,controller,trigger);
}

void GameLauncherMenu::OnAButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
   if(gamelauncherelementfocus == GamelaunchermenuFocus::Quit){
        OnQuitButtonClick(button,controller,trigger);
     }else if(gamelauncherelementfocus == GamelaunchermenuFocus::OK) {
        OnOKButtonClick(button,controller,trigger);
     }else{
        GuiSelectBox * selectBox = dynamic_cast<GuiSelectBox *>(focusElements.at(gamelauncherelementfocus));
        if(selectBox != NULL){
           selectBox->OnTopValueClicked(button,controller,trigger);
           return;
        }
        GuiToggle * toggle = dynamic_cast<GuiToggle *>(focusElements.at(gamelauncherelementfocus));
        if(toggle != NULL){
            toggle->OnToggleClick(button,controller,trigger);
            return;
        }
     }
}

void GameLauncherMenu::OnOKButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    CSettings::setValueAsU16(CSettings::GameStartIndex,gameIdx);
    gameLauncherMenuFrame.setState(STATE_DISABLED);
    progresswindow.setTitle(tr("Saving game settings!"));
    progresswindow.setProgress(0.0f);
    progresswindow.setVisible(true);
    bringToFront(&progresswindow);
    GameLauncherMenu::SaveSettingsAsync(this);
}

void GameLauncherMenu::SaveSettingsAsync(GameLauncherMenu * menu)
{
    GameLauncherMenu::pThread = CThread::create(GameLauncherMenu::SaveGameSettings, (void*)menu, CThread::eAttributeAffCore1 | CThread::eAttributePinnedAff, 10);
    GameLauncherMenu::pThread->resumeThread();
}

void GameLauncherMenu::SaveGameSettings(CThread *thread, void *arg){
    GameLauncherMenu * args = (GameLauncherMenu * )arg;
    bool result = false;
    if(args->gamesettingsChanged){
        result = CSettingsGame::getInstance()->SaveGameSettings(args->gamesettings);
    }
    args->ReturnFromSaving(thread,result);
}

void GameLauncherMenu::ReturnFromSaving(CThread * thread, int result)
{
    gameLauncherMenuFrame.clearState(STATE_DISABLED);
    progresswindow.setVisible(false);
    gameLauncherMenuQuitClicked(this, header,true);
}


void GameLauncherMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(trigger == &buttonATrigger)
	{
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }
		OnAButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonBTrigger){
		OnQuitButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonLTrigger){
		OnLeftArrowClick(button,controller,trigger);
	}
	else if(trigger == &buttonRTrigger){
        OnRightArrowClick(button,controller,trigger);
	}
	else if(trigger == &buttonUpTrigger){
	    gamelauncherelementfocus++;
	    if(gamelauncherelementfocus >= GamelaunchermenuFocus::MAX_VALUE) gamelauncherelementfocus = GamelaunchermenuFocus::INVALID+2;
	    bFocusChanged = true;
	}
	else if(trigger == &buttonDownTrigger){
	    gamelauncherelementfocus--;
	    if(gamelauncherelementfocus <= GamelaunchermenuFocus::INVALID) gamelauncherelementfocus = GamelaunchermenuFocus::MAX_VALUE-2;
	    bFocusChanged = true;
	}
	else if(trigger == &buttonRightTrigger){
	    gamelauncherelementfocus = GamelaunchermenuFocus::INVALID+1;
	    bFocusChanged = true;
	}
	else if(trigger == &buttonLeftTrigger){
        gamelauncherelementfocus = GamelaunchermenuFocus::MAX_VALUE-1;
	    bFocusChanged = true;
	}

}

void GameLauncherMenu::loadBgImage()
{
    if(header == NULL)
        return;

    std::string filepath = header->gamepath + META_PATH + "/bootTvTex.tga";

    //! remove image that is possibly still loading
    //! TODO: fix (state != STATE_DISABLED) its a cheap trick to make the thread not create new images when fading out because it causes issues
    if(bgNewImageDataAsync && !isStateSet(STATE_DISABLED))
    {
        bgNewImageDataAsync->imageLoaded.disconnect_all();
        GuiImageAsync::removeFromQueue(bgNewImageDataAsync);
        AsyncDeleter::pushForDelete(bgNewImageDataAsync);
        bgNewImageDataAsync = new GuiImageAsync(filepath, NULL);
    }
    else
    {
        delete bgNewImageDataAsync;
        bgNewImageDataAsync = new GuiImageAsync(filepath,  NULL);
    }

    bgNewImageDataAsync->imageLoaded.connect(this, &GameLauncherMenu::OnBgLoadedFinished);
}

void GameLauncherMenu::OnCoverLoadedFinished(GuiImageAsync *image)
{
    //! since this function is entered through an asynchron call from the gui image async thread
    //! the GUI has to be locked before accessing the data
    Application::instance()->getMainWindow()->lockGUI();
    if(image->imageLoaded.connected()){
        f32 oldScale = image->getScale();
        f32 coverScale = noCover.getHeight() / image->getHeight();
        image->setScale(oldScale * coverScale);
    }
    Application::instance()->getMainWindow()->unlockGUI();
}

void GameLauncherMenu::OnBgLoadedFinished(GuiImageAsync *image)
{
    //! since this function is entered through an asynchron call from the gui image async thread
    //! the GUI has to be locked before accessing the data
    Application::instance()->getMainWindow()->lockGUI();
    if(image->imageLoaded.connected())
    {
        if(bgNewImageDataAsync->getImageData())
        {
            if(bgUsedImageDataAsync)
            {
                bgFadingImageDataAsync = bgUsedImageDataAsync;
                bgFadingImageDataAsync->setEffect(EFFECT_FADE, -10, 0);
                bgFadingImageDataAsync->effectFinished.connect(this, &GameLauncherMenu::OnBgEffectFinished);
            }

            bgUsedImageDataAsync = bgNewImageDataAsync;
            bgNewImageDataAsync = NULL;
            bgUsedImageDataAsync->setColorIntensity(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
            bgUsedImageDataAsync->setParent(this);
            bgUsedImageDataAsync->setEffect(EFFECT_FADE, 10, 255);
            bgUsedImageDataAsync->setScale(windowScale);
        }
        else
        {
            bgNewImageDataAsync->imageLoaded.disconnect_all();
            AsyncDeleter::pushForDelete(bgNewImageDataAsync);
            bgNewImageDataAsync = NULL;
        }

        insert(bgUsedImageDataAsync,2);
    }
    Application::instance()->getMainWindow()->unlockGUI();
}

void GameLauncherMenu::OnBgEffectFinished(GuiElement *image)
{
    if(image == bgFadingImageDataAsync)
    {
        remove(bgFadingImageDataAsync);
        bgFadingImageDataAsync->imageLoaded.disconnect_all();
        AsyncDeleter::pushForDelete(bgFadingImageDataAsync);
        bgFadingImageDataAsync = NULL;
    }
}

void GameLauncherMenu::draw(CVideo *v)
{
    GuiFrame::draw(v);
}

void GameLauncherMenu::update(GuiController *c)
{
    if(bFocusChanged){
        std::map<int, GuiElement *>::iterator itr;
        for(itr = focusElements.begin(); itr != focusElements.end(); itr++) {
            if(itr->first == gamelauncherelementfocus){
                itr->second->setState(STATE_SELECTED);
            }else{
                itr->second->clearState(STATE_SELECTED);
            }
        }
        bFocusChanged = false;
    }
    if(bChanged){
        if(gamesettings.updateFolder.compare(COMMON_UPDATE_PATH) != 0){
            extraSaveBox.setAlpha(1.0f);
            extraSaveText.setAlpha(1.0f);
            extraSaveBox.clearState(STATE_DISABLED);
            extraSaveText.clearState(STATE_DISABLED);
        }else{
            if(gamelauncherelementfocus == GamelaunchermenuFocus::ExtraSave){
                gamelauncherelementfocus = GamelaunchermenuFocus::LaunchMethod;
                bFocusChanged =  true;
            }
            extraSaveBox.setAlpha(0.3f);
            extraSaveText.setAlpha(0.3f);
            extraSaveBox.setUnchecked();
            extraSaveBox.setState(STATE_DISABLED);
            extraSaveText.setState(STATE_DISABLED);
        }
        bChanged = false;
    }
    GuiFrame::update(c);
}
