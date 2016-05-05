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
#ifndef _CSETTINGS_H_
#define _CSETTINGS_H_

#include <string>
#include <stdio.h>
#include <gctypes.h>
#include <vector>
#include "SettingsEnums.h"

class CSettings
{
public:
    static CSettings *instance() {
        if(!settingsInstance)
            settingsInstance = new CSettings();

        return settingsInstance;
    }

    static void destroyInstance() {
        if(settingsInstance){
            delete settingsInstance;
            settingsInstance = NULL;
        }
    }

    //!Set Default Settings
    void SetDefault();
    //!Load Settings
    bool Load();
    //!Save Settings
    bool Save();
    //!Reset Settings
    bool Reset();



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


    enum SettingsIdx
    {
        INVALID = -1,
        GameViewModeTv,
        GameViewModeDrc,
        GameLaunchMethod,
        GamePath,
        GameSavePath,
        GameLogServer,
        GameLogServerIp,
        GameSaveMode,
        BgMusicPath,
        GameCover3DPath,
        ConsoleRegionCode,
		AppLanguage,
        GameStartIndex,
		PadconMode,
        LaunchPyGecko,
        HIDPadEnabled,
        ShowGameSettings,
        MAX_VALUE
    };

    static const u8 & getDataType(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE)
            return instance()->settingsValues[idx].dataType;

        return instance()->nullValue.dataType;
    }

    static const bool & getValueAsBool(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeBool)
            return instance()->settingsValues[idx].bValue;

        return instance()->nullValue.bValue;
    }
    static const s8 & getValueAsS8(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS8)
            return instance()->settingsValues[idx].cValue;

        return instance()->nullValue.cValue;
    }
    static const u8 & getValueAsU8(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU8)
            return instance()->settingsValues[idx].ucValue;

        return instance()->nullValue.ucValue;
    }
    static const s16 & getValueAsS16(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS16)
            return instance()->settingsValues[idx].sValue;

        return instance()->nullValue.sValue;
    }
    static const u16 & getValueAsU16(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU16)
            return instance()->settingsValues[idx].usValue;

        return instance()->nullValue.usValue;
    }
    static const s32 & getValueAsS32(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS32)
            return instance()->settingsValues[idx].iValue;

        return instance()->nullValue.iValue;
    }
    static const u32 & getValueAsU32(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU32)
            return instance()->settingsValues[idx].uiValue;

        return instance()->nullValue.uiValue;
    }
    static const f32 & getValueAsF32(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeF32)
            return instance()->settingsValues[idx].fValue;

        return instance()->nullValue.fValue;
    }
    static const std::string & getValueAsString(int idx)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeString && instance()->settingsValues[idx].strValue)
            return *(instance()->settingsValues[idx].strValue);

        return *(instance()->nullValue.strValue);
    }



    static void setValueAsBool(int idx, const bool & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeBool)
        {
            instance()->settingsValues[idx].bValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsS8(int idx, const s8 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS8)
        {
            instance()->settingsValues[idx].cValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsU8(int idx, const u8 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU8)
        {
            instance()->settingsValues[idx].ucValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsS16(int idx, const s16 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS16)
        {
            instance()->settingsValues[idx].sValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsU16(int idx, const u16 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU16)
        {
            instance()->settingsValues[idx].usValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsS32(int idx, const s32 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeS32)
        {
            instance()->settingsValues[idx].iValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsU32(int idx, const u32 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeU32)
        {
            instance()->settingsValues[idx].uiValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsF32(int idx, const f32 & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeF32)
        {
            instance()->settingsValues[idx].fValue = val;
            instance()->bChanged = true;
        }
    }
    static void setValueAsString(int idx, const std::string & val)
    {
        if(idx > INVALID && idx < MAX_VALUE && instance()->settingsValues[idx].dataType == TypeString && instance()->settingsValues[idx].strValue)
        {
            *(instance()->settingsValues[idx].strValue) = val;
            instance()->bChanged = true;
        }
    }
protected:
    //!Constructor
    CSettings();
    //!Destructor
    ~CSettings();

    bool ValidVersion(const std::string & versionString);

    static CSettings *settingsInstance;

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

    SettingValue nullValue;
    std::string configPath;
    std::vector<SettingValue> settingsValues;
    std::vector<const char*> settingsNames;

    bool bChanged;
};

#endif
