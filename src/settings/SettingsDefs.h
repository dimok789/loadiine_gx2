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
#ifndef __SETTINGS_DEFS_
#define __SETTINGS_DEFS_

enum SettingTypes
{
    TypeDisplayOnly,
    Type2Buttons,
    Type3Buttons,
    Type4Buttons,
    TypeIP,
    TypeList,
};

typedef struct
{
    int value;
    const char *name;
} ValueString;

typedef struct
{
    const char *name;
    const ValueString *valueStrings;
    int type;
    int index;
} SettingType;


#endif // __SETTINGS_DEFS_
