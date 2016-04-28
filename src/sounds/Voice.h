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
#ifndef _AXSOUND_H_
#define _AXSOUND_H_

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/ax_functions.h"

class Voice
{
public:

    enum VoicePriorities
    {
        PRIO_MIN = 1,
        PRIO_MAX = 31
    };

    enum VoiceStates
    {
        STATE_STOPPED,
        STATE_START,
        STATE_PLAYING,
        STATE_STOP,
    };

    Voice(int prio)
        : state(STATE_STOPPED)
    {
        lastLoopCounter = 0;
        nextBufferSize = 0;

        voice = AXAcquireVoice(prio, 0, 0);
        if(voice)
        {
            AXVoiceBegin(voice);
            AXSetVoiceType(voice, 0);
            setVolume(0x80000000);

            u32 mix[24];
            memset(mix, 0, sizeof(mix));
            mix[0] = 0x80000000;
            mix[4] = 0x80000000;

            AXSetVoiceDeviceMix(voice, 0, 0, mix);
            AXSetVoiceDeviceMix(voice, 1, 0, mix);

            AXVoiceEnd(voice);
        }
    }

    ~Voice()
    {
        if(voice)
        {
            AXFreeVoice(voice);
        }
    }

    void play(const u8 *buffer, u32 bufferSize, const u8 *nextBuffer, u32 nextBufSize, u16 format, u32 sampleRate)
    {
        if(!voice)
            return;

        memset(&voiceBuffer, 0, sizeof(voiceBuffer));

        voiceBuffer.samples = buffer;
        voiceBuffer.format = format;
        voiceBuffer.loop = (nextBuffer == NULL) ? 0 : 1;
        voiceBuffer.cur_pos = 0;
        voiceBuffer.end_pos = bufferSize >> 1;
        voiceBuffer.loop_offset = ((nextBuffer - buffer) >> 1);
        nextBufferSize = nextBufSize;

        u32 samplesPerSec = (AXGetInputSamplesPerSec != 0) ? AXGetInputSamplesPerSec() : 32000;

        ratioBits[0] = (u32)(0x00010000 * ((f32)sampleRate / (f32)samplesPerSec));
        ratioBits[1] = 0;
        ratioBits[2] = 0;
        ratioBits[3] = 0;

        AXSetVoiceOffsets(voice, &voiceBuffer);
        AXSetVoiceSrc(voice, ratioBits);
        AXSetVoiceSrcType(voice, 1);
        AXSetVoiceState(voice, 1);
    }

    void stop()
    {
        if(voice)
            AXSetVoiceState(voice, 0);
    }

    void setVolume(u32 vol)
    {
        if(voice)
            AXSetVoiceVe(voice, &vol);
    }


    void setNextBuffer(const u8 *buffer, u32 bufferSize)
    {
        voiceBuffer.loop_offset = ((buffer - voiceBuffer.samples) >> 1);
        nextBufferSize = bufferSize;

        AXSetVoiceLoopOffset(voice, voiceBuffer.loop_offset);
    }

    bool isBufferSwitched()
    {
        u32 loopCounter = AXGetVoiceLoopCount(voice);
        if(lastLoopCounter != loopCounter)
        {
            lastLoopCounter = loopCounter;
            AXSetVoiceEndOffset(voice, voiceBuffer.loop_offset  + (nextBufferSize >> 1));
            return true;
        }
        return false;
    }

    u32 getInternState() const {
        if(voice)
            return ((u32 *)voice)[1];
        return 0;
    }
    u32 getState() const {
        return state;
    }
    void setState(u32 s) {
        state = s;
    }

    void * getVoice() const {
        return voice;
    }

private:
    void *voice;
    u32 ratioBits[4];

    typedef struct _ax_buffer_t {
        u16 format;
        u16 loop;
        u32 loop_offset;
        u32 end_pos;
        u32 cur_pos;
        const unsigned char *samples;
    } ax_buffer_t;

    ax_buffer_t voiceBuffer;
    u32 state;
    u32 nextBufferSize;
    u32 lastLoopCounter;
};

#endif // _AXSOUND_H_
