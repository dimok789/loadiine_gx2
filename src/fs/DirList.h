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
#ifndef ___DIRLIST_H_
#define ___DIRLIST_H_

#include <vector>
#include <string>
#include <gctypes.h>

typedef struct
{
	char * FilePath;
	bool isDir;
} DirEntry;

class DirList
{
public:
	//!Constructor
	DirList(void);
	//!\param path Path from where to load the filelist of all files
	//!\param filter A fileext that needs to be filtered
	//!\param flags search/filter flags from the enum
	DirList(const std::string & path, const char *filter = NULL, u32 flags = Files | Dirs);
	//!Destructor
	virtual ~DirList();
	//! Load all the files from a directory
	bool LoadPath(const std::string & path, const char *filter = NULL, u32 flags = Files | Dirs);
	//! Get a filename of the list
	//!\param list index
	const char * GetFilename(int index) const;
	//! Get the a filepath of the list
	//!\param list index
	const char *GetFilepath(int index) const { if (!valid(index)) return ""; else return FileInfo[index].FilePath; }
	//! Get the a filesize of the list
	//!\param list index
	u64 GetFilesize(int index) const;
	//! Is index a dir or a file
	//!\param list index
	bool IsDir(int index) const { if(!valid(index)) return false; return FileInfo[index].isDir; };
	//! Get the filecount of the whole list
	int GetFilecount() const { return FileInfo.size(); };
	//! Sort list by filepath
	void SortList();
	//! Custom sort command for custom sort functions definitions
	void SortList(bool (*SortFunc)(const DirEntry &a, const DirEntry &b));
	//! Get the index of the specified filename
	int GetFileIndex(const char *filename) const;
	//! Enum for search/filter flags
	enum
	{
		Files = 0x01,
		Dirs = 0x02,
		CheckSubfolders = 0x08,
	};
protected:
	// Internal parser
	bool InternalLoadPath(std::string &path);
	//!Add a list entrie
	void AddEntrie(const std::string &filepath, const char * filename, bool isDir);
	//! Clear the list
	void ClearList();
	//! Check if valid pos is requested
	inline bool valid(u32 pos) const { return (pos < FileInfo.size()); };

	u32 Flags;
	const char *Filter;
	std::vector<DirEntry> FileInfo;
};

#endif
