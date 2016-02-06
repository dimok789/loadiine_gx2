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
#ifndef GUI_IMAGEDATA_H_
#define GUI_IMAGEDATA_H_

#include <gctypes.h>
#include <gd.h>
#include "dynamic_libs/gx2_functions.h"
#include "system/AsyncDeleter.h"

class GuiImageData : public AsyncDeleter::Element
{
public:
    //!Constructor
    GuiImageData();
    //!\param img Image data
    //!\param imgSize The image size
    GuiImageData(const u8 * img, int imgSize, int textureClamp = GX2_TEX_CLAMP_CLAMP, int textureFormat = GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM);
    //!Destructor
    virtual ~GuiImageData();
    //!Load image from buffer
    //!\param img Image data
    //!\param imgSize The image size
    void loadImage(const u8 * img, int imgSize, int textureClamp = GX2_TEX_CLAMP_CLAMP, int textureFormat = GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM);
    //! getter functions
    const GX2Texture * getTexture() const { return texture; };
    const GX2Sampler * getSampler() const { return sampler; };
    //!Gets the image width
    //!\return image width
    int getWidth() const { if(texture) return texture->surface.width; else return 0; };
    //!Gets the image height
    //!\return image height
    int getHeight() const { if(texture) return texture->surface.height; else return 0; };
    //! release memory of the image data
    void releaseData(void);
private:
    void gdImageToUnormR8G8B8A8(gdImagePtr gdImg, u32 *imgBuffer, u32 width, u32 height, u32 pitch);
    void gdImageToUnormR5G6B5(gdImagePtr gdImg, u16 *imgBuffer, u32 width, u32 height, u32 pitch);

    GX2Texture *texture;
    GX2Sampler *sampler;

    enum eMemoryTypes
    {
        eMemTypeMEM2,
        eMemTypeMEM1,
        eMemTypeMEMBucket
    };

    u8 memoryType;
};

#endif
