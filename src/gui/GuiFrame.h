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
#ifndef GUI_FRAME_H_
#define GUI_FRAME_H_

#include <vector>
#include "GuiElement.h"
#include "sigslot.h"

//!Allows GuiElements to be grouped together into a "window"
class GuiFrame : public GuiElement
{
	public:
		//!Constructor
		GuiFrame(GuiFrame *parent = 0);
		//!\overload
		//!\param w Width of window
		//!\param h Height of window
		GuiFrame(f32 w, f32 h, GuiFrame *parent = 0);
		//!Destructor
		virtual ~GuiFrame();
		//!Appends a GuiElement to the GuiFrame
		//!\param e The GuiElement to append. If it is already in the GuiFrame, it is removed first
		void append(GuiElement* e);
		//!Inserts a GuiElement into the GuiFrame at the specified index
		//!\param e The GuiElement to insert. If it is already in the GuiFrame, it is removed first
		//!\param i Index in which to insert the element
		void insert(GuiElement* e, u32 i);
		//!Removes the specified GuiElement from the GuiFrame
		//!\param e GuiElement to be removed
		void remove(GuiElement* e);
		//!Removes all GuiElements
		void removeAll();
		//!Bring element to front of the window
		void bringToFront(GuiElement *e) { remove(e); append(e); }
		//!Returns the GuiElement at the specified index
		//!\param index The index of the element
		//!\return A pointer to the element at the index, NULL on error (eg: out of bounds)
		GuiElement* getGuiElementAt(u32 index) const;
		//!Returns the size of the list of elements
		//!\return The size of the current element list
		u32 getSize();
		//!Sets the visibility of the window
		//!\param v visibility (true = visible)
		void setVisible(bool v);
		//!Resets the window's state to STATE_DEFAULT
		void resetState();
		//!Sets the window's state
		//!\param s State
		void setState(int s, int c = -1);
        void clearState(int s, int c = -1);
		//!Gets the index of the GuiElement inside the window that is currently selected
		//!\return index of selected GuiElement
		int getSelected();
		//!Dim the Window's background
		void dimBackground(bool d);
		//!Draws all the elements in this GuiFrame
		void draw(CVideo * v);
		//!Updates the window and all elements contains within
		//!Allows the GuiFrame and all elements to respond to the input data specified
		//!\param t Pointer to a GuiTrigger, containing the current input data from PAD/WPAD
		void update(GuiController * t);
		//!virtual Close Window - this will put the object on the delete queue in MainWindow
		virtual void close();
		//!virtual show window function
		virtual void show() {}
		//!virtual hide window function
		virtual void hide() {}
		//!virtual enter main loop function (blocking)
		virtual void exec() {}
		//!virtual updateEffects which is called by the main loop
		virtual void updateEffects();
		//! Signals
		//! On Closing
		sigslot::signal1<GuiFrame *> closing;
	protected:
		bool dim;   //! Enable/disable dim of a window only
		GuiFrame *parent; //!< Parent Window
		std::vector<GuiElement*> elements; //!< Contains all elements within the GuiFrame
};

#endif
