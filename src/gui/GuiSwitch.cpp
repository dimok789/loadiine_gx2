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
#include "GuiSwitch.h"
#include "GuiImage.h"
#include "GuiImageData.h"
/**
 * Constructor for the GuiSwitch class.
 */

GuiSwitch::GuiSwitch(bool checked,f32 switchscale)
 : GuiToggle(checked,90*switchscale,38*switchscale)
 ,switchbase_imgdata(Resources::GetImageData("switchIconBase.png"))
 ,switchbase_img(switchbase_imgdata)
 ,switchbase_highlighted_imgdata(Resources::GetImageData("switchIconBaseHighlighted.png"))
 ,switchbase_highlighted_img(switchbase_highlighted_imgdata)
 ,switchOn_imgdata(Resources::GetImageData("switchIconOn.png"))
 ,switchOn_img(switchOn_imgdata)
 ,switchOff_imgdata(Resources::GetImageData("switchIconOff.png"))
 ,switchOff_img(switchOff_imgdata)
{
    f32 scale = 0.0;
    if(switchbase_img.getHeight() > switchbase_img.getWidth()){
        scale = height*switchscale/switchbase_img.getHeight();
    }else{
        scale = width/switchbase_img.getWidth();
    }

    switchbase_img.setScale(scale);
    switchbase_highlighted_img.setScale(scale);
    switchOn_img.setScale(scale);
    switchOff_img.setScale(scale);

    switchOn_img.setParent(this);
    switchOn_img.setPosition((width/4.0),0);
    switchOff_img.setParent(this);
    switchOff_img.setPosition(-((width/4.0)),0);
    setImage(&switchbase_img);
    setIconOver(&switchbase_highlighted_img);
}
/**
 * Destructor for the GuiButton class.
 */
GuiSwitch::~GuiSwitch()
{
    Resources::RemoveImageData(switchbase_imgdata);
    Resources::RemoveImageData(switchbase_highlighted_imgdata);
    Resources::RemoveImageData(switchOn_imgdata);
    Resources::RemoveImageData(switchOff_imgdata);
}

void GuiSwitch::draw(CVideo *v){
    GuiToggle::draw(v);
    if(getValue()){
        switchOn_img.draw(v);
    }else{
        switchOff_img.draw(v);
    }
}
