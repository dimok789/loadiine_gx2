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
#include "SettingsMenu.h"
#include "Application.h"
#include "CreditsMenu.h"
#include "settings/CSettings.h"
#include "common/common.h"
#include "utils/StringTools.h"
#include "SettingsCategoryMenu.h"
#include "settings/SettingsDefs.h"
#include "settings/SettingsEnums.h"
#include "gitrev.h"

static const float smallIconScale = 0.4f;

static const ValueString ValueOnOff[] =
{
    { 0, trNOOP("Off") },
    { 1, trNOOP("On") }
};

static const ValueString ValueGameViewMode[] =
{
    { 0, trNOOP("Icon Carousel") },
    { 1, trNOOP("Grid View") },
    { 2, trNOOP("Cover Carousel") }
};

static const struct
{
    const char *name;
    const char *icon;
    const char *iconGlow;
	const char *imageTitle;
    const char *descriptions;
}
stSettingsCategories[] =
{
    { trNOOP("GUI"),     "guiSettingsIcon.png",    "guiSettingsIconGlow.png", "settingsTitleGUI.png",           trNOOP("Game View Selection") "\n"     trNOOP("Background customizations") },
    { trNOOP("Loader"),  "loaderSettingsIcon.png", "loaderSettingsIconGlow.png", "settingsTitleLoader.png",     trNOOP("Customize games path") "\n"    trNOOP("Customize save path") "\n" trNOOP("Set save mode") "\n"    trNOOP("Adjust log server IP and port") },
    { trNOOP("Game"),    "gameSettingsIcon.png",   "gameSettingsIconGlow.png", "settingsTitleGame.png",         trNOOP("Launch method selection") "\n" trNOOP("Log server control") "\n"  trNOOP("Read and write memory") "\n" trNOOP("Turn off the screen Gamepad") "\n" trNOOP("HID settings") },
    { trNOOP("Languages"), "languageSettingsIcon.png", "languageSettingsIcon.png", "settingsTitleLanguage.png", trNOOP("Download languages") "\n" trNOOP("Select language") },
	{ trNOOP("Credits"), "creditsIcon.png",        "creditsIconGlow.png", "settingsTitleCredits.png",           trNOOP("Credits to all contributors") }
};

static const SettingType GuiSettings[] =
{
    { trNOOP("Game View TV"), ValueGameViewMode, "guiTVIcon.png", "guiTVIconGlow.png", Type3Buttons, CSettings::GameViewModeTv },
    { trNOOP("Game View DRC"), ValueGameViewMode, "guiDRCIcon.png", "guiDRCIconGlow.png", Type3Buttons, CSettings::GameViewModeDrc }
};

static const SettingType LoaderSettings[] =
{
    { trNOOP("Show Game Settings"), ValueOnOff, "showGameSettings.png", "showGameSettings.png", Type2Buttons, CSettings::ShowGameSettings },
    { trNOOP("Host IP"), 0, "gameSettingsIP.png", "gameSettingsIP.png", TypeIP, CSettings::GameLogServerIp },
    { trNOOP("Game Path"), 0, "pathGameIcon.png", "pathGameIconGlow.png", TypeDisplayOnly, CSettings::GamePath },
    { trNOOP("Game Save Path"), 0, "pathSaveIcon.png", "pathSaveIconGlow.png", TypeDisplayOnly, CSettings::GameSavePath },
    { trNOOP("Game Save Mode"), ValueGameSaveModes, "modoSaveIcon.png", "modoSaveIconGlow.png", Type2Buttons, CSettings::GameSaveMode }
};

static const SettingType GameSettings[] =
{
    { trNOOP("Launch Mode"), ValueLaunchMode, "gameModeLoad.png", "gameModeLoadGlow.png", Type4Buttons, CSettings::GameLaunchMethod },
    { trNOOP("Log Server Control"), ValueOnOff, "gameLoadLog.png", "gameLoadLogGlow.png", Type2Buttons, CSettings::GameLogServer },
    { trNOOP("Load PyGecko"), ValueOnOff, "gamePyGeckoLoad.png", "gamePyGeckoLoadGlow.png", Type2Buttons, CSettings::LaunchPyGecko },
    { trNOOP("Enable Padcon"), ValueOnOff, "gamePadconIcon.png", "gamePadconIconGlow.png", Type2Buttons, CSettings::PadconMode },
    { trNOOP("HID-Pad"), ValueOnOff, "gameHIDIcon.png", "gameHIDIconGlow.png", Type2Buttons, CSettings::HIDPadEnabled }
};

static const SettingType LanguagesSettings[] =
{
    { trNOOP("Download"), 0, "downloadLanguageIcon.png", "downloadLanguageIconGlow.png", TypeDownloadLanguage, CSettings::LanguagesPath },
	{ trNOOP("Languages"), 0, "flagsLanguageIcon.png", "flagsLanguageIconGlow.png", TypeSelectLanguage, CSettings::AppLanguage }
};

SettingsMenu::SettingsMenu(int w, int h)
    : GuiFrame(w, h)
    , categorySelectionFrame(w, h)
    , particleBgImage(w, h, 50)
    , buttonClickSound(Resources::GetSound("settings_click_2.mp3"))
    , quitImageData(Resources::GetImageData("quitButton.png"))
    , categoryImageData(Resources::GetImageData("settingsCategoryButton.png"))
    , categoryBgImageData(Resources::GetImageData("settingsCategoryBg.png"))
	, arrowImageData(Resources::GetImageData("settingsIconArrow.png"))
    , quitImage(quitImageData)
    , quitButton(quitImage.getWidth(), quitImage.getHeight())
    , touchTrigger(GuiTrigger::CHANNEL_1, GuiTrigger::VPAD_TOUCH)
    , wpadTouchTrigger(GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5, GuiTrigger::BUTTON_A)
    , buttonATrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_A, true)
    , buttonBTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_B, true)
    , buttonLTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_L, true)
    , buttonRTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_R, true)
    , buttonLeftTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_LEFT | GuiTrigger::STICK_L_LEFT, true)
    , buttonRightTrigger(GuiTrigger::CHANNEL_ALL, GuiTrigger::BUTTON_RIGHT | GuiTrigger::STICK_L_RIGHT, true)
    , leftArrowImageData(Resources::GetImageData("leftArrow.png"))
    , rightArrowImageData(Resources::GetImageData("rightArrow.png"))
    , leftArrowImage(leftArrowImageData)
    , rightArrowImage(rightArrowImageData)
    , leftArrowButton(leftArrowImage.getWidth(), leftArrowImage.getHeight())
    , rightArrowButton(rightArrowImage.getWidth(), rightArrowImage.getHeight())
    , DPADButtons(w,h)
{
    currentPosition = 0;
    targetPosition = 0;
    selectedCategory = 0;
    animationSpeed = 25;
    bUpdatePositions = true;

    quitButton.setImage(&quitImage);
    quitButton.setAlignment(ALIGN_BOTTOM | ALIGN_LEFT);
    quitButton.clicked.connect(this, &SettingsMenu::OnQuitButtonClick);
    quitButton.setTrigger(&touchTrigger);
    quitButton.setTrigger(&wpadTouchTrigger);
    quitButton.setEffectGrow();
    quitButton.setSoundClick(buttonClickSound);
    categorySelectionFrame.append(&quitButton);

    versionText.setColor(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
    versionText.setFontSize(42);
    versionText.setAlignment(ALIGN_TOP | ALIGN_RIGHT);
    versionText.setPosition(-50, -80);
    versionText.setTextf("Loadiine GX2 %s (build %s)",  LOADIINE_VERSION, GetRev());
    categorySelectionFrame.append(&versionText);

    const u32 cuCategoriesCount = sizeof(stSettingsCategories) / sizeof(stSettingsCategories[0]);

    if(cuCategoriesCount > 0) selectedCategory = 0;

    for(u32 idx = 0; idx < cuCategoriesCount; idx++)
    {
        settingsCategories.resize(idx + 1);
        GuiSettingsCategory & category = settingsCategories[idx];

        std::vector<std::string> splitDescriptions = stringSplit(stSettingsCategories[idx].descriptions, "\n");

        category.categoryIconData = Resources::GetImageData(stSettingsCategories[idx].icon);
        category.categoryIconGlowData = Resources::GetImageData(stSettingsCategories[idx].iconGlow);

        category.categoryLabel = new GuiText(tr(stSettingsCategories[idx].name), 44, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        category.categoryLabel->setPosition(0, -120);

        category.categoryBgImage = new GuiImage(categoryBgImageData);
        category.categoryImages = new GuiImage(categoryImageData);
        category.categoryIcon = new GuiImage(category.categoryIconData);
        category.categoryIconGlow = new GuiImage(category.categoryIconGlowData);
        category.categoryButton = new GuiButton(category.categoryImages->getWidth(), category.categoryImages->getHeight());

        category.categoryIcon->setPosition(0, 40);
        category.categoryIconGlow->setPosition(0, 40);

        category.categoryButton->setLabel(category.categoryLabel);
        category.categoryButton->setImage(category.categoryImages);
        category.categoryButton->setPosition(-300, 0);
        category.categoryButton->setIcon(category.categoryIcon);
        category.categoryButton->setIconOver(category.categoryIconGlow);
        category.categoryButton->setTrigger(&touchTrigger);
        category.categoryButton->setTrigger(&wpadTouchTrigger);
        category.categoryButton->setSoundClick(buttonClickSound);
        category.categoryButton->setEffectGrow();
        category.categoryButton->clicked.connect(this, &SettingsMenu::OnCategoryClick);

        categorySelectionFrame.append(category.categoryBgImage);
        categorySelectionFrame.append(category.categoryButton);

        category.categoryButton->setParent(category.categoryBgImage);
        category.categoryBgImage->setPosition(currentPosition + (category.categoryBgImage->getWidth() + 40) * idx, 0);

        int line = 0;
		
        for(u32 n = 0; n < splitDescriptions.size(); n++)
        {

			switch(splitDescriptions.size())
				{
				default: //! max 5 descriptions with new line
					break;

				case 1: {
					line = category.categoryBgImage->getHeight() * 0.5f - 220.0f - n * 60.0f;
					break;
				}
				case 2: {
					line = category.categoryBgImage->getHeight() * 0.5f - 140.0f - (n * 3.0f) * 60.0f;
					break;
				}
				case 3: {
					line = category.categoryBgImage->getHeight() * 0.5f - 120.0f - (n * 2.0f) * 60.0f;
					break;
				}
				case 4: {
					line = category.categoryBgImage->getHeight() * 0.5f - 100.0f - (n * 1.5f) * 60.0f;
					break;
				}
				case 5: {
					line = category.categoryBgImage->getHeight() * 0.5f - 50.0f - (n * 1.5f) * 60.0f;
					break;
				}
			}
			
			category.categoryArrowImage = new GuiImage(arrowImageData);
            category.categoryArrowImage->setAlignment(ALIGN_MIDDLE | ALIGN_LEFT);
			category.categoryArrowImage->setPosition(category.categoryBgImage->getWidth() * 0.5f - 80.0f, line + 4);
		    categorySelectionFrame.append(category.categoryArrowImage);
			category.categoryArrowImage->setParent(category.categoryBgImage);
			
            if(splitDescriptions.size() <= 5) 
			{
				GuiText * descr = new GuiText(tr(splitDescriptions[n].c_str()), 42, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
				descr->setAlignment(ALIGN_MIDDLE | ALIGN_LEFT);
				descr->setMaxWidth(category.categoryBgImage->getWidth() * 0.5f, GuiText::WRAP);
				descr->setPosition(category.categoryBgImage->getWidth() * 0.5f - 50.0f, line);
				categorySelectionFrame.append(descr);
				descr->setParent(category.categoryBgImage);
				category.descriptions.push_back(descr);
			}
        }

        GuiImage *smallIconOver = new GuiImage(category.categoryIconGlowData);
        GuiImage *smallIcon = new GuiImage(category.categoryIconData);
        GuiButton *smallIconButton = new GuiButton(smallIcon->getWidth() * smallIconScale, smallIcon->getHeight() * smallIconScale);
        smallIcon->setScale(smallIconScale);
        smallIconOver->setScale(smallIconScale);
        smallIconButton->setImage(smallIcon);
        smallIconButton->setEffectGrow();
        smallIconButton->setTrigger(&touchTrigger);
        smallIconButton->setTrigger(&wpadTouchTrigger);
        smallIconButton->setSoundClick(buttonClickSound);
        smallIconButton->clicked.connect(this, &SettingsMenu::OnSmallIconClick);
        categorySelectionFrame.append(smallIconButton);

        categorySmallImages.push_back(smallIcon);
        categorySmallImagesOver.push_back(smallIconOver);
        categorySmallButtons.push_back(smallIconButton);
    }

    leftArrowButton.setImage(&leftArrowImage);
    leftArrowButton.setEffectGrow();
    leftArrowButton.setPosition(40, 0);
    leftArrowButton.setAlignment(ALIGN_LEFT | ALIGN_MIDDLE);
    leftArrowButton.setTrigger(&touchTrigger);
    leftArrowButton.setTrigger(&wpadTouchTrigger);

    leftArrowButton.setSoundClick(buttonClickSound);
    leftArrowButton.clicked.connect(this, &SettingsMenu::OnCategoryLeftClick);
    categorySelectionFrame.append(&leftArrowButton);

    rightArrowButton.setImage(&rightArrowImage);
    rightArrowButton.setEffectGrow();
    rightArrowButton.setPosition(-40, 0);
    rightArrowButton.setAlignment(ALIGN_RIGHT | ALIGN_MIDDLE);
    rightArrowButton.setTrigger(&touchTrigger);
    rightArrowButton.setTrigger(&wpadTouchTrigger);
    rightArrowButton.setSoundClick(buttonClickSound);
    rightArrowButton.clicked.connect(this, &SettingsMenu::OnCategoryRightClick);
    categorySelectionFrame.append(&rightArrowButton);

    DPADButtons.setTrigger(&buttonATrigger);
    DPADButtons.setTrigger(&buttonBTrigger);
    DPADButtons.setTrigger(&buttonLTrigger);
    DPADButtons.setTrigger(&buttonRTrigger);
    DPADButtons.setTrigger(&buttonLeftTrigger);
    DPADButtons.setTrigger(&buttonRightTrigger);
    DPADButtons.clicked.connect(this, &SettingsMenu::OnDPADClick);
    append(&DPADButtons);
	categorySelectionFrame.append(&DPADButtons);
    setTargetPosition(0);

    //! the particle BG is always appended in all sub menus
    append(&particleBgImage);
    append(&categorySelectionFrame);
}

SettingsMenu::~SettingsMenu()
{
    for(u32 i = 0; i < settingsCategories.size(); ++i)
    {
        delete settingsCategories[i].categoryLabel;
        delete settingsCategories[i].categoryBgImage;
        delete settingsCategories[i].categoryImages;
        delete settingsCategories[i].categoryIcon;
        delete settingsCategories[i].categoryIconGlow;
        delete settingsCategories[i].categoryButton;

        for(u32 n = 0; n < settingsCategories[i].descriptions.size() && n <=5; n++)
            delete settingsCategories[i].descriptions[n];

        Resources::RemoveImageData(settingsCategories[i].categoryIconData);
        Resources::RemoveImageData(settingsCategories[i].categoryIconGlowData);

        delete categorySmallImages[i];
        delete categorySmallImagesOver[i];
        delete categorySmallButtons[i];
    }
    Resources::RemoveImageData(quitImageData);
    Resources::RemoveImageData(categoryImageData);
    Resources::RemoveImageData(categoryBgImageData);
	Resources::RemoveImageData(arrowImageData);
    Resources::RemoveImageData(leftArrowImageData);
    Resources::RemoveImageData(rightArrowImageData);
    Resources::RemoveSound(buttonClickSound);
}

void SettingsMenu::setTargetPosition(int selectedIdx)
{
    if(selectedIdx < 0 || selectedIdx >= (int)settingsCategories.size())
        return;
    selectedCategory = selectedIdx;
    targetPosition = (settingsCategories[selectedCategory].categoryBgImage->getWidth() + 40) * -selectedCategory;

    if(selectedCategory == 0)
    {
        leftArrowButton.setClickable(false);
        leftArrowButton.setSelectable(false);
        leftArrowButton.setVisible(false);
    }
    else
    {
        leftArrowButton.setClickable(true);
        leftArrowButton.setSelectable(true);
        leftArrowButton.setVisible(true);
    }

    if(selectedCategory == (int)(settingsCategories.size()-1))
    {
        rightArrowButton.setClickable(false);
        rightArrowButton.setSelectable(false);
        rightArrowButton.setVisible(false);
    }
    else
    {
        rightArrowButton.setClickable(true);
        rightArrowButton.setSelectable(true);
        rightArrowButton.setVisible(true);
    }
}

void SettingsMenu::OnSubMenuCloseClicked(GuiElement *element)
{
    //! disable element for triggering buttons again
    element->setState(GuiElement::STATE_DISABLED);
    element->setEffect(EFFECT_FADE, -10, 0);
    element->effectFinished.connect(this, &SettingsMenu::OnSubMenuCloseEffectFinish);

    //! fade in category selection
    categorySelectionFrame.setEffect(EFFECT_FADE, 10, 255);
    append(&categorySelectionFrame);
}

void SettingsMenu::OnSubMenuOpenEffectFinish(GuiElement *element)
{
    //! make element clickable again
    element->clearState(GuiElement::STATE_DISABLED);
    element->effectFinished.disconnect(this);
    //! remove category selection from settings
    remove(&categorySelectionFrame);
}

void SettingsMenu::OnSubMenuCloseEffectFinish(GuiElement *element)
{
    remove(element);
    AsyncDeleter::pushForDelete(element);

    //! enable all elements again
    categorySelectionFrame.clearState(GuiElement::STATE_DISABLED);
}

void SettingsMenu::OnCategoryClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(currentPosition == targetPosition){

        int indexClicked = -1;

        for(u32 i = 0; i < settingsCategories.size(); ++i)
        {
            if(button == settingsCategories[i].categoryButton)
            {
                indexClicked = i;
                break;
            }
        }
        if(indexClicked == -1) indexClicked = selectedCategory;

        const SettingType * categorySettings = NULL;
        int categorySettingsCount = 0;

        switch(indexClicked)
        {
        case 0:
            categorySettings = GuiSettings;
            categorySettingsCount = sizeof(GuiSettings) / sizeof(SettingType);
            break;
        case 1:
            categorySettings = LoaderSettings;
            categorySettingsCount = sizeof(LoaderSettings) / sizeof(SettingType);
            break;
        case 2:
            categorySettings = GameSettings;
            categorySettingsCount = sizeof(GameSettings) / sizeof(SettingType);
            break;
        case 3:
            categorySettings = LanguagesSettings;
            categorySettingsCount = sizeof(LanguagesSettings) / sizeof(SettingType);
            break;
        case 4:
        {
            CreditsMenu * menu = new CreditsMenu(getWidth(), getHeight(), tr(stSettingsCategories[indexClicked].name), stSettingsCategories[indexClicked].imageTitle);
            menu->setEffect(EFFECT_FADE, 10, 255);
            menu->setState(STATE_DISABLED);
            menu->effectFinished.connect(this, &SettingsMenu::OnSubMenuOpenEffectFinish);
            menu->settingsBackClicked.connect(this, &SettingsMenu::OnSubMenuCloseClicked);

            //! disable all current elements and fade them out with fading in new menu
            categorySelectionFrame.setState(STATE_DISABLED);
            categorySelectionFrame.setEffect(EFFECT_FADE, -10, 0);

            //! now append new menu
            append(menu);

            return;
        }
        default:
            return;
        }

        SettingsCategoryMenu *menu = new SettingsCategoryMenu(getWidth(), getHeight(), tr(stSettingsCategories[indexClicked].name), stSettingsCategories[indexClicked].imageTitle, categorySettings, categorySettingsCount);
        menu->setEffect(EFFECT_FADE, 10, 255);
        menu->setState(STATE_DISABLED);
        menu->effectFinished.connect(this, &SettingsMenu::OnSubMenuOpenEffectFinish);
        menu->settingsBackClicked.connect(this, &SettingsMenu::OnSubMenuCloseClicked);

        //! disable all current elements and fade them out with fading in new menu
        categorySelectionFrame.setState(STATE_DISABLED);
        categorySelectionFrame.setEffect(EFFECT_FADE, -10, 0);

        //! now append new menu
        append(menu);
    }
}

void SettingsMenu::OnSmallIconClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    for(u32 i = 0; i < categorySmallButtons.size(); ++i)
    {
        if(button == categorySmallButtons[i])
        {
            setTargetPosition(i);
            animationSpeed = 70;
            break;
        }
    }
}

void SettingsMenu::OnCategoryRightClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(selectedCategory < (int)(settingsCategories.size()-1))
    {
        selectedCategory++;
        setTargetPosition(selectedCategory);
        animationSpeed = 25;
    }
}

void SettingsMenu::OnCategoryLeftClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
    if(selectedCategory > 0 && settingsCategories.size() > 0)
    {
        selectedCategory--;
        setTargetPosition(selectedCategory);
        animationSpeed = 25;
    }
}

void SettingsMenu::OnDPADClick(GuiButton *button, const GuiController *controller, GuiTrigger *trigger)
{
	if(trigger == &buttonATrigger)
	{
        //! do not auto launch when wiimote is pointing to screen and presses A
        if((controller->chan & (GuiTrigger::CHANNEL_2 | GuiTrigger::CHANNEL_3 | GuiTrigger::CHANNEL_4 | GuiTrigger::CHANNEL_5)) && controller->data.validPointer)
        {
            return;
        }
		OnCategoryClick(button,controller,trigger);
	}
	else if(trigger == &buttonBTrigger){
		OnQuitButtonClick(button,controller,trigger);
	}
	else if(trigger == &buttonLTrigger){
		OnCategoryLeftClick(button,controller,trigger);
	}
	else if(trigger == &buttonRTrigger){
		OnCategoryRightClick(button,controller,trigger);
	}
	else if(trigger == &buttonLeftTrigger){
		OnCategoryLeftClick(button,controller,trigger);
	}
	else if(trigger == &buttonRightTrigger){
		OnCategoryRightClick(button,controller,trigger);
	}
}


void SettingsMenu::update(GuiController *c)
{
    if(currentPosition < targetPosition)
    {
        currentPosition += animationSpeed;
        if(currentPosition >= targetPosition)
		{
            currentPosition = targetPosition;
        }


        bUpdatePositions = true;
    }
    else if(currentPosition > targetPosition)
    {
        currentPosition -= animationSpeed;
        if(currentPosition <= targetPosition)
		{
            currentPosition = targetPosition;
        }


        bUpdatePositions = true;
    }

    if(bUpdatePositions)
    {
        bUpdatePositions = false;

        for(u32 i = 0; i < settingsCategories.size(); ++i)
        {
            f32 posX = -350 + (categorySmallImages[i]->getWidth() * smallIconScale + 30) * i;
            f32 posY = -300;

            if((int)i == selectedCategory)
            {
                posY += 0.3f * categorySmallImages[i]->getHeight() * smallIconScale;
                categorySmallButtons[i]->setPosition(posX, posY);
                categorySmallButtons[i]->setImage(categorySmallImagesOver[i]);
            }
            else
            {
                categorySmallButtons[i]->setPosition(posX, posY);
                categorySmallButtons[i]->setImage(categorySmallImages[i]);
            }

            settingsCategories[i].categoryBgImage->setPosition(currentPosition + (settingsCategories[selectedCategory].categoryBgImage->getWidth() + 40) * i, 0.0f);

        }
    }

    GuiFrame::update(c);
}
