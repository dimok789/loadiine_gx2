/***************************************************************************
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
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef __STRING_TOOLS_H
#define __STRING_TOOLS_H

#include <gctypes.h>
#include <vector>
#include <string>

const char * fmt(const char * format, ...);
const wchar_t * wfmt(const char * format, ...);
int strprintf(std::string &str, const char * format, ...);
std::string strfmt(const char * format, ...);
bool char2wchar_t(const char * src, wchar_t * dest);
int strtokcmp(const char * string, const char * compare, const char * separator);
int strextcmp(const char * string, const char * extension, char seperator);

inline const char * FullpathToFilename(const char *path)
{
	if(!path) return path;

	const char * ptr = path;
	const char * Filename = ptr;

	while(*ptr != '\0')
	{
		if(ptr[0] == '/' && ptr[1] != '\0')
			Filename = ptr+1;

		++ptr;
	}

	return Filename;
}

inline void RemoveDoubleSlashs(std::string &str)
{
	u32 length = str.size();

	//! clear path of double slashes
	for(u32 i = 1; i < length; ++i)
	{
		if(str[i-1] == '/' && str[i] == '/')
		{
			str.erase(i, 1);
			i--;
			length--;
		}
	}
}

std::vector<std::string> stringSplit(const std::string & value, const std::string & splitter);

#endif /* __STRING_TOOLS_H */

