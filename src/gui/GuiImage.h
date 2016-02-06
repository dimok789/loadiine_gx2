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
#ifndef GUI_IMAGE_H_
#define GUI_IMAGE_H_

#include "video/shaders/Shader.h"
#include "GuiElement.h"
#include "GuiImageData.h"

//!Display, manage, and manipulate images in the GUI
class GuiImage : public GuiElement
{
public:
    enum ImageTypes
    {
        IMAGE_TEXTURE,
        IMAGE_COLOR
    };

    //!\overload
    //!\param img Pointer to GuiImageData element
    GuiImage(GuiImageData * img);
    //!\overload
    //!Creates an image filled with the specified color
    //!\param w Image width
    //!\param h Image height
    //!\param c Array with 4 x image color (BL, BR, TL, TR)
    GuiImage(int w, int h, const GX2Color & c, int imgType = IMAGE_COLOR);
    GuiImage(int w, int h, const GX2Color * c, u32 colorCount = 1, int imgType = IMAGE_COLOR);
    //!Destructor
    virtual ~GuiImage();
    //!Sets the number of times to draw the image horizontally
    //!\param t Number of times to draw the image
    void setTileHorizontal(int t) { tileHorizontal = t; }
    //!Sets the number of times to draw the image vertically
    //!\param t Number of times to draw the image
    void setTileVertical(int t) { tileVertical = t; }
    //!Constantly called to draw the image
    void draw(CVideo *pVideo);
    //!Gets the image data
    //!\return pointer to image data
    GuiImageData * getImageData() const { return imageData; }
    //!Sets up a new image using the GuiImageData object specified
    //!\param img Pointer to GuiImageData object
    void setImageData(GuiImageData * img);
    //!Gets the pixel color at the specified coordinates of the image
    //!\param x X coordinate
    //!\param y Y coordinate
    GX2Color getPixel(int x, int y);
    //!Sets the pixel color at the specified coordinates of the image
    //!\param x X coordinate
    //!\param y Y coordinate
    //!\param color Pixel color
    void setPixel(int x, int y, const GX2Color & color);
    //!Change ImageColor
    void setImageColor(const GX2Color & c, int idx = -1);
    //!Change ImageColor
    void setSize(int w, int h);

    void setPrimitiveVertex(s32 prim, const f32 *pos, const f32 *tex, u32 count);

    void setBlurDirection(u8 dir, f32 value)
    {
        if(dir < 2) {
            blurDirection[dir] = value;
        }
    }
    void setColorIntensity(const glm::vec4 & col)
    {
        colorIntensity = col;
    }
protected:
    void internalInit(int w, int h);

    int imgType;                //!< Type of image data (IMAGE_TEXTURE, IMAGE_COLOR, IMAGE_DATA)
    GuiImageData * imageData;   //!< Poiner to image data. May be shared with GuiImageData data
    int tileHorizontal;         //!< Number of times to draw (tile) the image horizontally
    int tileVertical;           //!< Number of times to draw (tile) the image vertically

    //! Internally used variables for rendering
    u8 *colorVtxs;
    u32 colorCount;
    bool colorVtxsDirty;
    glm::vec3 positionOffsets;
    glm::vec3 scaleFactor;
    glm::vec4 colorIntensity;
    f32 imageAngle;
    glm::vec3 blurDirection;

    const f32 * posVtxs;
    const f32 * texCoords;
    u32 vtxCount;
    s32 primitive;
};

#endif
