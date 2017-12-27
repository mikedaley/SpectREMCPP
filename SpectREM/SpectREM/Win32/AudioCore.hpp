//
//  AudioCore.hpp
//  ZXRetroEmulator
//
//  Created by Adrian Brown on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#pragma once

//-----------------------------------------------------------------------------------------

#include <stdint.h>
#include "xaudio2.h"

//-----------------------------------------------------------------------------------------

typedef void (*AUDIOCORE_Callback)(uint32_t nNumSamples, uint8_t *pBuffer);

//-----------------------------------------------------------------------------------------

class AudioCore : public IXAudio2VoiceCallback
{
public:
	AudioCore();
	~AudioCore();

public:
	void						Deinit();
	bool						Init(uint32_t sampleRate, float fps, AUDIOCORE_Callback callback);
	void						Start();

	void STDMETHODCALLTYPE		OnStreamEnd();
	void STDMETHODCALLTYPE		OnVoiceProcessingPassEnd();
	void STDMETHODCALLTYPE		OnVoiceProcessingPassStart(UINT32 SamplesRequired);
	void STDMETHODCALLTYPE		OnBufferEnd(void * pBufferContext);
	void STDMETHODCALLTYPE		OnBufferStart(void * pBufferContext);
	void STDMETHODCALLTYPE		OnLoopEnd(void * pBufferContext);
	void STDMETHODCALLTYPE		OnVoiceError(void * pBufferContext, HRESULT Error);

private:
	IXAudio2				*	m_pXAudio2;
	IXAudio2MasteringVoice	*	m_pMasteringVoice;
	IXAudio2SourceVoice		*	m_pSourceVoice;
	AUDIOCORE_Callback			m_pCallback;
};

//-----------------------------------------------------------------------------------------
