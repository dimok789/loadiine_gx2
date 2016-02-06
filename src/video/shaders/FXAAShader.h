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
#ifndef __FXAA_SHADER_H_
#define __FXAA_SHADER_H_

#include "VertexShader.h"
#include "PixelShader.h"
#include "FetchShader.h"

class FXAAShader : public Shader
{
public:
    static FXAAShader *instance() {
        if(!shaderInstance) {
            shaderInstance = new FXAAShader();
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

    void setAttributeBuffer() const
    {
        VertexShader::setAttributeBuffer(0, ciPositionVtxsSize, cuVertexAttrSize, posVtxs);
        VertexShader::setAttributeBuffer(1, ciTexCoordsVtxsSize, cuTexCoordAttrSize, texCoords);
    }

    void setResolution(const glm::vec2 & vec)
    {
        PixelShader::setUniformReg(resolutionLocation, 4, &vec[0]);
    }

    void setTextureAndSampler(const GX2Texture *texture, const GX2Sampler *sampler) const {
        GX2SetPixelTexture(texture, samplerLocation);
        GX2SetPixelSampler(sampler, samplerLocation);
    }

private:
    FXAAShader();
    virtual ~FXAAShader();

    static const u32 cuAttributeCount = 2;
    static const u32 ciPositionVtxsSize = 4 * cuVertexAttrSize;
    static const u32 ciTexCoordsVtxsSize = 4 * cuTexCoordAttrSize;

    static FXAAShader *shaderInstance;

    FetchShader *fetchShader;
    VertexShader vertexShader;
    PixelShader pixelShader;

    f32 *posVtxs;
    f32 *texCoords;

    u32 samplerLocation;
    u32 positionLocation;
    u32 texCoordLocation;
    u32 resolutionLocation;
};

#endif // __FXAA_SHADER_H_
