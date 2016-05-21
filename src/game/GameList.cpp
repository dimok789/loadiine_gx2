#include <algorithm>
#include <string>
#include <string.h>

#include "GameList.h"
#include "common/common.h"
#include "settings/CSettings.h"
#include "fs/DirList.h"
#include "utils/xml.h"

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
        char id6[7];
        int len = strlen(filename);

        discHeader newHeader;

        if (len <= 8 ||
                ((filename[len - 8] != '[' && filename[len - 6] != '[') || filename[len - 1] != ']'))
        {
            if (GetId6FromMeta((gamePath + "/" + filename + META_PATH).c_str(), id6) == 0)
            {
                newHeader.id = id6;
                newHeader.name = filename;
                newHeader.gamepath = gamePath + "/" + filename;

                fullGameList.push_back(newHeader);
            }
            continue;
        }

        bool id4Title = (filename[len - 8] != '[');

        std::string gamePathName = filename;
        if(id4Title)
        {
            newHeader.id = gamePathName.substr(gamePathName.size() - 5, 4);
            newHeader.name = gamePathName.substr(0, gamePathName.size() - 6);
        }
        else
        {
            newHeader.id = gamePathName.substr(gamePathName.size() - 7, 6);
            newHeader.name = gamePathName.substr(0, gamePathName.size() - 8);
        }
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

