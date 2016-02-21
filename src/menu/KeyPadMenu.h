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
#ifndef _KEY_PAD_MENU_H_
#define _KEY_PAD_MENU_H_

#include "gui/Gui.h"
#include "settings/SettingsDefs.h"

class KeyPadMenu : public GuiFrame, public sigslot::has_slots<>
{
public:
    KeyPadMenu(int w, int h, const std::string & strTitle, const std::string & prefil);
    virtual ~KeyPadMenu();

    void draw(CVideo *video);

    sigslot::signal2<GuiElement *, const std::string &> settingsOkClicked;
    sigslot::signal1<GuiElement *> settingsBackClicked;
private:
    void UpdateTextFields();
    void OnTextPositionChange(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnDeleteButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnKeyPadButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);
    void OnBackButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsBackClicked(this);
    }
    void OnOkButtonClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
    {
        settingsOkClicked(this, currentText);
    }

    void OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger);

    GuiSound *buttonClickSound;
    GuiImageData *backImageData;
    GuiImage backImage;
    GuiButton backButton;

    GuiImageData *okImageData;
    GuiImage okImage;
    GuiButton okButton;
    GuiText okText;

    GuiTrigger touchTrigger;
    GuiTrigger wpadTouchTrigger;
    GuiTrigger buttonATrigger;
    GuiTrigger buttonBTrigger;

    GuiText titleText;
    GuiImageData *keyPadBgImageData;
    GuiImageData *keyPadButtonImgData;
    GuiImageData *keyPadButtonClickImgData;
    GuiImageData *deleteButtonImgData;
    GuiImageData *deleteButtonClickImgData;
    GuiImageData *fieldImageData;
    GuiImageData *fieldBlinkerImageData;

    GuiImage bgImage;
    GuiImage fieldBlinkerImg;

    GuiImage deleteButtonImg;
    GuiImage deleteButtonImgClick;
    GuiButton deleteButton;

    GuiButton DPADButtons;
    std::vector<GuiText *> textFieldText;
    std::vector<GuiImage *> textFieldImg;
    std::vector<GuiButton *> textFieldBtn;


    std::vector<GuiText *> keyText;
    std::vector<GuiImage *> keyImg;
    std::vector<GuiImage *> keyImgOver;
    std::vector<GuiButton *> keyButton;

    int textPosition;
    std::string currentText;
    u32 lastFrameCount;
};

#endif //_KEY_PAD_MENU_H_
