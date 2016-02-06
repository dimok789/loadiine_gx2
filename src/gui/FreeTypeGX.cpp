/*
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * Modified by Dimok, 2015 for WiiU GX2
 *
 * This file is part of FreeTypeGX.
 *
 * FreeTypeGX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeTypeGX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FreeTypeGX.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeTypeGX.h"
#include "video/CVideo.h"
#include "video/shaders/Texture2DShader.h"

using namespace std;

#define ALIGN4(x) (((x) + 3) & ~3)

/**
 * Default constructor for the FreeTypeGX class for WiiXplorer.
 */
FreeTypeGX::FreeTypeGX(const uint8_t* fontBuffer, FT_Long bufferSize, bool lastFace)
{
	int faceIndex = 0;
	ftPointSize = 0;
    GX2InitSampler(&ftSampler, GX2_TEX_CLAMP_CLAMP_BORDER, GX2_TEX_XY_FILTER_BILINEAR);

	FT_Init_FreeType(&ftLibrary);
	if(lastFace)
	{
		FT_New_Memory_Face(ftLibrary, (FT_Byte *)fontBuffer, bufferSize, -1, &ftFace);
		faceIndex = ftFace->num_faces - 1; // Use the last face
		FT_Done_Face(ftFace);
		ftFace = NULL;
	}
	FT_New_Memory_Face(ftLibrary, (FT_Byte *) fontBuffer, bufferSize, faceIndex, &ftFace);

	ftKerningEnabled = FT_HAS_KERNING(ftFace);
}

/**
 * Default destructor for the FreeTypeGX class.
 */
FreeTypeGX::~FreeTypeGX()
{
	unloadFont();
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);
}

/**
 * Convert a short char string to a wide char string.
 *
 * This routine converts a supplied short character string into a wide character string.
 * Note that it is the user's responsibility to clear the returned buffer once it is no longer needed.
 *
 * @param strChar   Character string to be converted.
 * @return Wide character representation of supplied character string.
 */

wchar_t* FreeTypeGX::charToWideChar(const char* strChar)
{
	if (!strChar) return NULL;

	wchar_t *strWChar = new (std::nothrow) wchar_t[strlen(strChar) + 1];
	if (!strWChar) return NULL;

	int bt = mbstowcs(strWChar, strChar, strlen(strChar));
	if (bt > 0)
	{
		strWChar[bt] = 0;
		return strWChar;
	}

	wchar_t *tempDest = strWChar;
	while ((*tempDest++ = *strChar++))
		;

	return strWChar;
}

char *FreeTypeGX::wideCharToUTF8(const wchar_t* strChar)
{
    if(!strChar) {
        return NULL;
    }

	size_t len = 0;
	wchar_t wc;

	for (size_t i = 0; strChar[i]; ++i)
	{
		wc = strChar[i];
		if (wc < 0x80)
			++len;
		else if (wc < 0x800)
			len += 2;
		else if (wc < 0x10000)
			len += 3;
		else
			len += 4;
	}

    char *pOut = new (std::nothrow) char[len];
    if(!pOut)
        return NULL;

    size_t n = 0;

	for (size_t i = 0; strChar[i]; ++i)
	{
		wc = strChar[i];
		if (wc < 0x80)
			pOut[n++] = (char)wc;
		else if (wc < 0x800)
		{
			pOut[n++] = (char)((wc >> 6) | 0xC0);
			pOut[n++] = (char)((wc & 0x3F) | 0x80);
		}
		else if (wc < 0x10000)
		{
			pOut[n++] = (char)((wc >> 12) | 0xE0);
			pOut[n++] = (char)(((wc >> 6) & 0x3F) | 0x80);
			pOut[n++] = (char)((wc & 0x3F) | 0x80);
		}
		else
		{
			pOut[n++] = (char)(((wc >> 18) & 0x07) | 0xF0);
			pOut[n++] = (char)(((wc >> 12) & 0x3F) | 0x80);
			pOut[n++] = (char)(((wc >> 6) & 0x3F) | 0x80);
			pOut[n++] = (char)((wc & 0x3F) | 0x80);
		}
	}
	return pOut;
}

/**
 * Clears all loaded font glyph data.
 *
 * This routine clears all members of the font map structure and frees all allocated memory back to the system.
 */
void FreeTypeGX::unloadFont()
{
	map<int16_t, ftGX2Data >::iterator itr;
	map<wchar_t, ftgxCharData>::iterator itr2;

	for (itr = fontData.begin(); itr != fontData.end(); itr++)
	{
		for (itr2 = itr->second.ftgxCharMap.begin(); itr2 != itr->second.ftgxCharMap.end(); itr2++)
        {
            if(itr2->second.texture)
            {
                if(itr2->second.texture->surface.image_data)
                    free(itr2->second.texture->surface.image_data);

                delete itr2->second.texture;
                itr2->second.texture = NULL;
            }
        }
	}

	fontData.clear();
}

/**
 * Caches the given font glyph in the instance font texture buffer.
 *
 * This routine renders and stores the requested glyph's bitmap and relevant information into its own quickly addressible
 * structure within an instance-specific map.
 *
 * @param charCode  The requested glyph's character code.
 * @return A pointer to the allocated font structure.
 */
ftgxCharData * FreeTypeGX::cacheGlyphData(wchar_t charCode, int16_t pixelSize)
{
	map<int16_t, ftGX2Data>::iterator itr = fontData.find(pixelSize);
	if (itr != fontData.end())
	{
		map<wchar_t, ftgxCharData>::iterator itr2 = itr->second.ftgxCharMap.find(charCode);
		if (itr2 != itr->second.ftgxCharMap.end())
		{
			return &itr2->second;
		}
	}
    //!Cache ascender and decender as well
    ftGX2Data *ftData = &fontData[pixelSize];

    FT_UInt gIndex;
    uint16_t textureWidth = 0, textureHeight = 0;
    if (ftPointSize != pixelSize)
    {
        ftPointSize = pixelSize;
        FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);
        ftData->ftgxAlign.ascender = (int16_t) ftFace->size->metrics.ascender >> 6;
        ftData->ftgxAlign.descender = (int16_t) ftFace->size->metrics.descender >> 6;
        ftData->ftgxAlign.max = 0;
        ftData->ftgxAlign.min = 0;
    }

	gIndex = FT_Get_Char_Index(ftFace, (FT_ULong) charCode);
	if (gIndex != 0 && FT_Load_Glyph(ftFace, gIndex, FT_LOAD_DEFAULT | FT_LOAD_RENDER) == 0)
	{
		if (ftFace->glyph->format == FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Bitmap *glyphBitmap = &ftFace->glyph->bitmap;

			textureWidth = ALIGN4(glyphBitmap->width);
			textureHeight = ALIGN4(glyphBitmap->rows);
			if(textureWidth == 0)
				textureWidth = 4;
			if(textureHeight == 0)
				textureHeight = 4;

            ftgxCharData *charData = &ftData->ftgxCharMap[charCode];
			charData->renderOffsetX = (int16_t) ftFace->glyph->bitmap_left;
			charData->glyphAdvanceX = (uint16_t) (ftFace->glyph->advance.x >> 6);
			charData->glyphAdvanceY = (uint16_t) (ftFace->glyph->advance.y >> 6);
			charData->glyphIndex = (uint32_t) gIndex;
			charData->renderOffsetY = (int16_t) ftFace->glyph->bitmap_top;
			charData->renderOffsetMax = (int16_t) ftFace->glyph->bitmap_top;
			charData->renderOffsetMin = (int16_t) glyphBitmap->rows - ftFace->glyph->bitmap_top;

            //! Initialize texture
            charData->texture = new GX2Texture;
            GX2InitTexture(charData->texture, textureWidth,  textureHeight, 1, 0, GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM, GX2_SURFACE_DIM_2D, GX2_TILE_MODE_LINEAR_ALIGNED);

			loadGlyphData(glyphBitmap, charData);

			return charData;
		}
	}
	return NULL;
}

/**
 * Locates each character in this wrapper's configured font face and proccess them.
 *
 * This routine locates each character in the configured font face and renders the glyph's bitmap.
 * Each bitmap and relevant information is loaded into its own quickly addressible structure within an instance-specific map.
 */
uint16_t FreeTypeGX::cacheGlyphDataComplete(int16_t pixelSize)
{
	uint32_t i = 0;
	FT_UInt gIndex;

	FT_ULong charCode = FT_Get_First_Char(ftFace, &gIndex);
	while (gIndex != 0)
	{
		if (cacheGlyphData(charCode, pixelSize) != NULL) ++i;
		charCode = FT_Get_Next_Char(ftFace, charCode, &gIndex);
	}
	return (uint16_t) (i);
}

/**
 * Loads the rendered bitmap into the relevant structure's data buffer.
 *
 * This routine does a simple byte-wise copy of the glyph's rendered 8-bit grayscale bitmap into the structure's buffer.
 * Each byte is converted from the bitmap's intensity value into the a uint32_t RGBA value.
 *
 * @param bmp   A pointer to the most recently rendered glyph's bitmap.
 * @param charData  A pointer to an allocated ftgxCharData structure whose data represent that of the last rendered glyph.
 */

void FreeTypeGX::loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData)
{
	charData->texture->surface.image_data = (uint8_t *) memalign(charData->texture->surface.align, charData->texture->surface.image_size);
	if(!charData->texture->surface.image_data)
        return;

	memset(charData->texture->surface.image_data, 0x00, charData->texture->surface.image_size);

	uint8_t *src = (uint8_t *)bmp->buffer;
	uint16_t *dst = (uint16_t *)charData->texture->surface.image_data;
	int32_t x, y;

	for(y = 0; y < bmp->rows; y++)
	{
		for(x = 0; x < bmp->width; x++)
		{
		    uint8_t intensity = src[y * bmp->width + x] >> 3;
            dst[y * charData->texture->surface.pitch + x] = intensity ? ((intensity << 11) | (intensity << 6) | (intensity << 1) | 1) : 0;
		}
	}
    GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, charData->texture->surface.image_data, charData->texture->surface.image_size);
}

/**
 * Determines the x offset of the rendered string.
 *
 * This routine calculates the x offset of the rendered string based off of a supplied positional format parameter.
 *
 * @param width Current pixel width of the string.
 * @param format	Positional format of the string.
 */
int16_t FreeTypeGX::getStyleOffsetWidth(uint16_t width, uint16_t format)
{
	if (format & FTGX_JUSTIFY_LEFT)
		return 0;
	else if (format & FTGX_JUSTIFY_CENTER)
		return -(width >> 1);
	else if (format & FTGX_JUSTIFY_RIGHT) return -width;
	return 0;
}

/**
 * Determines the y offset of the rendered string.
 *
 * This routine calculates the y offset of the rendered string based off of a supplied positional format parameter.
 *
 * @param offset	Current pixel offset data of the string.
 * @param format	Positional format of the string.
 */
int16_t FreeTypeGX::getStyleOffsetHeight(int16_t format, uint16_t pixelSize)
{
	std::map<int16_t, ftGX2Data>::iterator itr = fontData.find(pixelSize);
	if (itr == fontData.end()) return 0;

	switch (format & FTGX_ALIGN_MASK)
	{
		case FTGX_ALIGN_TOP:
			return itr->second.ftgxAlign.descender;

		case FTGX_ALIGN_MIDDLE:
		default:
			return (itr->second.ftgxAlign.ascender + itr->second.ftgxAlign.descender + 1) >> 1;

		case FTGX_ALIGN_BOTTOM:
			return itr->second.ftgxAlign.ascender;

		case FTGX_ALIGN_BASELINE:
			return 0;

		case FTGX_ALIGN_GLYPH_TOP:
			return itr->second.ftgxAlign.max;

		case FTGX_ALIGN_GLYPH_MIDDLE:
			return (itr->second.ftgxAlign.max + itr->second.ftgxAlign.min + 1) >> 1;

		case FTGX_ALIGN_GLYPH_BOTTOM:
			return itr->second.ftgxAlign.min;
	}
	return 0;
}

/**
 * Processes the supplied text string and prints the results at the specified coordinates.
 *
 * This routine processes each character of the supplied text string, loads the relevant preprocessed bitmap buffer,
 * a texture from said buffer, and loads the resultant texture into the EFB.
 *
 * @param x Screen X coordinate at which to output the text.
 * @param y Screen Y coordinate at which to output the text. Note that this value corresponds to the text string origin and not the top or bottom of the glyphs.
 * @param text  NULL terminated string to output.
 * @param color Optional color to apply to the text characters. If not specified default value is ftgxWhite: (GXColor){0xff, 0xff, 0xff, 0xff}
 * @param textStyle Flags which specify any styling which should be applied to the rendered string.
 * @return The number of characters printed.
 */

uint16_t FreeTypeGX::drawText(CVideo *video, int16_t x, int16_t y, int16_t z, const wchar_t *text, int16_t pixelSize, const glm::vec4 & color, uint16_t textStyle, uint16_t textWidth, const float &textBlur, const float &colorBlurIntensity, const glm::vec4 & blurColor)
{
	if (!text)
        return 0;

	uint16_t fullTextWidth = (textWidth > 0) ? textWidth : getWidth(text, pixelSize);
	uint16_t x_pos = x, printed = 0;
	uint16_t x_offset = 0, y_offset = 0;
	FT_Vector pairDelta;

	if (textStyle & FTGX_JUSTIFY_MASK)
	{
		x_offset = getStyleOffsetWidth(fullTextWidth, textStyle);
	}
	if (textStyle & FTGX_ALIGN_MASK)
	{
		y_offset = getStyleOffsetHeight(textStyle, pixelSize);
	}

	int i = 0;

	while (text[i])
	{
		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && i > 0)
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize].ftgxCharMap[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				x_pos += (pairDelta.x >> 6);
			}

			copyTextureToFramebuffer(video, glyphData->texture, x_pos + glyphData->renderOffsetX + x_offset, y + glyphData->renderOffsetY - y_offset, z, color, textBlur, colorBlurIntensity, blurColor);

			x_pos += glyphData->glyphAdvanceX;
			++printed;
		}
		++i;
	}

	return printed;
}


/**
 * Processes the supplied string and return the width of the string in pixels.
 *
 * This routine processes each character of the supplied text string and calculates the width of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 *
 * @param text  NULL terminated string to calculate.
 * @return The width of the text string in pixels.
 */
uint16_t FreeTypeGX::getWidth(const wchar_t *text, int16_t pixelSize)
{
	if (!text) return 0;

	uint16_t strWidth = 0;
	FT_Vector pairDelta;

	int i = 0;
	while (text[i])
	{
		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			if (ftKerningEnabled && (i > 0))
			{
				FT_Get_Kerning(ftFace, fontData[pixelSize].ftgxCharMap[text[i - 1]].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
				strWidth += pairDelta.x >> 6;
			}

			strWidth += glyphData->glyphAdvanceX;
		}
		++i;
	}
	return strWidth;
}

/**
 * Single char width
 */
uint16_t FreeTypeGX::getCharWidth(const wchar_t wChar, int16_t pixelSize, const wchar_t prevChar)
{
	uint16_t strWidth = 0;
	ftgxCharData * glyphData = cacheGlyphData(wChar, pixelSize);

	if (glyphData != NULL)
	{
		if (ftKerningEnabled && prevChar != 0x0000)
		{
			FT_Vector pairDelta;
			FT_Get_Kerning(ftFace, fontData[pixelSize].ftgxCharMap[prevChar].glyphIndex, glyphData->glyphIndex, FT_KERNING_DEFAULT, &pairDelta);
			strWidth += pairDelta.x >> 6;
		}
		strWidth += glyphData->glyphAdvanceX;
	}

	return strWidth;
}

/**
 * Processes the supplied string and return the height of the string in pixels.
 *
 * This routine processes each character of the supplied text string and calculates the height of the entire string.
 * Note that if precaching of the entire font set is not enabled any uncached glyph will be cached after the call to this function.
 *
 * @param text  NULL terminated string to calculate.
 * @return The height of the text string in pixels.
 */
uint16_t FreeTypeGX::getHeight(const wchar_t *text, int16_t pixelSize)
{
	getOffset(text, pixelSize);
	return fontData[pixelSize].ftgxAlign.max - fontData[pixelSize].ftgxAlign.min;
}

/**
 * Get the maximum offset above and minimum offset below the font origin line.
 *
 * This function calculates the maximum pixel height above the font origin line and the minimum
 * pixel height below the font origin line and returns the values in an addressible structure.
 *
 * @param text  NULL terminated string to calculate.
 * @param offset returns the max and min values above and below the font origin line
 *
 */
void FreeTypeGX::getOffset(const wchar_t *text, int16_t pixelSize, uint16_t widthLimit)
{
	if (fontData.find(pixelSize) != fontData.end())
        return;

	int16_t strMax = 0, strMin = 9999;
	uint16_t currWidth = 0;

	int i = 0;

	while (text[i])
	{
		if (widthLimit > 0 && currWidth >= widthLimit) break;

		ftgxCharData* glyphData = cacheGlyphData(text[i], pixelSize);

		if (glyphData != NULL)
		{
			strMax = glyphData->renderOffsetMax > strMax ? glyphData->renderOffsetMax : strMax;
			strMin = glyphData->renderOffsetMin < strMin ? glyphData->renderOffsetMin : strMin;
			currWidth += glyphData->glyphAdvanceX;
		}

		++i;
	}

	if (ftPointSize != pixelSize)
	{
		ftPointSize = pixelSize;
		FT_Set_Pixel_Sizes(ftFace, 0, ftPointSize);
	}

	fontData[pixelSize].ftgxAlign.ascender = ftFace->size->metrics.ascender >> 6;
	fontData[pixelSize].ftgxAlign.descender = ftFace->size->metrics.descender >> 6;
	fontData[pixelSize].ftgxAlign.max = strMax;
	fontData[pixelSize].ftgxAlign.min = strMin;
}

/**
 * Copies the supplied texture quad to the EFB.
 *
 * This routine uses the in-built GX quad builder functions to define the texture bounds and location on the EFB target.
 *
 * @param texObj	A pointer to the glyph's initialized texture object.
 * @param texWidth  The pixel width of the texture object.
 * @param texHeight The pixel height of the texture object.
 * @param screenX   The screen X coordinate at which to output the rendered texture.
 * @param screenY   The screen Y coordinate at which to output the rendered texture.
 * @param color Color to apply to the texture.
 */
void FreeTypeGX::copyTextureToFramebuffer(CVideo *pVideo, GX2Texture *texture, int16_t x, int16_t y, int16_t z, const glm::vec4 & color, const float & defaultBlur, const float & blurIntensity, const glm::vec4 & blurColor)
{
    static const f32 imageAngle = 0.0f;
    static const f32 blurScale = 2.0f;

    f32 offsetLeft = 2.0f * ((f32)x + 0.5f * (f32)texture->surface.width) * (f32)pVideo->getWidthScaleFactor();
    f32 offsetTop = 2.0f * ((f32)y - 0.5f * (f32)texture->surface.height) * (f32)pVideo->getHeightScaleFactor();

    f32 widthScale = blurScale * (f32)texture->surface.width * pVideo->getWidthScaleFactor();
    f32 heightScale = blurScale * (f32)texture->surface.height * pVideo->getHeightScaleFactor();

    glm::vec3 positionOffsets( offsetLeft, offsetTop, (f32)z );

     //! blur doubles  due to blur we have to scale the texture
    glm::vec3 scaleFactor( widthScale, heightScale, 1.0f );

    glm::vec3 blurDirection;
    blurDirection[2] = 1.0f;

    Texture2DShader::instance()->setShaders();
    Texture2DShader::instance()->setAttributeBuffer();
    Texture2DShader::instance()->setAngle(imageAngle);
    Texture2DShader::instance()->setOffset(positionOffsets);
    Texture2DShader::instance()->setScale(scaleFactor);
    Texture2DShader::instance()->setTextureAndSampler(texture, &ftSampler);

    if(blurIntensity > 0.0f)
    {
        //! glow blur color
        Texture2DShader::instance()->setColorIntensity(blurColor);

        //! glow blur horizontal
        blurDirection[0] = blurIntensity;
        blurDirection[1] = 0.0f;
        Texture2DShader::instance()->setBlurring(blurDirection);
        Texture2DShader::instance()->draw();

        //! glow blur vertical
        blurDirection[0] = 0.0f;
        blurDirection[1] = blurIntensity;
        Texture2DShader::instance()->setBlurring(blurDirection);
        Texture2DShader::instance()->draw();
    }

    //! set text color
    Texture2DShader::instance()->setColorIntensity(color);

    //! blur horizontal
    blurDirection[0] = defaultBlur;
    blurDirection[1] = 0.0f;
    Texture2DShader::instance()->setBlurring(blurDirection);
    Texture2DShader::instance()->draw();

    //! blur vertical
    blurDirection[0] = 0.0f;
    blurDirection[1] = defaultBlur;
    Texture2DShader::instance()->setBlurring(blurDirection);
    Texture2DShader::instance()->draw();
}
