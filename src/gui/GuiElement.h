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
#ifndef GUI_ELEMENT_H_
#define GUI_ELEMENT_H_

#include <string>
#include <vector>
#include <gctypes.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <math.h>

#include "sigslot.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "dynamic_libs/gx2_types.h"
#include "resources/Resources.h"
#include "system/AsyncDeleter.h"
#include "utils/logger.h"

enum
{
	EFFECT_NONE				= 0x00,
	EFFECT_SLIDE_TOP		= 0x01,
	EFFECT_SLIDE_BOTTOM		= 0x02,
	EFFECT_SLIDE_RIGHT		= 0x04,
	EFFECT_SLIDE_LEFT		= 0x08,
	EFFECT_SLIDE_IN			= 0x10,
	EFFECT_SLIDE_OUT		= 0x20,
	EFFECT_SLIDE_FROM		= 0x40,
	EFFECT_FADE				= 0x80,
	EFFECT_SCALE			= 0x100,
	EFFECT_COLOR_TRANSITION	= 0x200
};

enum
{
	ALIGN_LEFT			= 0x01,
	ALIGN_CENTER		= 0x02,
	ALIGN_RIGHT			= 0x04,
	ALIGN_TOP			= 0x10,
	ALIGN_MIDDLE		= 0x20,
	ALIGN_BOTTOM		= 0x40,
	ALIGN_TOP_LEFT		= ALIGN_LEFT | ALIGN_TOP,
	ALIGN_TOP_CENTER	= ALIGN_CENTER | ALIGN_TOP,
	ALIGN_TOP_RIGHT		= ALIGN_RIGHT | ALIGN_TOP,
	ALIGN_CENTERED		= ALIGN_CENTER | ALIGN_MIDDLE,
};

//!Forward declaration
class GuiController;
class CVideo;

//!Primary GUI class. Most other classes inherit from this class.
class GuiElement : public AsyncDeleter::Element
{
	public:
		//!Constructor
		GuiElement();
		//!Destructor
		virtual ~GuiElement() {}
		//!Set the element's parent
		//!\param e Pointer to parent element
		virtual void setParent(GuiElement * e) { parentElement = e; }
		//!Gets the element's parent
		//!\return Pointer to parent element
		virtual GuiElement * getParent() { return parentElement; }
		//!Gets the current leftmost coordinate of the element
		//!Considers horizontal alignment, x offset, width, and parent element's GetLeft() / GetWidth() values
		//!\return left coordinate
		virtual f32 getLeft();
		//!Gets the current topmost coordinate of the element
		//!Considers vertical alignment, y offset, height, and parent element's GetTop() / GetHeight() values
		//!\return top coordinate
		virtual f32 getTop();
		//!Gets the current Z coordinate of the element
		//!\return Z coordinate
		virtual f32 getDepth()
		{
			f32 zParent = 0.0f;

			if(parentElement)
				zParent = parentElement->getDepth();

			return zParent+zoffset;
		}

		virtual f32 getCenterX(void)
		{
            f32 pCenterX = 0.0f;

            if(parentElement)
                pCenterX = parentElement->getCenterX();

            pCenterX += xoffset + xoffsetDyn;

            if(alignment & ALIGN_LEFT)
            {
                f32 pWidth = 0.0f;
                f32 pScale = 0.0f;

                if(parentElement)
                {
                    pWidth = parentElement->getWidth();
                    pScale = parentElement->getScaleX();
                }

                pCenterX -= pWidth * 0.5f * pScale - width * 0.5f * getScaleX();
            }
            else if(alignment & ALIGN_RIGHT)
            {
                f32 pWidth = 0.0f;
                f32 pScale = 0.0f;

                if(parentElement)
                {
                    pWidth = parentElement->getWidth();
                    pScale = parentElement->getScaleX();
                }

                pCenterX += pWidth * 0.5f * pScale - width * 0.5f * getScaleX();
            }
            return pCenterX;
		}

		virtual f32 getCenterY(void)
		{
            f32 pCenterY = 0.0f;

            if(parentElement)
                pCenterY = parentElement->getCenterY();

            pCenterY += yoffset + yoffsetDyn;

            if(alignment & ALIGN_TOP)
            {
                f32 pHeight = 0.0f;
                f32 pScale = 0.0f;

                if(parentElement)
                {
                    pHeight = parentElement->getHeight();
                    pScale = parentElement->getScaleY();
                }

                pCenterY += pHeight * 0.5f * pScale - height * 0.5f * getScaleY();
            }
            else if(alignment & ALIGN_BOTTOM)
            {
                f32 pHeight = 0.0f;
                f32 pScale = 0.0f;

                if(parentElement)
                {
                    pHeight = parentElement->getHeight();
                    pScale = parentElement->getScaleY();
                }

                pCenterY -= pHeight * 0.5f * pScale - height * 0.5f * getScaleY();
            }
            return pCenterY;
		}
		//!Gets elements xoffset
		virtual f32 getOffsetX() { return xoffset; }
		//!Gets elements yoffset
		virtual f32 getOffsetY() { return yoffset; }
		//!Gets the current width of the element. Does not currently consider the scale
		//!\return width
		virtual f32 getWidth() { return width; };
		//!Gets the height of the element. Does not currently consider the scale
		//!\return height
		virtual f32 getHeight() { return height; }
		//!Sets the size (width/height) of the element
		//!\param w Width of element
		//!\param h Height of element
		virtual void setSize(f32 w, f32 h)
		{
			width = w;
			height = h;
		}
		//!Sets the element's visibility
		//!\param v Visibility (true = visible)
		virtual void setVisible(bool v)
		{
			visible = v;
			visibleChanged(this, v);
		}
		//!Checks whether or not the element is visible
		//!\return true if visible, false otherwise
		virtual bool isVisible() const { return !isStateSet(STATE_HIDDEN) && visible; };
		//!Checks whether or not the element is selectable
		//!\return true if selectable, false otherwise
		virtual bool isSelectable()
		{
			return !isStateSet(STATE_DISABLED) && selectable;
		}
		//!Checks whether or not the element is clickable
		//!\return true if clickable, false otherwise
		virtual bool isClickable()
		{
			return !isStateSet(STATE_DISABLED) && clickable;
		}
		//!Checks whether or not the element is holdable
		//!\return true if holdable, false otherwise
		virtual bool isHoldable() { return !isStateSet(STATE_DISABLED) && holdable; }
		//!Sets whether or not the element is selectable
		//!\param s Selectable
		virtual void setSelectable(bool s) { selectable = s; }
		//!Sets whether or not the element is clickable
		//!\param c Clickable
		virtual void setClickable(bool c) { clickable = c; }
		//!Sets whether or not the element is holdable
		//!\param c Holdable
		virtual void setHoldable(bool d) { holdable = d; }
        //!Sets the element's state
        //!\param s State (STATE_DEFAULT, STATE_SELECTED, STATE_CLICKED, STATE_DISABLED)
        //!\param c Controller channel (0-3, -1 = none)
        virtual void setState(int s, int c = -1)
        {
            if(c >= 0 && c < 4)
            {
                state[c] |= s;
            }
            else
            {
                for(int i = 0; i < 4; i++)
                    state[i] |= s;
            }
            stateChan = c;
            stateChanged(this, s, c);
        }
        virtual void clearState(int s, int c = -1)
        {
            if(c >= 0 && c < 4)
            {
                state[c] &= ~s;
            }
            else
            {
                for(int i = 0; i < 4; i++)
                    state[i] &= ~s;
            }
            stateChan = c;
            stateChanged(this, s, c);
        }
        virtual bool isStateSet(int s, int c = -1) const
        {
            if(c >= 0 && c < 4)
            {
                return (state[c] & s) != 0;
            }
            else
            {
                for(int i = 0; i < 4; i++)
                   if((state[i] & s) != 0)
                        return true;

                return false;
            }
        }
		//!Gets the element's current state
		//!\return state
		virtual int getState(int c = 0) { return state[c]; };
		//!Gets the controller channel that last changed the element's state
		//!\return Channel number (0-3, -1 = no channel)
		virtual int getStateChan() { return stateChan; };
		//!Resets the element's state to STATE_DEFAULT
		virtual void resetState()
		{
            for(int i = 0; i < 4; i++)
                state[i] = STATE_DEFAULT;
            stateChan = -1;
		}
		//!Sets the element's alpha value
		//!\param a alpha value
		virtual void setAlpha(f32 a) { alpha = a; }
		//!Gets the element's alpha value
		//!Considers alpha, alphaDyn, and the parent element's getAlpha() value
		//!\return alpha
		virtual f32 getAlpha()
		{
			f32 a;

			if(alphaDyn >= 0)
				a = alphaDyn;
			else
				a = alpha;

			if(parentElement)
				a = (a * parentElement->getAlpha());

			return a;
		}
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		virtual void setScale(float s)
		{
			scaleX = s;
			scaleY = s;
			scaleZ = s;
		}
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		virtual void setScaleX(float s) { scaleX = s; }
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		virtual void setScaleY(float s) { scaleY = s; }
		//!Sets the element's scale
		//!\param s scale (1 is 100%)
		virtual void setScaleZ(float s) { scaleZ = s; }
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's getScale() value
		virtual float getScale()
		{
			float s = 0.5f * (scaleX+scaleY) * scaleDyn;

			if(parentElement)
				s *= parentElement->getScale();

			return s;
		}
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's getScale() value
		virtual float getScaleX()
		{
			float s = scaleX * scaleDyn;

			if(parentElement)
				s *= parentElement->getScaleX();

			return s;
		}
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's getScale() value
		virtual float getScaleY()
		{
			float s = scaleY * scaleDyn;

			if(parentElement)
				s *= parentElement->getScaleY();

			return s;
		}
		//!Gets the element's current scale
		//!Considers scale, scaleDyn, and the parent element's getScale() value
		virtual float getScaleZ()
		{
		    float s = scaleZ;

			if(parentElement)
				s *= parentElement->getScaleZ();

			return s;
		}
		//!Checks whether rumble was requested by the element
		//!\return true is rumble was requested, false otherwise
		virtual bool isRumbleActive() { return rumble; }
		//!Sets whether or not the element is requesting a rumble event
		//!\param r true if requesting rumble, false if not
		virtual void setRumble(bool r) { rumble = r; }
		//!Set an effect for the element
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		virtual void setEffect(int e, int a, int t=0);
		//!Sets an effect to be enabled on wiimote cursor over
		//!\param e Effect to enable
		//!\param a Amount of the effect (usage varies on effect)
		//!\param t Target amount of the effect (usage varies on effect)
		virtual void setEffectOnOver(int e, int a, int t=0);
		//!Shortcut to SetEffectOnOver(EFFECT_SCALE, 4, 110)
		virtual void setEffectGrow() { setEffectOnOver(EFFECT_SCALE, 4, 110); }
		//!Reset all applied effects
		virtual void resetEffects();
		//!Gets the current element effects
		//!\return element effects
		virtual int getEffect() const { return effects; }
		//!\return true if element animation is on going
		virtual bool isAnimated() const { return (parentElement != 0) && (getEffect() > 0); }
		//!Checks whether the specified coordinates are within the element's boundaries
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\return true if contained within, false otherwise
		virtual bool isInside(f32 x, f32 y)
		{
			return (	x > (this->getCenterX() - getScaleX() * getWidth() * 0.5f)
					&&	x < (this->getCenterX() + getScaleX() * getWidth() * 0.5f)
					&&	y > (this->getCenterY() - getScaleY() * getHeight() * 0.5f)
					&&	y < (this->getCenterY() + getScaleY() * getHeight() * 0.5f));
		}
		//!Sets the element's position
		//!\param x X coordinate
		//!\param y Y coordinate
		virtual void setPosition(f32 x, f32 y)
		{
			xoffset = x;
			yoffset = y;
		}
		//!Sets the element's position
		//!\param x X coordinate
		//!\param y Y coordinate
		//!\param z Z coordinate
		virtual void setPosition(f32 x, f32 y, f32 z)
		{
			xoffset = x;
			yoffset = y;
			zoffset = z;
		}
		//!Gets whether or not the element is in STATE_SELECTED
		//!\return true if selected, false otherwise
		virtual int getSelected() { return -1; }
		//!Sets the element's alignment respective to its parent element
		//!Bitwise ALIGN_LEFT | ALIGN_RIGHT | ALIGN_CENTRE, ALIGN_TOP, ALIGN_BOTTOM, ALIGN_MIDDLE)
		//!\param align Alignment
		virtual void setAlignment(int a) { alignment = a; }
		//!Gets the element's alignment
		virtual int getAlignment() const { return alignment; }
		//!Angle of the object
		virtual void setAngle(f32 a) { angle = a; }
		//!Angle of the object
		virtual f32 getAngle() const { f32 r_angle = angle; if(parentElement) r_angle += parentElement->getAngle(); return r_angle; }
		//!Called constantly to allow the element to respond to the current input data
		//!\param t Pointer to a GuiController, containing the current input data from PAD/WPAD/VPAD
		virtual void update(GuiController * t) { }
		//!Called constantly to redraw the element
		virtual void draw(CVideo * v) { }
		//!Updates the element's effects (dynamic values)
		//!Called by Draw(), used for animation purposes
		virtual void updateEffects();

        typedef struct _POINT {
            s32 x;
            s32 y;
        } POINT;

        enum
        {
            STATE_DEFAULT = 0,
            STATE_SELECTED = 0x01,
            STATE_CLICKED = 0x02,
            STATE_HELD = 0x04,
            STATE_OVER = 0x08,
            STATE_HIDDEN = 0x10,
            STATE_DISABLED = 0x80
        };

		//! Switch pointer from control to screen position
		POINT PtrToScreen(POINT p)
		{
		    //! TODO for 3D
			//POINT r = { p.x + getLeft(), p.y + getTop() };
			return p;
		}
		//! Switch pointer screen to control position
		POINT PtrToControl(POINT p)
		{
		    //! TODO for 3D
			//POINT r = { p.x - getLeft(), p.y - getTop() };
			return p;
		}
		//! Signals
		sigslot::signal2<GuiElement *, bool> visibleChanged;
		sigslot::signal3<GuiElement *, int, int> stateChanged;
		sigslot::signal1<GuiElement *> effectFinished;
	protected:
		bool rumble; //!< Wiimote rumble (on/off) - set to on when this element requests a rumble event
		bool visible; //!< Visibility of the element. If false, Draw() is skipped
		bool selectable; //!< Whether or not this element selectable (can change to SELECTED state)
		bool clickable; //!< Whether or not this element is clickable (can change to CLICKED state)
		bool holdable; //!< Whether or not this element is holdable (can change to HELD state)
		f32 width; //!< Element width
		f32 height; //!< Element height
		f32 xoffset; //!< Element X offset
		f32 yoffset; //!< Element Y offset
		f32 zoffset; //!< Element Z offset
		f32 alpha; //!< Element alpha value (0-255)
		f32 angle; //!< Angle of the object (0-360)
		f32 scaleX; //!< Element scale (1 = 100%)
		f32 scaleY; //!< Element scale (1 = 100%)
		f32 scaleZ; //!< Element scale (1 = 100%)
		int alignment; //!< Horizontal element alignment, respective to parent element
		int state[4]; //!< Element state (DEFAULT, SELECTED, CLICKED, DISABLED)
		int stateChan; //!< Which controller channel is responsible for the last change in state
		GuiElement * parentElement; //!< Parent element

		//! TODO: Move me to some Animator class
		int xoffsetDyn; //!< Element X offset, dynamic (added to xoffset value for animation effects)
		int yoffsetDyn; //!< Element Y offset, dynamic (added to yoffset value for animation effects)
		f32 alphaDyn; //!< Element alpha, dynamic (multiplied by alpha value for blending/fading effects)
		f32 scaleDyn; //!< Element scale, dynamic (multiplied by alpha value for blending/fading effects)
		int effects; //!< Currently enabled effect(s). 0 when no effects are enabled
		int effectAmount; //!< Effect amount. Used by different effects for different purposes
		int effectTarget; //!< Effect target amount. Used by different effects for different purposes
		int effectsOver; //!< Effects to enable when wiimote cursor is over this element. Copied to effects variable on over event
		int effectAmountOver; //!< EffectAmount to set when wiimote cursor is over this element
		int effectTargetOver; //!< EffectTarget to set when wiimote cursor is over this element
};

#endif
