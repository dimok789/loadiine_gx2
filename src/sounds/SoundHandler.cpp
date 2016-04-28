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
#include <unistd.h>
#include <malloc.h>
#include "common/common.h"
#include "dynamic_libs/ax_functions.h"
#include "fs/CFile.hpp"
#include "SoundHandler.hpp"
#include "WavDecoder.hpp"
#include "Mp3Decoder.hpp"
#include "OggDecoder.hpp"

SoundHandler * SoundHandler::handlerInstance = NULL;

SoundHandler::SoundHandler()
    : CThread(CThread::eAttributeAffCore1 | CThread::eAttributePinnedAff, 0, 0x8000)
{
	Decoding = false;
	ExitRequested = false;
	for(u32 i = 0; i < MAX_DECODERS; ++i)
    {
		DecoderList[i] = NULL;
        voiceList[i] = NULL;
    }

    resumeThread();

    //! wait for initialization
    while(!isThreadSuspended())
        usleep(1000);
}

SoundHandler::~SoundHandler()
{
	ExitRequested = true;
	ThreadSignal();

	ClearDecoderList();
}

void SoundHandler::AddDecoder(int voice, const char * filepath)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
		RemoveDecoder(voice);

	DecoderList[voice] = GetSoundDecoder(filepath);
}

void SoundHandler::AddDecoder(int voice, const u8 * snd, int len)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
		RemoveDecoder(voice);

	DecoderList[voice] = GetSoundDecoder(snd, len);
}

void SoundHandler::RemoveDecoder(int voice)
{
	if(voice < 0 || voice >= MAX_DECODERS)
		return;

	if(DecoderList[voice] != NULL)
    {
        if(voiceList[voice] && voiceList[voice]->getState() != Voice::STATE_STOPPED)
        {
            if(voiceList[voice]->getState() != Voice::STATE_STOP)
                voiceList[voice]->setState(Voice::STATE_STOP);

            while(voiceList[voice]->getState() != Voice::STATE_STOPPED)
                usleep(1000);
        }
        SoundDecoder *decoder = DecoderList[voice];
        decoder->Lock();
        DecoderList[voice] = NULL;
        decoder->Unlock();
		delete decoder;
    }
}

void SoundHandler::ClearDecoderList()
{
	for(u32 i = 0; i < MAX_DECODERS; ++i)
		RemoveDecoder(i);
}

static inline bool CheckMP3Signature(const u8 * buffer)
{
	const char MP3_Magic[][3] =
	{
		{'I', 'D', '3'},	//'ID3'
		{0xff, 0xfe},	   //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xff},	   //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xfa},	   //'MPEG ADTS, layer III, v1.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xfb},	   //'MPEG ADTS, layer III, v1.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf2},	   //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf3},	   //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf4},	   //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf5},	   //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xf6},	   //'MPEG ADTS, layer III, v2.0 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xf7},	   //'MPEG ADTS, layer III, v2.0', 'mp3', 'audio/mpeg'),
		{0xff, 0xe2},	   //'MPEG ADTS, layer III, v2.5 [protected]', 'mp3', 'audio/mpeg'),
		{0xff, 0xe3},	   //'MPEG ADTS, layer III, v2.5', 'mp3', 'audio/mpeg'),
	};

	if(buffer[0] == MP3_Magic[0][0] && buffer[1] == MP3_Magic[0][1] &&
	   buffer[2] == MP3_Magic[0][2])
	{
		return true;
	}

	for(int i = 1; i < 13; i++)
	{
		if(buffer[0] == MP3_Magic[i][0] && buffer[1] == MP3_Magic[i][1])
			return true;
	}

	return false;
}

SoundDecoder * SoundHandler::GetSoundDecoder(const char * filepath)
{
	u32 magic;
	CFile f(filepath, CFile::ReadOnly);
	if(f.size() == 0)
		return NULL;

	do
	{
		f.read((u8 *) &magic, 1);
	}
	while(((u8 *) &magic)[0] == 0 && f.tell() < f.size());

	if(f.tell() == f.size())
		return NULL;

	f.seek(f.tell()-1, SEEK_SET);
	f.read((u8 *) &magic, 4);
	f.close();

	if(magic == 0x4f676753) // 'OggS'
	{
	    return new OggDecoder(filepath);
	}
	else if(magic == 0x52494646) // 'RIFF'
	{
		return new WavDecoder(filepath);
	}
	else if(CheckMP3Signature((u8 *) &magic) == true)
	{
		return new Mp3Decoder(filepath);
	}

	return new SoundDecoder(filepath);
}

SoundDecoder * SoundHandler::GetSoundDecoder(const u8 * sound, int length)
{
	const u8 * check = sound;
	int counter = 0;

	while(check[0] == 0 && counter < length)
	{
		check++;
		counter++;
	}

	if(counter >= length)
		return NULL;

	u32 * magic = (u32 *) check;

	if(magic[0] == 0x4f676753) // 'OggS'
	{
	    return new OggDecoder(sound, length);
	}
	else if(magic[0] == 0x52494646) // 'RIFF'
	{
		return new WavDecoder(sound, length);
	}
	else if(CheckMP3Signature(check) == true)
	{
		return new Mp3Decoder(sound, length);
	}

	return new SoundDecoder(sound, length);
}

void SoundHandler::executeThread()
{
    // v2 sound lib can not properly end transition audio on old firmwares
    if (OS_FIRMWARE >= 400 && OS_FIRMWARE <= 410)
    {
        ProperlyEndTransitionAudio();
    }

    //! initialize 48 kHz renderer
    u32 params[3] = { 1, 0, 0 };

    if(AXInitWithParams != 0)
        AXInitWithParams(params);
    else
        AXInit();

    // The problem with last voice on 500 was caused by it having priority 0
    // We would need to change this priority distribution if for some reason
    // we would need MAX_DECODERS > Voice::PRIO_MAX
    for(u32 i = 0; i < MAX_DECODERS; ++i)
    {
        int priority = (MAX_DECODERS - i) * Voice::PRIO_MAX  / MAX_DECODERS;
        voiceList[i] = new Voice(priority); // allocate voice 0 with highest priority
    }

    AXRegisterFrameCallback((void*)&axFrameCallback);


	u16 i = 0;
	while (!ExitRequested)
	{
		suspendThread();

		for(i = 0; i < MAX_DECODERS; ++i)
		{
			if(DecoderList[i] == NULL)
				continue;

			Decoding = true;
			if(DecoderList[i])
                DecoderList[i]->Lock();
			if(DecoderList[i])
                DecoderList[i]->Decode();
			if(DecoderList[i])
                DecoderList[i]->Unlock();
		}
		Decoding = false;
	}

	for(u32 i = 0; i < MAX_DECODERS; ++i)
        voiceList[i]->stop();

    AXRegisterFrameCallback(NULL);
    AXQuit();

    for(u32 i = 0; i < MAX_DECODERS; ++i)
    {
        delete voiceList[i];
        voiceList[i] = NULL;
    }
}

void SoundHandler::axFrameCallback(void)
{
    for (u32 i = 0; i < MAX_DECODERS; i++)
    {
        Voice *voice = handlerInstance->getVoice(i);

        switch (voice->getState())
        {
            default:
            case Voice::STATE_STOPPED:
                break;

            case Voice::STATE_START: {
                SoundDecoder * decoder = handlerInstance->getDecoder(i);
                decoder->Lock();
                if(decoder->IsBufferReady())
                {
                    const u8 *buffer = decoder->GetBuffer();
                    const u32 bufferSize = decoder->GetBufferSize();
                    decoder->LoadNext();

                    const u8 *nextBuffer = NULL;
                    u32 nextBufferSize = 0;

                    if(decoder->IsBufferReady())
                    {
                        nextBuffer = decoder->GetBuffer();
                        nextBufferSize = decoder->GetBufferSize();
                        decoder->LoadNext();
                    }

                    voice->play(buffer, bufferSize, nextBuffer, nextBufferSize, decoder->GetFormat() & 0xff, decoder->GetSampleRate());

                    handlerInstance->ThreadSignal();

                    voice->setState(Voice::STATE_PLAYING);
                }
                decoder->Unlock();
                break;
            }
            case Voice::STATE_PLAYING:
                if(voice->getInternState() == 1)
                {
                    if(voice->isBufferSwitched())
                    {
                        SoundDecoder * decoder = handlerInstance->getDecoder(i);
                        decoder->Lock();
                        if(decoder->IsBufferReady())
                        {
                            voice->setNextBuffer(decoder->GetBuffer(), decoder->GetBufferSize());
                            decoder->LoadNext();
                            handlerInstance->ThreadSignal();
                        }
                        else if(decoder->IsEOF())
                        {
                            voice->setState(Voice::STATE_STOP);
                        }
                        decoder->Unlock();
                    }
                }
                else
                {
                    voice->setState(Voice::STATE_STOPPED);
                }
                break;
            case Voice::STATE_STOP:
                if(voice->getInternState() != 0)
                    voice->stop();
                voice->setState(Voice::STATE_STOPPED);
                break;
        }
    }
}
