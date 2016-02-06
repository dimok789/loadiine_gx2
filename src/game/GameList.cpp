#include <algorithm>
#include <string>
#include <string.h>

#include "GameList.h"
#include "common/common.h"
#include "settings/CSettings.h"
#include "fs/DirList.h"

GameList *GameList::gameListInstance = NULL;

void GameList::clear()
{
	gameFilter.clear();
	fullGameList.clear();
	filteredList.clear();
	//! Clear memory of the vector completely
	std::vector<discHeader *>().swap(filteredList);
	std::vector<discHeader>().swap(fullGameList);
}

discHeader * GameList::getDiscHeader(const std::string & gameID) const
{
	for (u32 i = 0; i < filteredList.size(); ++i)
	{
		if(gameID == filteredList[i]->id)
			return filteredList[i];
	}

	return NULL;
}

int GameList::readGameList()
{
	// Clear list
	fullGameList.clear();
	//! Clear memory of the vector completely
	std::vector<discHeader>().swap(fullGameList);

	int cnt = 0;

	std::string gamePath = CSettings::getValueAsString(CSettings::GamePath);

	DirList dirList(gamePath, 0, DirList::Dirs);
	dirList.SortList();

    for(int i = 0; i < dirList.GetFilecount(); i++)
    {
        const char *filename = dirList.GetFilename(i);
        int len = strlen(filename);
        if (len <= 8)
            continue;

        if (filename[len - 8] != '[' || filename[len - 1] != ']')
            continue;

        std::string gamePathName = filename;
        discHeader newHeader;
        newHeader.id = gamePathName.substr(gamePathName.size() - 7, 6);
        newHeader.name = gamePathName.substr(0, gamePathName.size() - 8);
        newHeader.gamepath = gamePath + "/" + filename;

        while(newHeader.name.size() > 0 && newHeader.name[newHeader.name.size()-1] == ' ')
            newHeader.name.resize(newHeader.name.size()-1);

        fullGameList.push_back(newHeader);
    }

	return cnt;
}

void GameList::internalFilterList(std::vector<discHeader> &fullList)
{
	for (u32 i = 0; i < fullList.size(); ++i)
	{
		discHeader *header = &fullList[i];

		//! TODO: do filtering as needed

		filteredList.push_back(header);
	}
}

int GameList::filterList(const char * filter)
{
    if(filter)
        gameFilter = filter;

	if(fullGameList.size() == 0)
		readGameList();

	filteredList.clear();

	// Filter current game list if selected
    internalFilterList(fullGameList);

	sortList();

	return filteredList.size();
}

void GameList::internalLoadUnfiltered(std::vector<discHeader> & fullList)
{
	for (u32 i = 0; i < fullList.size(); ++i)
	{
		discHeader *header = &fullList[i];

		filteredList.push_back(header);
	}
}

int GameList::loadUnfiltered()
{
	if(fullGameList.size() == 0)
		readGameList();

	gameFilter.clear();
	filteredList.clear();

	// Filter current game list if selected
	internalLoadUnfiltered(fullGameList);

	sortList();

	return filteredList.size();
}

void GameList::sortList()
{
    std::sort(filteredList.begin(), filteredList.end(), nameSortCallback);
}

bool GameList::nameSortCallback(const discHeader *a, const discHeader *b)
{
	return (strcasecmp(((discHeader *) a)->name.c_str(), ((discHeader *) b)->name.c_str()) < 0);
}

