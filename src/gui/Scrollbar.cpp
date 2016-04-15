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
#include "Scrollbar.h"
#include "resources/Resources.h"

Scrollbar::Scrollbar(int h)
    : touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
{
	SelItem = 0;
	SelInd = 0;
	PageSize = 0;
	EntrieCount = 0;
	ScrollSpeed = 15;
	ScrollState = 0;

	listChanged.connect(this, &Scrollbar::setScrollboxPosition);

	btnSoundClick = Resources::GetSound("button_click.mp3");
	scrollbarLine = Resources::GetImageData("scrollbarLine.png");
	arrowDown = Resources::GetImageData("scrollbarArrowDown.png");
	arrowUp = Resources::GetImageData("scrollbarArrowUp.png");
	scrollbarBox = Resources::GetImageData("scrollbarButton.png");

	height = h;
	width = scrollbarBox->getWidth();

	MaxHeight = height * 0.5f - (scrollbarBox ? (scrollbarBox->getHeight() * 0.5f) : 0) - (arrowUp ? arrowUp->getHeight() : 0);
	MinHeight = -height * 0.5f + (scrollbarBox ? (scrollbarBox->getHeight() * 0.5f) : 0) + (arrowDown ? arrowDown->getHeight() : 0);

	scrollbarLineImg = new GuiImage(scrollbarLine);
	scrollbarLineImg->setParent(this);
	scrollbarLineImg->setAlignment(ALIGN_CENTER | ALIGN_MIDDLE);
	scrollbarLineImg->setPosition(0, 0);

	arrowDownImg = new GuiImage(arrowDown);
	arrowUpImg = new GuiImage(arrowUp);
	scrollbarBoxImg = new GuiImage(scrollbarBox);

	arrowUpBtn = new GuiButton(arrowUpImg->getWidth(), arrowUpImg->getHeight());
	arrowUpBtn->setParent(this);
	arrowUpBtn->setImage(arrowUpImg);
	arrowUpBtn->setAlignment(ALIGN_CENTER | ALIGN_TOP);
	arrowUpBtn->setPosition(0, 0);
	arrowUpBtn->setTrigger(&touchTrigger, 0);
	arrowUpBtn->setTrigger(&wpadTouchTrigger, 1);
	arrowUpBtn->setSoundClick(btnSoundClick);
	arrowUpBtn->setEffectGrow();
	arrowUpBtn->clicked.connect(this, &Scrollbar::OnUpButtonClick);

	arrowDownBtn = new GuiButton(arrowDownImg->getWidth(), arrowDownImg->getHeight());
	arrowDownBtn->setParent(this);
	arrowDownBtn->setImage(arrowDownImg);
	arrowDownBtn->setAlignment(ALIGN_CENTER | ALIGN_BOTTOM);
	arrowDownBtn->setPosition(0, 0);
	arrowDownBtn->setTrigger(&touchTrigger, 0);
	arrowDownBtn->setTrigger(&wpadTouchTrigger, 1);
	arrowDownBtn->setSoundClick(btnSoundClick);
	arrowDownBtn->setEffectGrow();
	arrowDownBtn->clicked.connect(this, &Scrollbar::OnDownButtonClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->getWidth(), height);
	scrollbarBoxBtn->setParent(this);
	scrollbarBoxBtn->setImage(scrollbarBoxImg);
	scrollbarBoxBtn->setAlignment(ALIGN_CENTER | ALIGN_TOP);
	scrollbarBoxBtn->setPosition(0, MaxHeight);
	scrollbarBoxBtn->setHoldable(true);
	scrollbarBoxBtn->setTrigger(&touchTrigger, 0);
	scrollbarBoxBtn->setTrigger(&wpadTouchTrigger, 1);
	scrollbarBoxBtn->setEffectGrow();
	scrollbarBoxBtn->held.connect(this, &Scrollbar::OnBoxButtonHold);
}

Scrollbar::~Scrollbar()
{
	Resources::RemoveSound(btnSoundClick);
	Resources::RemoveImageData(scrollbarLine);
	Resources::RemoveImageData(arrowDown);
	Resources::RemoveImageData(arrowUp);
	Resources::RemoveImageData(scrollbarBox);

	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete scrollbarLineImg;

	delete arrowDownImg;
	delete arrowUpImg;
	delete scrollbarBoxImg;
}

void Scrollbar::ScrollOneUp()
{
    if(SelItem == 0 && SelInd > 0)
    {
        // move list up by 1
        --SelInd;
    }
    else if(SelInd+SelItem > 0)
    {
        --SelItem;
    }
}

void Scrollbar::ScrollOneDown()
{
    if(SelInd+SelItem + 1 < EntrieCount)
    {
        if(SelItem == PageSize-1)
        {
            // move list down by 1
            SelInd++;
        }
        else
        {
            SelItem++;
        }
    }
}

void Scrollbar::OnUpButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(ScrollState < ScrollSpeed)
		return;

	ScrollOneUp();

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void Scrollbar::OnDownButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(ScrollState < ScrollSpeed)
		return;

	ScrollOneDown();

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void Scrollbar::OnBoxButtonHold(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(EntrieCount == 0)
        return;

	if(!controller->data.validPointer)
		return;

	int y = controller->data.y - this->getCenterY();

	int positionWiimote = LIMIT(y - MinHeight, 0, MaxHeight - MinHeight);

	int newSelected = (EntrieCount - 1) - (int) ((float) positionWiimote / (float) (MaxHeight-MinHeight) * (float) (EntrieCount-1));

    int diff = newSelected-SelInd-SelItem;

    if(newSelected <= 0)
    {
        SelItem = 0;
        SelInd = 0;
    }
    else if(newSelected >= EntrieCount-1)
    {
        SelItem = (PageSize-1 < EntrieCount-1) ? PageSize-1 : EntrieCount-1;
        SelInd = EntrieCount-PageSize;
    }
    else if(newSelected < PageSize && SelInd == 0 && diff < 0)
    {
        SelItem = std::max(SelItem+diff, 0);
    }
    else if(EntrieCount-newSelected < PageSize && SelInd == EntrieCount-PageSize && diff > 0)
    {
        SelItem = std::min(SelItem+diff, PageSize-1);
    }
    else
    {
        SelInd = LIMIT(SelInd+diff, 0, ((EntrieCount-PageSize < 0) ? 0 : EntrieCount-PageSize));
    }

	ScrollState = 0;
	listChanged(SelItem, SelInd);
}

void Scrollbar::SetPageSize(int size)
{
	if(PageSize == size)
		return;

	PageSize = size;
	listChanged(SelItem, SelInd);
}

void Scrollbar::SetSelectedItem(int pos)
{
	if(SelItem == pos)
		return;

	SelItem = LIMIT(pos, 0, EntrieCount-1);
	listChanged(SelItem, SelInd);
}

void Scrollbar::SetSelectedIndex(int pos)
{
	if(SelInd == pos)
		return;

	SelInd = pos;
	listChanged(SelItem, SelInd);
}

void Scrollbar::SetEntrieCount(int cnt)
{
	if(EntrieCount == cnt)
		return;

	EntrieCount = cnt;
	listChanged(SelItem, SelInd);
}

void Scrollbar::setScrollboxPosition(int SelItem, int SelInd)
{
    int position = MaxHeight-(MaxHeight-MinHeight)*(SelInd+SelItem)/(EntrieCount-1);

    if(position < MinHeight || (SelInd+SelItem >= EntrieCount-1))
        position = MinHeight;
    else if(position > MaxHeight || (SelInd+SelItem) == 0)
        position = MaxHeight;

    scrollbarBoxBtn->setPosition(0, position);
}

void Scrollbar::draw(CVideo * video)
{
	scrollbarLineImg->draw(video);
	arrowUpBtn->draw(video);
	arrowDownBtn->draw(video);
	scrollbarBoxBtn->draw(video);

	updateEffects();
}

void Scrollbar::update(GuiController * t)
{
	if(this->isStateSet(STATE_DISABLED))
		return;

	arrowUpBtn->update(t);
	arrowDownBtn->update(t);
	scrollbarBoxBtn->update(t);

	++ScrollState;
}

