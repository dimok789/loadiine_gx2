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
#include "GuiElement.h"

//! TODO remove this!
static int screenwidth = 1280;
static int screenheight = 720;

/**
 * Constructor for the Object class.
 */
GuiElement::GuiElement()
{
	xoffset = 0.0f;
	yoffset = 0.0f;
	zoffset = 0.0f;
	width = 0.0f;
	height = 0.0f;
	alpha = 1.0f;
	scaleX = 1.0f;
	scaleY = 1.0f;
	scaleZ = 1.0f;
	for(int i = 0; i < 4; i++)
        state[i] = STATE_DEFAULT;
	stateChan = -1;
	parentElement = NULL;
	rumble = true;
	selectable = false;
	clickable = false;
	holdable = false;
	visible = true;
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = EFFECT_NONE;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = EFFECT_NONE;
	effectAmountOver = 0;
	effectTargetOver = 0;
	angle = 0.0f;

	// default alignment - align to top left
	alignment = (ALIGN_CENTER | ALIGN_MIDDLE);
}

/**
 * Get the left position of the GuiElement.
 * @see SetLeft()
 * @return Left position in pixel.
 */
f32 GuiElement::getLeft()
{
	f32 pWidth = 0;
	f32 pLeft = 0;
	f32 pScaleX = 1.0f;

	if(parentElement)
	{
		pWidth = parentElement->getWidth();
		pLeft = parentElement->getLeft();
		pScaleX = parentElement->getScaleX();
	}

	pLeft += xoffsetDyn;

	f32 x = pLeft;

    //! TODO: the conversion from int to float and back to int is bad for performance, change that
	if(alignment & ALIGN_CENTER)
	{
		x = pLeft + pWidth * 0.5f * pScaleX - width * 0.5f * getScaleX();
	}
	else if(alignment & ALIGN_RIGHT)
	{
		x = pLeft + pWidth * pScaleX - width * getScaleX();
	}

	return x + xoffset;
}

/**
 * Get the top position of the GuiElement.
 * @see SetTop()
 * @return Top position in pixel.
 */
f32 GuiElement::getTop()
{
	f32 pHeight = 0;
	f32 pTop = 0;
	f32 pScaleY = 1.0f;

	if(parentElement)
	{
		pHeight = parentElement->getHeight();
		pTop = parentElement->getTop();
		pScaleY = parentElement->getScaleY();
	}

	pTop += yoffsetDyn;

	f32 y = pTop;

    //! TODO: the conversion from int to float and back to int is bad for performance, change that
	if(alignment & ALIGN_MIDDLE)
	{
		y = pTop + pHeight * 0.5f * pScaleY - height * 0.5f * getScaleY();
	}
	else if(alignment & ALIGN_BOTTOM)
	{
		y = pTop + pHeight * pScaleY - height * getScaleY();
	}

	return y + yoffset;
}

void GuiElement::setEffect(int eff, int amount, int target)
{
	if(eff & EFFECT_SLIDE_IN)
	{
		// these calculations overcompensate a little
		if(eff & EFFECT_SLIDE_TOP)
		{
			if(eff & EFFECT_SLIDE_FROM)
				yoffsetDyn = (int) -getHeight()*scaleY;
			else
				yoffsetDyn = -screenheight;
		}
		else if(eff & EFFECT_SLIDE_LEFT)
		{
			if(eff & EFFECT_SLIDE_FROM)
				xoffsetDyn = (int) -getWidth()*scaleX;
			else
				xoffsetDyn = -screenwidth;
		}
		else if(eff & EFFECT_SLIDE_BOTTOM)
		{
			if(eff & EFFECT_SLIDE_FROM)
				yoffsetDyn = (int) getHeight()*scaleY;
			else
				yoffsetDyn = screenheight;
		}
		else if(eff & EFFECT_SLIDE_RIGHT)
		{
			if(eff & EFFECT_SLIDE_FROM)
				xoffsetDyn = (int) getWidth()*scaleX;
			else
				xoffsetDyn = screenwidth;
		}
	}
	if((eff & EFFECT_FADE) && amount > 0)
	{
		alphaDyn = 0;
	}
	else if((eff & EFFECT_FADE) && amount < 0)
	{
		alphaDyn = alpha;
	}
	effects |= eff;
	effectAmount = amount;
	effectTarget = target;
}

//!Sets an effect to be enabled on wiimote cursor over
//!\param e Effect to enable
//!\param a Amount of the effect (usage varies on effect)
//!\param t Target amount of the effect (usage varies on effect)
void GuiElement::setEffectOnOver(int e, int a, int t)
{
	effectsOver |= e;
	effectAmountOver = a;
	effectTargetOver = t;
}

void GuiElement::resetEffects()
{
	yoffsetDyn = 0;
	xoffsetDyn = 0;
	alphaDyn = -1;
	scaleDyn = 1;
	effects = EFFECT_NONE;
	effectAmount = 0;
	effectTarget = 0;
	effectsOver = EFFECT_NONE;
	effectAmountOver = 0;
	effectTargetOver = 0;
}
void GuiElement::updateEffects()
{
	if(!this->isVisible() && parentElement)
		return;

	if(effects & (EFFECT_SLIDE_IN | EFFECT_SLIDE_OUT | EFFECT_SLIDE_FROM))
	{
		if(effects & EFFECT_SLIDE_IN)
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= 0)
				{
					xoffsetDyn = 0;
					effects = 0;
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= 0)
				{
					yoffsetDyn = 0;
					effects = 0;
                    effectFinished(this);
				}
			}
		}
		else
		{
			if(effects & EFFECT_SLIDE_LEFT)
			{
				xoffsetDyn -= effectAmount;

				if(xoffsetDyn <= -screenwidth) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
				else if((effects & EFFECT_SLIDE_FROM) && xoffsetDyn <= -getWidth()) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_RIGHT)
			{
				xoffsetDyn += effectAmount;

				if(xoffsetDyn >= screenwidth) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
				else if((effects & EFFECT_SLIDE_FROM) && xoffsetDyn >= getWidth()*scaleX) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_TOP)
			{
				yoffsetDyn -= effectAmount;

				if(yoffsetDyn <= -screenheight) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
				else if((effects & EFFECT_SLIDE_FROM) && yoffsetDyn <= -getHeight()) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
			}
			else if(effects & EFFECT_SLIDE_BOTTOM)
			{
				yoffsetDyn += effectAmount;

				if(yoffsetDyn >= screenheight) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
				else if((effects & EFFECT_SLIDE_FROM) && yoffsetDyn >= getHeight()) {
					effects = 0; // shut off effect
                    effectFinished(this);
				}
			}
		}
	}
	else if(effects & EFFECT_FADE)
	{
		alphaDyn += effectAmount * (1.0f / 255.0f);

		if(effectAmount < 0 && alphaDyn <= 0)
		{
			alphaDyn = 0;
			effects = 0; // shut off effect
			effectFinished(this);
		}
		else if(effectAmount > 0 && alphaDyn >= alpha)
		{
			alphaDyn = alpha;
			effects = 0; // shut off effect
			effectFinished(this);
		}
	}
	else if(effects & EFFECT_SCALE)
	{
		scaleDyn += effectAmount * 0.01f;

		if((effectAmount < 0 && scaleDyn <= (effectTarget * 0.01f))
			|| (effectAmount > 0 && scaleDyn >= (effectTarget * 0.01f)))
		{
			scaleDyn = effectTarget * 0.01f;
			effects = 0; // shut off effect
			effectFinished(this);
		}
	}
}
