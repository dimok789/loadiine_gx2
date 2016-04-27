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
#include "Application.h"
#include "dynamic_libs/os_functions.h"
#include "gui/FreeTypeGX.h"
#include "gui/GuiImageAsync.h"
#include "gui/VPadController.h"
#include "gui/WPadController.h"
#include "game/GameList.h"
#include "resources/Resources.h"
#include "settings/CSettings.h"
#include "settings/CSettingsGame.h"
#include "sounds/SoundHandler.hpp"
#include "system/exception_handler.h"
#include "utils/logger.h"

Application *Application::applicationInstance = NULL;
bool Application::exitApplication = false;

Application::Application()
	: CThread(CThread::eAttributeAffCore0 | CThread::eAttributePinnedAff, 0, 0x20000)
	, bgMusic(NULL)
	, video(NULL)
    , mainWindow(NULL)
{
    controller[0] = new VPadController(GuiTrigger::CHANNEL_1);
    controller[1] = new WPadController(GuiTrigger::CHANNEL_2);
    controller[2] = new WPadController(GuiTrigger::CHANNEL_3);
    controller[3] = new WPadController(GuiTrigger::CHANNEL_4);
    controller[4] = new WPadController(GuiTrigger::CHANNEL_5);

    CSettings::instance()->Load();
     
    //! reload logger to change IP to settings IP
    log_deinit();
    log_init(CSettings::getValueAsString(CSettings::DebugLoggerIP).c_str());

    //! load resources
    Resources::LoadFiles("sd:/wiiu/apps/loadiine_gx2/resources");

    //! create bgMusic
    if(CSettings::getValueAsString(CSettings::BgMusicPath).empty())
    {
        bgMusic = new GuiSound(Resources::GetFile("bgMusic.ogg"), Resources::GetFileSize("bgMusic.ogg"));
    }
    else
    {
        bgMusic = new GuiSound(CSettings::getValueAsString(CSettings::BgMusicPath).c_str());
    }
    bgMusic->SetLoop(true);
    bgMusic->Play();
    bgMusic->SetVolume(50);

	//! load language
    if(!CSettings::getValueAsString(CSettings::AppLanguage).empty())
    {
        std::string languagePath = "sd:/wiiu/apps/loadiine_gx2/languages/" + CSettings::getValueAsString(CSettings::AppLanguage) + ".lang";
		gettextLoadLanguage(languagePath.c_str());
    }

	exitApplication = false;
}

Application::~Application()
{
    GameList::destroyInstance();

    CSettings::instance()->Save();
    CSettings::destroyInstance();
	
	CSettingsGame::destroyInstance();

    delete bgMusic;

    for(int i = 0; i < 5; i++)
        delete controller[i];

	AsyncDeleter::destroyInstance();
    GuiImageAsync::threadExit();
    Resources::Clear();

	SoundHandler::DestroyInstance();

	gettextCleanUp();
}

void Application::exec()
{
    //! start main GX2 thread
    resumeThread();
    //! now wait for thread to finish
	shutdownThread();
}

void Application::fadeOut()
{
    GuiImage fadeOut(video->getTvWidth(), video->getTvHeight(), (GX2Color){ 0, 0, 0, 255 });

	for(int i = 0; i < 255; i += 10)
    {
        if(i > 255)
            i = 255;

        fadeOut.setAlpha(i / 255.0f);

        //! start rendering DRC
	    video->prepareDrcRendering();
	    mainWindow->drawDrc(video);

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS);
        fadeOut.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_LEQUAL);

	    video->drcDrawDone();

        //! start rendering TV
	    video->prepareTvRendering();

	    mainWindow->drawTv(video);

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS);
        fadeOut.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_LEQUAL);

	    video->tvDrawDone();

	    //! as last point update the effects as it can drop elements
	    mainWindow->updateEffects();

	    video->waitForVSync();
    }

    //! one last cleared black screen
    video->prepareDrcRendering();
    video->drcDrawDone();
    video->prepareTvRendering();
    video->tvDrawDone();
    video->waitForVSync();
    video->tvEnable(false);
    video->drcEnable(false);
}

void Application::executeThread(void)
{
    //! setup exceptions on the main GX2 core
    setup_os_exceptions();

    log_printf("Loading game list\n");
    GameList::instance()->loadUnfiltered();

    log_printf("Initialize video\n");
    video = new CVideo(GX2_TV_SCAN_MODE_720P, GX2_DRC_SINGLE);

    log_printf("Video size %i x %i\n", video->getTvWidth(), video->getTvHeight());

    //! setup default Font
    log_printf("Initialize main font system\n");
    FreeTypeGX *fontSystem = new FreeTypeGX(Resources::GetFile("font.ttf"), Resources::GetFileSize("font.ttf"), true);
    GuiText::setPresetFont(fontSystem);

    log_printf("Initialize main window\n");

    mainWindow = new MainWindow(video->getTvWidth(), video->getTvHeight());

    log_printf("Entering main loop\n");

    //! main GX2 loop (60 Hz cycle with max priority on core 1)
	while(!exitApplication)
	{
	    //! Read out inputs
	    for(int i = 0; i < 5; i++)
        {
            if(controller[i]->update(video->getTvWidth(), video->getTvHeight()) == false)
                continue;

            if(controller[i]->data.buttons_d & VPAD_BUTTON_HOME)
                exitApplication = true;

            //! update controller states
            mainWindow->update(controller[i]);
        }

        //! start rendering DRC
	    video->prepareDrcRendering();
	    mainWindow->drawDrc(video);
	    video->drcDrawDone();

        //! start rendering TV
	    video->prepareTvRendering();
	    mainWindow->drawTv(video);
	    video->tvDrawDone();

        //! enable screen after first frame render
	    if(video->getFrameCount() == 0) {
            video->tvEnable(true);
            video->drcEnable(true);
	    }

	    //! as last point update the effects as it can drop elements
	    mainWindow->updateEffects();

	    video->waitForVSync();

        //! transfer elements to real delete list here after all processes are finished
        //! the elements are transfered to another list to delete the elements in a separate thread
        //! and avoid blocking the GUI thread
        AsyncDeleter::triggerDeleteProcess();
	}

	fadeOut();

    delete mainWindow;
    delete fontSystem;
    delete video;
}
