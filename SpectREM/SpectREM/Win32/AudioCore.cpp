//
//  AudioCore.cpp
//  ZXRetroEmulator
//
//  Created by Adrian Brown on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#include "AudioCore.hpp"
#include "xaudio2.h"
#include <stdio.h>

//-----------------------------------------------------------------------------------------

AudioCore::AudioCore()
{
	m_pXAudio2 = NULL;
}

//-----------------------------------------------------------------------------------------

AudioCore::~AudioCore()
{
	Deinit();
}

//-----------------------------------------------------------------------------------------

void AudioCore::Deinit()
{
	if (m_pXAudio2 != NULL)
	{
		m_pXAudio2->Release();
		m_pXAudio2 = NULL;
	}
}

//-----------------------------------------------------------------------------------------

bool AudioCore::Init(uint32_t sampleRate, float fps, AUDIOCORE_Callback callback)
{
	// Remember the callback
	m_pCallback = callback;

	if (FAILED(XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		return false;
	}

	if (FAILED(m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice)))
	{
		m_pXAudio2->Release();
		return false;
	}

	// Setup the wave format structure
	WAVEFORMATEX wavFmt;
	wavFmt.wFormatTag = WAVE_FORMAT_PCM;
	wavFmt.nChannels = 2;
	wavFmt.nSamplesPerSec = sampleRate;
	wavFmt.wBitsPerSample = 16;
	wavFmt.nAvgBytesPerSec = (wavFmt.nSamplesPerSec * wavFmt.nChannels * wavFmt.wBitsPerSample) >> 3;
	wavFmt.nBlockAlign = (wavFmt.nChannels * wavFmt.wBitsPerSample) >> 3;
	wavFmt.cbSize = sizeof(wavFmt);

	if (FAILED(m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &wavFmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this)))
	{
		m_pXAudio2->Release();
		m_pXAudio2 = NULL;

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------
unsigned char *pBuffer;
unsigned int buffer_idx = 0;
unsigned int bytes_per_frame;
void AudioCore::Start()
{
	pBuffer = new unsigned char[((44100 * 2 * 2) / 50) * 2];
	bytes_per_frame = (44100 * 2 * 2) / 50;

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = bytes_per_frame;
	buf.pAudioData = (BYTE *)pBuffer;

	m_pSourceVoice->SubmitSourceBuffer(&buf);
	buffer_idx = 1 - buffer_idx;

	m_pSourceVoice->Start();
}

//-----------------------------------------------------------------------------------------

void AudioCore::OnStreamEnd()
{
}

//-----------------------------------------------------------------------------------------

void AudioCore::OnVoiceProcessingPassEnd()
{
}

//-----------------------------------------------------------------------------------------

void AudioCore::OnVoiceProcessingPassStart(UINT32 SamplesRequired)
{

}

//-----------------------------------------------------------------------------------------

void AudioCore::OnBufferEnd(void * pBufferContext)
{
	m_pCallback((44100 * 2) / 50, &pBuffer[buffer_idx * bytes_per_frame]);

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = bytes_per_frame;
	buf.pAudioData = (BYTE *)&pBuffer[buffer_idx * bytes_per_frame];

	m_pSourceVoice->SubmitSourceBuffer(&buf);
	buffer_idx = 1 - buffer_idx;
}

//-----------------------------------------------------------------------------------------

void AudioCore::OnBufferStart(void * pBufferContext)
{

}

//-----------------------------------------------------------------------------------------

void AudioCore::OnLoopEnd(void * pBufferContext)
{
}

//-----------------------------------------------------------------------------------------

void AudioCore::OnVoiceError(void * pBufferContext, HRESULT Error)
{
}

//-----------------------------------------------------------------------------------------
