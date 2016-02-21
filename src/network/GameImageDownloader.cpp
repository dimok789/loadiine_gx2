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
#include "GameImageDownloader.h"
#include "settings/CSettings.h"
#include "game/GameList.h"
#include "fs/CFile.hpp"
#include "fs/fs_utils.h"
#include "utils/logger.h"

using namespace std;

#define VALID_IMAGE(size) (!(size == 36864 || size <= 1024 || size == 7386 || size <= 1174 || size == 4446))

static const char *serverURL3D = "http://art.gametdb.com/wiiu/cover3D/";
static const char *serverURL2D = "http://art.gametdb.com/wiiu/cover/";
static const char *serverURLFullHQ = "http://art.gametdb.com/wiiu/coverfullHQ/";
static const char *serverURLFull = "http://art.gametdb.com/wiiu/coverfull/";
static const char *serverURLOrigDiscs = "http://art.gametdb.com/wiiu/discM/";
static const char *serverURLCustomDiscs = "http://art.gametdb.com/wiiu/disccustom/";

void GameImageDownloader::executeThread()
{
	MissingImagesCount = 0;
	FindMissingImages();

	if(MissingImagesCount == 0)
	{
	    log_printf("No images missing\n");
        asyncLoadFinished(this, MissingImages.size());
		return;
	}

	u32 TotalDownloadCount = MissingImagesCount;


	DownloadProcess(TotalDownloadCount);

	asyncLoadFinished(this, MissingImages.size());
}

void GameImageDownloader::FindMissingImages()
{
	//if(choices & CheckedBox1)
		FindMissing(CSettings::getValueAsString(CSettings::GameCover3DPath).c_str(), serverURL3D, NULL, "Downloading 3D Covers", NULL, ".png");

    /*
	if(choices & CheckedBox2)
		FindMissing(Settings.covers2d_path, serverURL2D, NULL, tr("Downloading Flat Covers"), NULL, ".png");

	if(choices & CheckedBox3)
	{
		const char * downloadURL = (Settings.coversfull == COVERSFULL_HQ || Settings.coversfull == COVERSFULL_HQ_LQ ) ? serverURLFullHQ : serverURLFull;
		const char * progressTitle = (Settings.coversfull == COVERSFULL_HQ || Settings.coversfull == COVERSFULL_HQ_LQ ) ? tr("Downloading Full HQ Covers") : tr("Downloading Full LQ Covers");
		const char * backupURL = (Settings.coversfull == COVERSFULL_HQ_LQ || Settings.coversfull == COVERSFULL_LQ_HQ) ? ((Settings.coversfull == COVERSFULL_HQ_LQ) ? serverURLFull : serverURLFullHQ) : NULL;
		const char * backupProgressTitle = (Settings.coversfull == COVERSFULL_HQ_LQ || Settings.coversfull == COVERSFULL_LQ_HQ) ? ((Settings.coversfull == COVERSFULL_HQ_LQ) ? tr("Downloading Full LQ Covers") : tr("Downloading Full HQ Covers")) : NULL;
		FindMissing(Settings.coversFull_path, downloadURL, backupURL, progressTitle, backupProgressTitle, ".png");
	}

	if(choices & CheckedBox4)
	{
		const char * downloadURL = (Settings.discart == DISCARTS_ORIGINALS || Settings.discart == DISCARTS_ORIGINALS_CUSTOMS ) ? serverURLOrigDiscs : serverURLCustomDiscs;
		const char * progressTitle = (Settings.discart == DISCARTS_ORIGINALS || Settings.discart == DISCARTS_ORIGINALS_CUSTOMS ) ? tr("Downloading original Discarts") : tr("Downloading custom Discarts");
		const char * backupURL = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || Settings.discart == DISCARTS_CUSTOMS_ORIGINALS) ? ((Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? serverURLCustomDiscs : serverURLOrigDiscs) : NULL;
		const char * backupProgressTitle = (Settings.discart == DISCARTS_ORIGINALS_CUSTOMS || Settings.discart == DISCARTS_CUSTOMS_ORIGINALS) ? ((Settings.discart == DISCARTS_ORIGINALS_CUSTOMS) ? tr("Downloading custom Discarts") : tr("Downloading original Discarts")) : NULL;
		FindMissing(Settings.disc_path, downloadURL, backupURL, progressTitle, backupProgressTitle, ".png");
	}
	*/
}

int GameImageDownloader::GetMissingGameFiles(const string & path, const string & fileext, vector<string> & MissingFilesList)
{
	MissingFilesList.clear();

	for (int i = 0; i < GameList::instance()->size(); ++i)
	{
		const discHeader* header = GameList::instance()->at(i);

        string filepath = path + "/" + header->id + fileext;

		if (CheckFile(filepath.c_str()))
			continue;

		//! Not found add to missing list
		MissingFilesList.push_back(header->id);
	}
	return MissingFilesList.size();
}


void GameImageDownloader::FindMissing(const char * writepath, const  char * downloadURL, const char * backupURL, const  char * progressTitle, const  char * backupProgressTitle, const  char * fileExt)
{
	if (!CreateSubfolder(writepath))
	{
		//WindowPrompt(tr( "Error !" ), fmt("%s %s", tr("Can't create directory"), writepath), tr( "OK" ));
		return;
	}

	vector<string> MissingFilesList;
    GetMissingGameFiles(writepath, fileExt, MissingFilesList);

	int size = MissingImages.size();
	MissingImages.resize(size+MissingFilesList.size());

	for(u32 i = 0, n = size; i < MissingFilesList.size(); ++i, ++n)
	{
		MissingImages[n].gameID = MissingFilesList[i];
		MissingImages[n].downloadURL = downloadURL;
		MissingImages[n].backupURL = backupURL;
		MissingImages[n].writepath = writepath;
		MissingImages[n].progressTitle = progressTitle;
		MissingImages[n].backupProgressTitle = backupProgressTitle;
		MissingImages[n].fileExt = fileExt;
	}

    MissingImagesCount += MissingFilesList.size();
}

int GameImageDownloader::DownloadProcess(int TotalDownloadCount)
{
	for(u32 i = 0, pos = 0; i < MissingImages.size(); ++i, ++pos)
	{
		//if(ProgressCanceled())
		//	break;

        //snprintf(progressMsg, sizeof(progressMsg), "http://gametdb.com : %s.png", MissingImages[i].gameID.c_str());

		//ShowProgress(MissingImages[i].progressTitle, fmt("%i %s", TotalDownloadCount - pos, tr( "files left" )), progressMsg, pos, TotalDownloadCount);

        char progressMsg[100];
        snprintf(progressMsg, sizeof(progressMsg), "http://gametdb.com : %s.png - %i files left", MissingImages[i].gameID.c_str(), TotalDownloadCount - pos);
        progressWindow.setTitle(progressMsg);
        progressWindow.setProgress(100.0f * (f32)pos / (f32)TotalDownloadCount);

        std::string imageData;

		DownloadImage(MissingImages[i].downloadURL, MissingImages[i].gameID.c_str(), MissingImages[i].fileExt, imageData);
		if(!imageData.size())
		{
			if(MissingImages[i].backupURL)
			{
				log_printf("Trying backup URL.\n");
				MissingImages[i].downloadURL = MissingImages[i].backupURL;
				MissingImages[i].backupURL = NULL;
				MissingImages[i].progressTitle = MissingImages[i].backupProgressTitle;
				--i;
				--pos;
			}
			continue;
		}

		log_printf(" - OK\n");

		std::string strOutpath;
		strOutpath = MissingImages[i].writepath;
		strOutpath += "/";
		strOutpath += MissingImages[i].gameID;
		strOutpath += MissingImages[i].fileExt;

		CFile file(strOutpath, CFile::WriteOnly);
		if(file.isOpen())
        {
			file.write((u8*)imageData.c_str(), imageData.size());
            file.close();
        }

		//! Remove the image from the vector since it's done
		MissingImages.erase(MissingImages.begin()+i);
		--i;
	}

	return MissingImages.size();
}

bool GameImageDownloader::DownloadImage(const char * url, const char * gameID, const char * fileExt, std::string & imageData)
{
	std::string checkedRegion;
	std::string downloadURL;
	bool PAL = false;
    const char *db_language = CSettings::getValueAsString(CSettings::ConsoleRegionCode).c_str(); // TODO: read from wiiu settings

	//Creates URL depending from which Country the game is
	switch (gameID[3])
	{
		case 'J':
		    downloadURL = std::string(url) + "JA/" + gameID + fileExt;
			checkedRegion = "JA";
			break;
		case 'W':
		    downloadURL = std::string(url) + "ZH/" + gameID + fileExt;
			checkedRegion = "ZH";
			break;
		case 'K':
		    downloadURL = std::string(url) + "KO/" + gameID + fileExt;
			checkedRegion = "KO";
			break;
		case 'P':
		case 'D':
		case 'F':
		case 'I':
		case 'S':
		case 'H':
		case 'U':
		case 'X':
		case 'Y':
		case 'Z':
		    downloadURL = std::string(url) + db_language + "/" + gameID + fileExt;
			checkedRegion = db_language;
			PAL = true;
			break;
		case 'E':
		    downloadURL = std::string(url) + "US/" + gameID + fileExt;
			checkedRegion = "US";
			break;
		default:
			break;
	}

	log_printf("downloadURL %s", downloadURL.c_str());

	FileDownloader::getFile(downloadURL, imageData);
	if(VALID_IMAGE(imageData.size()))
		return true;

    imageData.clear();

    //! those are not needed yet -> later
    //! just shutup the compiler
    (void)PAL;
    (void)serverURL2D;
    (void)serverURLFullHQ;
    (void)serverURLFull;
    (void)serverURLOrigDiscs;
    (void)serverURLCustomDiscs;

    return false;
/*
	if(PAL && strcmp(CheckedRegion, "EN") != 0)
	{
		snprintf(downloadURL, sizeof(downloadURL), "%sEN/%s.png", url, gameID);
		log_printf(" - Not found.\n%s", downloadURL);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;
	}
	else if(strcmp(CheckedRegion, "") == 0)
	{
		const char * lang = Settings.db_language;

		if(strcmp(lang, "EN") == 0 && CONF_GetRegion() == CONF_REGION_US)
			lang = "US";

		snprintf(downloadURL, sizeof(downloadURL), "%s%s/%s.png", url, lang, gameID);
		log_printf(" - Not found.\n%s", downloadURL);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;

		free(file.data);

		snprintf(downloadURL, sizeof(downloadURL), "%sOTHER/%s.png", url, gameID);
		log_printf(" - Not found.\n%s", downloadURL);
		file = downloadfile(downloadURL);
		if(VALID_IMAGE(file))
			return file;
	}

	log_printf(" - Not found.\n");
	free(file.data);

	memset(&file, 0, sizeof(struct block));
*/
}

