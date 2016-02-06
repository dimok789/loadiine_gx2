#include <stdarg.h>
#include <stdlib.h>
#include "CFile.hpp"

CFile::CFile()
{
	iFd = -1;
	mem_file = NULL;
	filesize = 0;
	pos = 0;
}

CFile::CFile(const std::string & filepath, eOpenTypes mode)
{
	iFd = -1;
	this->open(filepath, mode);
}

CFile::CFile(const u8 * mem, int size)
{
	iFd = -1;
	this->open(mem, size);
}

CFile::~CFile()
{
	this->close();
}

int CFile::open(const std::string & filepath, eOpenTypes mode)
{
	this->close();

	s32 openMode = 0;

	switch(mode)
	{
    default:
    case ReadOnly:
        openMode = O_RDONLY;
        break;
    case WriteOnly:
        openMode = O_WRONLY;
        break;
    case ReadWrite:
        openMode = O_RDWR;
        break;
    case Append:
        openMode = O_APPEND | O_WRONLY;
        break;
	}

    //! Using fopen works only on the first launch as expected
    //! on the second launch it causes issues because we don't overwrite
    //! the .data sections which is needed for a normal application to re-init
    //! this will be added with launching as RPX
	iFd = ::open(filepath.c_str(), openMode);
	if(iFd < 0)
		return iFd;


	filesize = ::lseek(iFd, 0, SEEK_END);
	::lseek(iFd, 0, SEEK_SET);

	return 0;
}

int CFile::open(const u8 * mem, int size)
{
	this->close();

	mem_file = mem;
	filesize = size;

	return 0;
}

void CFile::close()
{
	if(iFd >= 0)
		::close(iFd);

	iFd = -1;
	mem_file = NULL;
	filesize = 0;
	pos = 0;
}

int CFile::read(u8 * ptr, size_t size)
{
	if(iFd >= 0)
	{
		int ret = ::read(iFd, ptr,size);
		if(ret > 0)
			pos += ret;
		return ret;
	}

	int readsize = size;

	if(readsize > (s64) (filesize-pos))
		readsize = filesize-pos;

	if(readsize <= 0)
		return readsize;

	if(mem_file != NULL)
	{
		memcpy(ptr, mem_file+pos, readsize);
		pos += readsize;
		return readsize;
	}

	return -1;
}

int CFile::write(const u8 * ptr, size_t size)
{
	if(iFd >= 0)
	{
	    size_t done = 0;
	    while(done < size)
        {
            int ret = ::write(iFd, ptr, size - done);
            if(ret <= 0)
                return ret;

            ptr += ret;
            done += ret;
            pos += ret;
        }
		return done;
	}

	return -1;
}

int CFile::seek(long int offset, int origin)
{
	int ret = 0;
	s64 newPos = pos;

	if(origin == SEEK_SET)
	{
		newPos = offset;
	}
	else if(origin == SEEK_CUR)
	{
		newPos += offset;
	}
	else if(origin == SEEK_END)
	{
		newPos = filesize+offset;
	}

	if(newPos < 0)
	{
		pos = 0;
	}
	else {
        pos = newPos;
	}

	if(iFd >= 0)
		ret = ::lseek(iFd, pos, SEEK_SET);

	if(mem_file != NULL)
	{
		if(pos > filesize)
		{
			pos = filesize;
		}
	}

	return ret;
}

int CFile::fwrite(const char *format, ...)
{
    int result = -1;
	char * tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        result = this->write((u8 *)tmp, strlen(tmp));
	}
	va_end(va);

	if(tmp)
		free(tmp);

    return result;
}


