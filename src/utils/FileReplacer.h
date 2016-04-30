#include <string>
#include "Directory.h"
#include "menu/ProgressWindow.h"
#include <sys/dirent.h>
extern "C" {
  #include "fs/sd_fat_devoptab.h"
  #include "patcher/fs_logger.h"
  #include "dynamic_libs/fs_functions.h"
}

class FileReplacer
{
   public:
	  FileReplacer(std::string path,std::string content,std::string filename,void * pClient,void * pCmd);
      FileReplacer(std::string filepath,std::string content,std::string filename, ProgressWindow & progressWindow);
	  ~FileReplacer();
	  bool readFromFile(void *pClient, void *pCmd, const std::string & path , Directory* dir);
	  int read_dir(const std::string  & path, Directory * dir,int * entries,ProgressWindow & progressWindow);
	  int isFileExisting(std::string param);
	  int getSize();
	  std::string getFileListAsString();
	private:
	  Directory * dir_all;
};
