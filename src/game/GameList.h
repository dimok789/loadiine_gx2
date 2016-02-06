#ifndef GAME_LIST_H_
#define GAME_LIST_H_

#include <vector>
#include <gctypes.h>

typedef struct _discHeader
{
    std::string id;
    std::string name;
    std::string gamepath;
} discHeader;

class GameList
{
	public:
	    static GameList *instance() {
            if(!gameListInstance) {
                gameListInstance = new GameList;
            }
            return gameListInstance;
	    }
	    static void destroyInstance() {
	        if(gameListInstance) {
                delete gameListInstance;
                gameListInstance = NULL;
	        }
	    }

		int size() const { return filteredList.size(); }
		int gameCount() const { return fullGameList.size(); }
		int filterList(const char * gameFilter = NULL);
		int loadUnfiltered();

		discHeader * at(int i) const { return operator[](i); }
		discHeader * operator[](int i) const { if (i < 0 || i >= (int) filteredList.size()) return NULL; return filteredList[i]; }
		discHeader * getDiscHeader(const std::string & gameID) const;

		const char * getCurrentFilter() const { return gameFilter.c_str(); }
		void sortList();
		void clear();
		bool operator!() const { return (fullGameList.size() == 0); }

		//! Gamelist scrolling operators
		int operator+=(int i) { return (selectedGame = (selectedGame+i) % filteredList.size()); }
		int operator-=(int i) { return (selectedGame = (selectedGame-i+filteredList.size()) % filteredList.size()); }
		int operator++() { return (selectedGame = (selectedGame+1) % filteredList.size()); }
		int operator--() { return (selectedGame = (selectedGame-1+filteredList.size()) % filteredList.size()); }
		int operator++(int i) { return operator++(); }
		int operator--(int i) { return operator--(); }
		discHeader * GetCurrentSelected() const { return operator[](selectedGame); }

		std::vector<discHeader *> & getfilteredList(void) { return filteredList; }
		std::vector<discHeader> & getFullGameList(void) { return fullGameList; }
	protected:
		GameList() : selectedGame(0) { };

		int readGameList();

		void internalFilterList(std::vector<discHeader> & fullList);
		void internalLoadUnfiltered(std::vector<discHeader> & fullList);

		static bool nameSortCallback(const discHeader *a, const discHeader *b);

        static GameList *gameListInstance;

		std::string gameFilter;
		int selectedGame;
		std::vector<discHeader *> filteredList;
		std::vector<discHeader> fullGameList;
};

#endif
