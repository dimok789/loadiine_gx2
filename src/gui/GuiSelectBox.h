/****************************************************************************
 * Copyright (C) 2016 Maschell
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
#ifndef GUI_SELECTBOX_H_
#define GUI_SELECTBOX_H_

#include "Gui.h"
#include "GuiImage.h"
#include "GuiImageData.h"

//!A simple CheckBox
class GuiSelectBox : public GuiFrame, public sigslot::has_slots<>

{
	public:
		//!Constructor
		//!\param checked Checked
		GuiSelectBox(std::string caption,GuiFrame *parent = 0);
		//!Destructor
		virtual ~GuiSelectBox();

        sigslot::signal2<GuiSelectBox *, std::string> valueChanged;
        sigslot::signal2<GuiSelectBox *, bool> showhide;
        void OnTopValueClicked(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
        void Init(std::map<std::string,std::string> values, int valueID);

        void setState(int s, int c = -1);

        virtual f32 getTopValueHeight();
        virtual f32 getTopValueWidth();

	protected:
	    void DeleteValueData();
        void update(GuiController * c);

	    void OnValueClicked(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

        void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
        void OnValueOpenEffectFinish(GuiElement *element);
        void OnValueCloseEffectFinish(GuiElement *element);
        void ShowHideValues(bool showhide);
        void SelectValue(u32 value);

        u32 selected;
        bool bChanged;
        bool bSelectedChanged;
        bool showValues;
        bool opened;
        std::string captionText;
        GuiFrame valuesFrame;
        GuiImageData *topValueImageData;
        GuiImage topValueImage;
        GuiImageData *topValueImageSelectedData;
        GuiImage topValueImageSelected;

        GuiButton topValueButton;
        GuiImageData * valueImageData;
        GuiImageData * valueSelectedImageData;
        GuiImageData * valueHighlightedImageData;
        GuiText topValueText;

        GuiTrigger touchTrigger;
        GuiTrigger wpadTouchTrigger;

        GuiTrigger buttonATrigger;
        GuiTrigger buttonBTrigger;
        GuiTrigger buttonLeftTrigger;
        GuiTrigger buttonRightTrigger;
        GuiTrigger buttonUpTrigger;
        GuiTrigger buttonDownTrigger;

        GuiButton DPADButtons;

        GuiSound* buttonClickSound;

        std::string getCaptionWithValue(std::string value);
        typedef struct
        {
            GuiImage *valueButtonImg;
            GuiImage *valueButtonCheckedImg;
            GuiImage *valueButtonHighlightedImg;
            GuiButton *valueButton;
            GuiText *valueButtonText;
        } SelectBoxValueButton;

        std::map<GuiButton * ,std::string> buttonToValue;
        std::vector<SelectBoxValueButton> valueButtons;

};

#endif
