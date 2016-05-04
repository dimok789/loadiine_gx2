/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * DirList Class
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <sys/dirent.h>

#include "DirList.h"
#include "utils/StringTools.h"

DirList::DirList()
{
	Flags = 0;
	Filter = 0;
}

DirList::DirList(const std::string & path, const char *filter, u32 flags)
{
	this->LoadPath(path, filter, flags);
	this->SortList();
}

DirList::~DirList()
{
	ClearList();
}

bool DirList::LoadPath(const std::string & folder, const char *filter, u32 flags)
{
	if(folder.empty()) return false;

	Flags = flags;
	Filter = filter;

	std::string folderpath(folder);
	u32 length = folderpath.size();

	//! clear path of double slashes
	RemoveDoubleSlashs(folderpath);

	//! remove last slash if exists
	if(length > 0 && folderpath[length-1] == '/')
		folderpath.erase(length-1);

	return InternalLoadPath(folderpath);
}

bool DirList::InternalLoadPath(std::string &folderpath)
{
	if(folderpath.size() < 3)
		return false;

	struct dirent *dirent = NULL;
	DIR *dir = NULL;

	dir = opendir(folderpath.c_str());
	if (dir == NULL)
		return false;

	while ((dirent = readdir(dir)) != 0)
	{
		bool isDir = dirent->d_type & DT_DIR;
		const char *filename = dirent->d_name;

		if(isDir)
		{
			if(strcmp(filename,".") == 0 || strcmp(filename,"..") == 0)
				continue;

			if(Flags & CheckSubfolders)
			{
				int length = folderpath.size();
				if(length > 2 && folderpath[length-1] != '/')
					folderpath += '/';
				folderpath += filename;
				InternalLoadPath(folderpath);
				folderpath.erase(length);
			}

			if(!(Flags & Dirs))
				continue;
		}
		else if(!(Flags & Files))
		{
			continue;
		}

		if(Filter)
		{
			char * fileext = strrchr(filename, '.');
			if(!fileext)
				continue;

			if(strtokcmp(fileext, Filter, ",") == 0)
				AddEntrie(folderpath, filename, isDir);
		}
		else
		{
			AddEntrie(folderpath, filename, isDir);
		}
	}
	closedir(dir);

	return true;
}

void DirList::AddEntrie(const std::string &filepath, const char * filename, bool isDir)
{
	if(!filename)
		return;

	// Don't list hidden OS X files
	if(filename[0] == '.' && filename[1] == '_')
		return;

	int pos = FileInfo.size();

	FileInfo.resize(pos+1);

	FileInfo[pos].FilePath = (char *) malloc(filepath.size()+strlen(filename)+2);
	if(!FileInfo[pos].FilePath)
	{
		FileInfo.resize(pos);
		return;
	}

	sprintf(FileInfo[pos].FilePath, "%s/%s", filepath.c_str(), filename);
	FileInfo[pos].isDir = isDir;
}

void DirList::ClearList()
{
	for(u32 i = 0; i < FileInfo.size(); ++i)
	{
		if(FileInfo[i].FilePath)
			free(FileInfo[i].FilePath);
	}

	FileInfo.clear();
	std::vector<DirEntry>().swap(FileInfo);
}

const char * DirList::GetFilename(int ind) const
{
	if (!valid(ind))
		return "";

	return FullpathToFilename(FileInfo[ind].FilePath);
}

static bool SortCallback(const DirEntry & f1, const DirEntry & f2)
{
	if(f1.isDir && !(f2.isDir)) return true;
	if(!(f1.isDir) && f2.isDir) return false;

	if(f1.FilePath && !f2.FilePath) return true;
	if(!f1.FilePath) return false;

	if(strcasecmp(f1.FilePath, f2.FilePath) > 0)
		return false;

	return true;
}

void DirList::SortList()
{
	if(FileInfo.size() > 1)
		std::sort(FileInfo.begin(), FileInfo.end(), SortCallback);
}

void DirList::SortList(bool (*SortFunc)(const DirEntry &a, const DirEntry &b))
{
	if(FileInfo.size() > 1)
		std::sort(FileInfo.begin(), FileInfo.end(), SortFunc);
}

u64 DirList::GetFilesize(int index) const
{
	struct stat st;
	const char *path = GetFilepath(index);

	if(!path || stat(path, &st) != 0)
		return 0;

	return st.st_size;
}

int DirList::GetFileIndex(const char *filename) const
{
	if(!filename)
		return -1;

	for (u32 i = 0; i < FileInfo.size(); ++i)
	{
		if (strcasecmp(GetFilename(i), filename) == 0)
			return i;
	}

	return -1;
}
