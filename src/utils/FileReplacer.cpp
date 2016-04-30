#include "FileReplacer.h"
#include "utils/logger.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <malloc.h>
#include "utils/StringTools.h"
#include "common/common.h"
#include "fs/fs_utils.c"


FileReplacer::FileReplacer(std::string path,std::string content,std::string filename,void * pClient,void * pCmd){
	bool result = false;
	dir_all = new Directory("content");     
    std::string filepath = path + std::string("/") + filename;
    log_printf("Read from file for replacement: %s\n",filename.c_str());
    result = this->readFromFile(pClient,pCmd,filepath,dir_all);
    if(!result){
        log_print("Error!\n");
    }else{
        //dir_all->printFolderRecursive("");
    }
}

FileReplacer::FileReplacer(std::string filepath,std::string content,std::string filename, ProgressWindow & progressWindow){
    int entries = 0;
    progressWindow.setTitle("Creating filelist.txt:");

    dir_all = new Directory("content");
    read_dir(filepath + std::string("/") + content, dir_all, &entries, progressWindow);
    log_printf("Found %d entries\n",entries);
}

std::string FileReplacer::getFileListAsString(){
	return dir_all->getFileList();
}

int FileReplacer::getSize()
{
	int size = sizeof(FileReplacer);
	size += dir_all->getSize();
	return size;
}

FileReplacer::~FileReplacer()
{
	delete dir_all;
}

bool FileReplacer::readFromFile(void *pClient, void *pCmd, const std::string & path , Directory* dir){
	int handle = 0;
	FSStat stats;
	int ret = -1;
	if(FSGetStat(pClient, pCmd, path.c_str(),&stats,FS_RET_ALL_ERROR) == FS_STATUS_OK){
		char * file  = (char *) malloc((sizeof(char)*stats.size)+1);
		if(!file){
			log_print("Failed to allocate space for reading the file\n");
			return false;
		}
		file[stats.size] = '\0';
		if ((ret = FSOpenFile(pClient, pCmd, path.c_str(),"r", &handle, FS_RET_ALL_ERROR)) == FS_STATUS_OK){
			int total_read = 0;
			int ret2 = 0;
			while ((ret2 = FSReadFile(pClient,  pCmd, file+total_read, 1, stats.size-total_read, handle, 0, FS_RET_ALL_ERROR)) > 0){
				log_print("Reading filelist.txt\n");
				total_read += ret2;
			}
		}
		Directory * dir_cur = dir;
		char * ptr;
		char delimiter[] = { '\n', '\r' };
		ptr = strtok (file,delimiter);
		while (ptr != NULL)
		{
            std::string dirname(ptr);
            if(dirname.compare(PARENT_DIR_IDENTIFY) == 0){ // back dir
                dir_cur = dir_cur->getParent();
                if(dir_cur == NULL){
                    log_print("Something went wrong. Try to delete the filelist.txt\n");
                    free(file);
                    return false;
                }
            }else if(dirname.substr(0,1).compare(DIR_IDENTIFY) == 0){ //DIR
                dirname = dirname.substr(1);
                Directory * dir_new = new Directory(dirname);
                dir_cur->addFolder(dir_new);
                dir_new->setParent(dir_cur);
                dir_cur = dir_new;
            }else {
                dir_cur->addFile(dirname);
            }
			ptr = strtok(NULL, delimiter);
		}		
		free(file);
		return true;
	}else{
		return false;
	}
}

int FileReplacer::read_dir(const std::string & path , Directory* dir, int * entries, ProgressWindow & progressWindow){
	std::vector<std::string *> dirlist;

    struct dirent *dirent = NULL;
	DIR *dir_ = NULL;

	dir_ = opendir(path.c_str());
	if (dir_ == NULL)
		return -1;
	while ((dirent = readdir(dir_)) != 0)
	{
        (*entries)++;
		bool isDir = dirent->d_type & DT_DIR;
		const char *filename = dirent->d_name;
        if(*entries %25 == 0){
            progressWindow.setTitle(strfmt("Creating filelist.txt: %d entries found", *entries));
        }
        if(isDir){
            
            dirlist.push_back(new std::string(filename));
        }else{
            dir->addFile(filename);
        }
    }
    closedir(dir_);

    for(unsigned int i = 0; i < dirlist.size(); i++){
        Directory * dir_new = new Directory(*dirlist[i]);
        dir->addFolder(dir_new);
        read_dir((path + "/"+ *dirlist[i]),dir_new,entries,progressWindow);
    }
    
    for(unsigned int i = 0; i < dirlist.size(); i++){
        delete dirlist[i];
    }

	return 0;
}

int FileReplacer::isFileExisting(std::string param){
	if(dir_all != 0){
		if(dir_all->isInFolder(param)){
			return 0;
		}
	}
	return -1;
}
