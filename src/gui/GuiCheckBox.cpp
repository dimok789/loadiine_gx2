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
#include "GuiCheckBox.h"
#include "GuiImage.h"
#include "GuiImageData.h"
/**
 * Constructor for the GuiCheckBox class.
 */

GuiCheckBox::GuiCheckBox(bool checked)
 : GuiToggle(checked,50,50)
 ,checkbox_imgdata(Resources::GetImageData("checkbox.png"))
 ,checkbox_img(checkbox_imgdata)
 ,checkbox_selected_imgdata(Resources::GetImageData("checkbox_selected.png"))
 ,checkbox_selected_img(checkbox_selected_imgdata)
 ,highlighted_imgdata(Resources::GetImageData("checkbox_highlighted.png"))
 ,highlighted_img(highlighted_imgdata)
{
    checkbox_selected_img.setScale(height/checkbox_selected_img.getHeight());
    checkbox_img.setScale(height/checkbox_img.getHeight());
    highlighted_img.setScale(height/highlighted_img.getHeight());

    setImage(&checkbox_img);
    setIconOver(&highlighted_img);
}

/**
 * Destructor for the GuiButton class.
 */
GuiCheckBox::~GuiCheckBox()
{
    Resources::RemoveImageData(checkbox_imgdata);
    Resources::RemoveImageData(checkbox_selected_imgdata);
    Resources::RemoveImageData(highlighted_imgdata);
}


void GuiCheckBox::update(GuiController * c){
    if(bChanged){
        if(selected){
            GuiButton::setImage(&checkbox_selected_img);
        }else{
            GuiButton::setImage(&checkbox_img);
        }
        bChanged = false;
    }
     GuiToggle::update(c);
}

