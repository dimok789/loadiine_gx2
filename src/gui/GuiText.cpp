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
#include "GuiText.h"
#include "FreeTypeGX.h"
#include "video/CVideo.h"

FreeTypeGX * GuiText::presentFont = NULL;
int GuiText::presetSize = 28;
int GuiText::presetMaxWidth = 0xFFFF;
int GuiText::presetAlignment = ALIGN_CENTER | ALIGN_MIDDLE;
GX2ColorF32 GuiText::presetColor = (GX2ColorF32){ 1.0f, 1.0f, 1.0f, 1.0f };

#define TEXT_SCROLL_DELAY			6
#define	TEXT_SCROLL_INITIAL_DELAY	10
#define MAX_LINES_TO_DRAW		    10

/**
 * Constructor for the GuiText class.
 */

GuiText::GuiText()
{
	text = NULL;
	size = presetSize;
	currentSize = size;
	color = glm::vec4(presetColor.r, presetColor.g, presetColor.b, presetColor.a);
	alpha = presetColor.a;
	alignment = presetAlignment;
	maxWidth = presetMaxWidth;
	wrapMode = 0;
	textWidth = 0;
	font = presentFont;
	linestodraw = MAX_LINES_TO_DRAW;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
	defaultBlur = 4.0f;
	blurGlowIntensity = 0.0f;
	blurAlpha = 0.0f;
	blurGlowColor = glm::vec4(0.0f);
}

GuiText::GuiText(const char * t, int s, const glm::vec4 & c)
{
	text = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c[3];
	alignment = ALIGN_CENTER | ALIGN_MIDDLE;
	maxWidth = presetMaxWidth;
	wrapMode = 0;
	textWidth = 0;
	font = presentFont;
	linestodraw = MAX_LINES_TO_DRAW;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
	defaultBlur = 4.0f;
	blurGlowIntensity = 0.0f;
	blurAlpha = 0.0f;
	blurGlowColor = glm::vec4(0.0f);

	if(t)
	{
		text = FreeTypeGX::charToWideChar(t);
		if(!text)
			return;

		textWidth = font->getWidth(text, currentSize);
	}
}

GuiText::GuiText(const wchar_t * t, int s, const glm::vec4 & c)
{
	text = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c[3];
	alignment = ALIGN_CENTER | ALIGN_MIDDLE;
	maxWidth = presetMaxWidth;
	wrapMode = 0;
	textWidth = 0;
	font = presentFont;
	linestodraw = MAX_LINES_TO_DRAW;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
	defaultBlur = 4.0f;
	blurGlowIntensity = 0.0f;
	blurAlpha = 0.0f;
	blurGlowColor = glm::vec4(0.0f);

	if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t)+1];
		if(!text)
			return;

		wcscpy(text, t);

		textWidth = font->getWidth(text, currentSize);
	}
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	text = NULL;
	size = presetSize;
	currentSize = size;
	color = glm::vec4(presetColor.r, presetColor.g, presetColor.b, presetColor.a);
	alpha = presetColor.a;
	alignment = presetAlignment;
	maxWidth = presetMaxWidth;
	wrapMode = 0;
	textWidth = 0;
	font = presentFont;
	linestodraw = MAX_LINES_TO_DRAW;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;
	defaultBlur = 4.0f;
	blurGlowIntensity = 0.0f;
	blurAlpha = 0.0f;
	blurGlowColor = glm::vec4(0.0f);

	if(t)
	{
		text = FreeTypeGX::charToWideChar(t);
		if(!text)
			return;

		textWidth = font->getWidth(text, currentSize);
	}
}


/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(text)
		delete [] text;
	text = NULL;

	clearDynamicText();
}

void GuiText::setText(const char * t)
{
	if(text)
		delete [] text;
	text = NULL;

	clearDynamicText();

	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		text = FreeTypeGX::charToWideChar(t);
		if(!text)
			return;

		textWidth = font->getWidth(text, currentSize);
	}
}

void GuiText::setTextf(const char *format, ...)
{
	if(!format)
    {
		setText((char *) NULL);
		return;
    }

    int max_len = strlen(format) + 8192;
	char *tmp = new char[max_len];
	va_list va;
	va_start(va, format);
	if((vsnprintf(tmp, max_len, format, va) >= 0) && tmp)
	{
		setText(tmp);
	}
	va_end(va);

	if(tmp)
		delete [] tmp;
}


void GuiText::setText(const wchar_t * t)
{
	if(text)
		delete [] text;
	text = NULL;

	clearDynamicText();

	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t)+1];
		if(!text)
			return;

		wcscpy(text, t);

		textWidth = font->getWidth(text, currentSize);
	}
}

void GuiText::clearDynamicText()
{
	for(u32 i = 0; i < textDyn.size(); i++)
	{
		if(textDyn[i])
			delete [] textDyn[i];
	}
	textDyn.clear();
	textDynWidth.clear();
}

void GuiText::setPresets(int sz, const glm::vec4 & c, int w, int a)
{
	presetSize = sz;
	presetColor = (GX2ColorF32) { (f32)c.r / 255.0f, (f32)c.g / 255.0f, (f32)c.b / 255.0f, (f32)c.a / 255.0f };
	presetMaxWidth = w;
	presetAlignment = a;
}

void GuiText::setPresetFont(FreeTypeGX *f)
{
	presentFont = f;
}

void GuiText::setFontSize(int s)
{
	size = s;
}

void GuiText::setMaxWidth(int width, int w)
{
	maxWidth = width;
	wrapMode = w;

	if(w == SCROLL_HORIZONTAL)
	{
		textScrollPos = 0;
		textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
		textScrollDelay = TEXT_SCROLL_DELAY;
	}

	clearDynamicText();
}

void GuiText::setColor(const glm::vec4 & c)
{
	color = c;
	alpha = c[3];
}

void GuiText::setBlurGlowColor(float blur, const glm::vec4 & c)
{
	blurGlowColor = c;
	blurGlowIntensity = blur;
	blurAlpha = c[3];
}

int GuiText::getTextWidth(int ind)
{
	if(ind < 0 || ind >= (int) textDyn.size())
		return this->getTextWidth();

	return font->getWidth(textDyn[ind], currentSize);
}

const wchar_t * GuiText::getDynText(int ind)
{
	if(ind < 0 || ind >= (int) textDyn.size())
		return text;

	return textDyn[ind];
}

/**
 * Change font
 */
bool GuiText::setFont(FreeTypeGX *f)
{
	if(!f)
		return false;

	font = f;
	textWidth = font->getWidth(text, currentSize);
	return true;
}

std::string GuiText::toUTF8(void) const
{
	if(!text)
		return std::string();

    char *pUtf8 = FreeTypeGX::wideCharToUTF8(text);
    if(!pUtf8)
		return std::string();

    std::string strOutput(pUtf8);

    delete [] pUtf8;

	return strOutput;
}

void GuiText::makeDottedText()
{
	int pos = textDyn.size();
	textDyn.resize(pos + 1);

	int i = 0, currentWidth = 0;
	textDyn[pos] = new (std::nothrow) wchar_t[maxWidth];
	if(!textDyn[pos]) {
		textDyn.resize(pos);
		return;
	}

	while (text[i])
	{
		currentWidth += font->getCharWidth(text[i], currentSize, i > 0 ? text[i - 1] : 0);
		if (currentWidth >= maxWidth && i > 2)
		{
			textDyn[pos][i - 2] = '.';
			textDyn[pos][i - 1] = '.';
			textDyn[pos][i] = '.';
			i++;
			break;
		}

		textDyn[pos][i] = text[i];

		i++;
	}
	textDyn[pos][i] = 0;
}

void GuiText::scrollText(u32 frameCount)
{
	if (textDyn.size() == 0)
	{
		int pos = textDyn.size();
		int i = 0, currentWidth = 0;
		textDyn.resize(pos + 1);

		textDyn[pos] = new (std::nothrow) wchar_t[maxWidth];
		if(!textDyn[pos]) {
			textDyn.resize(pos);
			return;
		}

		while (text[i] && currentWidth < maxWidth)
		{
			textDyn[pos][i] = text[i];

			currentWidth += font->getCharWidth(text[i], currentSize, i > 0 ? text[i - 1] : 0);

			++i;
		}
		textDyn[pos][i] = 0;

		return;
	}

	if (frameCount % textScrollDelay != 0)
	{
		return;
	}

	if (textScrollInitialDelay)
	{
		--textScrollInitialDelay;
		return;
	}

	int stringlen = wcslen(text);

	++textScrollPos;
	if (textScrollPos > stringlen)
	{
		textScrollPos = 0;
		textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	}

	int ch = textScrollPos;
	int pos = textDyn.size() - 1;

	if (!textDyn[pos])
		textDyn[pos] = new (std::nothrow) wchar_t[maxWidth];

	if(!textDyn[pos]) {
		textDyn.resize(pos);
		return;
	}

	int i = 0, currentWidth = 0;

	while (currentWidth < maxWidth)
	{
		if (ch > stringlen - 1)
		{
			textDyn[pos][i++] = ' ';
			currentWidth += font->getCharWidth(L' ', currentSize, ch > 0 ? text[ch - 1] : 0);
			textDyn[pos][i++] = ' ';
			currentWidth += font->getCharWidth(L' ', currentSize, L' ');
			textDyn[pos][i++] = ' ';
			currentWidth += font->getCharWidth(L' ', currentSize, L' ');
			ch = 0;

			if(currentWidth >= maxWidth)
				break;
		}

		textDyn[pos][i] = text[ch];
		currentWidth += font->getCharWidth(text[ch], currentSize, ch > 0 ? text[ch - 1] : 0);
		++ch;
		++i;
	}
	textDyn[pos][i] = 0;
}

void GuiText::wrapText()
{
	if (textDyn.size() > 0) return;

	int i = 0;
	int ch = 0;
	int linenum = 0;
	int lastSpace = -1;
	int lastSpaceIndex = -1;
	int currentWidth = 0;

	while (text[ch] && linenum < linestodraw)
	{
		if (linenum >= (int) textDyn.size())
		{
			textDyn.resize(linenum + 1);
			textDyn[linenum] = new (std::nothrow) wchar_t[maxWidth];
			if(!textDyn[linenum]) {
				textDyn.resize(linenum);
				break;
			}
		}

		textDyn[linenum][i] = text[ch];
		textDyn[linenum][i + 1] = 0;

		currentWidth += font->getCharWidth(text[ch], currentSize, ch > 0 ? text[ch - 1] : 0x0000);

		if (currentWidth >= maxWidth || (text[ch] == '\n'))
		{
            if(text[ch] == '\n')
            {
				lastSpace = -1;
				lastSpaceIndex = -1;
            }
			else if (lastSpace >= 0)
			{
				textDyn[linenum][lastSpaceIndex] = 0; // discard space, and everything after
				ch = lastSpace; // go backwards to the last space
				lastSpace = -1; // we have used this space
				lastSpaceIndex = -1;
			}

			if (linenum + 1 == linestodraw && text[ch + 1] != 0x0000)
			{
			    if(i < 2)
                    i = 2;

				textDyn[linenum][i - 2] = '.';
				textDyn[linenum][i - 1] = '.';
				textDyn[linenum][i] = '.';
				textDyn[linenum][i + 1] = 0;
			}

			currentWidth = 0;
			++linenum;
			i = -1;
		}
		if (text[ch] == ' ' && i >= 0)
		{
			lastSpace = ch;
			lastSpaceIndex = i;
		}
		++ch;
		++i;
	}
}

/**
 * Draw the text on screen
 */
void GuiText::draw(CVideo *pVideo)
{
	if(!text)
		return;

	if(!isVisible())
		return;

    color[3] = getAlpha();
    blurGlowColor[3] = blurAlpha * getAlpha();
	int newSize = size * getScale();

	if(newSize != currentSize)
	{
		currentSize = newSize;

		if(text)
			textWidth = font->getWidth(text, currentSize);
	}

	if(maxWidth > 0 && maxWidth <= textWidth)
	{
		if(wrapMode == DOTTED) // text dotted
		{
			if(textDyn.size() == 0)
				makeDottedText();

			if(textDynWidth.size() != textDyn.size())
            {
                textDynWidth.resize(textDyn.size());

                for(u32 i = 0; i < textDynWidth.size(); i++)
                    textDynWidth[i] = font->getWidth(textDyn[i], currentSize);
            }

			if(textDyn.size() > 0)
				font->drawText(pVideo, getCenterX(), getCenterY(), getDepth(), textDyn[textDyn.size()-1], currentSize, color, alignment, textDynWidth[textDyn.size()-1], defaultBlur, blurGlowIntensity, blurGlowColor);
		}

		else if(wrapMode == SCROLL_HORIZONTAL)
		{
			scrollText(pVideo->getFrameCount());

			if(textDyn.size() > 0)
				font->drawText(pVideo, getCenterX(), getCenterY(), getDepth(), textDyn[textDyn.size()-1], currentSize, color, alignment, maxWidth, defaultBlur, blurGlowIntensity, blurGlowColor);
		}
		else if(wrapMode == WRAP)
		{
			int lineheight = currentSize + 6;
			int yoffset = 0;
			int voffset = 0;

			if(textDyn.size() == 0)
				wrapText();

			if(textDynWidth.size() != textDyn.size())
            {
                textDynWidth.resize(textDyn.size());

                for(u32 i = 0; i < textDynWidth.size(); i++)
                    textDynWidth[i] = font->getWidth(textDyn[i], currentSize);
            }

			if(alignment & ALIGN_MIDDLE)
				voffset = (lineheight * (textDyn.size()-1)) >> 1;

			for(u32 i = 0; i < textDyn.size(); i++)
			{
				font->drawText(pVideo, getCenterX(), getCenterY() + voffset + yoffset, getDepth(), textDyn[i], currentSize, color, alignment, textDynWidth[i], defaultBlur, blurGlowIntensity, blurGlowColor);
                yoffset -= lineheight;
			}
		}
	}
	else
	{
		font->drawText(pVideo, getCenterX(), getCenterY(), getDepth(), text, currentSize, color, alignment, textWidth, defaultBlur, blurGlowIntensity, blurGlowColor);
	}
}
