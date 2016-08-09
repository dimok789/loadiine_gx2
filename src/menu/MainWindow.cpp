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
#include "MainWindow.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "Application.h"
#include "game/GameList.h"
#include "game/GameLauncher.h"
#include "menu/SettingsMenu.h"
#include "menu/GameLauncherMenu.h"
#include "gui/GuiGameCarousel.h"
#include "gui/GuiIconCarousel.h"
#include "gui/GuiIconGrid.h"
#include "settings/CSettings.h"
#include "settings/CSettingsGame.h"
#include "common/retain_vars.h"
#include "utils/StringTools.h"
#include "utils/logger.h"

MainWindow::MainWindow(int w, int h)
    : width(w)
    , height(h)
    , gameClickSound(Resources::GetSound("game_click.mp3"))
    , mainSwitchButtonFrame(NULL)
    , currentTvFrame(NULL)
    , currentDrcFrame(NULL)
    , launchingGame(false)

{
    for(int i = 0; i < 4; i++)
    {
        std::string filename = strfmt("player%i_point.png", i+1);
        pointerImgData[i] = Resources::GetImageData(filename.c_str());
        pointerImg[i] = new GuiImage(pointerImgData[i]);
        pointerImg[i]->setScale(1.5f);
        pointerValid[i] = false;
    }

    SetupMainView();
}

MainWindow::~MainWindow()
{
    while(!tvElements.empty())
    {
        delete tvElements[0];
        remove(tvElements[0]);
    }
    while(!drcElements.empty())
    {
        delete drcElements[0];
        remove(drcElements[0]);
    }
    for(int i = 0; i < 4; i++)
    {
        delete pointerImg[i];
        Resources::RemoveImageData(pointerImgData[i]);
    }

    Resources::RemoveSound(gameClickSound);
}

void MainWindow::updateEffects()
{
    //! dont read behind the initial elements in case one was added
    u32 tvSize = tvElements.size();
    u32 drcSize = drcElements.size();

    for(u32 i = 0; (i < drcSize) && (i < drcElements.size()); ++i)
    {
        drcElements[i]->updateEffects();
    }

    //! only update TV elements that are not updated yet because they are on DRC
    for(u32 i = 0; (i < tvSize) && (i < tvElements.size()); ++i)
    {
        u32 n;
        for(n = 0; (n < drcSize) && (n < drcElements.size()); n++)
        {
            if(tvElements[i] == drcElements[n])
                break;
        }
        if(n == drcElements.size())
        {
            tvElements[i]->updateEffects();
        }
    }
}

void MainWindow::update(GuiController *controller)
{
    //! dont read behind the initial elements in case one was added
    //u32 tvSize = tvElements.size();

    if(controller->chan & GuiTrigger::CHANNEL_1)
    {
        u32 drcSize = drcElements.size();

        for(u32 i = 0; (i < drcSize) && (i < drcElements.size()); ++i)
        {
            drcElements[i]->update(controller);
        }
    }
    else
    {
        u32 tvSize = tvElements.size();

        for(u32 i = 0; (i < tvSize) && (i < tvElements.size()); ++i)
        {
            tvElements[i]->update(controller);
        }
    }

//    //! only update TV elements that are not updated yet because they are on DRC
//    for(u32 i = 0; (i < tvSize) && (i < tvElements.size()); ++i)
//    {
//        u32 n;
//        for(n = 0; (n < drcSize) && (n < drcElements.size()); n++)
//        {
//            if(tvElements[i] == drcElements[n])
//                break;
//        }
//        if(n == drcElements.size())
//        {
//            tvElements[i]->update(controller);
//        }
//    }

    if(controller->chanIdx >= 1 && controller->chanIdx <= 4 && controller->data.validPointer)
    {
        int wpadIdx = controller->chanIdx - 1;
        f32 posX = controller->data.x;
        f32 posY = controller->data.y;
        pointerImg[wpadIdx]->setPosition(posX, posY);
        pointerImg[wpadIdx]->setAngle(controller->data.pointerAngle);
        pointerValid[wpadIdx] = true;
    }
}

void MainWindow::drawDrc(CVideo *video)
{
    for(u32 i = 0; i < drcElements.size(); ++i)
    {
        drcElements[i]->draw(video);
    }

    for(int i = 0; i < 4; i++)
    {
        if(pointerValid[i])
        {
            pointerImg[i]->setAlpha(0.5f);
            pointerImg[i]->draw(video);
            pointerImg[i]->setAlpha(1.0f);
        }
    }
}

void MainWindow::drawTv(CVideo *video)
{
    for(u32 i = 0; i < tvElements.size(); ++i)
    {
        tvElements[i]->draw(video);
    }

    for(int i = 0; i < 4; i++)
    {
        if(pointerValid[i])
        {
            pointerImg[i]->draw(video);
            pointerValid[i] = false;
        }
    }
}

void MainWindow::SetupMainView()
{
    if(CSettings::getValueAsU16(CSettings::GameStartIndex) > GameList::instance()->size())
        CSettings::setValueAsU16(CSettings::GameStartIndex,0);

    switch(CSettings::getValueAsU8(CSettings::GameViewModeTv))
    {
        case VIEW_ICON_GRID: {
            currentTvFrame = new GuiIconGrid(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
            break;
        }
        default:
        case VIEW_ICON_CAROUSEL: {
            currentTvFrame = new GuiIconCarousel(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
            break;
        }
        case VIEW_COVER_CAROUSEL: {
            currentTvFrame = new GuiGameCarousel(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
            break;
        }
    }

    currentTvFrame->setEffect(EFFECT_FADE, 10, 255);
    currentTvFrame->setState(GuiElement::STATE_DISABLED);
    currentTvFrame->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
    appendTv(currentTvFrame);

    if(CSettings::getValueAsU8(CSettings::GameViewModeDrc) == CSettings::getValueAsU8(CSettings::GameViewModeTv))
    {
        currentDrcFrame = currentTvFrame;
    }
    else
    {
        switch(CSettings::getValueAsU8(CSettings::GameViewModeDrc))
        {
            default:
            case VIEW_ICON_GRID: {
                currentDrcFrame = new GuiIconGrid(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
                break;
            }
            case VIEW_ICON_CAROUSEL: {
                currentDrcFrame = new GuiIconCarousel(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
                break;
            }
            case VIEW_COVER_CAROUSEL: {
                currentDrcFrame = new GuiGameCarousel(width, height,(int)CSettings::getValueAsU16(CSettings::GameStartIndex));
                break;
            }
        }
    }

    if(currentTvFrame != currentDrcFrame)
    {
        currentDrcFrame->setEffect(EFFECT_FADE, 10, 255);
        currentDrcFrame->setState(GuiElement::STATE_DISABLED);
        currentDrcFrame->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
    }

    //! reconnect only to DRC game selection change
    currentTvFrame->gameSelectionChanged.disconnect(this);
    currentDrcFrame->gameSelectionChanged.disconnect(this);
    currentTvFrame->gameLaunchClicked.disconnect(this);
    currentDrcFrame->gameLaunchClicked.disconnect(this);


    if(currentTvFrame != currentDrcFrame)
    {
        currentTvFrame->gameSelectionChanged.connect(this, &MainWindow::OnGameSelectionChange);
        currentTvFrame->gameLaunchClicked.connect(this, &MainWindow::OnGameLaunchMenu);
    }

    currentDrcFrame->gameSelectionChanged.connect(this, &MainWindow::OnGameSelectionChange);
    currentDrcFrame->gameLaunchClicked.connect(this, &MainWindow::OnGameLaunchMenu);

    mainSwitchButtonFrame = new MainDrcButtonsFrame(width, height);
    mainSwitchButtonFrame->settingsButtonClicked.connect(this, &MainWindow::OnSettingsButtonClicked);
    mainSwitchButtonFrame->layoutSwitchClicked.connect(this, &MainWindow::OnLayoutSwitchClicked);
    mainSwitchButtonFrame->gameImageDownloadClicked.connect(this, &MainWindow::OnImageDownloadClicked);
    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
    mainSwitchButtonFrame->setEffect(EFFECT_FADE, 10, 255);
    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
    mainSwitchButtonFrame->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);

    appendDrc(currentDrcFrame);
    append(mainSwitchButtonFrame);
}

void MainWindow::OnOpenEffectFinish(GuiElement *element)
{
    //! once the menu is open reset its state and allow it to be "clicked/hold"
    element->effectFinished.disconnect(this);
    element->clearState(GuiElement::STATE_DISABLED);
}

void MainWindow::OnCloseEffectFinish(GuiElement *element)
{
    //! remove element from draw list and push to delete queue
    remove(element);
    AsyncDeleter::pushForDelete(element);
}

void MainWindow::OnSettingsButtonClicked(GuiElement *element)
{
    CSettings::setValueAsU16(CSettings::GameStartIndex,currentDrcFrame->getSelectedGame());

    //! disable element for triggering buttons again
    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
    mainSwitchButtonFrame->setEffect(EFFECT_FADE, -10, 0);
    mainSwitchButtonFrame->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);
    mainSwitchButtonFrame = NULL;

    currentTvFrame->setState(GuiElement::STATE_DISABLED);
    currentTvFrame->setEffect(EFFECT_FADE, -10, 0);
    currentTvFrame->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);

    //! only fade out and delete the element once on equal screens
    if(currentTvFrame != currentDrcFrame)
    {
        currentDrcFrame->setState(GuiElement::STATE_DISABLED);
        currentDrcFrame->setEffect(EFFECT_FADE, -10, 0);
        currentDrcFrame->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);
    }

    currentTvFrame = NULL;
    currentDrcFrame = NULL;

    //! show equal screen on settings
    SettingsMenu * settings = new SettingsMenu(width, height);
    settings->setEffect(EFFECT_FADE, 10, 255);
    settings->setState(GuiElement::STATE_DISABLED);
    settings->settingsQuitClicked.connect(this, &MainWindow::OnSettingsQuit);
    settings->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
    append(settings);
}

void MainWindow::OnSettingsQuit(GuiElement *element)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);

    SetupMainView();

    //! re-append the deleting element at the end of the draw list
    append(element);
}

void MainWindow::OnLayoutSwitchEffectFinish(GuiElement *element)
{
    if(!currentTvFrame || !currentDrcFrame || !mainSwitchButtonFrame)
        return;

    element->effectFinished.disconnect(this);
    remove(currentDrcFrame);
    remove(currentTvFrame);

    GuiGameBrowser *tmpElement = currentDrcFrame;
    currentDrcFrame = currentTvFrame;
    currentTvFrame = tmpElement;

    appendTv(currentTvFrame);
    appendDrc(currentDrcFrame);
    //! re-append on top
    append(mainSwitchButtonFrame);

    currentTvFrame->resetState();
    currentTvFrame->setEffect(EFFECT_FADE, 15, 255);

    currentDrcFrame->resetState();
    currentDrcFrame->setEffect(EFFECT_FADE, 15, 255);

    mainSwitchButtonFrame->clearState(GuiElement::STATE_DISABLED);

    //! reconnect only to DRC game selection change
    currentTvFrame->gameSelectionChanged.disconnect(this);
    currentDrcFrame->gameSelectionChanged.disconnect(this);
    currentTvFrame->gameLaunchClicked.disconnect(this);
    currentDrcFrame->gameLaunchClicked.disconnect(this);

    currentTvFrame->gameSelectionChanged.connect(this, &MainWindow::OnGameSelectionChange);
    currentTvFrame->gameLaunchClicked.connect(this, &MainWindow::OnGameLaunchMenu);
    currentDrcFrame->gameSelectionChanged.connect(this, &MainWindow::OnGameSelectionChange);
    currentDrcFrame->gameLaunchClicked.connect(this, &MainWindow::OnGameLaunchMenu);

    u8 tmpMode = CSettings::getValueAsU8(CSettings::GameViewModeDrc);

    CSettings::setValueAsU8(CSettings::GameViewModeDrc, CSettings::getValueAsU8(CSettings::GameViewModeTv));
    CSettings::setValueAsU8(CSettings::GameViewModeTv, tmpMode);
}

void MainWindow::OnLayoutSwitchClicked(GuiElement *element)
{
    if(!currentTvFrame || !currentDrcFrame || !mainSwitchButtonFrame)
        return;

    if(currentTvFrame == currentDrcFrame)
        return;

    currentTvFrame->setState(GuiElement::STATE_DISABLED);
    currentTvFrame->setEffect(EFFECT_FADE, -15, 0);
    currentTvFrame->effectFinished.connect(this, &MainWindow::OnLayoutSwitchEffectFinish);

    currentDrcFrame->setState(GuiElement::STATE_DISABLED);
    currentDrcFrame->setEffect(EFFECT_FADE, -15, 0);

    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
}

void MainWindow::OnGameLaunchMenu(GuiGameBrowser *element, int gameIdx)
{
    if(GameList::instance()->size() == 0)
        return;

    if (launchingGame)
        return;

    launchingGame = true;

    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
    currentTvFrame->setState(GuiElement::STATE_DISABLED);
    if(currentTvFrame != currentDrcFrame)
    {
        currentDrcFrame->setState(GuiElement::STATE_DISABLED);
    }

    if(CSettings::getValueAsU8(CSettings::ShowGameSettings) == SETTING_ON)
    {
        GameLauncherMenu * gameSettings = new GameLauncherMenu(gameIdx);
        gameSettings->setEffect(EFFECT_FADE, 10, 255);
        gameSettings->setState(GuiElement::STATE_DISABLED);
        gameSettings->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
        gameSettings->gameLauncherMenuQuitClicked.connect(this, &MainWindow::OnGameLaunchMenuFinish);
        gameSettings->gameLauncherGetHeaderClicked.connect(this, &MainWindow::OnGameLaunchGetHeader);
        append(gameSettings);
    }
    else
    {
        OnGameLaunchMenuFinish(NULL, GameList::instance()->at(gameIdx), true);
    }
}

void MainWindow::OnGameLaunchGetHeader(GuiElement *element,int gameIdx, int next)
{
    if(next == 1){
        gameIdx += 1;
    }else if (next == -1){
        gameIdx -= 1;
    }
    if(gameIdx >= GameList::instance()->size()){
        gameIdx = 0;
    }else if(gameIdx < 0){
        gameIdx = GameList::instance()->size() -1;
    }

    CSettings::setValueAsU16(CSettings::GameStartIndex,gameIdx);

    OnExternalGameSelectionChange(gameIdx);
    gameLauncherMenuNextClicked(element,gameIdx);
}

void MainWindow::OnGameLaunchMenuFinish(GuiElement *element,const discHeader *header, bool result)
{
    mainSwitchButtonFrame->clearState(GuiElement::STATE_DISABLED);
    currentTvFrame->clearState(GuiElement::STATE_DISABLED);
    if(currentTvFrame != currentDrcFrame)
    {
        currentDrcFrame->clearState(GuiElement::STATE_DISABLED);
    }

    if(element)
    {
        element->setState(GuiElement::STATE_DISABLED);
        element->setEffect(EFFECT_FADE, -15, 0);
        element->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);
    }

    if(result == true){
        OnGameLaunch(header);
    }
    else {
        launchingGame = false;
    }
}

void MainWindow::OnGameLaunch(const discHeader *gameHdr)
{
    if(gameClickSound)
        gameClickSound->Play();

    log_printf("Loading game %s\n", gameHdr->name.c_str());

    GameLauncher *launcher = GameLauncher::loadGameToMemoryAsync(gameHdr);
    launcher->setEffect(EFFECT_FADE, 15, 255);
    launcher->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
    launcher->asyncLoadFinished.connect(this, &MainWindow::OnGameLoadFinish);
    append(launcher);
}

void MainWindow::OnGameLoadFinish(GameLauncher * launcher, const discHeader *header, int result)
{
    if(result == GameLauncher::SUCCESS)
    {

        if(CSettings::getValueAsBool(CSettings::GameLogServer))
        {
            const char * ip = CSettings::getValueAsString(CSettings::GameLogServerIp).c_str();
            if(strlen(ip) > 0 && strlen(ip) < 16){
                 memcpy(gServerIP,ip,strlen(ip)+1);
            }
            log_printf("FS log server on %s\n", CSettings::getValueAsString(CSettings::GameLogServerIp).c_str());
        }
        else
        {
            gServerIP[0] = '\0';
            log_printf("FS log server is off\n");
        }


        GAME_LAUNCHED = 1;
        GAME_RPX_LOADED = 0;
        LOADIINE_MODE = CSettings::getValueAsU8(CSettings::GameLaunchMethod);
        gSettingLaunchPyGecko = CSettings::getValueAsU8(CSettings::LaunchPyGecko);
        gSettingPadconMode= CSettings::getValueAsU8(CSettings::PadconMode);
        //! load HID settings
        gHIDPADEnabled = CSettings::getValueAsU8(CSettings::HIDPadEnabled);
        gEnableDLC = 0;

		GameSettings gs;
		bool result = CSettingsGame::getInstance()->LoadGameSettings(header->id,gs);
		if(result){
			switch(gs.launch_method){
				case LOADIINE_MODE_DEFAULT:
					log_printf("Using launch method from Settings\n");
					break; //leave it from settings
				case LOADIINE_MODE_MII_MAKER:
					log_printf("Using Mii Maker cfg\n");
					LOADIINE_MODE = LOADIINE_MODE_MII_MAKER;
					break;
				case LOADIINE_MODE_SMASH_BROS:
					log_printf("Using Super Smash Bros cfg\n");
					LOADIINE_MODE = LOADIINE_MODE_SMASH_BROS;
					break;
				case LOADIINE_MODE_KARAOKE:
					log_printf("Using KARAOKE BY U Joysound cfg\n");
					LOADIINE_MODE = LOADIINE_MODE_KARAOKE;
					break;
				case LOADIINE_MODE_ART_ATELIER:
					log_printf("Using Art Academy Atelier cfg\n");
					LOADIINE_MODE = LOADIINE_MODE_ART_ATELIER;
					break;
				default:
					log_printf("No valid value found. Using launch method from settings\n");
					break;
			}

			gEnableDLC = gs.EnableDLC;
		}
        Application::instance()->quit();
    }
    else {
        launchingGame = false;
    }

    mainSwitchButtonFrame->resetState();
    if(currentTvFrame)
        currentTvFrame->resetState();
    if(currentDrcFrame)
        currentDrcFrame->resetState();

    launcher->setState(GuiElement::STATE_DISABLED);
    launcher->setEffect(EFFECT_FADE, -15, 0);
    launcher->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);
}

void MainWindow::OnGameSelectionChange(GuiGameBrowser *element, int selectedIdx)
{
    if(!currentDrcFrame || !currentTvFrame)
        return;

    if(element == currentDrcFrame && currentDrcFrame != currentTvFrame)
        currentTvFrame->setSelectedGame(selectedIdx);
    else if(element == currentTvFrame && currentDrcFrame != currentTvFrame)
        currentDrcFrame->setSelectedGame(selectedIdx);
}

void MainWindow::OnExternalGameSelectionChange(int selectedIdx)
{
    currentTvFrame->setSelectedGame(selectedIdx);
    if(currentDrcFrame != currentTvFrame)
    {
        currentDrcFrame->setSelectedGame(selectedIdx);
    }
}

void MainWindow::OnImageDownloadClicked(GuiElement *element)
{
    mainSwitchButtonFrame->setState(GuiElement::STATE_DISABLED);
    if(currentTvFrame)
        currentTvFrame->setState(GuiElement::STATE_DISABLED);
    if(currentDrcFrame)
        currentDrcFrame->setState(GuiElement::STATE_DISABLED);

    GameImageDownloader *downloader = new GameImageDownloader();
    downloader->setEffect(EFFECT_FADE, 15, 255);
    downloader->effectFinished.connect(this, &MainWindow::OnOpenEffectFinish);
    downloader->asyncLoadFinished.connect(this, &MainWindow::OnImageDownloadFinish);
    downloader->startDownloading();
    append(downloader);
}

void MainWindow::OnImageDownloadFinish(GameImageDownloader *downloader, int fileLeft)
{
    mainSwitchButtonFrame->resetState();
    if(currentTvFrame)
        currentTvFrame->resetState();
    if(currentDrcFrame)
        currentDrcFrame->resetState();

    downloader->setState(GuiElement::STATE_DISABLED);
    downloader->setEffect(EFFECT_FADE, -15, 0);
    downloader->effectFinished.connect(this, &MainWindow::OnCloseEffectFinish);
}
