/****************************************************************************
 * Copyright (C) 2011 Dimok
 * Copyright (C) 2012 Cyan
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
#include <string.h>
#include "UpdateLanguage.h"
#include "settings/CSettings.h"
#include "fs/CFile.hpp"
#include "fs/fs_utils.h"
#include "utils/logger.h"
#include "utils/StringTools.h"
#include "language/gettext.h"

using namespace std;

static const char *LanguageURL = "http://masqchips.github.io/loadiine_gx2/";
static const char *LanguageFilesURL = "http://masqchips.github.io/loadiine_gx2/languages/";

void UpdateLanguage::executeThread()
{
	LanguageCount = 0;
	
	messageBox.setTitle(tr("Downloading languages"));
	
	GetLanguages();

	if(LanguageCount == 0)
	{
	    log_printf("No languages availables\n");
        asyncLoadFinished(this);
		return;
	}


	DownloadFile(LanguageCount);
	log_printf("OK!!\n");
	
	asyncLoadFinished(this);
}

void UpdateLanguage::GetLanguages()
{
	
	messageBox.setMessage(tr("Connecting.."));
	
	string sData;
	FileDownloader::getFile(LanguageURL, sData);
	
	messageBox.setMessage(tr("Getting file list"));
	
	log_printf("downloadURL %s\n", LanguageURL);
		
	vector<string> lines = stringSplit(sData, "\n");
		
	if(!lines.empty())
	{		
		vector<string> languageFile;
		for(u32 i = 0; i < lines.size(); ++i)
		{
			vector<string> valueSplit = stringSplit(lines[i], "<!--");
			
			if(valueSplit.size() != 2)
			continue;
			
			vector<string> value = stringSplit(valueSplit[1].c_str(), "-->");
            languageFile.push_back(value[0]);
            
			log_printf("languages files %s\n", value[0].c_str());
		}
		
		GetLanguagesFiles(languageFile); 
	}
		
}

void UpdateLanguage::GetLanguagesFiles(std::vector<std::string> languageList)
{
	/*if (!CreateSubfolder(CSettings::getValueAsString(CSettings::LanguagesPath).c_str()))
	{
		//WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
		return;
	}*/

	LanguageFiles.resize(languageList.size());

	for(u32 i = 0; i < languageList.size(); ++i)
	{
		LanguageFiles[i].languageID = languageList[i];
	}

    LanguageCount = languageList.size();
}

void UpdateLanguage::DownloadFile(int DownloadCount)
{
	
	for(u32 i = 0, pos = 1; i < LanguageFiles.size(); ++i, ++pos)
	{
		char buffer[100];
		snprintf(buffer,sizeof(buffer), "%d", DownloadCount - pos);
        
		string progressMsg = tr("Languages files:");
		progressMsg += " ";
		progressMsg += LanguageFiles[i].languageID.c_str();
		progressMsg += " - ";
		progressMsg += std::string(buffer);
		progressMsg += " ";
		progressMsg += tr("files left.");
		
		messageBox.setInfo(progressMsg.c_str());
		
        messageBox.setProgress(100.0f * (f32)pos / (f32)DownloadCount);

        string fileData;
		
        string downloadFileURL = LanguageFilesURL + LanguageFiles[i].languageID;
		
		log_printf("download languages files URL %s\n", downloadFileURL.c_str());

		FileDownloader::getFile(downloadFileURL, fileData);
		
		messageBox.setMessage(tr("Downloading files"));

		if(!fileData.size())
		{
			continue;
		}

		string strOutpath = CSettings::getValueAsString(CSettings::LanguagesPath).c_str(); 
		
		if (!CreateSubfolder(strOutpath.c_str()))
		{
			//WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
		}
		strOutpath += "/";
		strOutpath += LanguageFiles[i].languageID.c_str();
		
		CFile file(strOutpath, CFile::WriteOnly);
		if(file.isOpen())
        {
			file.write((u8*)fileData.c_str(), fileData.size());
            file.close();
        }
		
		log_printf("Save language file %s\n", strOutpath.c_str());
		
		fileData.clear();

		//! Remove the file from the vector since it's done
		LanguageFiles.erase(LanguageFiles.begin()+i);
		--i;
	}
}

