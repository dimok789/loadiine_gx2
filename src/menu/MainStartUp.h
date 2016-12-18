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
#ifndef _MAIN_STARTUP_H_
#define _MAIN_STARTUP_H_

#include <vector>
#include "gui/Gui.h"
#include "gui/GuiParticleImage.h"

class CVideo;

class MainStartUp
{
public:
    MainStartUp(CVideo *Video);
    virtual ~MainStartUp();

	void SetText(std::string msg);
	
private:

    void append(GuiElement *e)
    {
        if(!e)
            return;

        remove(e);
        elements.push_back(e);
    }
    
    void remove(GuiElement *e)
    {
        for(u32 i = 0; i < elements.size(); ++i)
        {
            if(e == elements[i])
            {
                elements.erase(elements.begin() + i);
                break;
            }
        }
    }
	
	void draw();

	void textFade(int direction);
	void fadeIn(void);
	
    int width, height;
    std::vector<GuiElement *> elements;
	
	CVideo *video;
    GuiParticleImage particleBgImage;
	GuiImageData *ImageData;
    GuiImage Image;
	GuiText versionText;
	GuiText msgText;

};

#endif //_MAIN_STARTUP_H_
