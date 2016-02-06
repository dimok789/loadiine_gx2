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
#ifndef FETCH_SHADER_H
#define FETCH_SHADER_H

#include "Shader.h"

class FetchShader : public Shader
{
public:
    FetchShader(GX2AttribStream * attributes, u32 attrCount, s32 type = GX2_FETCH_SHADER_TESSELATION_NONE, s32 tess = GX2_TESSELLATION_MODE_DISCRETE)
        : fetchShader(NULL)
        , fetchShaderProgramm(NULL)
    {
        u32 shaderSize = GX2CalcFetchShaderSizeEx(attrCount, type, tess);
        fetchShaderProgramm = memalign(GX2_SHADER_ALIGNMENT, shaderSize);
        if(fetchShaderProgramm)
        {
            fetchShader = new GX2FetchShader;
            GX2InitFetchShaderEx(fetchShader, fetchShaderProgramm, attrCount, attributes, type, tess);
            GX2Invalidate(GX2_INVALIDATE_CPU_SHADER, fetchShaderProgramm, shaderSize);
        }
    }
    virtual ~FetchShader() {
        if(fetchShaderProgramm)
            free(fetchShaderProgramm);
        if(fetchShader)
            delete fetchShader;
    }

    GX2FetchShader *getFetchShader() const {
        return fetchShader;
    }

    void setShader(void) const {
        GX2SetFetchShader(fetchShader);
    }

protected:
    GX2FetchShader *fetchShader;
    void *fetchShaderProgramm;
};

#endif // FETCH_SHADER_H
