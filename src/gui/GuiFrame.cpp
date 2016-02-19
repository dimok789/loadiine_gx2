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
#include "GuiFrame.h"

GuiFrame::GuiFrame(GuiFrame *p)
{
	parent = p;
	width = 0;
	height = 0;
	dim = false;

	if(parent)
        parent->append(this);
}

GuiFrame::GuiFrame(f32 w, f32 h, GuiFrame *p)
{
	parent = p;
	width = w;
	height = h;
	dim = false;

	if(parent)
		parent->append(this);
}

GuiFrame::~GuiFrame()
{
	closing(this);

	if(parent)
		parent->remove(this);
}

void GuiFrame::append(GuiElement* e)
{
	if (e == NULL)
		return;

	remove(e);
	elements.push_back(e);
	e->setParent(this);
}

void GuiFrame::insert(GuiElement* e, u32 index)
{
	if (e == NULL || (index >= elements.size()))
		return;

	remove(e);
	elements.insert(elements.begin()+index, e);
	e->setParent(this);
}

void GuiFrame::remove(GuiElement* e)
{
	if (e == NULL)
		return;

	for (u32 i = 0; i < elements.size(); ++i)
	{
		if(e == elements[i])
		{
			elements.erase(elements.begin()+i);
			break;
		}
	}
}

void GuiFrame::removeAll()
{
	elements.clear();
}

void GuiFrame::close()
{
	//Application::instance()->pushForDelete(this);
}

void GuiFrame::dimBackground(bool d)
{
	dim = d;
}

GuiElement* GuiFrame::getGuiElementAt(u32 index) const
{
	if (index >= elements.size())
		return NULL;
	return elements[index];
}

u32 GuiFrame::getSize()
{
	return elements.size();
}

void GuiFrame::resetState()
{
	GuiElement::resetState();

	for (u32 i = 0; i < elements.size(); ++i)
	{
		elements[i]->resetState();
	}
}

void GuiFrame::setState(int s, int c)
{
	GuiElement::setState(s, c);

	for (u32 i = 0; i < elements.size(); ++i)
	{
		elements[i]->setState(s, c);
	}
}

void GuiFrame::clearState(int s, int c)
{
	GuiElement::clearState(s, c);

	for (u32 i = 0; i < elements.size(); ++i)
	{
		elements[i]->clearState(s, c);
	}
}

void GuiFrame::setVisible(bool v)
{
	visible = v;

	for (u32 i = 0; i < elements.size(); ++i)
	{
		elements[i]->setVisible(v);
	}
}

int GuiFrame::getSelected()
{
	// find selected element
	int found = -1;
	for (u32 i = 0; i < elements.size(); ++i)
	{
		if(elements[i]->isStateSet(STATE_SELECTED | STATE_OVER))
		{
			found = i;
			break;
		}
	}
	return found;
}

void GuiFrame::draw(CVideo * v)
{
	if(!this->isVisible() && parentElement)
		return;

	if(parentElement && dim == true)
	{
		//GXColor dimColor = (GXColor){0, 0, 0, 0x70};
		//Menu_DrawRectangle(0, 0, GetZPosition(), screenwidth,screenheight, &dimColor, false, true);
	}

	//! render appended items next frame but allow stop of render if size is reached
	u32 size = elements.size();

	for (u32 i = 0; i < size && i < elements.size(); ++i)
	{
		elements[i]->draw(v);
	}
}

void GuiFrame::updateEffects()
{
	if(!this->isVisible() && parentElement)
		return;

    GuiElement::updateEffects();

	//! render appended items next frame but allow stop of render if size is reached
	u32 size = elements.size();

	for (u32 i = 0; i < size && i < elements.size(); ++i)
	{
		elements[i]->updateEffects();
	}
}

void GuiFrame::update(GuiController * c)
{
	if(isStateSet(STATE_DISABLED) && parentElement)
		return;

	//! update appended items next frame
	u32 size = elements.size();

	for (u32 i = 0; i < size && i < elements.size(); ++i)
	{
		elements[i]->update(c);
	}
}
