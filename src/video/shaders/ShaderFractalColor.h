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
#ifndef SHADER_FRACTAL_COLOR_H_
#define SHADER_FRACTAL_COLOR_H_

#include "VertexShader.h"
#include "PixelShader.h"
#include "FetchShader.h"

class ShaderFractalColor : public Shader
{
private:
    ShaderFractalColor();
    virtual ~ShaderFractalColor();

    static ShaderFractalColor * shaderInstance;

    static const unsigned char cuAttributeCount = 3;
    static const u32 ciPositionVtxsSize = 4 * cuVertexAttrSize;
    static const u32 ciTexCoordsVtxsSize = 4 * cuTexCoordAttrSize;
    static const u32 ciColorVtxsSize = 4 * cuColorAttrSize;

    FetchShader *fetchShader;
    VertexShader vertexShader;
    PixelShader pixelShader;

    f32 *posVtxs;
    f32 *texCoords;
    u8 *colorVtxs;

    u32 modelMatrixLocation;
    u32 viewMatrixLocation;
    u32 projectionMatrixLocation;
    u32 positionLocation;
    u32 colorLocation;
    u32 texCoordLocation;

    u32 blurLocation;
    u32 colorIntensityLocation;
    u32 fadeOutLocation;
    u32 fractalLocation;
public:
    static ShaderFractalColor *instance() {
        if(!shaderInstance) {
            shaderInstance = new ShaderFractalColor();
        }
        return shaderInstance;
    }
    static void destroyInstance() {
        if(shaderInstance) {
            delete shaderInstance;
            shaderInstance = NULL;
        }
    }

    void setShaders(void) const
    {
        fetchShader->setShader();
        vertexShader.setShader();
        pixelShader.setShader();
    }

    void setAttributeBuffer(const u32 & vtxCount = 0, const f32 * posVtxs_in = NULL, const f32 * texCoords_in = NULL, const u8 * colorVtxs_in = NULL) const
    {
        if(posVtxs_in && texCoords_in && vtxCount)
        {
            VertexShader::setAttributeBuffer(0, vtxCount * cuVertexAttrSize, cuVertexAttrSize, posVtxs_in);
            VertexShader::setAttributeBuffer(1, vtxCount * cuTexCoordAttrSize, cuTexCoordAttrSize, texCoords_in);
            VertexShader::setAttributeBuffer(2, vtxCount * cuColorAttrSize, cuColorAttrSize, colorVtxs_in);
        }
        else {
            //! use default quad vertex and texture coordinates if nothing is passed
            VertexShader::setAttributeBuffer(0, ciPositionVtxsSize, cuVertexAttrSize, posVtxs);
            VertexShader::setAttributeBuffer(1, ciTexCoordsVtxsSize, cuTexCoordAttrSize, texCoords);
            VertexShader::setAttributeBuffer(2, ciColorVtxsSize, cuColorAttrSize, colorVtxs);
        }
    }

    void setProjectionMtx(const glm::mat4 & mtx)
    {
        VertexShader::setUniformReg(projectionMatrixLocation, 16, &mtx[0][0]);
    }
    void setViewMtx(const glm::mat4 & mtx)
    {
        VertexShader::setUniformReg(viewMatrixLocation, 16, &mtx[0][0]);
    }
    void setModelViewMtx(const glm::mat4 & mtx)
    {
        VertexShader::setUniformReg(modelMatrixLocation, 16, &mtx[0][0]);
    }

    void setBlurBorder(const float & blurBorderSize)
    {
        PixelShader::setUniformReg(blurLocation, 4, &blurBorderSize);
    }
    void setColorIntensity(const glm::vec4 & vec)
    {
        PixelShader::setUniformReg(colorIntensityLocation, 4, &vec[0]);
    }
    void setAlphaFadeOut(const glm::vec4 & vec)
    {
        PixelShader::setUniformReg(fadeOutLocation, 4, &vec[0]);
    }
    void setFractalColor(const int & fractalColorEnable)
    {
        PixelShader::setUniformReg(fractalLocation, 4, &fractalColorEnable);
    }
};

#endif // SHADER_FRACTAL_COLOR_H_
