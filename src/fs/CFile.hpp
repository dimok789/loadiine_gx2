#ifndef CFILE_HPP_
#define CFILE_HPP_

#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gctypes.h>

class CFile
{
	public:
		enum eOpenTypes
		{
		    ReadOnly,
		    WriteOnly,
		    ReadWrite,
		    Append
		};

		CFile();
		CFile(const std::string & filepath, eOpenTypes mode);
		CFile(const u8 * memory, int memsize);
		virtual ~CFile();

		int open(const std::string & filepath, eOpenTypes mode);
		int open(const u8 * memory, int memsize);

		bool isOpen() const {
            if(iFd >= 0)
                return true;

            if(mem_file)
                return true;

            return false;
		}

		void close();

		int read(u8 * ptr, size_t size);
		int write(const u8 * ptr, size_t size);
		int fwrite(const char *format, ...);
		int seek(long int offset, int origin);
		u64 tell() { return pos; };
		u64 size() { return filesize; };
		void rewind() { this->seek(0, SEEK_SET); };

	protected:
		int iFd;
		const u8 * mem_file;
		u64 filesize;
		u64 pos;
};

#endif
