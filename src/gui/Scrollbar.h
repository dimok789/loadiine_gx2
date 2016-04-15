/***************************************************************************
 * Copyright (C) 2011
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef SCROLLBAR_HPP_
#define SCROLLBAR_HPP_

#include "gui/GuiElement.h"
#include "gui/GuiButton.h"

class Scrollbar : public GuiElement, public sigslot::has_slots<>
{
	public:
		Scrollbar(int height);
		virtual ~Scrollbar();
		void ScrollOneUp();
		void ScrollOneDown();
		int GetSelectedItem() { return SelItem; }
		int GetSelectedIndex() { return SelInd; }
		void draw(CVideo * video);
		void update(GuiController * t);

		//! Signals
		sigslot::signal2<int, int> listChanged;
		//! Slots
		void SetPageSize(int size);
		void SetRowSize(int size);
		void SetSelectedItem(int pos);
		void SetSelectedIndex(int pos);
		void SetEntrieCount(int cnt);
	protected:
		void setScrollboxPosition(int SelItem, int SelInd);
		void OnUpButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
		void OnDownButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
		void OnBoxButtonHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

		u32 ScrollState;
		u16 ScrollSpeed;

		int MinHeight;
		int MaxHeight;
		int SelItem;
		int SelInd;
		int PageSize;
		int EntrieCount;
		int pressedChan;

		GuiButton * arrowUpBtn;
		GuiButton * arrowDownBtn;
		GuiButton * scrollbarBoxBtn;
		GuiImage * scrollbarLineImg;
		GuiImage * arrowDownImg;
		GuiImage * arrowUpImg;
		GuiImage * scrollbarBoxImg;
		GuiImageData * scrollbarLine;
		GuiImageData * arrowDown;
		GuiImageData * arrowUp;
		GuiImageData * scrollbarBox;
		GuiSound * btnSoundClick;

		GuiTrigger touchTrigger;
		GuiTrigger wpadTouchTrigger;
};

#endif
