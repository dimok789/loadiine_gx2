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
#include "GuiImageData.h"
#include "system/memory.h"
/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData()
{
    texture = NULL;
    sampler = NULL;
	memoryType = eMemTypeMEM2;
}

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * img, int imgSize, int textureClamp, int textureFormat)
{
    texture = NULL;
    sampler = NULL;
	loadImage(img, imgSize, textureClamp, textureFormat);
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData()
{
    releaseData();
}

void GuiImageData::releaseData(void)
{
    if(texture) {
        if(texture->surface.image_data)
        {
            switch(memoryType)
            {
            default:
            case eMemTypeMEM2:
                free(texture->surface.image_data);
                break;
            case eMemTypeMEM1:
                MEM1_free(texture->surface.image_data);
                break;
            case eMemTypeMEMBucket:
                MEMBucket_free(texture->surface.image_data);
                break;
            }
        }
        delete texture;
        texture = NULL;
    }
    if(sampler) {
        delete sampler;
        sampler = NULL;
    }
}

void GuiImageData::loadImage(const u8 *img, int imgSize, int textureClamp, int textureFormat)
{
	if(!img || (imgSize < 8))
		return;

	releaseData();
	gdImagePtr gdImg = 0;

	if (img[0] == 0xFF && img[1] == 0xD8)
	{
		//! not needed for now therefore comment out to safe ELF size
		//! if needed uncomment, adds 200 kb to the ELF size
		// IMAGE_JPEG
		gdImg = gdImageCreateFromJpegPtr(imgSize, (u8*) img);
	}
	else if (img[0] == 'B' && img[1] == 'M')
	{
		// IMAGE_BMP
		gdImg = gdImageCreateFromBmpPtr(imgSize, (u8*) img);
	}
	else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G')
	{
		// IMAGE_PNG
		gdImg = gdImageCreateFromPngPtr(imgSize, (u8*) img);
	}
	//!This must be last since it can also intefere with outher formats
	else if(img[0] == 0x00)
	{
		// Try loading TGA image
		gdImg = gdImageCreateFromTgaPtr(imgSize, (u8*) img);
	}

	if(gdImg == 0)
		return;

	u32 width = (gdImageSX(gdImg));
	u32 height = (gdImageSY(gdImg));

    //! Initialize texture
    texture = new GX2Texture;
    GX2InitTexture(texture, width,  height, 1, 0, textureFormat, GX2_SURFACE_DIM_2D, GX2_TILE_MODE_LINEAR_ALIGNED);

    //! if this fails something went horribly wrong
    if(texture->surface.image_size == 0) {
        delete texture;
        texture = NULL;
        gdImageDestroy(gdImg);
        return;
    }

    //! allocate memory for the surface
	memoryType = eMemTypeMEM2;
    texture->surface.image_data = memalign(texture->surface.align, texture->surface.image_size);
    //! try MEM1 on failure
    if(!texture->surface.image_data) {
        memoryType = eMemTypeMEM1;
        texture->surface.image_data = MEM1_alloc(texture->surface.image_size, texture->surface.align);
    }
    //! try MEM bucket on failure
    if(!texture->surface.image_data) {
        memoryType = eMemTypeMEMBucket;
        texture->surface.image_data = MEMBucket_alloc(texture->surface.image_size, texture->surface.align);
    }
    //! check if memory is available for image
    if(!texture->surface.image_data) {
        gdImageDestroy(gdImg);
        delete texture;
        texture = NULL;
        return;
    }
    //! set mip map data pointer
    texture->surface.mip_data = NULL;
    //! convert image to texture
    switch(textureFormat)
    {
    default:
    case GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM:
        gdImageToUnormR8G8B8A8(gdImg, (u32*)texture->surface.image_data, texture->surface.width, texture->surface.height, texture->surface.pitch);
        break;
    case GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM:
        gdImageToUnormR5G6B5(gdImg, (u16*)texture->surface.image_data, texture->surface.width, texture->surface.height, texture->surface.pitch);
        break;
    }

	//! free memory of image as its not needed anymore
	gdImageDestroy(gdImg);

	//! invalidate the memory
    GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, texture->surface.image_data, texture->surface.image_size);
    //! initialize the sampler
    sampler = new GX2Sampler;
    GX2InitSampler(sampler, textureClamp, GX2_TEX_XY_FILTER_BILINEAR);
}

void GuiImageData::gdImageToUnormR8G8B8A8(gdImagePtr gdImg, u32 *imgBuffer, u32 width, u32 height, u32 pitch)
{
    for(u32 y = 0; y < height; ++y)
    {
        for(u32 x = 0; x < width; ++x)
        {
			u32 pixel = gdImageGetPixel(gdImg, x, y);

			u8 a = 254 - 2*((u8)gdImageAlpha(gdImg, pixel));
			if(a == 254) a++;

            u8 r = gdImageRed(gdImg, pixel);
            u8 g = gdImageGreen(gdImg, pixel);
            u8 b = gdImageBlue(gdImg, pixel);

            imgBuffer[y * pitch + x] = (r << 24) | (g << 16) | (b << 8) | (a);
        }
    }
}

//! TODO: figure out why this seems to not work correct yet
void GuiImageData::gdImageToUnormR5G6B5(gdImagePtr gdImg, u16 *imgBuffer, u32 width, u32 height, u32 pitch)
{
    for(u32 y = 0; y < height; ++y)
    {
        for(u32 x = 0; x < width; ++x)
        {
			u32 pixel = gdImageGetPixel(gdImg, x, y);

            u8 r = gdImageRed(gdImg, pixel);
            u8 g = gdImageGreen(gdImg, pixel);
            u8 b = gdImageBlue(gdImg, pixel);

            imgBuffer[y * pitch + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
    }
}
