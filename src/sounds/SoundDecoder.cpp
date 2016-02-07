/****************************************************************************
 * Copyright (C) 2009-2013 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <gctypes.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "dynamic_libs/os_functions.h"
#include "SoundDecoder.hpp"

static const u32 FixedPointShift = 15;
static const u32 FixedPointScale = 1 << FixedPointShift;

SoundDecoder::SoundDecoder()
{
	file_fd = NULL;
	Init();
}

SoundDecoder::SoundDecoder(const std::string & filepath)
{
	file_fd = new CFile(filepath, CFile::ReadOnly);
	Init();
}

SoundDecoder::SoundDecoder(const u8 * buffer, int size)
{
	file_fd = new CFile(buffer, size);
	Init();
}

SoundDecoder::~SoundDecoder()
{
	ExitRequested = true;
	while(Decoding)
		usleep(1000);

	//! lock unlock once to make sure it's really not decoding
	Lock();
	Unlock();

	if(file_fd)
		delete file_fd;
	file_fd = NULL;

	if(ResampleBuffer)
		free(ResampleBuffer);
}

void SoundDecoder::Init()
{
	SoundType = SOUND_RAW;
	SoundBlocks = 8;
	SoundBlockSize = 0x4000;
	ResampleTo48kHz = false;
	CurPos = 0;
	whichLoad = 0;
	Loop = false;
	EndOfFile = false;
	Decoding = false;
	ExitRequested = false;
	SoundBuffer.SetBufferBlockSize(SoundBlockSize);
	SoundBuffer.Resize(SoundBlocks);
	ResampleBuffer = NULL;
	ResampleRatio = 0;
}

int SoundDecoder::Rewind()
{
	CurPos = 0;
	EndOfFile = false;
	file_fd->rewind();

	return 0;
}

int SoundDecoder::Read(u8 * buffer, int buffer_size, int pos)
{
	int ret = file_fd->read(buffer, buffer_size);
	CurPos += ret;

	return ret;
}

void SoundDecoder::EnableUpsample(void)
{
	if(   (ResampleBuffer == NULL)
	   && IsStereo() && Is16Bit()
	   && SampleRate != 32000
	   && SampleRate != 48000)
	{
		ResampleBuffer = (u8*)memalign(32, SoundBlockSize);
		ResampleRatio =  ( FixedPointScale * SampleRate ) / 48000;
		SoundBlockSize = ( SoundBlockSize * ResampleRatio ) / FixedPointScale;
		SoundBlockSize &= ~0x03;
		// set new sample rate
		SampleRate = 48000;
	}
}

void SoundDecoder::Upsample(s16 *src, s16 *dst, u32 nr_src_samples, u32 nr_dst_samples)
{
	int timer = 0;

	for(u32 i = 0, n = 0; i < nr_dst_samples; i += 2)
	{
		if((n+3) < nr_src_samples) {
			// simple fixed point linear interpolation
			dst[i]   = src[n] +   ( ((src[n+2] - src[n]  ) * timer) >> FixedPointShift );
			dst[i+1] = src[n+1] + ( ((src[n+3] - src[n+1]) * timer) >> FixedPointShift );
		}
		else {
			dst[i]   = src[n];
			dst[i+1] = src[n+1];
		}

		timer += ResampleRatio;

		if(timer >= (int)FixedPointScale) {
			n += 2;
			timer -= FixedPointScale;
		}
	}
}

void SoundDecoder::Decode()
{
	if(!file_fd || ExitRequested || EndOfFile)
		return;

	// check if we are not at the pre-last buffer (last buffer is playing)
	u16 whichPlaying = SoundBuffer.Which();
	if(	   ((whichPlaying == 0) && (whichLoad == SoundBuffer.Size()-2))
		|| ((whichPlaying == 1) && (whichLoad == SoundBuffer.Size()-1))
        || (whichLoad == (whichPlaying-2)))
	{
		return;
	}

	Decoding = true;

	int done  = 0;
	u8 * write_buf = SoundBuffer.GetBuffer(whichLoad);
	if(!write_buf)
	{
		ExitRequested = true;
		Decoding = false;
		return;
	}

	if(ResampleTo48kHz && !ResampleBuffer)
		EnableUpsample();

	while(done < SoundBlockSize)
	{
		int ret = Read(&write_buf[done], SoundBlockSize-done, Tell());

		if(ret <= 0)
		{
			if(Loop)
			{
				Rewind();
				continue;
			}
			else
			{
				EndOfFile = true;
				break;
			}
		}

		done += ret;
	}

	if(done > 0)
	{
		// check if we need to resample
		if(ResampleBuffer && ResampleRatio)
		{
			memcpy(ResampleBuffer, write_buf, done);

			int src_samples = done >> 1;
			int dest_samples = ( src_samples * FixedPointScale ) / ResampleRatio;
			dest_samples &= ~0x01;
			Upsample((s16*)ResampleBuffer, (s16*)write_buf, src_samples, dest_samples);
			done = dest_samples << 1;
		}

		//! TODO: remove this later and add STEREO support with two voices, for now we convert to MONO
		if(IsStereo())
		{
            s16* monoBuf = (s16*)write_buf;
			done = done >> 1;

            for(int i = 0; i < done; i++)
                monoBuf[i] = monoBuf[i << 1];
		}

        DCFlushRange(write_buf, done);
		SoundBuffer.SetBufferSize(whichLoad, done);
		SoundBuffer.SetBufferReady(whichLoad, true);
		if(++whichLoad >= SoundBuffer.Size())
			whichLoad = 0;
	}

	// check if next in queue needs to be filled as well and do so
	if(!SoundBuffer.IsBufferReady(whichLoad))
		Decode();

	Decoding = false;
}

