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
#ifndef BUFFER_CIRCLE_HPP_
#define BUFFER_CIRCLE_HPP_

#include <vector>
#include <gctypes.h>

class BufferCircle
{
	public:
		//!> Constructor
		BufferCircle();
		//!> Destructor
		~BufferCircle();
		//!> Set circle size
		void Resize(int size);
		//!> Get the circle size
		int Size() { return SoundBuffer.size(); };
		//!> Set/resize the buffer size
		void SetBufferBlockSize(int size);
		//!> Remove a buffer
		void RemoveBuffer(int pos);
		//!> Set all buffers clear
		void ClearBuffer();
		//!> Free all buffers
		void FreeBuffer();
		//!> Switch to next buffer
		void LoadNext();
		//!> Get the current buffer
		u8 * GetBuffer() { return GetBuffer(which); };
		//!> Get a buffer at a position
		u8 * GetBuffer(int pos) { if(!Valid(pos)) return NULL; else return SoundBuffer[pos]; };
		//!> Get current buffer size
		u32 GetBufferSize() { return GetBufferSize(which); };
		//!> Get buffer size at position
		u32 GetBufferSize(int pos) { if(!Valid(pos)) return 0; else return BufferSize[pos]; };
		//!> Is current buffer ready
		bool IsBufferReady() { return IsBufferReady(which); };
		//!> Is  a buffer at a position ready
		bool IsBufferReady(int pos) { if(!Valid(pos)) return false; else return BufferReady[pos]; };
		//!> Set a buffer at a position to a ready state
		void SetBufferReady(int pos, bool st);
		//!> Set the buffersize at a position
		void SetBufferSize(int pos, int size);
		//!> Get the current position in the circle
		u16 Which() { return which; };

		//!> Get the next location
		inline u16 Next() { return (which+1 >= Size()) ? 0 : which+1; }
		inline u16 Prev() { if(Size() == 0) return 0; else return ((int)which-1 < 0) ? Size()-1 : which-1; }
	protected:
		//!> Check if the position is a valid position in the vector
		bool Valid(int pos) { return !(pos < 0 || pos >= Size()); };

		u16 which;
		u32 BufferBlockSize;
		std::vector<u8 *> SoundBuffer;
		std::vector<u32> BufferSize;
		std::vector<bool> BufferReady;
};

#endif
