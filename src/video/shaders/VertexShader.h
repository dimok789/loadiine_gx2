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
#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

#include <string.h>
#include "Shader.h"

class VertexShader : public Shader
{
public:
    VertexShader(u32 numAttr)
        : attributesCount( numAttr )
        , attributes( new GX2AttribStream[attributesCount] )
        , vertexShader( (GX2VertexShader*) memalign(0x40, sizeof(GX2VertexShader)) )
    {
        if(vertexShader)
        {
            memset(vertexShader, 0, sizeof(GX2VertexShader));
            vertexShader->shader_mode = GX2_SHADER_MODE_UNIFORM_REGISTER;
        }
    }

    virtual ~VertexShader() {
        delete [] attributes;

        if(vertexShader)
        {
            if(vertexShader->shader_data)
                free(vertexShader->shader_data);

            for(u32 i = 0; i < vertexShader->uniform_blocks_count; i++)
                free((void*)vertexShader->uniform_block[i].name);

            if(vertexShader->uniform_block)
                free((void*)vertexShader->uniform_block);

            for(u32 i = 0; i < vertexShader->uniform_vars_count; i++)
                free((void*)vertexShader->uniform_var[i].name);

            if(vertexShader->uniform_var)
                free((void*)vertexShader->uniform_var);

            if(vertexShader->initial_value)
                free((void*)vertexShader->initial_value);

            for(u32 i = 0; i < vertexShader->sampler_vars_count; i++)
                free((void*)vertexShader->sampler_var[i].name);

            if(vertexShader->sampler_var)
                free((void*)vertexShader->sampler_var);

            for(u32 i = 0; i < vertexShader->attribute_vars_count; i++)
                free((void*)vertexShader->attribute_var[i].name);

            if(vertexShader->attribute_var)
                free((void*)vertexShader->attribute_var);

            if(vertexShader->loops_data)
                free((void*)vertexShader->loops_data);

            free(vertexShader);
        }
    }

    void setProgram(const u32 * program, const u32 & programSize, const u32 * regs, const u32 & regsSize)
    {
        if(!vertexShader)
            return;

        //! this must be moved into an area where the graphic engine has access to and must be aligned to 0x100
        vertexShader->shader_size = programSize;
        vertexShader->shader_data = memalign(GX2_SHADER_ALIGNMENT, vertexShader->shader_size);
        if(vertexShader->shader_data)
        {
            memcpy(vertexShader->shader_data, program, vertexShader->shader_size);
            GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, vertexShader->shader_data, vertexShader->shader_size);
        }

        memcpy(vertexShader->regs, regs, regsSize);
    }

    void addUniformVar(const GX2UniformVar & var)
    {
        if(!vertexShader)
            return;

        u32 idx = vertexShader->uniform_vars_count;

        GX2UniformVar* newVar = (GX2UniformVar*) malloc((vertexShader->uniform_vars_count + 1) * sizeof(GX2UniformVar));
        if(newVar)
        {
            if(vertexShader->uniform_vars_count > 0)
            {
                memcpy(newVar, vertexShader->uniform_var, vertexShader->uniform_vars_count * sizeof(GX2UniformVar));
                free(vertexShader->uniform_var);
            }
            vertexShader->uniform_var = newVar;

            memcpy(vertexShader->uniform_var + idx, &var, sizeof(GX2UniformVar));
            vertexShader->uniform_var[idx].name = (char*) malloc(strlen(var.name) + 1);
            strcpy((char*)vertexShader->uniform_var[idx].name, var.name);

            vertexShader->uniform_vars_count++;
        }
    }

    void addAttribVar(const GX2AttribVar & var)
    {
        if(!vertexShader)
            return;

        u32 idx = vertexShader->attribute_vars_count;

        GX2AttribVar* newVar = (GX2AttribVar*) malloc((vertexShader->attribute_vars_count + 1) * sizeof(GX2AttribVar));
        if(newVar)
        {
            if(vertexShader->attribute_vars_count > 0)
            {
                memcpy(newVar, vertexShader->attribute_var, vertexShader->attribute_vars_count * sizeof(GX2AttribVar));
                free(vertexShader->attribute_var);
            }
            vertexShader->attribute_var = newVar;

            memcpy(vertexShader->attribute_var + idx, &var, sizeof(GX2AttribVar));
            vertexShader->attribute_var[idx].name = (char*) malloc(strlen(var.name) + 1);
            strcpy((char*)vertexShader->attribute_var[idx].name, var.name);

            vertexShader->attribute_vars_count++;
        }
    }

    static inline void setAttributeBuffer(u32 bufferIdx, u32 bufferSize, u32 stride, const void * buffer) {
        GX2SetAttribBuffer(bufferIdx, bufferSize, stride, buffer);
    }

    GX2VertexShader *getVertexShader() const {
        return vertexShader;
    }

    void setShader(void) const {
        GX2SetVertexShader(vertexShader);
    }

    GX2AttribStream * getAttributeBuffer(u32 idx = 0) const {
        if(idx >= attributesCount) {
            return NULL;
        }
        return &attributes[idx];
    }
    u32 getAttributesCount() const {
        return attributesCount;
    }

    static void setUniformReg(u32 location, u32 size, const void * reg) {
        GX2SetVertexUniformReg(location, size, reg);
    }
protected:
    u32 attributesCount;
    GX2AttribStream *attributes;
    GX2VertexShader *vertexShader;
};

#endif // VERTEX_SHADER_H
