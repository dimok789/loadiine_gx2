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
#ifndef GUI_TEXT_H_
#define GUI_TEXT_H_

#include "GuiElement.h"
//!Forward declaration
class FreeTypeGX;

//!Display, manage, and manipulate text in the GUI
class GuiText : public GuiElement
{
public:
    //!Constructor
    GuiText();
    //!\param t Text
    //!\param s Font size
    //!\param c Font color
    GuiText(const char * t, s32 s, const glm::vec4 & c);
    //!\overload
    //!\param t Text
    //!\param s Font size
    //!\param c Font color
    GuiText(const wchar_t * t, s32 s, const glm::vec4 & c);
    //!\overload
    //!\Assumes SetPresets() has been called to setup preferred text attributes
    //!\param t Text
    GuiText(const char * t);
    //!Destructor
    virtual ~GuiText();
    //!Sets the text of the GuiText element
    //!\param t Text
    virtual void setText(const char * t);
    virtual void setText(const wchar_t * t);
    virtual void setTextf(const char *format, ...) __attribute__((format(printf,2,3)));
    //!Sets up preset values to be used by GuiText(t)
    //!Useful when printing multiple text elements, all with the same attributes set
    //!\param sz Font size
    //!\param c Font color
    //!\param w Maximum width of texture image (for text wrapping)
    //!\param wrap Wrapmode when w>0
    //!\param a Text alignment
    static void setPresets(s32 sz, const glm::vec4 & c, s32 w, s32 a);
    static void setPresetFont(FreeTypeGX *font);
    //!Sets the font size
    //!\param s Font size
    void setFontSize(s32 s);
    //!Sets the maximum width of the drawn texture image
    //!If the text exceeds this, it is wrapped to the next line
    //!\param w Maximum width
    //!\param m WrapMode
    void setMaxWidth(s32 w = 0, s32 m = WRAP);
    //!Sets the font color
    //!\param c Font color
    void setColor(const glm::vec4 & c);

    void setBlurGlowColor(float blurIntensity,  const glm::vec4 & c);

    void setTextBlur(float blur) { defaultBlur = blur; }
    //!Get the original text as char
    virtual const wchar_t * getText() const { return text; }
    virtual std::string toUTF8(void) const;
    //!Get the Horizontal Size of Text
    s32 getTextWidth() { return textWidth; }
    s32 getTextWidth(s32 ind);
    //!Get the max textwidth
    s32 getTextMaxWidth() { return maxWidth; }
    //!Get fontsize
    s32 getFontSize() { return size; };
    //!Set max lines to draw
    void setLinesToDraw(s32 l) { linestodraw = l; }
    //!Get current Textline (for position calculation)
    const wchar_t * getDynText(s32 ind = 0);
    virtual const wchar_t * getTextLine(s32 ind) { return getDynText(ind); };
    //!Change the font
    bool setFont(FreeTypeGX *font);
    //! virtual function used in child classes
    virtual s32 getStartWidth() { return 0; };
    //!Constantly called to draw the text
    void draw(CVideo *pVideo);
    //! text enums
    enum
    {
        WRAP,
        DOTTED,
        SCROLL_HORIZONTAL,
        SCROLL_NONE
    };
protected:
    static FreeTypeGX * presentFont;
    static s32 presetSize;
    static s32 presetMaxWidth;
    static float presetInternalRenderingScale;
    static s32 presetAlignment;
    static GX2ColorF32 presetColor;

    //!Clear the dynamic text
    void clearDynamicText();
    //!Create a dynamic dotted text if the text is too long
    void makeDottedText();
    //!Scroll the text once
    void scrollText(u32 frameCount);
    //!Wrap the text to several lines
    void wrapText();

    wchar_t * text;
    std::vector<wchar_t *> textDyn;
    std::vector<uint16_t> textDynWidth;
    s32 wrapMode; //!< Wrapping toggle
    s32 textScrollPos; //!< Current starting index of text string for scrolling
    s32 textScrollInitialDelay; //!< Delay to wait before starting to scroll
    s32 textScrollDelay; //!< Scrolling speed
    s32 size; //!< Font size
    s32 maxWidth; //!< Maximum width of the generated text object (for text wrapping)
    FreeTypeGX *font;
    s32 textWidth;
    s32 currentSize;
    s32 linestodraw;
    glm::vec4 color;
    float defaultBlur;
    float blurGlowIntensity;
    float blurAlpha;
    glm::vec4 blurGlowColor;
    float internalRenderingScale;
};

#endif
