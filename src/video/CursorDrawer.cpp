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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynamic_libs/gx2_functions.h"
#include "video/shaders/ColorShader.h"
#include "CursorDrawer.h"

CursorDrawer *CursorDrawer::instance = NULL;

CursorDrawer::CursorDrawer()
{
    init_colorVtxs();
}

CursorDrawer::~CursorDrawer()
{
    //! destroy shaders
    ColorShader::destroyInstance();
    if(this->colorVtxs){
        free(this->colorVtxs);
        this->colorVtxs = NULL;
    }
}

void CursorDrawer::init_colorVtxs(){
    if(!this->colorVtxs){
        this->colorVtxs = (u8*)memalign(0x40, sizeof(u8) * 16);
        if(this->colorVtxs == NULL) return;

    }
    memset(this->colorVtxs,0xFF,16*sizeof(u8));

    GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, this->colorVtxs, 16 * sizeof(u8));
}

// Could be improved. It be more generic.
void CursorDrawer::draw_Cursor(f32 x,f32 y)
{
    if(this->colorVtxs == NULL){
        init_colorVtxs();
        return;
    }

    f32 widthScaleFactor = 1.0f / (f32)1280;
    f32 heightScaleFactor = 1.0f / (f32)720;

    int width = 20;

    glm::vec3 positionOffsets = glm::vec3(0.0f);

    positionOffsets[0] = (x-((1280)/2)+(width/2)) * widthScaleFactor * 2.0f;
    positionOffsets[1] = -(y-((720)/2)+(width/2)) * heightScaleFactor * 2.0f;

    glm::vec3 scale(width*widthScaleFactor,width*heightScaleFactor,1.0f);

    ColorShader::instance()->setShaders();
    ColorShader::instance()->setAttributeBuffer(this->colorVtxs, NULL, 4);
    ColorShader::instance()->setAngle(0);
    ColorShader::instance()->setOffset(positionOffsets);
    ColorShader::instance()->setScale(scale);
    ColorShader::instance()->setColorIntensity(glm::vec4(1.0f));
    ColorShader::instance()->draw(GX2_PRIMITIVE_QUADS, 4);
}
