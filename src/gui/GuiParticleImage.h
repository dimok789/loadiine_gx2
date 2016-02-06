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
#ifndef _GUI_PARTICLE_IMAGE_H_
#define _GUI_PARTICLE_IMAGE_H_

#include "GuiImage.h"

class GuiParticleImage : public GuiImage, public sigslot::has_slots<>
{
public:
    GuiParticleImage(int w, int h, u32 particleCount);
    virtual ~GuiParticleImage();

    void draw(CVideo *pVideo);
private:
    f32 *posVertexs;
    u8 *colorVertexs;

    typedef struct
    {
        glm::vec3 position;
        glm::vec4 colors;
        f32 radius;
        f32 speed;
        f32 direction;
    } Particle;

    std::vector<Particle> particles;
};

#endif // _GUI_ICON_GRID_H_
