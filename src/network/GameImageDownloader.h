#ifndef GAME_IMAGE_DOWNLOADER_H_
#define GAME_IMAGE_DOWNLOADER_H_

#include <vector>
#include <string>
#include "FileDownloader.h"
#include "system/AsyncDeleter.h"
#include "menu/ProgressWindow.h"

class GameImageDownloader : public GuiFrame, public CThread
{
public:
    GameImageDownloader()
        : GuiFrame(0, 0)
        , CThread(CThread::eAttributeAffCore0 | CThread::eAttributePinnedAff)
        , progressWindow("Downloading images...")
	{
	    append(&progressWindow);

	    width = progressWindow.getWidth();
	    height = progressWindow.getHeight();
	}

    void startDownloading()
    {
        resumeThread();
    }

    sigslot::signal2<GameImageDownloader *, int> asyncLoadFinished;
private:
    void executeThread();

    void FindMissingImages();
    void FindMissing(const char *writepath, const char *downloadURL, const char *backupURL, const char *progressTitle, const char *backupProgressTitle, const char *fileExt);
    int GetMissingGameFiles(const std::string & path, const std::string & fileext, std::vector<std::string> & MissingFilesList);
    int DownloadProcess(int TotalDownloadCount);
    bool DownloadImage(const char * url, const char * gameID, const char * fileExt, std::string & imageData);

    struct ImageLink
    {
        std::string gameID;
        const char *downloadURL;
        const char *backupURL;
        const char *writepath;
        const char *progressTitle;
        const char *backupProgressTitle;
        const char *fileExt;
    };
    u32 MissingImagesCount;
    std::vector<ImageLink> MissingImages;

    ProgressWindow progressWindow;
};

#endif
