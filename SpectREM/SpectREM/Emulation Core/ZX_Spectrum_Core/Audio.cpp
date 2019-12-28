//
//  Audio.cpp
//  SpectREM
//
//  Created by Michael Daley on 02/09/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"
#include <math.h>
#include <iomanip>

const float cBEEPER_VOLUME_MULTIPLIER = 8192;

// AY chip envelope flag type
enum
{
    eENVFLAG_HOLD = 0x01,
    eENVFLAG_ALTERNATE = 0x02,
    eENVFLAG_ATTACK = 0x04,
    eENVFLAG_CONTINUE = 0x08
};

static const double fAYVolBase[] = {
    0.0000, 0.0137, 0.0205, 0.0291, 0.0423, 0.0618, 0.0847, 0.1369,
    0.1691, 0.2647, 0.3527, 0.4499, 0.5704, 0.6873, 0.8482, 1.0000
};

void ZXSpectrum::audioBuildAYVolumesTable()
{
    for (int i = 0; i < 16; i++)
    {
        audioAYVolumes[ i ] = static_cast<uint16_t>(fAYVolBase[ i ] * 8192);
    }
}

void ZXSpectrum::audioSetup(double sampleRate, double fps)
{
    audioBufferSize = static_cast<uint32_t>((sampleRate / fps) * 4.0);
    audioBuffer = new int16_t[ audioBufferSize ]();
    audioBeeperTsStep = machineInfo.tsPerFrame / (sampleRate / fps);
    audioAYTsStep = 32;
}

void ZXSpectrum::audioReset()
{
    if (audioBuffer)
    {
        delete audioBuffer;
    }
    
    audioBuffer = new int16_t[ audioBufferSize ]();
    audioBufferIndex = 0;
    audioTsCounter = 0;
    audioTsStepCounter = 0;
	audioOutputLevelLeft = 0;
	audioOutputLevelRight = 0;
	audioAYLevelLeft = 0;
	audioAYLevelRight = 0;
    audioAYOutput = 0;
    audioAYrandom = 1;
    audioAYChannelOutput[0] = 0;
    audioAYChannelOutput[1] = 0;
    audioAYChannelOutput[2] = 0;
    audioAYChannelCount[0] = 0;
    audioAYChannelCount[1] = 0;
    audioAYChannelCount[2] = 0;
    audioAYNoiseCount = 0;
    audioATaudioAYEnvelopeCount = 0;
    audioAYaudioAYEnvelopeStep = 15;
    audioAYaudioAYaudioAYEnvelopeHolding = false;
    specdrumDACValue = 0;
    
    for (uint8_t i = 0; i < eAY_MAX_REGISTERS; i++)
    {
        audioAYSetRegister(i);
        audioAYWriteData(0);
    }
}

// - Generate audio output from Beeper and AY chip

void ZXSpectrum::audioUpdateWithTs(uint32_t tStates)
{
    if (emuPaused)
    {
        return;
    }
    
    // Grab the current state of the audio ear output & the tapeLevel which is used to register input when loading tapes.
    // Only need to do this once per audio update
    float audioEarLevel = (audioEarBit | tape->inputBit) ? cBEEPER_VOLUME_MULTIPLIER : 0;
    
    // Add in any output from SpecDRUM if it's being used
    if (emuUseSpecDRUM)
    {
        audioEarLevel += specdrumDACValue;
    }
    
    // Loop over each tState so that the necessary audio samples can be generated
    for(int32_t i = 0; i < tStates; i++)
    {
        // If we have done more cycles now than the audio step counter, generate a new sample
        audioTsCounter += 1.0f;
		audioOutputLevelLeft += audioEarLevel;
		audioOutputLevelRight += audioEarLevel;
		audioOutputLevelLeft += audioAYLevelLeft;
		audioOutputLevelRight += audioAYLevelRight;

        // Loop over each tState so that the necessary audio samples can be generated
       if (emuUseAYSound)
       {
           audioAYTs += 1.0f;
           if (audioAYTs >= audioAYTsStep)
           {
               audioAYUpdate();

			   audioAYLevelLeft = audioAYChannelOutput[0];      // A - Left
			   audioAYLevelLeft += audioAYChannelOutput[1];     // B - Left
               audioAYLevelLeft += audioAYChannelOutput[2];     // C - Left

               audioAYLevelRight = audioAYChannelOutput[0];     // A - Right
			   audioAYLevelRight += audioAYChannelOutput[1];    // B - Right
               audioAYLevelRight += audioAYChannelOutput[2];    // C - Right

               audioAYChannelOutput[0] = 0;
               audioAYChannelOutput[1] = 0;
               audioAYChannelOutput[2] = 0;
               
               audioAYTs -= audioAYTsStep;
           }
       }
        
        if (audioTsCounter >= audioBeeperTsStep)
        {
            // Scale down
			audioOutputLevelLeft /= audioTsCounter;
			audioOutputLevelRight /= audioTsCounter;
            
            // Load the buffer with the sample for both left and right channels
            audioBuffer[ audioBufferIndex++ ] = static_cast< int16_t >(audioOutputLevelLeft);
            audioBuffer[ audioBufferIndex++ ] = static_cast< int16_t >(audioOutputLevelRight);

            audioTsCounter -= static_cast<float>(audioBeeperTsStep); // should this not be a float already?
			audioOutputLevelLeft = (audioEarLevel + audioAYLevelLeft) * audioTsCounter;
			audioOutputLevelRight = (audioEarLevel + audioAYLevelRight) * audioTsCounter;
		}
    }
}

// - AY Chip

void ZXSpectrum::audioAYSetRegister(uint8_t reg)
{
    if (reg < eAY_MAX_REGISTERS)
    {
        audioAYCurrentRegister = reg;
    }
    else
    {
        // If an AY register > 16 is selected then point it at the floating register used to
        // emulate this behaviour
        audioAYCurrentRegister = eAYREGISTER_FLOATING;
    }
}

void ZXSpectrum::audioAYWriteData(uint8_t data)
{
    switch (audioAYCurrentRegister) {
        case eAYREGISTER_A_FINE:
        case eAYREGISTER_B_FINE:
        case eAYREGISTER_C_FINE:
        case eAYREGISTER_ENABLE:
        case eAYREGISTER_E_FINE:
        case eAYREGISTER_E_COARSE:
        case eAYREGISTER_PORT_A:
        case eAYREGISTER_PORT_B:
            break;
            
        case eAYREGISTER_A_COARSE:
        case eAYREGISTER_B_COARSE:
        case eAYREGISTER_C_COARSE:
            data &= 0x0f;
            break;
            
        case eAYREGISTER_E_SHAPE:
            audioAYaudioAYaudioAYEnvelopeHolding = false;
            audioAYaudioAYEnvelopeStep = 15;
            data &= 0x0f;
            
            audioAYAttackEndVol = (data & eENVFLAG_ATTACK) != 0 ? 15 : 0;
            
            if ((data & eENVFLAG_CONTINUE) == 0)
            {
                audioAYaudioAYEnvelopeHold = true;
                audioAYaudioAYEnvelopeAlt = (data & eENVFLAG_ATTACK) ? false: true;
            }
            else
            {
                audioAYaudioAYEnvelopeHold = (data & eENVFLAG_HOLD) ? true : false;
                audioAYaudioAYEnvelopeAlt = (data & eENVFLAG_ALTERNATE) ? true : false;
            }
            break;
            
        case eAYREGISTER_NOISEPER:
        case eAYREGISTER_A_VOL:
        case eAYREGISTER_B_VOL:
        case eAYREGISTER_C_VOL:
            data &= 0xff;
            break;
            
        case eAYREGISTER_FLOATING:
            break;
            
        default:
            break;
    }
    
    audioAYRegisters[ audioAYCurrentRegister ] = data;
}

void ZXSpectrum::audioDecayAYFloatingRegister()
{
    // Decay the AY registers result returned for registers above 15
    audioAYRegisters[ eAYREGISTER_FLOATING ] >>= 1;
}

uint8_t ZXSpectrum::audioAYReadData()
{
    return audioAYRegisters[ audioAYCurrentRegister ];
}

void ZXSpectrum::audioAYUpdate()
{
    
    if (!audioAYaudioAYaudioAYEnvelopeHolding)
    {
        audioATaudioAYEnvelopeCount++;
        
        if ( audioATaudioAYEnvelopeCount >= static_cast<uint32_t>(audioAYRegisters[ eAYREGISTER_E_FINE ] | (audioAYRegisters[ eAYREGISTER_E_COARSE] << 8)))
        {
            audioATaudioAYEnvelopeCount = 0;
            audioAYaudioAYEnvelopeStep--;
            
            if (audioAYaudioAYEnvelopeStep < 0)
            {
                audioAYaudioAYEnvelopeStep = 15;
                
                if ( audioAYaudioAYEnvelopeAlt )
                {
                    audioAYAttackEndVol ^= 15;
                }
                
                if (audioAYaudioAYEnvelopeHold)
                {
                    audioAYaudioAYaudioAYEnvelopeHolding = true;
                }
            }
        }
    }
    
    if ((audioAYRegisters[eAYREGISTER_ENABLE] & 0x38) != 0x38)
    {
        audioAYNoiseCount++;
        
        uint32_t freq = audioAYRegisters[ eAYREGISTER_NOISEPER ];
        
        // 0 is assumed to be 1
        if (freq == 0)
        {
            freq = 1;
        }
        
        if (audioAYNoiseCount >= freq)
        {
            audioAYNoiseCount = 0;
            
            if (((audioAYrandom & 1) ^ ((audioAYrandom >> 1) & 1)) == 1)
            {
                audioAYOutput ^= (1 << 3);
            }
            
            audioAYrandom = (((audioAYrandom & 1) ^ ((audioAYrandom >> 3) & 1)) << 16) | ((audioAYrandom >> 1) & 0x1ffff);
        }
    }
    
    // Channel 0
    audioAYChannelCount[0] += 2;
    
    // Noise frequency
    uint32_t freq = audioAYRegisters[ (0 << 1) + eAYREGISTER_A_FINE ] | (audioAYRegisters[ (0 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audioAYChannelCount[0] >= freq)
    {
        audioAYChannelCount[0]  -= freq;
        audioAYOutput ^= 1;
    }
    
    uint32_t tone_output = ((audioAYOutput >> 0) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> 0) & 1);
    uint32_t noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> (0 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = audioAYRegisters[eAYREGISTER_A_VOL + 0];
        
        if ((vol & 0x10) != 0)
        {
            vol = audioAYaudioAYEnvelopeStep ^ audioAYAttackEndVol;
        }

        audioAYChannelOutput[0] += audioAYVolumes[vol];
    }
    
    // Channel 1
    audioAYChannelCount[1] += 2;
    
    // Noise frequency
    freq = audioAYRegisters[ (1 << 1) + eAYREGISTER_A_FINE ] | (audioAYRegisters[ (1 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audioAYChannelCount[1] >= freq)
    {
        audioAYChannelCount[1]  -= freq;
        audioAYOutput ^= 2;
    }
    
    tone_output = ((audioAYOutput >> 1) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> 1) & 1);
    noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> (1 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = audioAYRegisters[eAYREGISTER_A_VOL + 1];
        
        if ((vol & 0x10) != 0)
        {
            vol = audioAYaudioAYEnvelopeStep ^ audioAYAttackEndVol;
        }
        
        audioAYChannelOutput[1] += audioAYVolumes[vol];
    }
    
    // Channel 2
    audioAYChannelCount[2] += 2;
    
    // Noise frequency
    freq = audioAYRegisters[ (2 << 1) + eAYREGISTER_A_FINE ] | (audioAYRegisters[ (2 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audioAYChannelCount[2] >= freq)
    {
        audioAYChannelCount[2]  -= freq;
        audioAYOutput ^= 4;
    }
    
    tone_output = ((audioAYOutput >> 2) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> 2) & 1);
    noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> (2 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = audioAYRegisters[eAYREGISTER_A_VOL + 2];
        
        if ((vol & 0x10) != 0)
        {
            vol = audioAYaudioAYEnvelopeStep ^ audioAYAttackEndVol;
        }
        
        audioAYChannelOutput[2] += audioAYVolumes[vol];
    }
    
}



