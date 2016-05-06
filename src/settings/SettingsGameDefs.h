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
#ifndef __SETTINGSGAME_DEFS_
#define __SETTINGSGAME_DEFS_
typedef struct
    {
      std::string ID6;
	  std::string updateFolder;
	  bool extraSave;
	  u8 save_method;
	  u8 launch_method;
	  u8 EnableDLC;
    } GameSettings;

#endif // __SETTINGSGAME_DEFS_
