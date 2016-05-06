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
#ifndef _CSettingsGame_H_
#define _CSettingsGame_H_

#include <string>
#include <map>
#include <stdio.h>
#include <gctypes.h>
#include <vector>
#include "SettingsGameDefs.h"
#include "utils/logger.h"
#include "SettingsEnums.h"

class CSettingsGame
{
public:
    static CSettingsGame *getInstance() {
        if(!instance)
            instance = new CSettingsGame();
        return instance;
    }

    static void destroyInstance() {
        if(instance){
            delete instance;
            instance = NULL;
        }
    }


	enum SettingsGameIdx
    {
        INVALID = -1,
        UpdateFolder,
		ExtraSaveFile,
		SaveMethod,
		LaunchMethod,
		EnableDLC,
        MAX_VALUE
    };

	enum DataTypes
    {
        TypeNone,
        TypeBool,
        TypeS8,
        TypeU8,
        TypeS16,
        TypeU16,
        TypeS32,
        TypeU32,
        TypeF32,
        TypeString
    };

	bool SaveGameSettings(const GameSettings & gSetttings);
    bool LoadGameSettings(std::string ID6, GameSettings & result);

private:
    //!Constructor
    CSettingsGame();
    //!Destructor
    ~CSettingsGame();

	void InitSettingsNames(void){
		settingsNames.resize(MAX_VALUE);
		settingsNames[UpdateFolder] = "UpdateFolder";
		settingsNames[ExtraSaveFile] = "ExtraSaveFile";
		settingsNames[SaveMethod] = "SaveMethod";
		settingsNames[LaunchMethod] = "LaunchMethod";
		settingsNames[EnableDLC] = "EnableDLC";
	}

	typedef struct
    {
        u8 dataType;

        union
        {
            bool bValue;
            s8 cValue;
            u8 ucValue;
            s16 sValue;
            u16 usValue;
            s32 iValue;
            u32 uiValue;
            f32 fValue;
            std::string *strValue;
        };
    } SettingValue;

	std::vector<const char*> settingsNames;

    bool ValidVersion(const std::string & versionString);

	std::map<std::string,GameSettings> settingsGames;
    static CSettingsGame *instance;
	std::string configPath;
	std::string filename;
	bool bChanged = false;
	bool Save();
	bool Load();
	void PrintLoadedGames();

	GameSettings * GetGameSettingsBySettingGameValue(std::string ID6,std::vector<SettingValue> settings);
	std::vector<SettingValue> getSettingValuesFromGameSettings(GameSettings gameSettings);
	std::vector<SettingValue> getSettingValuesFromGameSettings(std::string updateFolder,bool extraSave,u8 save_method,u8 launch_method,u8 enableDlc);
};
#endif
