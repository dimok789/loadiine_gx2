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
#ifndef _CSETTINGS_LANGUAGES_H_
#define _CSETTINGS_LANGUAGES_H_

#include <vector>
#include <string>

class CSettingsLanguages
{
public:
    static CSettingsLanguages *getInstance() {
        if(!instance)
            instance = new CSettingsLanguages();
        return instance;
    }

    static void destroyInstance() {
        if(instance){
            delete instance;
            instance = NULL;
        }
    }
	
	std::vector<std::string> LoadLanguages(int *languageSelect);
	void SetLanguage(int languageSelect);
	
private:
    //!Constructor
    CSettingsLanguages();
    //!Destructor
    ~CSettingsLanguages();
	
    static CSettingsLanguages *instance;
	
	std::string languagesPath;
	const char *filter;
	std::map<int, std::string> enumLanguages;
};
#endif // SETTINGS_LANGUAGES_H_