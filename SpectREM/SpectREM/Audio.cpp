//
//  Audio.cpp
//  SpectREM
//
//  Created by Michael Daley on 02/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

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

void ZXSpectrum::buildAYVolumesTable()
{
    for (int i = 0; i < 16; i++)
    {
        AYVolumes[i] = (unsigned short)(fAYVolBase[i] * 16384);
    }
}

#pragma mark - AY Chip

void ZXSpectrum::setAYRegister(unsigned char reg)
{
    if (reg < eAY_MAX_REGISTERS)
    {
        currentAYRegister = reg;
    }
    else
    {
        // If an AY register > 16 is selected then point it at the floating register used to
        // emulate this behaviour
        currentAYRegister = eAYREGISTER_FLOATING;
    }
}

void ZXSpectrum::writeAYData(unsigned char data)
{
    switch (currentAYRegister) {
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
            envelopeHolding = false;
            envelopeStep = 15;
            data &= 0x0f;
            
            attackEndVol = (data & eENVFLAG_ATTACK) != 0 ? 15 : 0;
            
            if ((data & eENVFLAG_CONTINUE) == 0)
            {
                envelopeHold = true;
                envelopeAlt = (data & eENVFLAG_ATTACK) ? false: true;
            }
            else
            {
                envelopeHold = (data & eENVFLAG_HOLD) ? true : false;
                envelopeAlt = (data & eENVFLAG_ALTERNATE) ? true : false;
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
    
    AYRegisters[ currentAYRegister ] = data;
}

unsigned char ZXSpectrum::readAYData()
{
    return AYRegisters[ currentAYRegister ];
}

void ZXSpectrum::updateAY(int audioSteps)
{
    if (!envelopeHolding)
    {
        envelopeCount++;
        
        if ( envelopeCount >= (AYRegisters[ eAYREGISTER_E_FINE ] | (AYRegisters[ eAYREGISTER_E_COARSE] << 8)))
        {
            envelopeCount = 0;
            envelopeStep--;
            
            if (envelopeStep < 0)
            {
                envelopeStep = 15;
                
                if ( envelopeAlt )
                {
                    attackEndVol ^= 15;
                }
                
                if (envelopeHold)
                {
                    envelopeHolding = true;
                }
            }
        }
    }
    
    if ((AYRegisters[eAYREGISTER_ENABLE] & 0x38) != 0x38)
    {
        noiseCount++;
        
        int freq = AYRegisters[ eAYREGISTER_NOISEPER ];
        
        // 0 is assumed to be 1
        if (freq == 0)
        {
            freq = 1;
        }
        
        if (noiseCount >= freq)
        {
            noiseCount = 0;
            
            if (((random & 1) ^ ((random >> 1) & 1)) == 1)
            {
                AYOutput ^= (1 << 3);
            }
            
            random = (((random & 1) ^ ((random >> 3) & 1)) << 16) | ((random >> 1) & 0x1ffff);
        }
    }
    
    // Channel 0
    AYChannelCount[0] += 2;
    
    // Noise frequency
    int freq = AYRegisters[ (0 << 1) + eAYREGISTER_A_FINE ] | (AYRegisters[ (0 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (AYChannelCount[0] >= freq)
    {
        AYChannelCount[0]  -= freq;
        AYOutput ^= 1;
    }
    
    unsigned int tone_output = ((AYOutput >> 0) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> 0) & 1);
    unsigned int noise_output = ((AYOutput >> 3) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> (0 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = AYRegisters[eAYREGISTER_A_VOL + 0];
        
        if ((vol & 0x10) != 0)
        {
            vol = envelopeStep ^ attackEndVol;
        }
        
        channelOutput[0] += AYVolumes[vol];
    }
    
    // Channel 1
    AYChannelCount[1] += 2;
    
    // Noise frequency
    freq = AYRegisters[ (1 << 1) + eAYREGISTER_A_FINE ] | (AYRegisters[ (1 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (AYChannelCount[1] >= freq)
    {
        AYChannelCount[1]  -= freq;
        AYOutput ^= 2;
    }
    
    tone_output = ((AYOutput >> 1) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> 1) & 1);
    noise_output = ((AYOutput >> 3) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> (1 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = AYRegisters[eAYREGISTER_A_VOL + 1];
        
        if ((vol & 0x10) != 0)
        {
            vol = envelopeStep ^ attackEndVol;
        }
        
        channelOutput[1] += AYVolumes[vol];
    }
    
    // Channel 2
    AYChannelCount[2] += 2;
    
    // Noise frequency
    freq = AYRegisters[ (2 << 1) + eAYREGISTER_A_FINE ] | (AYRegisters[ (2 << 1) + eAYREGISTER_A_COARSE] << 8);
    
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (AYChannelCount[2] >= freq)
    {
        AYChannelCount[2]  -= freq;
        AYOutput ^= 4;
    }
    
    tone_output = ((AYOutput >> 2) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> 2) & 1);
    noise_output = ((AYOutput >> 3) & 1) | ((AYRegisters[eAYREGISTER_ENABLE] >> (2 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        int vol = AYRegisters[eAYREGISTER_A_VOL + 2];
        
        if ((vol & 0x10) != 0)
        {
            vol = envelopeStep ^ attackEndVol;
        }
        
        channelOutput[2] += AYVolumes[vol];
    }
}

void ZXSpectrum::resetAudio()
{
    AYOutput = 0;
    random = 1;
    channelOutput[0] = 0;
    channelOutput[1] = 0;
    channelOutput[2] = 0;
    AYChannelCount[0] = 0;
    AYChannelCount[1] = 0;
    AYChannelCount[2] = 0;
    noiseCount = 0;
    envelopeCount = 0;
    envelopeStep = 15;
    envelopeHolding = false;
    
    for (int i = 0; i < eAY_MAX_REGISTERS; i++)
    {
        setAYRegister(i);
        writeAYData(0);
    }
}

void ZXSpectrum::setupAudio(int sampleRate, int fps)
{
    audioBufferSize = (sampleRate / fps) * 6;
    audioBuffer = new signed short[ audioBufferSize ];

    audioTsStep = machineInfo.tsPerFrame / (sampleRate / fps);
    audioAYTStatesStep = 32;
}

#pragma mark - Audio Queue


