//
//  Audio.cpp
//  SpectREM
//
//  Created by Michael Daley on 02/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#define kExponent 18
#define kUsed ((audioQueueBufferWritten - audioQueueBufferRead) & (audioQueueBufferCapacity - 1))
#define kSpace (audioQueueBufferCapacity - 1 - kUsed)
#define kSize (audioQueueBufferCapacity - 1)

static float fAYVolBase[] = {
    0.0000,
    0.0079,
    0.0141,
    0.0202,
    0.0299,
    0.0404,
    0.0580,
    0.0773,
    0.1107,
    0.1485,
    0.2109,
    0.2812,
    0.4007,
    0.5351,
    0.7583,
    1.0000
};

void ZXSpectrum::buildaudioAYVolumesTable()
{
    for (int i = 0; i < 16; i++)
    {
        audioAYVolumes[i] = (unsigned short)(fAYVolBase[i] * 10240);
    }
}

void ZXSpectrum::audioSetup(float sampleRate, float fps)
{
    audioBufferSize = (sampleRate / fps) * 6;
    audioBuffer = new short[ audioBufferSize ];
    
    audioBeeperTsStep = machineInfo.tsPerFrame / (sampleRate / fps);
    audioAYTsStep = 32;
    
    audioQueueBufferCapacity = 1 << kExponent;
    audioQueueBuffer = new short[ audioQueueBufferCapacity << 2 ];
}

void ZXSpectrum::audioReset()
{
    audioBufferIndex = 0;
    audioTsCounter = 0;
    audioTsStepCounter = 0;
    audioBeeperLeft = 0;
    audioBeeperRight = 0;
    memset(audioBuffer, 0, audioBufferSize);

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
    
    for (int i = 0; i < eAY_MAX_REGISTERS; i++)
    {
        audioAYSetRegister(i);
        audioAYWriteData(0);
    }
}

#pragma mark - Audio Update

void ZXSpectrum::audioUpdateWithTs(int tStates)
{
    if (paused)
    {
        return;
    }

    // Grab the current state of the audio ear output & the tapeLevel which is used to register input when loading tapes.
    // Only need to do this once per audio update
    int localBeeperLevel = audioEarBit * 512;
    int beeperLevelLeft = localBeeperLevel;
    int beeperLevelRight = localBeeperLevel;
    
    // Loop over each tState so that the necessary audio samples can be generated
    for(int i = 0; i < tStates; i++)
    {
        if (audioAYTs++ >= audioAYTsStep)
        {
            audioAYUpdate(1);
            
            beeperLevelLeft += audioAYChannelOutput[0];
            beeperLevelLeft += audioAYChannelOutput[1];
            beeperLevelLeft += audioAYChannelOutput[2];
            beeperLevelRight += audioAYChannelOutput[0];
            beeperLevelRight += audioAYChannelOutput[1];
            beeperLevelRight += audioAYChannelOutput[2];
            
            audioAYChannelOutput[0] = 0;
            audioAYChannelOutput[1] = 0;
            audioAYChannelOutput[2] = 0;
            audioAYTs -= audioAYTsStep;
        }
        
        // If we have done more cycles now than the audio step counter, generate a new sample
        if (audioTsCounter++ >= audioTsStepCounter)
        {
            // Quantize the value loaded into the audio buffer e.g. if cycles = 19 and step size is 18.2
            // 0.2 of the beeper value goes into this sample and 0.8 goes into the next sample
            double delta1 = audioTsStepCounter - (audioTsCounter - 1);
            double delta2 = (1 - delta1);
            
            // Quantize for the current sample
            audioBeeperLeft += beeperLevelLeft * delta1;
            audioBeeperRight += beeperLevelRight * delta1;
            
            // Load the buffer with the sample for both left and right channels
            audioBuffer[ audioBufferIndex++ ] = (short)audioBeeperLeft;
            audioBuffer[ audioBufferIndex++ ] = (short)audioBeeperRight;
            
            // Quantize for the next sample
            audioBeeperLeft = beeperLevelLeft * delta2;
            audioBeeperRight = beeperLevelRight * delta2;
            
            // Increment the step counter so that the next sample will be taken after another 18.2 T-States
            audioTsStepCounter += audioBeeperTsStep;
        }
        else
        {
            audioBeeperLeft += beeperLevelLeft;
            audioBeeperRight += beeperLevelRight;
        }
        
        beeperLevelLeft = beeperLevelRight = localBeeperLevel;
    }
}

#pragma mark - AY Chip

void ZXSpectrum::audioAYSetRegister(unsigned char reg)
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

void ZXSpectrum::audioAYWriteData(unsigned char data)
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

unsigned char ZXSpectrum::audioAYReadData()
{
    return audioAYRegisters[ audioAYCurrentRegister ];
}

void ZXSpectrum::audioAYUpdate(int audioSteps)
{
    if (!audioAYaudioAYaudioAYEnvelopeHolding)
    {
        audioATaudioAYEnvelopeCount++;
        
        if ( audioATaudioAYEnvelopeCount >= (audioAYRegisters[ eAYREGISTER_E_FINE ] | (audioAYRegisters[ eAYREGISTER_E_COARSE] << 8)))
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
        
        int freq = audioAYRegisters[ eAYREGISTER_NOISEPER ];
        
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
    int freq = audioAYRegisters[ (0 << 1) + eAYREGISTER_A_FINE ] | (audioAYRegisters[ (0 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audioAYChannelCount[0] >= freq)
    {
        audioAYChannelCount[0]  -= freq;
        audioAYOutput ^= 1;
    }
    
    unsigned int tone_output = ((audioAYOutput >> 0) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> 0) & 1);
    unsigned int noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[eAYREGISTER_ENABLE] >> (0 + 3)) & 1);
    
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

#pragma mark - Audio Queue

// Write the supplied number of bytes into the queues buffer from the supplied buffer pointer
int ZXSpectrum::audioQueueWrite(signed short *buffer, int count)
{
    if (!count) {
        return 0;
    }
    
    int t;
    int i;
    
    t = kSpace;
    
    if (count > t)
    {
        count = t;
    } else {
        t = count;
    }
    
    i = audioQueueBufferWritten;
    
    if ((i + count) > audioQueueBufferCapacity)
    {
        memcpy(audioQueueBuffer + i, buffer, (audioQueueBufferCapacity - i) << 1);
        buffer += audioQueueBufferCapacity - i;
        count -= audioQueueBufferCapacity - i;
        i = 0;
    }
    
    memcpy(audioQueueBuffer + i, buffer, count << 1);
    audioQueueBufferWritten = i + count;
    
    return t;
 
}

// Read the supplied number of bytes from the queues buffer into the supplied buffer pointer
int ZXSpectrum::audioQueueRead(signed short *buffer, int count)
{
    int t;
    int i;
    
    t = kUsed;
    
    if (count > t)
    {
        count = t;
    } else {
        t = count;
    }
    
    i = audioQueueBufferRead;
    
    if ((i + count) > audioQueueBufferCapacity)
    {
        memcpy(buffer, audioQueueBuffer + i, (audioQueueBufferCapacity - i) << 1);
        buffer += audioQueueBufferCapacity - i;
        count -= audioQueueBufferCapacity - i;
        i = 0;
    }
    
    memcpy(buffer, audioQueueBuffer + i, count << 1);
    audioQueueBufferRead = i + count;
    
    return t;
}

// Return the number of used samples in the buffer
int ZXSpectrum::audioQueueBufferUsed()
{
    return kUsed;
}


