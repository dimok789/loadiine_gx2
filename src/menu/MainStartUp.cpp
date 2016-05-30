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
#include "MainStartUp.h"
#include "Application.h"
#include "gitrev.h"
#include "language/gettext.h"

MainStartUp::MainStartUp(CVideo *Video)
    : video(Video)
	, ImageData(Resources::GetImageData("splash.png"))
    , Image(ImageData)

{	
	versionText.setColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    versionText.setFontSize(26);
    versionText.setAlignment(ALIGN_TOP | ALIGN_RIGHT);
    versionText.setPosition(600, 312);
    versionText.setTextf("%s (build %s)",  LOADIINE_VERSION, GetRev());
    
	msgText.setColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    msgText.setFontSize(36);
	msgText.setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
	msgText.setMaxWidth(video->getTvHeight(), GuiText::DOTTED);
    msgText.setPosition(0, - 324);
	
	append(&Image);	
	append(&versionText);
	append(&msgText);	
	
	fadeIn();
	
}

MainStartUp::~MainStartUp()
{
	
	while(!elements.empty())
    {
        delete elements[0];
        remove(elements[0]);
    }
	
	Resources::RemoveImageData(ImageData);
}

void MainStartUp::draw()
{
    //! start rendering Drc
	video->prepareDrcRendering();
	for(u32 i = 0; i < elements.size(); ++i)
    {
        elements[i]->draw(video);
    }
	video->drcDrawDone();
	
	//! start rendering Tv
	video->prepareTvRendering();
	for(u32 i = 0; i < elements.size(); ++i)
    {
        elements[i]->draw(video);
    }
	video->tvDrawDone();
	
	//! enable screen after first frame render
	if(video->getFrameCount() == 0) {
        video->tvEnable(true);
        video->drcEnable(true);
	}

	//! as last point update the effects as it can drop elements
	video->waitForVSync();

    //! transfer elements to real delete list here after all processes are finished
    //! the elements are transfered to another list to delete the elements in a separate thread
    //! and avoid blocking the GUI thread
    AsyncDeleter::triggerDeleteProcess();	
}

void MainStartUp::textFade(int direction)
{
	if(direction > 0)
	{
		for(int i = 0; i < 255; i += direction)
		{
			msgText.setAlpha(i);
			draw();
		}
		msgText.setAlpha(255);
		draw();
	}
	else if(direction < 0)
	{
		for(int i = 255; i > 0; i += direction)
		{
			msgText.setAlpha(i);
			draw();
		}
		msgText.setAlpha(0);
		draw();
	}
}

void MainStartUp::SetText(std::string msg)
{
    if(!msg.empty())
	{
		textFade(-50);
		msgText.setTextf(tr(msg.c_str()));
		textFade(50);
	}
	
	msg.clear();	
}

void MainStartUp::fadeIn()
{
    GuiImage fadeIn(video->getTvWidth(), video->getTvHeight(), (GX2Color){ 0, 0, 0, 255 });

	for(int i = 255; i > 0; i -= 10)
    {
        if(i < 0)
            i = 0;

        fadeIn.setAlpha(i / 255.0f);

	    video->prepareDrcRendering();
	    
		for(u32 i = 0; i < elements.size(); ++i)
		{
			elements[i]->draw(video);
		}

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS);
        fadeIn.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_LEQUAL);

	    video->drcDrawDone();

	    video->prepareTvRendering();

	    for(u32 i = 0; i < elements.size(); ++i)
		{
			elements[i]->draw(video);
		}

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS);
        fadeIn.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_LEQUAL);

	    video->tvDrawDone();

	    video->waitForVSync();
    }

}