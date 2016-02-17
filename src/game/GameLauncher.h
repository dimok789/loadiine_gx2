#ifndef GAME_LAUNCHER_H_
#define GAME_LAUNCHER_H_

#include <vector>
#include <string>
#include <gctypes.h>
#include "common/common.h"
#include "rpx_rpl_table.h"
#include "memory_area_table.h"
#include "GameList.h"
#include "system/CThread.h"
#include "gui/sigslot.h"

class GameLauncher
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

    static int loadGameToMemory(const discHeader *hdr);

    static CThread * loadGameToMemoryAsync(const discHeader *hdr, sigslot::signal2<const discHeader *, int> * asyncLoadFinished);
private:
    static int LoadRpxRplToMem(const std::string & path, const std::string & name, bool isRPX, int entryIndex, const std::vector<std::string> & rplImportList);
    static void GetRpxImports(s_rpx_rpl * rpxArray, std::vector<std::string> & rplImports);

    static void gameLoadCallback(CThread *thread, void *arg);
};


#endif
