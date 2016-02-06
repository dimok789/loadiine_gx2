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
#include "CVideo.h"
#include "system/memory.h"
#include "shaders/Texture2DShader.h"
#include "shaders/ColorShader.h"
#include "shaders/Shader3D.h"
#include "shaders/ShaderFractalColor.h"
#include "shaders/FXAAShader.h"
#include "dynamic_libs/os_functions.h"

CVideo::CVideo(s32 forceTvScanMode, s32 forceDrcScanMode)
{
    tvEnabled = false;
    drcEnabled = false;

    //! allocate MEM2 command buffer memory
    gx2CommandBuffer = MEM2_alloc(GX2_COMMAND_BUFFER_SIZE, 0x40);

    //! initialize GX2 command buffer
    u32 gx2_init_attributes[9];
    gx2_init_attributes[0] = GX2_INIT_ATTRIB_CB_BASE;
    gx2_init_attributes[1] = (u32)gx2CommandBuffer;
    gx2_init_attributes[2] = GX2_INIT_ATTRIB_CB_SIZE;
    gx2_init_attributes[3] = GX2_COMMAND_BUFFER_SIZE;
    gx2_init_attributes[4] = GX2_INIT_ATTRIB_ARGC;
    gx2_init_attributes[5] = 0;
    gx2_init_attributes[6] = GX2_INIT_ATTRIB_ARGV;
    gx2_init_attributes[7] = 0;
    gx2_init_attributes[8] = GX2_INIT_ATTRIB_NULL;
    GX2Init(gx2_init_attributes);

    //! GX2 resources are not used in this application but if needed, the allocator is setup
    GX2RSetAllocator(&CVideo::GX2RAlloc, &CVideo::GX2RFree);

    u32 scanBufferSize = 0;
    s32 scaleNeeded = 0;

    s32 tvScanMode = (forceTvScanMode >= 0) ? forceTvScanMode : GX2GetSystemTVScanMode();
    s32 drcScanMode = (forceDrcScanMode >= 0) ? forceDrcScanMode : GX2GetSystemDRCScanMode();

    s32 tvRenderMode;
    u32 tvWidth = 0;
    u32 tvHeight = 0;

    switch(tvScanMode)
    {
    case GX2_TV_SCAN_MODE_480I:
    case GX2_TV_SCAN_MODE_480P:
        tvWidth = 854;
        tvHeight = 480;
        tvRenderMode = GX2_TV_RENDER_480_WIDE;
        break;
    case GX2_TV_SCAN_MODE_1080I:
    case GX2_TV_SCAN_MODE_1080P:
        tvWidth = 1920;
        tvHeight = 1080;
        tvRenderMode = GX2_TV_RENDER_1080;
        break;
    case GX2_TV_SCAN_MODE_720P:
    default:
        tvWidth = 1280;
        tvHeight = 720;
        tvRenderMode = GX2_TV_RENDER_720;
        break;
    }

    s32 tvAAMode = GX2_AA_MODE_1X;
    s32 drcAAMode = GX2_AA_MODE_4X;

    //! calculate the scale factor for later texture resize
    widthScaleFactor = 1.0f / (f32)tvWidth;
    heightScaleFactor = 1.0f / (f32)tvHeight;
    depthScaleFactor = widthScaleFactor;

    //! calculate the size needed for the TV scan buffer and allocate the buffer from bucket memory
    GX2CalcTVSize(tvRenderMode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &scanBufferSize, &scaleNeeded);
    tvScanBuffer = MEMBucket_alloc(scanBufferSize, GX2_SCAN_BUFFER_ALIGNMENT);
    GX2Invalidate(GX2_INVALIDATE_CPU, tvScanBuffer, scanBufferSize);
    GX2SetTVBuffer(tvScanBuffer, scanBufferSize, tvRenderMode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);

    //! calculate the size needed for the DRC scan buffer and allocate the buffer from bucket memory
    GX2CalcDRCSize(drcScanMode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &scanBufferSize, &scaleNeeded);
    drcScanBuffer = MEMBucket_alloc(scanBufferSize, GX2_SCAN_BUFFER_ALIGNMENT);
    GX2Invalidate(GX2_INVALIDATE_CPU, drcScanBuffer, scanBufferSize);
    GX2SetDRCBuffer(drcScanBuffer, scanBufferSize, drcScanMode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);

    //! Setup color buffer for TV rendering
    GX2InitColorBuffer(&tvColorBuffer, GX2_SURFACE_DIM_2D, tvWidth, tvHeight, 1, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, tvAAMode);
    tvColorBuffer.surface.image_data = MEM1_alloc(tvColorBuffer.surface.image_size, tvColorBuffer.surface.align);
    GX2Invalidate(GX2_INVALIDATE_CPU, tvColorBuffer.surface.image_data, tvColorBuffer.surface.image_size);

    //! due to AA we can only use 16 bit depth buffer in MEM1 otherwise we would have to switch to mem2 for depth buffer
    //! this should be ok for our purpose i guess

    //! Setup TV depth buffer (can be the same for both if rendered one after another)
    u32 size, align;
    GX2InitDepthBuffer(&tvDepthBuffer, GX2_SURFACE_DIM_2D, tvColorBuffer.surface.width, tvColorBuffer.surface.height, 1, GX2_SURFACE_FORMAT_TCD_R32_FLOAT, tvAAMode);
    tvDepthBuffer.surface.image_data = MEM1_alloc(tvDepthBuffer.surface.image_size, tvDepthBuffer.surface.align);
    GX2Invalidate(GX2_INVALIDATE_CPU, tvDepthBuffer.surface.image_data, tvDepthBuffer.surface.image_size);

    //! Setup TV HiZ buffer
    GX2CalcDepthBufferHiZInfo(&tvDepthBuffer, &size, &align);
    tvDepthBuffer.hiZ_data = MEM1_alloc(size, align);
    GX2Invalidate(GX2_INVALIDATE_CPU, tvDepthBuffer.hiZ_data, size);
    GX2InitDepthBufferHiZEnable(&tvDepthBuffer, GX2_ENABLE);

    //! Setup color buffer for DRC rendering
    GX2InitColorBuffer(&drcColorBuffer, GX2_SURFACE_DIM_2D, 854, 480, 1, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, drcAAMode);
    drcColorBuffer.surface.image_data = MEM1_alloc(drcColorBuffer.surface.image_size, drcColorBuffer.surface.align);
    GX2Invalidate(GX2_INVALIDATE_CPU, drcColorBuffer.surface.image_data, drcColorBuffer.surface.image_size);

    //! Setup DRC depth buffer (can be the same for both if rendered one after another)
    GX2InitDepthBuffer(&drcDepthBuffer, GX2_SURFACE_DIM_2D, drcColorBuffer.surface.width, drcColorBuffer.surface.height, 1, GX2_SURFACE_FORMAT_TCD_R32_FLOAT, drcAAMode);
    drcDepthBuffer.surface.image_data = MEM1_alloc(drcDepthBuffer.surface.image_size, drcDepthBuffer.surface.align);
    GX2Invalidate(GX2_INVALIDATE_CPU, drcDepthBuffer.surface.image_data, drcDepthBuffer.surface.image_size);

    //! Setup DRC HiZ buffer
    GX2CalcDepthBufferHiZInfo(&drcDepthBuffer, &size, &align);
    drcDepthBuffer.hiZ_data = MEM1_alloc(size, align);
    GX2Invalidate(GX2_INVALIDATE_CPU, drcDepthBuffer.hiZ_data, size);
    GX2InitDepthBufferHiZEnable(&drcDepthBuffer, GX2_ENABLE);


    //! allocate auxilary buffer last as there might not be enough MEM1 left for other stuff after that
    if (tvColorBuffer.surface.aa)
    {
        u32 auxSize, auxAlign;
        GX2CalcColorBufferAuxInfo(&tvColorBuffer, &auxSize, &auxAlign);
        tvColorBuffer.aux_data = MEM1_alloc(auxSize, auxAlign);
        if(!tvColorBuffer.aux_data)
            tvColorBuffer.aux_data = MEM2_alloc(auxSize, auxAlign);

        tvColorBuffer.aux_size = auxSize;
        memset(tvColorBuffer.aux_data, GX2_AUX_BUFFER_CLEAR_VALUE, auxSize);
        GX2Invalidate(GX2_INVALIDATE_CPU, tvColorBuffer.aux_data, auxSize);
    }

    if (drcColorBuffer.surface.aa)
    {
        u32 auxSize, auxAlign;
        GX2CalcColorBufferAuxInfo(&drcColorBuffer, &auxSize, &auxAlign);
        drcColorBuffer.aux_data = MEM1_alloc(auxSize, auxAlign);
        if(!drcColorBuffer.aux_data)
            drcColorBuffer.aux_data = MEM2_alloc(auxSize, auxAlign);
        drcColorBuffer.aux_size = auxSize;
        memset(drcColorBuffer.aux_data, GX2_AUX_BUFFER_CLEAR_VALUE, auxSize);
        GX2Invalidate(GX2_INVALIDATE_CPU, drcColorBuffer.aux_data, auxSize );
    }

    //! allocate memory and setup context state TV
    tvContextState = (GX2ContextState*)MEM2_alloc(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT);
    GX2SetupContextStateEx(tvContextState, GX2_TRUE);

    //! allocate memory and setup context state DRC
    drcContextState = (GX2ContextState*)MEM2_alloc(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT);
    GX2SetupContextStateEx(drcContextState, GX2_TRUE);

    //! set initial context state and render buffers
    GX2SetContextState(tvContextState);
    GX2SetColorBuffer(&tvColorBuffer, GX2_RENDER_TARGET_0);
    GX2SetDepthBuffer(&tvDepthBuffer);

    GX2SetContextState(drcContextState);
    GX2SetColorBuffer(&drcColorBuffer, GX2_RENDER_TARGET_0);
    GX2SetDepthBuffer(&drcDepthBuffer);

    //! set initial viewport
    GX2SetViewport(0.0f, 0.0f, tvColorBuffer.surface.width, tvColorBuffer.surface.height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, tvColorBuffer.surface.width, tvColorBuffer.surface.height);

    //! this is not necessary but can be used for swap counting and vsyncs
    GX2SetSwapInterval(1);

    //GX2SetTVGamma(0.8f);
    //GX2SetDRCGamma(0.8f);

    //! initialize perspective matrix
	const float cam_X_rot = 25.0f;

	projectionMtx = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);

	viewMtx = glm::mat4(1.0f);
	viewMtx = glm::translate(viewMtx, glm::vec3(0.0f, 0.0f, -2.5f));
	viewMtx = glm::rotate(viewMtx, DegToRad(cam_X_rot), glm::vec3(1.0f, 0.0f, 0.0f));

    GX2InitSampler(&aaSampler, GX2_TEX_CLAMP_CLAMP, GX2_TEX_XY_FILTER_BILINEAR);
    GX2InitTexture(&tvAaTexture, tvColorBuffer.surface.width, tvColorBuffer.surface.height, 1, 0, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_SURFACE_DIM_2D, GX2_TILE_MODE_DEFAULT);
    tvAaTexture.surface.image_data = tvColorBuffer.surface.image_data;
    tvAaTexture.surface.image_size = tvColorBuffer.surface.image_size;
    tvAaTexture.surface.mip_data = tvColorBuffer.surface.mip_data;
}

CVideo::~CVideo()
{
    //! flush buffers
    GX2Flush();
    GX2DrawDone();
    //! shutdown
    GX2Shutdown();
    //! free command buffer memory
    MEM2_free(gx2CommandBuffer);
    //! free scan buffers
    MEMBucket_free(tvScanBuffer);
    MEMBucket_free(drcScanBuffer);
    //! free color buffers
    MEM1_free(tvColorBuffer.surface.image_data);
    MEM1_free(drcColorBuffer.surface.image_data);
    //! free depth buffers
    MEM1_free(tvDepthBuffer.surface.image_data);
    MEM1_free(tvDepthBuffer.hiZ_data);
    MEM1_free(drcDepthBuffer.surface.image_data);
    MEM1_free(drcDepthBuffer.hiZ_data);
    //! free context buffers
    MEM2_free(tvContextState);
    MEM2_free(drcContextState);
    //! free aux buffer
    if(tvColorBuffer.aux_data)
    {
        if(((u32)tvColorBuffer.aux_data & 0xF0000000) == 0xF0000000)
            MEM1_free(tvColorBuffer.aux_data);
        else
            MEM2_free(tvColorBuffer.aux_data);
    }
    if(drcColorBuffer.aux_data)
    {
        if(((u32)drcColorBuffer.aux_data & 0xF0000000) == 0xF0000000)
            MEM1_free(drcColorBuffer.aux_data);
        else
            MEM2_free(drcColorBuffer.aux_data);
    }
    //! destroy shaders
    ColorShader::destroyInstance();
    FXAAShader::destroyInstance();
    Shader3D::destroyInstance();
    ShaderFractalColor::destroyInstance();
    Texture2DShader::destroyInstance();
}

void CVideo::renderFXAA(const GX2Texture * texture, const GX2Sampler *sampler)
{
    resolution[0] = texture->surface.width;
    resolution[1] = texture->surface.height;

    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER | GX2_INVALIDATE_TEXTURE, texture->surface.image_data, texture->surface.image_size);

    GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_ALWAYS);
    FXAAShader::instance()->setShaders();
    FXAAShader::instance()->setAttributeBuffer();
    FXAAShader::instance()->setResolution(resolution);
    FXAAShader::instance()->setTextureAndSampler(texture, sampler);
    FXAAShader::instance()->draw();
    GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_LEQUAL);
}

void* CVideo::GX2RAlloc(u32 flags, u32 size, u32 align)
{
    //! min. alignment
    if (align < 4)
        align = 4;

    if ((flags & 0x2040E) && !(flags & 0x40000))
        return MEM1_alloc(size, align);
    else
        return MEM2_alloc(size, align);
}

void CVideo::GX2RFree(u32 flags, void* p)
{
    if ((flags & 0x2040E) && !(flags & 0x40000))
        MEM1_free(p);
    else
        MEM2_free(p);
}
