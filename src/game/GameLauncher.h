#ifndef GAME_LAUNCHER_H_
#define GAME_LAUNCHER_H_

#include <vector>
#include <string>
#include <gctypes.h>
#include "common/common.h"
#include "menu/ProgressWindow.h"
#include "rpx_rpl_table.h"
#include "memory_area_table.h"
#include "GameList.h"
#include "system/CThread.h"
#include "gui/sigslot.h"
#include "fs/CFile.hpp"

class GameLauncher : public GuiFrame, public CThread
{
public:
    enum eLoadResults
    {
        SUCCESS = 0,
        INVALID_INPUT = -1,
        RPX_NOT_FOUND = -2,
        TOO_MANY_RPX_NOT_FOUND = -3,
        FILE_OPEN_FAILURE = -4,
        FILE_READ_ERROR = -5,
        NOT_ENOUGH_MEMORY = -6,
    };


    static GameLauncher * loadGameToMemoryAsync(const discHeader *hdr);
    sigslot::signal3<GameLauncher *, const discHeader *, int> asyncLoadFinished;
private:

    GameLauncher(const discHeader *hdr)
        : GuiFrame(0, 0)
        , CThread(CThread::eAttributeAffCore0 | CThread::eAttributePinnedAff)
        , discHdr(hdr)
        , progressWindow("Loading game...")
	{
	    append(&progressWindow);

	    width = progressWindow.getWidth();
	    height = progressWindow.getHeight();
	}
    void executeThread();

    int loadGameToMemory(const discHeader *hdr);

    int LoadRpxRplToMem(const std::string & path, const std::string & name, bool isRPX, int entryIndex, std::vector<std::string> & rplImportList);
    void GetRpxImportsRecursive(CFile file, std::vector<std::string> & rplImports, std::map<std::string, std::string> & rplNameList);

    static void gameLoadCallback(CThread *thread, void *arg);
    bool createFileList(const std::string  & filepath);
    const discHeader *discHdr;
    ProgressWindow progressWindow;
};


#endif
