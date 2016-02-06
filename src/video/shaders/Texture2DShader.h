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
#ifndef __TEXTURE_2D_SHADER_H_
#define __TEXTURE_2D_SHADER_H_

#include "VertexShader.h"
#include "PixelShader.h"
#include "FetchShader.h"

class Texture2DShader : public Shader
{
private:
    Texture2DShader();
    virtual ~Texture2DShader();

    static const u32 cuAttributeCount = 2;
    static const u32 ciPositionVtxsSize = 4 * cuVertexAttrSize;
    static const u32 ciTexCoordsVtxsSize = 4 * cuTexCoordAttrSize;

    static Texture2DShader *shaderInstance;

    FetchShader *fetchShader;
    VertexShader vertexShader;
    PixelShader pixelShader;

    f32 *posVtxs;
    f32 *texCoords;

    u32 angleLocation;
    u32 offsetLocation;
    u32 scaleLocation;
    u32 colorIntensityLocation;
    u32 blurLocation;
    u32 samplerLocation;
    u32 positionLocation;
    u32 texCoordLocation;
public:
    static Texture2DShader *instance() {
        if(!shaderInstance) {
            shaderInstance = new Texture2DShader();
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

    void setAttributeBuffer(const f32 * texCoords_in = NULL, const f32 * posVtxs_in = NULL, const u32 & vtxCount = 0) const
    {
        if(posVtxs_in && texCoords_in && vtxCount)
        {
            VertexShader::setAttributeBuffer(0, vtxCount * cuVertexAttrSize, cuVertexAttrSize, posVtxs_in);
            VertexShader::setAttributeBuffer(1, vtxCount * cuTexCoordAttrSize, cuTexCoordAttrSize, texCoords_in);
        }
        else {
            VertexShader::setAttributeBuffer(0, ciPositionVtxsSize, cuVertexAttrSize, posVtxs);
            VertexShader::setAttributeBuffer(1, ciTexCoordsVtxsSize, cuTexCoordAttrSize, texCoords);
        }
    }

    void setAngle(const float & val)
    {
        VertexShader::setUniformReg(angleLocation, 4, &val);
    }
    void setOffset(const glm::vec3 & vec)
    {
        VertexShader::setUniformReg(offsetLocation, 4, &vec[0]);
    }
    void setScale(const glm::vec3 & vec)
    {
        VertexShader::setUniformReg(scaleLocation, 4, &vec[0]);
    }
    void setColorIntensity(const glm::vec4 & vec)
    {
        PixelShader::setUniformReg(colorIntensityLocation, 4, &vec[0]);
    }
    void setBlurring(const glm::vec3 & vec)
    {
        PixelShader::setUniformReg(blurLocation, 4, &vec[0]);
    }

    void setTextureAndSampler(const GX2Texture *texture, const GX2Sampler *sampler) const {
        GX2SetPixelTexture(texture, samplerLocation);
        GX2SetPixelSampler(sampler, samplerLocation);
    }
};

#endif // __TEXTURE_2D_SHADER_H_
