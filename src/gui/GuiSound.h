/****************************************************************************
 * Copyright (C) 2015 Dimok
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
#ifndef GUI_SOUND_H_
#define GUI_SOUND_H_

#include <gctypes.h>
#include "system/AsyncDeleter.h"

//!Sound conversion and playback. A wrapper for other sound libraries - ASND, libmad, ltremor, etc
class GuiSound : public AsyncDeleter::Element
{
	public:
		//!Constructor
		//!\param sound Pointer to the sound data
		//!\param filesize Length of sound data
		GuiSound(const char * filepath);
		GuiSound(const u8 * sound, int length);
		//!Destructor
		virtual ~GuiSound();
		//!Load a file and replace the old one
		bool Load(const char * filepath);
		//!Load a file and replace the old one
		bool Load(const u8 * snd, s32 len);
		//!Start sound playback
		void Play();
		//!Stop sound playback
		void Stop();
		//!Pause sound playback
		void Pause();
		//!Resume sound playback
		void Resume();
		//!Checks if the sound is currently playing
		//!\return true if sound is playing, false otherwise
		bool IsPlaying();
		//!Rewind the music
		void Rewind();
		//!Set sound volume
		//!\param v Sound volume (0-100)
		void SetVolume(u32 v);
		//!\param l Loop (true to loop)
		void SetLoop(bool l);
	protected:
		s32 voice; //!< Currently assigned ASND voice channel
};

#endif
