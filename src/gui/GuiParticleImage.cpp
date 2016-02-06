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
#include "GuiParticleImage.h"
#include "video/CVideo.h"
#include "video/shaders/ColorShader.h"

#define CIRCLE_VERTEX_COUNT     36

static inline f32 getRandZeroToOneF32()
{
    return (rand() % 10000) * 0.0001f;
}

static inline f32 getRandMinusOneToOneF32()
{
    return getRandZeroToOneF32() * 2.0f - 1.0f;
}

GuiParticleImage::GuiParticleImage(int w, int h, u32 particleCount)
    : GuiImage(NULL)
{
    width = w;
    height = h;
	imgType = IMAGE_COLOR;

    posVertexs = (f32 *) memalign(GX2_VERTEX_BUFFER_ALIGNMENT, ColorShader::cuVertexAttrSize * CIRCLE_VERTEX_COUNT);
    colorVertexs = (u8 *) memalign(GX2_VERTEX_BUFFER_ALIGNMENT, ColorShader::cuColorAttrSize * CIRCLE_VERTEX_COUNT);

    for(u32 i = 0; i < CIRCLE_VERTEX_COUNT; i++)
    {
        posVertexs[i * 3 + 0] = cosf(DegToRad(i * 360.0f / CIRCLE_VERTEX_COUNT));
        posVertexs[i * 3 + 1] = sinf(DegToRad(i * 360.0f / CIRCLE_VERTEX_COUNT));
        posVertexs[i * 3 + 2] = 0.0f;

        colorVertexs[i * 4 + 0] = 0xff;
        colorVertexs[i * 4 + 1] = 0xff;
        colorVertexs[i * 4 + 2] = 0xff;
        colorVertexs[i * 4 + 3] = 0xff;
    }
    GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, posVertexs, ColorShader::cuVertexAttrSize * CIRCLE_VERTEX_COUNT);
    GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, colorVertexs, ColorShader::cuColorAttrSize * CIRCLE_VERTEX_COUNT);

    particles.resize(particleCount);

    for(u32 i = 0; i < particleCount; i++)
    {
        particles[i].position.x = getRandMinusOneToOneF32() * getWidth() * 0.5f;
        particles[i].position.y = getRandMinusOneToOneF32() * getHeight() * 0.5f;
        particles[i].position.z = 0.0f;
        particles[i].colors = glm::vec4(1.0f, 1.0f, 1.0f, (getRandZeroToOneF32() * 0.6f) + 0.05f);
        particles[i].radius = getRandZeroToOneF32() * 30.0f;
        particles[i].speed = (getRandZeroToOneF32() * 0.6f) + 0.2f;
        particles[i].direction = getRandMinusOneToOneF32();
    }
}

GuiParticleImage::~GuiParticleImage()
{
    free(posVertexs);
    free(colorVertexs);
}

void GuiParticleImage::draw(CVideo *pVideo)
{
	if(!this->isVisible())
		return;


	f32 currScaleX = getScaleX();
	f32 currScaleY = getScaleY();

    positionOffsets[2] = getDepth() * pVideo->getDepthScaleFactor() * 2.0f;

    scaleFactor[2] = getScaleZ();

    //! add other colors intensities parameters
    colorIntensity[3] = getAlpha();

    for(u32 i = 0; i < particles.size(); ++i)
    {
        if(particles[i].position.y > (getHeight() * 0.5f + 30.0f))
        {
            particles[i].position.x = getRandMinusOneToOneF32() * getWidth() * 0.5f;
            particles[i].position.y = -getHeight() * 0.5f - 30.0f;
            particles[i].colors = glm::vec4(1.0f, 1.0f, 1.0f, (getRandZeroToOneF32() * 0.6f) + 0.05f);
            particles[i].radius = getRandZeroToOneF32() * 30.0f;
            particles[i].speed = (getRandZeroToOneF32() * 0.6f) + 0.2f;
            particles[i].direction = getRandMinusOneToOneF32();
        }
        if(particles[i].position.x < (-getWidth() * 0.5f - 50.0f))
        {
            particles[i].position.x = -particles[i].position.x;
        }


        particles[i].direction += getRandMinusOneToOneF32() * 0.03f;
        particles[i].position.x += particles[i].speed * particles[i].direction;
        particles[i].position.y += particles[i].speed;

        positionOffsets[0] = (getCenterX() + particles[i].position.x) * pVideo->getWidthScaleFactor() * 2.0f;
        positionOffsets[1] = (getCenterY() + particles[i].position.y) * pVideo->getHeightScaleFactor() * 2.0f;

        scaleFactor[0] = currScaleX * particles[i].radius * pVideo->getWidthScaleFactor();
        scaleFactor[1] = currScaleY * particles[i].radius * pVideo->getHeightScaleFactor();

        ColorShader::instance()->setShaders();
        ColorShader::instance()->setAttributeBuffer(colorVertexs, posVertexs, CIRCLE_VERTEX_COUNT);
        ColorShader::instance()->setAngle(0.0f);
        ColorShader::instance()->setOffset(positionOffsets);
        ColorShader::instance()->setScale(scaleFactor);
        ColorShader::instance()->setColorIntensity(colorIntensity * particles[i].colors);
        ColorShader::instance()->draw(GX2_PRIMITIVE_TRIANGLE_FAN, CIRCLE_VERTEX_COUNT);
    }
}
