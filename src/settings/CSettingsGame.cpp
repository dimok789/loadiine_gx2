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
#include "CSettingsGame.h"
#include "SettingsEnums.h"
#include "fs/CFile.hpp"
#include "fs/fs_utils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"

#define VERSION_LINE        "# Loadiine GX2 - Game settings file v"
#define VALID_VERSION       1

CSettingsGame *CSettingsGame::instance = NULL;

CSettingsGame::CSettingsGame()
{
	InitSettingsNames();
    this->configPath = SD_PATH  WIIU_PATH "/apps/loadiine_gx2";
	this->filename = "loadiine_gx2_games.cfg";
	Load();
	PrintLoadedGames();
}

CSettingsGame::~CSettingsGame()
{
	/*
    for(u32 i = 0; i < settingsGames.size(); i++){
		for(u32 j = 0; j < settingsGames[i].settingsValues.size(); j++){
			if(settingsGames[i].settingsValues.at(j).dataType == TypeString)
				delete settingsGames[i].settingsValues.at(j).strValue;
		}
    }*/
}

bool CSettingsGame::ValidVersion(const std::string & versionString)
{
	int version = 0;

    if(versionString.find(VERSION_LINE) != 0)
        return false;

    version = atoi(versionString.c_str() + strlen(VERSION_LINE));

	return version == VALID_VERSION;
}

void CSettingsGame::PrintLoadedGames(){
	log_print("Loaded Games\n");
	std::map<std::string,GameSettings>::iterator itr;

	for(itr = settingsGames.begin(); itr != settingsGames.end(); itr++) {
		GameSettings set = itr->second;
		log_printf("%s=%s;%s=%s;%s=%d;%s=%d,%s=%d\n","ID6",	itr->first.c_str(),
															settingsNames[UpdateFolder],set.updateFolder.c_str(),
															settingsNames[ExtraSaveFile],set.extraSave,
															settingsNames[SaveMethod],set.save_method,
															settingsNames[LaunchMethod],set.launch_method);
	}

	log_print("End Loaded Games\n");
 }

bool CSettingsGame::Load()
{
	std::string filepath = configPath;
	filepath += "/" +this->filename;

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

	std::vector<std::string> lines_ = stringSplit(strBuffer, "\n");
	if(lines_.empty() || !ValidVersion(lines_[0]))
		return false;

	for(u32 j = 1; j < lines_.size(); ++j){
		std::vector<std::string> lines = stringSplit(lines_[j], ";");

		if(lines.empty())
			return false;

		std::string ID6 = lines[0];

		std::vector<CSettingsGame::SettingValue> newValues = getSettingValuesFromGameSettings(std::string(COMMON_UPDATE_PATH), false, GAME_SAVES_DEFAULT, LOADIINE_MODE_DEFAULT, SETTING_OFF);

		for(u32 i = 1; i < lines.size(); ++i)
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
					switch(newValues.at(n).dataType)
					{
						case TypeBool:
							newValues.at(n).bValue = atoi(valueSplit[1].c_str());
							break;
						case TypeS8:
							newValues.at(n).cValue = atoi(valueSplit[1].c_str());
							break;
						case TypeU8:
							newValues.at(n).ucValue = atoi(valueSplit[1].c_str());
							break;
						case TypeS16:
							newValues.at(n).sValue = atoi(valueSplit[1].c_str());
							break;
						case TypeU16:
							newValues.at(n).usValue = atoi(valueSplit[1].c_str());
							break;
						case TypeS32:
							newValues.at(n).iValue = atoi(valueSplit[1].c_str());
							break;
						case TypeU32:
							newValues.at(n).uiValue = strtoul(valueSplit[1].c_str(), 0, 10);
							break;
						case TypeF32:
							newValues.at(n).fValue = atof(valueSplit[1].c_str());
							break;
						case TypeString:
							if(newValues.at(n).strValue == NULL)
								newValues.at(n).strValue = new std::string();
							*newValues.at(n).strValue = valueSplit[1];
							break;
						default:
							break;
					}

				}
			}
		}
		GameSettings * set = GetGameSettingsBySettingGameValue(ID6,newValues);

		//Clean pointer
		for(u32 j = 0; j < newValues.size(); j++){
			if(newValues.at(j).dataType == TypeString)
				delete newValues.at(j).strValue;
		}

		settingsGames[ID6] = *set;
		delete set;
	}

	return true;
}

bool CSettingsGame::LoadGameSettings(std::string ID6, GameSettings & result){
    //log_printf("LoadGameSettings for ID6: %s\n",ID6.c_str());
    std::map<std::string,GameSettings>::iterator itr;
    itr = settingsGames.find(ID6);
    if(itr != settingsGames.end()){// existiert
        result = itr->second;
        return true;
    }
    else
    {
        result.ID6 = ID6;
        result.extraSave = false;
        result.launch_method = LOADIINE_MODE_DEFAULT;
        result.save_method = GAME_SAVES_DEFAULT;
        result.updateFolder = COMMON_UPDATE_PATH;
        result.EnableDLC = SETTING_OFF;
        return false;
    }
}

bool CSettingsGame::SaveGameSettings(const GameSettings & gSetttings)
{
	settingsGames[gSetttings.ID6] = gSetttings;

	bChanged = true;
	return Save();
}

bool CSettingsGame::Save()
{
    if(!bChanged)
        return true;

    CreateSubfolder(configPath.c_str());

	std::string filepath = configPath;
	filepath += "/" + this->filename;

	CFile file(filepath, CFile::WriteOnly);
	if (!file.isOpen())
        return false;
	std::string strBuffer;
	strBuffer += strfmt("%s%i\n", VERSION_LINE, VALID_VERSION);

	std::map<std::string,GameSettings>::iterator itr;
	for(itr = settingsGames.begin(); itr != settingsGames.end(); itr++) {

		GameSettings game_settings = itr->second;
		std::vector<CSettingsGame::CSettingsGame::SettingValue> setvalues = getSettingValuesFromGameSettings(game_settings);
		CSettingsGame::SettingValue value;
		strBuffer += strfmt("%s;", itr->first.c_str());
		for(u32 j = 0; j < MAX_VALUE; j++){
			value = setvalues.at(j);
			switch(value.dataType){
				case TypeBool:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.bValue);
					break;
				case TypeS8:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.cValue);
					break;
				case TypeU8:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.ucValue);
					break;
				case TypeS16:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.sValue);
					break;
				case TypeU16:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.usValue);
					break;
				case TypeS32:
					strBuffer += strfmt("%s=%i", settingsNames[j], value.iValue);
					break;
				case TypeU32:
					strBuffer += strfmt("%s=%u", settingsNames[j], value.uiValue);
					break;
				case TypeF32:
					strBuffer += strfmt("%s=%f", settingsNames[j], value.fValue);
					break;
				case TypeString:
					if(value.strValue != NULL)
						strBuffer += strfmt("%s=%s", settingsNames[j], value.strValue->c_str());
					break;
				default:
					break;
			}
			strBuffer += strfmt(";");
		}
		strBuffer += strfmt("\n");
		//Clean pointer
		for(u32 j = 0; j < setvalues.size(); j++){
			if(setvalues.at(j).dataType == TypeString)
				delete setvalues.at(j).strValue;
		}
    }
	file.write((u8*)strBuffer.c_str(), strBuffer.size());
    file.close();
    bChanged = false;

	return true;
}

std::vector<CSettingsGame::SettingValue> CSettingsGame::getSettingValuesFromGameSettings(GameSettings gameSettings){
	return getSettingValuesFromGameSettings(gameSettings.updateFolder,gameSettings.extraSave,gameSettings.save_method,gameSettings.launch_method,gameSettings.EnableDLC);
}

std::vector<CSettingsGame::SettingValue> CSettingsGame::getSettingValuesFromGameSettings(std::string updateFolder,bool extraSave,u8 save_method,u8 launch_method, u8 enableDlc){
	std::vector<CSettingsGame::SettingValue> result;

	result = std::vector<CSettingsGame::SettingValue>();
	result.resize(MAX_VALUE);

	result.at(UpdateFolder).dataType = TypeString;
	result.at(UpdateFolder).strValue = new std::string(updateFolder);
	result.at(ExtraSaveFile).dataType = TypeBool;
	result.at(ExtraSaveFile).bValue = extraSave;
	result.at(SaveMethod).dataType = TypeU8;
	result.at(SaveMethod).ucValue = save_method;
	result.at(LaunchMethod).dataType = TypeU8;
	result.at(LaunchMethod).ucValue = launch_method;
	result.at(EnableDLC).dataType = TypeU8;
	result.at(EnableDLC).ucValue = enableDlc;
	return result;
}


GameSettings * CSettingsGame::GetGameSettingsBySettingGameValue(std::string ID6,std::vector<CSettingsGame::SettingValue> settings){
	GameSettings *  set = new GameSettings;
	set->ID6 = ID6;
	set->updateFolder = *(settings.at(UpdateFolder).strValue);
	set->extraSave = settings.at(ExtraSaveFile).bValue;
	set->save_method = settings.at(SaveMethod).ucValue;
	set->launch_method = settings.at(LaunchMethod).ucValue;
	set->EnableDLC = settings.at(EnableDLC).ucValue;

	return set;
}

