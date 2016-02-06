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
#include <malloc.h>
#include <string.h>
#include "ColorShader.h"

static const u32 cpVertexShaderProgram[] =
{
    0x00000000,0x00008009,0x20000000,0x000078a0,
    0x3c200000,0x88060094,0x00c00000,0x88062014,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00a11f00,0xfc00620f,0x02490001,0x80000040,
    0xfd041f80,0x900c0060,0x83f9223e,0x0000803f,
    0xfe282001,0x10000040,0xfe001f80,0x00080060,
    0xfeac9f80,0xfd00624f,0xdb0f49c0,0xdb0fc940,
    0xfea81f80,0x9000e02f,0x83f9223e,0x00000000,
    0xfe041f80,0x00370000,0xffa01f00,0x80000000,
    0xff101f00,0x800c0020,0x7f041f80,0x80370000,
    0x0000103f,0x00000000,0x02c51f00,0x80000000,
    0xfea41f00,0x80000020,0xffa09f00,0x80000040,
    0xff001f80,0x800c0060,0x398ee33f,0x0000103f,
    0x02c41f00,0x9000e00f,0x02c59f01,0x80000020,
    0xfea81f00,0x80000040,0x02c19f80,0x9000e06f,
    0x398ee33f,0x00000000,0x02c11f01,0x80000000,
    0x02c49f80,0x80000060,0x02e08f01,0xfe0c620f,
    0x02c01f80,0x7f00622f,0xfe242000,0x10000000,
    0xfe20a080,0x10000020,0xf2178647,0x49c0e9fb,
    0xfbbdb2ab,0x768ac733
};

static const u32 cpVertexShaderRegs[] = {
    0x00000103,0x00000000,0x00000000,0x00000001,
    0xffffff00,0xffffffff,0xffffffff,0xffffffff,
    0xffffffff,0xffffffff,0xffffffff,0xffffffff,
    0xffffffff,0xffffffff,0x00000000,0xfffffffc,
    0x00000002,0x00000001,0x00000000,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x000000ff,0x000000ff,0x000000ff,
    0x000000ff,0x00000000,0x0000000e,0x00000010
};

static const u32 cpPixelShaderProgram[] =
{
    0x20000000,0x00000ca0,0x00000000,0x88062094,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00002000,0x90000000,0x0004a000,0x90000020,
    0x00082001,0x90000040,0x000ca081,0x90000060,
    0xbb7dd898,0x9746c59c,0xc69b00e7,0x03c36218
};
static const u32 cpPixelShaderRegs[] = {
    0x00000001,0x00000002,0x14000001,0x00000000,
    0x00000001,0x00000100,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x0000000f,0x00000001,0x00000010,
    0x00000000
};

ColorShader * ColorShader::shaderInstance = NULL;

ColorShader::ColorShader()
    : vertexShader(cuAttributeCount)
{
    //! create pixel shader
    pixelShader.setProgram(cpPixelShaderProgram, sizeof(cpPixelShaderProgram), cpPixelShaderRegs, sizeof(cpPixelShaderRegs));

    colorIntensityLocation = 0;
    pixelShader.addUniformVar((GX2UniformVar){ "unf_color_intensity", GX2_VAR_TYPE_VEC4, 1, colorIntensityLocation, 0xffffffff });

    //! create vertex shader
    vertexShader.setProgram(cpVertexShaderProgram, sizeof(cpVertexShaderProgram), cpVertexShaderRegs, sizeof(cpVertexShaderRegs));

    angleLocation = 0;
    offsetLocation = 4;
    scaleLocation = 8;
    vertexShader.addUniformVar((GX2UniformVar){ "unf_angle", GX2_VAR_TYPE_FLOAT, 1, angleLocation, 0xffffffff });
    vertexShader.addUniformVar((GX2UniformVar){ "unf_offset", GX2_VAR_TYPE_VEC3, 1, offsetLocation, 0xffffffff });
    vertexShader.addUniformVar((GX2UniformVar){ "unf_scale", GX2_VAR_TYPE_VEC3, 1, scaleLocation, 0xffffffff });

    colorLocation = 1;
    positionLocation = 0;
    vertexShader.addAttribVar((GX2AttribVar){ "attr_color", GX2_VAR_TYPE_VEC4, 0, colorLocation });
    vertexShader.addAttribVar((GX2AttribVar){ "attr_position", GX2_VAR_TYPE_VEC3, 0, positionLocation });

    //! setup attribute streams
    GX2InitAttribStream(vertexShader.getAttributeBuffer(0), positionLocation, 0, 0, GX2_ATTRIB_FORMAT_32_32_32_FLOAT);
    GX2InitAttribStream(vertexShader.getAttributeBuffer(1), colorLocation, 1, 0, GX2_ATTRIB_FORMAT_8_8_8_8_UNORM);

    //! create fetch shader
    fetchShader = new FetchShader(vertexShader.getAttributeBuffer(), vertexShader.getAttributesCount());

    //! model vertex has to be align and cannot be in unknown regions for GX2 like 0xBCAE1000
    positionVtxs = (f32*)memalign(GX2_VERTEX_BUFFER_ALIGNMENT, cuPositionVtxsSize);
    if(positionVtxs)
    {
        //! position vertex structure
        int i = 0;
        positionVtxs[i++] = -1.0f; positionVtxs[i++] = -1.0f; positionVtxs[i++] = 0.0f;
        positionVtxs[i++] =  1.0f; positionVtxs[i++] = -1.0f; positionVtxs[i++] = 0.0f;
        positionVtxs[i++] =  1.0f; positionVtxs[i++] =  1.0f; positionVtxs[i++] = 0.0f;
        positionVtxs[i++] = -1.0f; positionVtxs[i++] =  1.0f; positionVtxs[i++] = 0.0f;
        GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, positionVtxs, cuPositionVtxsSize);
    }
}

ColorShader::~ColorShader()
{
    if(positionVtxs)
    {
        free(positionVtxs);
        positionVtxs = NULL;
    }

    delete fetchShader;
    fetchShader = NULL;
}
