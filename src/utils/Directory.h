#include <string>
#include <vector>

class Directory
{
   public:
		Directory(std::string name);
		~Directory();
		void addFile(std::string name_);
		void addFolder(Directory * dir);
		std::string getFolderName(void);
		void printFolderRecursive(std::string base);
		Directory * getFolder(std::string foldername);
		bool isInFolder(std::string file_path);
		bool checkFile(std::string filename);
		int getSize();
		std::string getFileList();
		Directory * getParent();
		void setParent(Directory * parent);
	private:
		std::string name;
		std::vector<std::string> files;
		std::vector<Directory*> folder;
		Directory * parent = NULL;
};