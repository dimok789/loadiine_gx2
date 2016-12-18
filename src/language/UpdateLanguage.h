#ifndef UPDATE_LANGUAGE_H_
#define UPDATE_LANGUAGE_H_

#include <vector>
#include <string>
#include "network/FileDownloader.h"
#include "system/AsyncDeleter.h"
#include "gui/MessageBox.h"

class UpdateLanguage : public GuiFrame, public CThread
{
public:
    UpdateLanguage()
        : GuiFrame(0, 0)
        , CThread(CThread::eAttributeAffCore0 | CThread::eAttributePinnedAff)
		, messageBox(MessageBox::BT_NOBUTTON, MessageBox::IT_ICONINFORMATION, true)
	{   
		
		append(&messageBox);
		
	}

    void startDownloadingLanguage()
    {
        resumeThread();
    }
	
	sigslot::signal1<GuiElement *> asyncLoadFinished;

private:
    void executeThread();
    void GetLanguages();
    void GetLanguagesFiles(std::vector<std::string> languageList);
    void DownloadFile(int DownloadCount);
	
    struct FileLink
    {
        std::string languageID;
        const char *progressTitle;
    };
    std::vector<FileLink> LanguageFiles;
	
	u32 LanguageCount;
	MessageBox messageBox;

};

#endif
