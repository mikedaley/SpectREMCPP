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

static constexpr float cBEEPER_VOLUME_MULTIPLIER = 8192;

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
        audio_ay_volumes[ i ] = static_cast<uint16_t>(fAYVolBase[ i ] * 8192);
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioSetup(double sampleRate, double fps)
{
    audio_buffer_size = static_cast<uint32_t>((sampleRate / fps) * 4.0);
    audio_buffer = new int16_t[ audio_buffer_size ]();
    audio_beeper_ts_step = machine_info.ts_per_frame / (sampleRate / fps);
    audio_ay_ts_step = 32;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioReset()
{
    if (audio_buffer)
    {
        delete audio_buffer;
    }
    
    audio_buffer = new int16_t[ audio_buffer_size ]();
    audio_buffer_index = 0;
    audio_ts_counter = 0;
    audio_ts_step_counter = 0;
    audio_output_level_left = 0;
    audio_output_level_right = 0;
    audio_ay_level_left = 0;
    audio_ay_level_right = 0;
    audio_ay_output = 0;
    audio_ay_random = 1;
    audio_ay_channel_output[0] = 0;
    audio_ay_channel_output[1] = 0;
    audio_ay_channel_output[2] = 0;
    audio_ay_channel_count[0] = 0;
    audio_ay_channel_count[1] = 0;
    audio_ay_channel_count[2] = 0;
    audio_ay_noise_count = 0;
    audio_ay_envelope_count = 0;
    audio_ay_envelope_holding = false;
    audio_specdrum_dac_value = 0;
    
    for (int32_t i = 0; i < AyRegister::MAX_REGISTERS; i++)
    {
        audioAYSetRegister(i);
        audioAYWriteData(0);
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Generate audio output from Beeper and AY chip

void ZXSpectrum::audioUpdateWithTs(int32_t tStates)
{
    if (emu_paused)
    {
        return;
    }
    
    // Grab the current state of the audio ear output & the tapeLevel which is used to register input when loading tapes.
    // Only need to do this once per audio update
    float audioEarLevel = (audio_ear_bit | virtual_tape->input_bit) ? cBEEPER_VOLUME_MULTIPLIER : 0;
    
    // Add in any output from SpecDRUM if it's being used
    if (emu_use_specdrum)
    {
        audioEarLevel += audio_specdrum_dac_value;
    }
    
    // Loop over each tState so that the necessary audio samples can be generated
    for(int32_t i = 0; i < tStates; i++)
    {
        // If we have done more cycles now than the audio step counter, generate a new sample
        audio_ts_counter += 1.0f;
        audio_output_level_left += audioEarLevel;
        audio_output_level_right += audioEarLevel;
        audio_output_level_left += audio_ay_level_left;
        audio_output_level_right += audio_ay_level_right;

        // Loop over each tState so that the necessary audio samples can be generated
       if (emu_use_ay_sound)
       {
           audio_ay_ts += 1.0f;
           if (audio_ay_ts >= audio_ay_ts_step)
           {
               audioAYUpdate();

               audio_ay_level_left = audio_ay_channel_output[0];      // A - Left
               audio_ay_level_left += audio_ay_channel_output[1];     // B - Left
               audio_ay_level_left += audio_ay_channel_output[2];     // C - Left

               audio_ay_level_right = audio_ay_channel_output[0];     // A - Right
               audio_ay_level_right += audio_ay_channel_output[1];    // B - Right
               audio_ay_level_right += audio_ay_channel_output[2];    // C - Right

               audio_ay_channel_output[0] = 0;
               audio_ay_channel_output[1] = 0;
               audio_ay_channel_output[2] = 0;
               
               audio_ay_ts -= audio_ay_ts_step;
           }
       }
        
        if (audio_ts_counter >= audio_beeper_ts_step)
        {
            // Scale down
            audio_output_level_left /= audio_ts_counter;
            audio_output_level_right /= audio_ts_counter;
            
            // Load the buffer with the sample for both left and right channels
            audio_buffer[ audio_buffer_index++ ] = static_cast< int16_t >(audio_output_level_left);
            audio_buffer[ audio_buffer_index++ ] = static_cast< int16_t >(audio_output_level_right);

            audio_ts_counter -= audio_beeper_ts_step;
            audio_output_level_left = (audioEarLevel + audio_ay_level_left) * audio_ts_counter;
            audio_output_level_right = (audioEarLevel + audio_ay_level_right) * audio_ts_counter;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------
// - AY Chip

void ZXSpectrum::audioAYSetRegister(uint8_t reg)
{
    if (reg < AyRegister::MAX_REGISTERS)
    {
        audio_ay_current_register = reg;
    }
    else
    {
        // If an AY register > 16 is selected then point it at the floating register used to
        // emulate this behaviour
        audio_ay_current_register = AyRegister::FLOATING;
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioAYWriteData(uint8_t data)
{
    uint8_t envelopeType;

    switch (audio_ay_current_register) {
        case AyRegister::A_FINE:
        case AyRegister::B_FINE:
        case AyRegister::C_FINE:
        case AyRegister::ENABLE:
        case AyRegister::E_FINE:
        case AyRegister::E_COARSE:
        case AyRegister::PORT_A:
        case AyRegister::PORT_B:
            break;
            
        case AyRegister::A_COARSE:
        case AyRegister::B_COARSE:
        case AyRegister::C_COARSE:
            data &= 0x0f;
            break;
            
        case AyRegister::E_SHAPE:
            audio_ay_envelope_holding = false;
            audio_ay_envelope_count = 0;
            data &= 0x0f;

            audio_ay_attack_end_volume = (data & eENVFLAG_ATTACK) != 0 ? 15 : 0;

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

            audio_ay_envelope_hold = (envelopeType & eENVFLAG_HOLD) != 0 ? true : false;
            audio_ay_envelope_alternate = (envelopeType & eENVFLAG_ALTERNATE) != 0 ? true : false;
            audio_ay_envelope_attack = (envelopeType & eENVFLAG_ATTACK) != 0 ? true : false;
            audio_ay_envelope_continue = (envelopeType & eENVFLAG_CONTINUE) != 0 ? true : false;

            audio_ay_one_shot = false;

            audio_ay_attack_end_volume = (audio_ay_envelope_attack) ? 0 : 15;
            break;

        case AyRegister::NOISEPER:
            data &= 0x1f;
            break;
        case AyRegister::A_VOL:
        case AyRegister::B_VOL:
        case AyRegister::C_VOL:
            data &= 0xff;
            break;
            
        case AyRegister::FLOATING:
            break;
            
        default:
            break;
    }
    
    audio_ay_registers[ audio_ay_current_register ] = data;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::audioDecayAYFloatingRegister()
{
    // Decay the AY registers result returned for registers above 15
    audio_ay_registers[ AyRegister::FLOATING ] >>= 1;
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum::audioAYReadData()
{
    return audio_ay_registers[ audio_ay_current_register ];
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
    if (!audio_ay_envelope_holding)
    {
        audio_ay_envelope_count++;
        
        if ( audio_ay_envelope_count >= static_cast<uint32_t>(audio_ay_registers[ AyRegister::E_FINE ] | (audio_ay_registers[ AyRegister::E_COARSE] << 8)))
        {
            audio_ay_envelope_count = 0;

            if (audio_ay_one_shot)
            {
                audio_ay_one_shot = !audio_ay_one_shot;

                if (audio_ay_envelope_hold)
                {
                    audio_ay_envelope_holding = true;
                    if (audio_ay_envelope_alternate)
                    {
                        audio_ay_attack_end_volume = audio_ay_attack_end_volume ^ 15;
                    }
                }
                else
                {
                    if (audio_ay_envelope_alternate)
                    {
                        audio_ay_envelope_attack = !audio_ay_envelope_attack;
                    }
                    audio_ay_attack_end_volume = (audio_ay_envelope_attack) ? audio_ay_attack_end_volume = 0 : audio_ay_attack_end_volume = 15;
                }
            }
            else
            {
                if (audio_ay_envelope_attack)
                {
                    if (audio_ay_attack_end_volume < 15)
                    {
                        audio_ay_attack_end_volume += 1;
                        if (audio_ay_attack_end_volume == 15)
                            { audio_ay_one_shot = true; }
                    }
                }
                else
                {
                    if (audio_ay_attack_end_volume > 0)
                    {
                        audio_ay_attack_end_volume -= 1;
                        if (audio_ay_attack_end_volume == 0)
                            { audio_ay_one_shot = true; }
                    }
                }
            }
        }
    }
    
    if ((audio_ay_registers[AyRegister::ENABLE] & 0x38) != 0x38)
    {
        audio_ay_noise_count++;
        
        uint16_t freq = audio_ay_registers[ AyRegister::NOISEPER ];
        
        // 0 is assumed to be 1
        if (freq == 0)
        {
            freq = 1;
        }
        
        if (audio_ay_noise_count >= freq)
        {
            audio_ay_noise_count = 0;

            // Better random noise from Woody :)
            bool carry = (audio_ay_random & 1);
            audio_ay_random = audio_ay_random >> 1;
            audio_ay_output &= ~(1 << 3);
            if (carry)
            {
                audio_ay_output |= (1 << 3);
                audio_ay_random ^= (0x24000 >> 1);
            }
        }
    }
    
    // Channel 0
    audio_ay_channel_count[0] += 2;
    
    // Noise frequency
    uint16_t freq = audio_ay_registers[ AyRegister::A_FINE ] | (audio_ay_registers[ AyRegister::A_COARSE] << 8);
    if (freq == 0)
    {
        freq = 1;
    }
    
    if (audio_ay_channel_count[0] >= freq)
    {
        audio_ay_channel_count[0]  -= freq;
        audio_ay_output ^= 1;
    }
    
    uint8_t tone_output = ((audio_ay_output >> 0) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> 0) & 1);
    uint8_t noise_output = ((audio_ay_output >> 3) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> (0 + 3)) & 1);
    
    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audio_ay_registers[AyRegister::A_VOL + 0];
        
        if ((vol & 0x10) != 0)
        {
            vol = audio_ay_attack_end_volume;
        }

        audio_ay_channel_output[0] += audio_ay_volumes[vol];
    }
    
    // Channel 1
    audio_ay_channel_count[1] += 2;

    // Noise frequency
    freq = audio_ay_registers[ AyRegister::B_FINE ] | (audio_ay_registers[ AyRegister::B_COARSE] << 8);

    if (freq == 0)
    {
        freq = 1;
    }

    if (audio_ay_channel_count[1] >= freq)
    {
        audio_ay_channel_count[1]  -= freq;
        audio_ay_output ^= 2;
    }

    tone_output = ((audio_ay_output >> 1) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> 1) & 1);
    noise_output = ((audio_ay_output >> 3) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> (1 + 3)) & 1);

    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audio_ay_registers[AyRegister::A_VOL + 1];

        if ((vol & 0x10) != 0)
        {
            vol = audio_ay_attack_end_volume;
        }

        audio_ay_channel_output[1] += audio_ay_volumes[vol];
    }

    // Channel 2
    audio_ay_channel_count[2] += 2;

    // Noise frequency
    freq = audio_ay_registers[ AyRegister::C_FINE ] | (audio_ay_registers[ AyRegister::C_COARSE] << 8);

    if (freq == 0)
    {
        freq = 1;
    }

    if (audio_ay_channel_count[2] >= freq)
    {
        audio_ay_channel_count[2]  -= freq;
        audio_ay_output ^= 4;
    }

    tone_output = ((audio_ay_output >> 2) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> 2) & 1);
    noise_output = ((audio_ay_output >> 3) & 1) | ((audio_ay_registers[AyRegister::ENABLE] >> (2 + 3)) & 1);

    if ((tone_output & noise_output) == 1)
    {
        uint8_t vol = audio_ay_registers[AyRegister::C_VOL];

        if ((vol & 0x10) != 0)
        {
            vol = audio_ay_attack_end_volume;
        }

        audio_ay_channel_output[2] += audio_ay_volumes[vol];
    }
    
}



