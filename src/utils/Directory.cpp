#include "Directory.h"
#include <string.h>
#include <stdio.h>

extern "C" {
  // Get declaration for f(int i, char c, float x)
  #include "utils/logger.h" 
  #include "common/common.h"
}

Directory::Directory(std::string name_){
	name = name_;
}

Directory::~Directory(){
	for(unsigned int i = 0; i < folder.size(); i++){
		delete folder[i];
	}
}

int Directory::getSize()
{
	int dir_size = sizeof(Directory);
	int file_size = sizeof(std::string) * files.capacity();
	for(unsigned int i = 0; i < files.size(); i++){	
		file_size += files[i].capacity();
	}
	int folder_size = sizeof(Directory*) * folder.capacity();
	int size = 0;
	for(unsigned int i = 0; i < folder.size(); i++){	
		size += folder[i]->getSize();
	}
	char log_size[100];
	size += dir_size +file_size + folder_size;
	sprintf(log_size,"Size of %s: %d Bytes | Dirsize: %d | Folder %d(%d Bytes) Files: %d (%d Bytes)",getFolderName().c_str(),size,dir_size,folder.size(),folder_size,files.size(),file_size);
	return size;
}


void Directory::addFile(std::string name_){
	files.push_back(name_);
}
void Directory::addFolder(Directory * dir){
	folder.push_back(dir);
}

std::string Directory::getFolderName(){
	return name;
}

bool Directory::isInFolder(std::string file_path){
	char * start = (char*)file_path.c_str();
	char * end = start;
	while(*end != '/' && *end != '\0'){
		end++;
	}
	if(*end == '\0'){ // only a file
		if(checkFile(file_path)){
			return true;
		}
	}else{
		std::string rest = std::string(end+1);
		*end = '\0';
		Directory * folder = getFolder(std::string(start));
		if(folder != NULL){
			return folder->isInFolder(rest);
		}		
	}
	return false;
}

bool Directory::checkFile(std::string filename){
	for(unsigned int i = 0; i < files.size(); i++){	
		if(files[i].compare(filename) == 0){
			return true;
		}
	}
	return false;
}


Directory * Directory::getFolder(std::string foldername){
	Directory * result = NULL;
	for(unsigned int i = 0; i < folder.size(); i++){
		if(folder[i]->getFolderName().compare(foldername) == 0){
			result = folder[i];
			break;
		}
	}
	return result;
}

Directory * Directory::getParent(){
	return this->parent;
}

void Directory::setParent(Directory * parent){
	this->parent = parent;
}

void Directory::printFolderRecursive(std::string base){
	base += "/";
	base += getFolderName();
	
	for(unsigned int i = 0; i < folder.size(); i++){
		folder[i]->printFolderRecursive(std::string(base));
	}		
	
	for(unsigned int i = 0; i < files.size(); i++){
		log_printf("%s/%s",base.c_str(),files[i].c_str()); ;
	}
}

std::string Directory::getFileList(){
	std::string strBuffer;
	
	for(unsigned int i = 0; i < folder.size(); i++){
		strBuffer += DIR_IDENTIFY + folder[i]->getFolderName() + "\n";
		strBuffer += folder[i]->getFileList();
		strBuffer += std::string(PARENT_DIR_IDENTIFY) + "\n";		
	}		
	
	for(unsigned int i = 0; i < files.size(); i++){
		strBuffer += files[i] + "\n";
	}
	return strBuffer;
}




