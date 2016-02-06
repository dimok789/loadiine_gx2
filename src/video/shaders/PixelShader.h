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
#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

#include "Shader.h"

class PixelShader : public Shader
{
public:
    PixelShader()
        : pixelShader((GX2PixelShader*) memalign(0x40, sizeof(GX2PixelShader)))
    {
        if(pixelShader)
        {
            memset(pixelShader, 0, sizeof(GX2PixelShader));
            pixelShader->shader_mode = GX2_SHADER_MODE_UNIFORM_REGISTER;
        }
    }
    virtual ~PixelShader()
    {
        if(pixelShader)
        {
            if(pixelShader->shader_data)
                free(pixelShader->shader_data);

            for(u32 i = 0; i < pixelShader->uniform_blocks_count; i++)
                free((void*)pixelShader->uniform_block[i].name);

            if(pixelShader->uniform_block)
                free((void*)pixelShader->uniform_block);

            for(u32 i = 0; i < pixelShader->uniform_vars_count; i++)
                free((void*)pixelShader->uniform_var[i].name);

            if(pixelShader->uniform_var)
                free((void*)pixelShader->uniform_var);

            if(pixelShader->initial_value)
                free((void*)pixelShader->initial_value);

            for(u32 i = 0; i < pixelShader->sampler_vars_count; i++)
                free((void*)pixelShader->sampler_var[i].name);

            if(pixelShader->sampler_var)
                free((void*)pixelShader->sampler_var);

            if(pixelShader->loops_data)
                free((void*)pixelShader->loops_data);

            free(pixelShader);
        }
    }

    void setProgram(const u32 * program, const u32 & programSize, const u32 * regs, const u32 & regsSize)
    {
        if(!pixelShader)
            return;

        //! this must be moved into an area where the graphic engine has access to and must be aligned to 0x100
        pixelShader->shader_size = programSize;
        pixelShader->shader_data = memalign(GX2_SHADER_ALIGNMENT, pixelShader->shader_size);
        if(pixelShader->shader_data)
        {
            memcpy(pixelShader->shader_data, program, pixelShader->shader_size);
            GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, pixelShader->shader_data, pixelShader->shader_size);
        }

        memcpy(pixelShader->regs, regs, regsSize);
    }

    void addUniformVar(const GX2UniformVar & var)
    {
        if(!pixelShader)
            return;

        u32 idx = pixelShader->uniform_vars_count;

        GX2UniformVar* newVar = (GX2UniformVar*) malloc((pixelShader->uniform_vars_count + 1) * sizeof(GX2UniformVar));
        if(newVar)
        {
            if(pixelShader->uniform_var)
            {
                memcpy(newVar, pixelShader->uniform_var, pixelShader->uniform_vars_count * sizeof(GX2UniformVar));
                free(pixelShader->uniform_var);
            }
            pixelShader->uniform_var = newVar;

            memcpy(pixelShader->uniform_var + idx, &var, sizeof(GX2UniformVar));
            pixelShader->uniform_var[idx].name = (char*) malloc(strlen(var.name) + 1);
            strcpy((char*)pixelShader->uniform_var[idx].name, var.name);

            pixelShader->uniform_vars_count++;
        }
    }

    void addSamplerVar(const GX2SamplerVar & var)
    {
        if(!pixelShader)
            return;

        u32 idx = pixelShader->sampler_vars_count;

        GX2SamplerVar* newVar = (GX2SamplerVar*) malloc((pixelShader->sampler_vars_count + 1) * sizeof(GX2SamplerVar));
        if(newVar)
        {
            if(pixelShader->sampler_var)
            {
                memcpy(newVar, pixelShader->sampler_var, pixelShader->sampler_vars_count * sizeof(GX2SamplerVar));
                free(pixelShader->sampler_var);
            }
            pixelShader->sampler_var = newVar;

            memcpy(pixelShader->sampler_var + idx, &var, sizeof(GX2SamplerVar));
            pixelShader->sampler_var[idx].name = (char*) malloc(strlen(var.name) + 1);
            strcpy((char*)pixelShader->sampler_var[idx].name, var.name);

            pixelShader->sampler_vars_count++;
        }
    }
    GX2PixelShader * getPixelShader() const {
        return pixelShader;
    }

    void setShader(void) const {
        GX2SetPixelShader(pixelShader);
    }

    static inline void setUniformReg(u32 location, u32 size, const void * reg) {
        GX2SetPixelUniformReg(location, size, reg);
    }
protected:
    GX2PixelShader *pixelShader;
};

#endif // PIXEL_SHADER_H
