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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "CSettings.h"
#include "SettingsEnums.h"
#include "fs/CFile.hpp"
#include "fs/fs_utils.h"
#include "utils/StringTools.h"
#include "language/gettext.h"

#define VERSION_LINE        "# Loadiine GX2 - Main settings file v"
#define VALID_VERSION       1

CSettings *CSettings::settingsInstance = NULL;

CSettings::CSettings()
{
    bChanged = false;
    memset(&nullValue, 0, sizeof(nullValue));
    nullValue.strValue = new std::string();

    configPath = SD_PATH WIIU_PATH "/apps/loadiine_gx2";
	this->SetDefault();
}

CSettings::~CSettings()
{
    for(u32 i = 0; i < settingsValues.size(); i++)
    {
        if(settingsValues[i].dataType == TypeString)
            delete settingsValues[i].strValue;
    }
    delete nullValue.strValue;
}

void CSettings::SetDefault()
{
    for(u32 i = 0; i < settingsValues.size(); i++)
    {
        if(settingsValues[i].dataType == TypeString)
            delete settingsValues[i].strValue;
    }

    settingsNames.resize(MAX_VALUE);
    settingsValues.resize(MAX_VALUE);

    settingsNames[GameViewModeTv] = "GameViewModeTv";
    settingsValues[GameViewModeTv].dataType = TypeU8;
    settingsValues[GameViewModeTv].ucValue = VIEW_ICON_CAROUSEL;

    settingsNames[GameViewModeDrc] = "GameViewModeDrc";
    settingsValues[GameViewModeDrc].dataType = TypeU8;
    settingsValues[GameViewModeDrc].ucValue = VIEW_ICON_GRID;

    settingsNames[GameLaunchMethod] = "GameLaunchMethod";
    settingsValues[GameLaunchMethod].dataType = TypeU8;
    settingsValues[GameLaunchMethod].ucValue = LOADIINE_MODE_MII_MAKER;

    settingsNames[GamePath] = "GamePath";
    settingsValues[GamePath].dataType = TypeString;
    settingsValues[GamePath].strValue = new std::string(SD_PATH SD_GAMES_PATH);

    settingsNames[GameSavePath] = "GameSavePath";
    settingsValues[GameSavePath].dataType = TypeString;
    settingsValues[GameSavePath].strValue = new std::string(SD_PATH SD_SAVES_PATH);

    settingsNames[GameLogServer] = "GameLogServer";
    settingsValues[GameLogServer].dataType = TypeBool;
    settingsValues[GameLogServer].bValue = false;

    settingsNames[GameLogServerIp] = "GameLogServerIp";
    settingsValues[GameLogServerIp].dataType = TypeString;
    settingsValues[GameLogServerIp].strValue = new std::string("0.0.0.0");

    settingsNames[GameSaveMode] = "GameSaveMode";
    settingsValues[GameSaveMode].dataType = TypeU8;
    settingsValues[GameSaveMode].ucValue = GAME_SAVES_SHARED;

    settingsNames[BgMusicPath] = "BgMusicPath";
    settingsValues[BgMusicPath].dataType = TypeString;
    settingsValues[BgMusicPath].strValue = new std::string();

    settingsNames[GameCover3DPath] = "GameCover3DPath";
    settingsValues[GameCover3DPath].dataType = TypeString;
    settingsValues[GameCover3DPath].strValue = new std::string(SD_PATH WIIU_PATH "/apps/loadiine_gx2/covers3d");

    settingsNames[ConsoleRegionCode] = "ConsoleRegionCode";
    settingsValues[ConsoleRegionCode].dataType = TypeString;
    settingsValues[ConsoleRegionCode].strValue = new std::string("EN");

	settingsNames[AppLanguage] = "AppLanguage";
    settingsValues[AppLanguage].dataType = TypeString;
    settingsValues[AppLanguage].strValue = new std::string();

    settingsNames[GameStartIndex] = "GameStartIndex";
    settingsValues[GameStartIndex].dataType = TypeU16;
    settingsValues[GameStartIndex].uiValue = 0;

	settingsNames[PadconMode] = "PadconMode";
    settingsValues[PadconMode].dataType = TypeU8;
    settingsValues[PadconMode].ucValue = SETTING_OFF;

    settingsNames[LaunchPyGecko] = "LaunchPyGecko";
    settingsValues[LaunchPyGecko].dataType = TypeU8;
    settingsValues[LaunchPyGecko].ucValue = SETTING_OFF;

    settingsNames[HIDPadEnabled] = "HIDPadUse";
    settingsValues[HIDPadEnabled].dataType = TypeU8;
    settingsValues[HIDPadEnabled].ucValue = SETTING_OFF;

    settingsNames[ShowGameSettings] = "ShowGameSettings";
    settingsValues[ShowGameSettings].dataType = TypeU8;
    settingsValues[ShowGameSettings].ucValue = SETTING_ON;

}

bool CSettings::Load()
{
	//! Reset default path variables to the right device
	SetDefault();


	std::string filepath = configPath;
	filepath += "/loadiine_gx2.cfg";

	CFile file(filepath, CFile::ReadOnly);
	if (!file.isOpen())
        return false;

    std::string strBuffer;
    strBuffer.resize(file.size());
    file.read((u8 *) &strBuffer[0], strBuffer.size());
    file.close();

    //! remove all windows crap signs
    size_t position;
    while(1)
    {
        position = strBuffer.find('\r');
        if(position == std::string::npos)
            break;

        strBuffer.erase(position, 1);
    }

	std::vector<std::string> lines = stringSplit(strBuffer, "\n");


	if(lines.empty() || !ValidVersion(lines[0]))
		return false;

	for(u32 i = 0; i < lines.size(); ++i)
    {
        std::vector<std::string> valueSplit = stringSplit(lines[i], "=");
        if(valueSplit.size() != 2)
            continue;

        while((valueSplit[0].size() > 0) && valueSplit[0][0] == ' ')
            valueSplit[0].erase(0, 1);

        while((valueSplit[1].size() > 0) && valueSplit[1][ valueSplit[1].size() - 1 ] == ' ')
            valueSplit[1].resize(valueSplit[1].size() - 1);

        for(u32 n = 0; n < settingsNames.size(); n++)
        {
            if(!settingsNames[n])
                continue;

            if(valueSplit[0] == settingsNames[n])
            {
                switch(settingsValues[n].dataType)
                {
                    case TypeBool:
                        settingsValues[n].bValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeS8:
                        settingsValues[n].cValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeU8:
                        settingsValues[n].ucValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeS16:
                        settingsValues[n].sValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeU16:
                        settingsValues[n].usValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeS32:
                        settingsValues[n].iValue = atoi(valueSplit[1].c_str());
                        break;
                    case TypeU32:
                        settingsValues[n].uiValue = strtoul(valueSplit[1].c_str(), 0, 10);
                        break;
                    case TypeF32:
                        settingsValues[n].fValue = atof(valueSplit[1].c_str());
                        break;
                    case TypeString:
                        if(settingsValues[n].strValue == NULL)
                            settingsValues[n].strValue = new std::string();

                        *settingsValues[n].strValue = valueSplit[1];
                        break;
                    default:
                        break;
                }
            }
        }
    }

	return true;
}

bool CSettings::ValidVersion(const std::string & versionString)
{
	int version = 0;

    if(versionString.find(VERSION_LINE) != 0)
        return false;

    version = atoi(versionString.c_str() + strlen(VERSION_LINE));

	return version == VALID_VERSION;
}

bool CSettings::Reset()
{
	this->SetDefault();
	bChanged = true;

	if (this->Save())
        return true;

	return false;
}

bool CSettings::Save()
{
    if(!bChanged)
        return true;

    CreateSubfolder(configPath.c_str());

	std::string filepath = configPath;
	filepath += "/loadiine_gx2.cfg";

	CFile file(filepath, CFile::WriteOnly);
	if (!file.isOpen())
        return false;

	file.fwrite("%s%i\n", VERSION_LINE, VALID_VERSION);

	for(u32 i = 0; i < settingsValues.size(); i++)
    {
        switch(settingsValues[i].dataType)
        {
            case TypeBool:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].bValue);
                break;
            case TypeS8:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].cValue);
                break;
            case TypeU8:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].ucValue);
                break;
            case TypeS16:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].sValue);
                break;
            case TypeU16:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].usValue);
                break;
            case TypeS32:
                file.fwrite("%s=%i\n", settingsNames[i], settingsValues[i].iValue);
                break;
            case TypeU32:
                file.fwrite("%s=%u\n", settingsNames[i], settingsValues[i].uiValue);
                break;
            case TypeF32:
                file.fwrite("%s=%f\n", settingsNames[i], settingsValues[i].fValue);
                break;
            case TypeString:
                if(settingsValues[i].strValue != NULL)
                    file.fwrite("%s=%s\n", settingsNames[i], settingsValues[i].strValue->c_str());
                break;
            default:
                break;
        }
    }

    file.close();
    bChanged = false;

	return true;
}
