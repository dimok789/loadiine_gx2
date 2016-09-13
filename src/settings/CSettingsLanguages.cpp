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
#include <map>
#include <string.h>
#include "common/common.h"
#include "CSettingsLanguages.h"
#include "utils/StringTools.h"
#include "CSettings.h"
#include "fs/DirList.h"
#include "language/gettext.h"

CSettingsLanguages *CSettingsLanguages::instance = NULL;

CSettingsLanguages::CSettingsLanguages()
{
	this->languagesPath = SD_PATH  WIIU_PATH "/apps/loadiine_gx2/languages";
	this->filter = ".lang";
}

CSettingsLanguages::~CSettingsLanguages()
{
	enumLanguages.clear();	
}

std::vector<std::string> CSettingsLanguages::LoadLanguages(int *languageSelect)
{
	DirList dirList;			
	
	std::string appLanguage(CSettings::getValueAsString(CSettings::AppLanguage));
	std::vector<std::string> languageName;
	
	//typedef std::map<std::string, std::map<std::string, std::string > > languagesMap;

	//languagesMap languagesNames;
	
	enumLanguages.insert(std::pair<int, std::string>(languageName.size(), "default"));
	languageName.push_back("Default");	
	if (appLanguage == "")
		*languageSelect = 0;
	
	if(dirList.LoadPath(languagesPath, filter))
	{				
		for(int i = 0; i < dirList.GetFilecount(); i++) 
		{
			
			
			std::vector<std::string> languageNameID = stringSplit(dirList.GetFilename(i), ".lang");
			
			std::string languageNameUpper = languageNameID[0];
			languageNameUpper[0] = ::toupper( languageNameUpper[0] );
			for ( std::string::iterator it = languageNameUpper.begin(); it != languageNameUpper.end(); it++ )
				if ( *it == ' ' )
			*( it+1 ) = ::toupper( *( it+1 ) );
  
			Languages.push_back ({dirList.GetFilename(i), languageNameID[0], trNOOP(languageNameUpper)});	
			
			if (appLanguage == Languages[i].languageID) 
				*languageSelect = languageName.size();
			enumLanguages.insert(std::pair<int, std::string>(languageName.size(), Languages[i].languageID.c_str()));
			languageName.push_back(Languages[i].languageName.c_str());		
		}
	}
	else
		*languageSelect = 0;
		
	return languageName;
	
}
void CSettingsLanguages::SetLanguage(int languageSelect)
{
	std::string nameLanguage;
	for ( std::map<int,std::string>::iterator nameSetLanguage = enumLanguages.begin() ; nameSetLanguage != enumLanguages.end(); nameSetLanguage++ ){
		if (nameSetLanguage->first == languageSelect){
			nameSetLanguage->second == "default" ? nameLanguage = "" : nameLanguage = nameSetLanguage->second ;
			break;			
		}
	}
	
	CSettings::setValueAsString(CSettings::AppLanguage, nameLanguage);

	//! load language
	languagesPath = "sd:/wiiu/apps/loadiine_gx2/languages/" + CSettings::getValueAsString(CSettings::AppLanguage) + ".lang";
	gettextLoadLanguage(languagesPath.c_str());
}