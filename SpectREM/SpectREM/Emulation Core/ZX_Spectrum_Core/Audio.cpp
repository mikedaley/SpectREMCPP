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

static const float cBEEPER_VOLUME_MULTIPLIER = 8192;

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

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioBuildAYVolumesTable()
{
    for (uint32_t i = 0; i < 16; i++)
    {
        audioAYVolumes[ i ] = static_cast<uint16_t>(fAYVolBase[ i ] * 8192);
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioSetup(double sampleRate, double fps)
{
    audioBufferSize = static_cast<uint32_t>((sampleRate / fps) * 4.0);
    audioBuffer = new int16_t[ audioBufferSize ]();
    audioBeeperTsStep = machineInfo.tsPerFrame / (sampleRate / fps);
    audioAYTsStep = 32;
}

// ------------------------------------------------------------------------------------------------------------

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
    audioAYEnvelopeCount = 0;
    audioAYEnvelopeHolding = false;
    specdrumDACValue = 0;
    
    for (int32_t i = 0; i < E_AYREGISTER::MAX_REGISTERS; i++)
    {
        audioAYSetRegister(i);
        audioAYWriteData(0);
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Generate audio output from Beeper and AY chip

void ZXSpectrum::audioUpdateWithTs(int32_t tStates)
{
    if (emuPaused)
    {
        return;
    }
    
    // Grab the current state of the audio ear output & the tapeLevel which is used to register input when loading tapes.
    // Only need to do this once per audio update
    float audioEarLevel = (audioEarBit | tapePlayer->inputBit) ? cBEEPER_VOLUME_MULTIPLIER : 0;
    
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

            audioTsCounter -= audioBeeperTsStep;
            audioOutputLevelLeft = (audioEarLevel + audioAYLevelLeft) * audioTsCounter;
            audioOutputLevelRight = (audioEarLevel + audioAYLevelRight) * audioTsCounter;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------
// - AY Chip

void ZXSpectrum::audioAYSetRegister(uint8_t reg)
{
    if (reg < E_AYREGISTER::MAX_REGISTERS)
    {
        audioAYCurrentRegister = reg;
    }
    else
    {
        // If an AY register > 16 is selected then point it at the floating register used to
        // emulate this behaviour
        audioAYCurrentRegister = E_AYREGISTER::FLOATING;
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioAYWriteData(uint8_t data)
{
    uint8_t envelopeType;

    switch (audioAYCurrentRegister) {
        case E_AYREGISTER::A_FINE:
        case E_AYREGISTER::B_FINE:
        case E_AYREGISTER::C_FINE:
        case E_AYREGISTER::ENABLE:
        case E_AYREGISTER::E_FINE:
        case E_AYREGISTER::E_COARSE:
        case E_AYREGISTER::PORT_A:
        case E_AYREGISTER::PORT_B:
            break;
            
        case E_AYREGISTER::A_COARSE:
        case E_AYREGISTER::B_COARSE:
        case E_AYREGISTER::C_COARSE:
            data &= 0x0f;
            break;
            
        case E_AYREGISTER::E_SHAPE:
            audioAYEnvelopeHolding = false;
            audioAYEnvelopeCount = 0;
            data &= 0x0f;

            audioAYAttackEndVol = (data & eENVFLAG_ATTACK) != 0 ? 15 : 0;

            envelopeType = data;
            switch (envelopeType & 12)
            {
            case 0:
                envelopeType = 9;
                break;
            case 4:
                envelopeType = 15;
                break;
            }

            audioAYEnvelopeHold = (envelopeType & eENVFLAG_HOLD) != 0 ? true : false;
            audioAYEnvelopeAlt = (envelopeType & eENVFLAG_ALTERNATE) != 0 ? true : false;
            audioAYEnvelopeAttack = (envelopeType & eENVFLAG_ATTACK) != 0 ? true : false;
            audioAYEnvelopeContinue = (envelopeType & eENVFLAG_CONTINUE) != 0 ? true : false;

            audioAYOneShot = false;

            audioAYAttackEndVol = (audioAYEnvelopeAttack) ? 0 : 15;
            break;

        case E_AYREGISTER::NOISEPER:
            data &= 0x1f;
            break;
        case E_AYREGISTER::A_VOL:
        case E_AYREGISTER::B_VOL:
        case E_AYREGISTER::C_VOL:
            data &= 0xff;
            break;
            
        case E_AYREGISTER::FLOATING:
            break;
            
        default:
            break;
    }
    
    audioAYRegisters[ audioAYCurrentRegister ] = data;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioDecayAYFloatingRegister()
{
    // Decay the AY registers result returned for registers above 15
    audioAYRegisters[ E_AYREGISTER::FLOATING ] >>= 1;
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum::audioAYReadData()
{
    return audioAYRegisters[ audioAYCurrentRegister ];
}

// ------------------------------------------------------------------------------------------------------------

// The possible combinations and resulting shapes are :
//
// Binary  Hex      Shape
// 00XX    00h - 03h  \_________ (same as 09h)
// 01XX    04h - 07h  /|________ (same as 0Fh)
// 1000    08h        \\\\\\\\\\
// 1001    09h        \_________ (volume remains quiet)
// 1010    0Ah        \/\/\/\/\/
// 1011    0Bh        \|```````` (volume remains high)
// 1100    0Ch        /|/|/|/|/|
// 1101    0Dh        /````````` (volume remains high)
// 1110    0Eh        /\/\/\/\/\
// 1111    0Fh        /|________ (volume remains quiet)

// Bit 0  Hold(1 = stop envelope past first cycle)
// Bit 1  Alternate(1 = reverse direction at end of each cycle)
// Bit 2  Attack(1 = initial direction increase)
// Bit 3  Continue(0 = same as if Bit0 = 1 and Bit1 = Bit2)

void ZXSpectrum::audioAYUpdate()
{
    if (!audioAYEnvelopeHolding)
    {
        audioAYEnvelopeCount++;
        
        if ( audioAYEnvelopeCount >= static_cast<uint32_t>(audioAYRegisters[ E_AYREGISTER::E_FINE ] | (audioAYRegisters[ E_AYREGISTER::E_COARSE] << 8)))
        {
            audioAYEnvelopeCount = 0;

            if (audioAYOneShot)
            {
                audioAYOneShot = !audioAYOneShot;

                if (audioAYEnvelopeHold)
                {
                    audioAYEnvelopeHolding = true;
                    if (audioAYEnvelopeAlt)
                    {
                        audioAYAttackEndVol = audioAYAttackEndVol ^ 15;
                    }
                }
                else
                {
                    if (audioAYEnvelopeAlt)
                    {
                        audioAYEnvelopeAttack = !audioAYEnvelopeAttack;
                    }
                    audioAYAttackEndVol = (audioAYEnvelopeAttack) ? audioAYAttackEndVol = 0 : audioAYAttackEndVol = 15;
                }
            }
            else
            {
                if (audioAYEnvelopeAttack)
                {
                    if (audioAYAttackEndVol < 15)
                    {
                        audioAYAttackEndVol += 1;
                        if (audioAYAttackEndVol == 15)
                            { audioAYOneShot = true; }
                    }
                }
                else
                {
                    if (audioAYAttackEndVol > 0)
                    {
                        audioAYAttackEndVol -= 1;
                        if (audioAYAttackEndVol == 0)
                            { audioAYOneShot = true; }
                    }
                }
            }
        }
    }
    
    if ((audioAYRegisters[E_AYREGISTER::ENABLE] & 0x38) != 0x38)
    {
        audioAYNoiseCount++;
        
        uint16_t freq = audioAYRegisters[ E_AYREGISTER::NOISEPER ];
        
        // 0 is assumed to be 1
        if (freq == 0)
        {
            freq = 1;
        }
        
        if (audioAYNoiseCount >= freq)
        {
            audioAYNoiseCount = 0;

            // Better random noise from Woody :)
            bool carry = (audioAYrandom & 1);
            audioAYrandom = audioAYrandom >> 1;
            audioAYOutput &= ~(1 << 3);
            if (carry)
            {
                audioAYOutput |= (1 << 3);
                audioAYrandom ^= (0x24000 >> 1);
            }
        }
    }
    
    // Channel 0
    audioAYChannelCount[0] += 2;
    
    // Noise frequency
    uint16_t freq = audioAYRegisters[ E_AYREGISTER::A_FINE ] | (audioAYRegisters[ E_AYREGISTER::A_COARSE] << 8);
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audioAYChannelCount[0] >= freq)
    {
        audioAYChannelCount[0]  -= freq;
        audioAYOutput ^= 1;
    }
    
    uint8_t tone_output = ((audioAYOutput >> 0) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> 0) & 1);
    uint8_t noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> (0 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audioAYRegisters[E_AYREGISTER::A_VOL + 0];
        
        if ((vol & 0x10) != 0)
        {
            vol = audioAYAttackEndVol;
        }

        audioAYChannelOutput[0] += audioAYVolumes[vol];
    }
    
    // Channel 1
    audioAYChannelCount[1] += 2;

    // Noise frequency
    freq = audioAYRegisters[ E_AYREGISTER::B_FINE ] | (audioAYRegisters[ E_AYREGISTER::B_COARSE] << 8);

    if (freq == 0)
    {
        freq = 1;
    }

    if (audioAYChannelCount[1] >= freq)
    {
        audioAYChannelCount[1]  -= freq;
        audioAYOutput ^= 2;
    }

    tone_output = ((audioAYOutput >> 1) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> 1) & 1);
    noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> (1 + 3)) & 1);

    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audioAYRegisters[E_AYREGISTER::A_VOL + 1];

        if ((vol & 0x10) != 0)
        {
            vol = audioAYAttackEndVol;
        }

        audioAYChannelOutput[1] += audioAYVolumes[vol];
    }

    // Channel 2
    audioAYChannelCount[2] += 2;

    // Noise frequency
    freq = audioAYRegisters[ E_AYREGISTER::C_FINE ] | (audioAYRegisters[ E_AYREGISTER::C_COARSE] << 8);

    if (freq == 0)
    {
        freq = 1;
    }

    if (audioAYChannelCount[2] >= freq)
    {
        audioAYChannelCount[2]  -= freq;
        audioAYOutput ^= 4;
    }

    tone_output = ((audioAYOutput >> 2) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> 2) & 1);
    noise_output = ((audioAYOutput >> 3) & 1) | ((audioAYRegisters[E_AYREGISTER::ENABLE] >> (2 + 3)) & 1);

    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audioAYRegisters[E_AYREGISTER::C_VOL];

        if ((vol & 0x10) != 0)
        {
            vol = audioAYAttackEndVol;
        }

        audioAYChannelOutput[2] += audioAYVolumes[vol];
    }
    
}



