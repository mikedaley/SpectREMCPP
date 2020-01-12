//
//  Keyboard.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

// ------------------------------------------------------------------------------------------------------------
// - Keyboard

ZXSpectrum::KEYBOARD_ENTRY ZXSpectrum::keyboardLookup[] =
{
    { eZXSpectrumKey::Key_Shift,         0,  0, -1, -1 },    // CAPS SHIFT
    { eZXSpectrumKey::Key_Z,             0,	1, -1, -1 },    // Z
    { eZXSpectrumKey::Key_X,             0,	2, -1, -1 },    // X
    { eZXSpectrumKey::Key_C,             0,	3, -1, -1 },    // C
    { eZXSpectrumKey::Key_V,             0,	4, -1, -1 },    // V
    
    { eZXSpectrumKey::Key_A,             1,	0, -1, -1 },    // A
    { eZXSpectrumKey::Key_S,             1,	1, -1, -1 },    // S
    { eZXSpectrumKey::Key_D,             1,	2, -1, -1 },    // D
    { eZXSpectrumKey::Key_F,             1,	3, -1, -1 },    // F
    { eZXSpectrumKey::Key_G,             1,	4, -1, -1 },    // G
    
    { eZXSpectrumKey::Key_Q,             2, 0, -1, -1 },     // Q
    { eZXSpectrumKey::Key_W,             2, 1, -1, -1 },     // W
    { eZXSpectrumKey::Key_E,             2, 2, -1, -1 },     // E
    { eZXSpectrumKey::Key_R,             2, 3, -1, -1 },     // R
    { eZXSpectrumKey::Key_T,             2, 4, -1, -1 },     // T
    
    { eZXSpectrumKey::Key_1,             3, 0, -1, -1 },     // 1
    { eZXSpectrumKey::Key_2,             3, 1, -1, -1 },     // 2
    { eZXSpectrumKey::Key_3,             3, 2, -1, -1 },     // 3
    { eZXSpectrumKey::Key_4,             3, 3, -1, -1 },     // 4
    { eZXSpectrumKey::Key_5,             3, 4, -1, -1 },     // 5
    
    { eZXSpectrumKey::Key_0,             4, 0, -1, -1 },     // 0
    { eZXSpectrumKey::Key_9,             4, 1, -1, -1 },     // 9
    { eZXSpectrumKey::Key_8,             4, 2, -1, -1 },     // 8
    { eZXSpectrumKey::Key_7,             4, 3, -1, -1 },     // 7
    { eZXSpectrumKey::Key_6,             4, 4, -1, -1 },     // 6
    
    { eZXSpectrumKey::Key_P,             5, 0, -1, -1 },     // P
    { eZXSpectrumKey::Key_O,             5, 1, -1, -1 },     // O
    { eZXSpectrumKey::Key_I,             5, 2, -1, -1 },     // I
    { eZXSpectrumKey::Key_U,             5, 3, -1, -1 },     // U
    { eZXSpectrumKey::Key_Y,             5, 4, -1, -1 },     // Y
    
    { eZXSpectrumKey::Key_Enter,         6, 0, -1, -1 },     // ENTER
    { eZXSpectrumKey::Key_L,             6, 1, -1, -1 },     // L
    { eZXSpectrumKey::Key_K,             6, 2, -1, -1 },     // K
    { eZXSpectrumKey::Key_J,             6, 3, -1, -1 },     // J
    { eZXSpectrumKey::Key_H,             6, 4, -1, -1 },     // H
    
    { eZXSpectrumKey::Key_Space,         7, 0, -1, -1 },     // Space
    { eZXSpectrumKey::Key_SymbolShift,   7, 1, -1, -1 },     // SYMBOL SHIFT
    { eZXSpectrumKey::Key_M,             7, 2, -1, -1 },     // M
    { eZXSpectrumKey::Key_N,             7, 3, -1, -1 },     // N
    { eZXSpectrumKey::Key_B,             7, 4, -1, -1 },     // B

    { eZXSpectrumKey::Key_InvVideo,      0, 0, 3, 4 },       // Inv Video
    { eZXSpectrumKey::Key_TrueVideo,     0, 0, 3, 3 },       // True Video
    { eZXSpectrumKey::Key_Quote,         7, 1, 5, 0 },       // "
    { eZXSpectrumKey::Key_SemiColon,     7, 1, 5, 1 },       // ;
    { eZXSpectrumKey::Key_Comma,         7, 1, 7, 3 },       // ,
    { eZXSpectrumKey::Key_Minus,         7, 1, 6, 3 },       // -
    { eZXSpectrumKey::Key_Plus,          7, 1, 6, 2 },       // +
    { eZXSpectrumKey::Key_Period,        7, 1, 7, 2 },       // .
    { eZXSpectrumKey::Key_Edit,          0, 0, 3, 0 },       // Edit
    { eZXSpectrumKey::Key_Graph,         0, 0, 4, 1 },       // Graph
    { eZXSpectrumKey::Key_Break,         0, 0, 7, 0 },       // Break
    { eZXSpectrumKey::Key_Backspace,     0, 0, 4, 0 },       // Backspace
    { eZXSpectrumKey::Key_ArrowUp,       0, 0, 4, 3 },       // Arrow up
    { eZXSpectrumKey::Key_ArrowDown,     0, 0, 4, 4 },       // Arrow down
    { eZXSpectrumKey::Key_ArrowLeft,     0, 0, 3, 4 },       // Arrow left
    { eZXSpectrumKey::Key_ArrowRight,    0, 0, 4, 2 },       // Arrow right
    { eZXSpectrumKey::Key_ExtendMode,    0, 0, 7, 1 },       // Extend Mode
    { eZXSpectrumKey::Key_CapsLock,      0, 0, 3, 1 },       // Caps lock
};

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::keyboardKeyDown(eZXSpectrumKey key)
{
    for (int i = 0; i < static_cast<int>(sizeof(keyboardLookup) / sizeof(keyboardLookup[0])); i++)
    {
        if (keyboardLookup[i].key == key)
        {
            KEYBOARD_ENTRY& entry = keyboardLookup[i];
            if (entry.mapEntry1 != -1 && entry.mapBit1 != -1)
            {
                keyboardMap[entry.mapEntry1] &= ~(1 << entry.mapBit1);
            }
            if (entry.mapEntry2 != -1 && entry.mapBit2 != -1)
            {
                keyboardMap[entry.mapEntry2] &= ~(1 << entry.mapBit2);
            }
            if (key == eZXSpectrumKey::Key_CapsLock)
            {
                keyboardCapsLockPressed = true;
            }
            break;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::keyboardKeyUp(eZXSpectrumKey key)
{
    if (key != eZXSpectrumKey::Key_CapsLock)
    {
        for (int i = 0; i < static_cast<int>(sizeof(keyboardLookup) / sizeof(keyboardLookup[0])); i++)
        {
            if (keyboardLookup[i].key == key)
            {
                KEYBOARD_ENTRY& entry = keyboardLookup[i];
                if (entry.mapEntry1 != -1 && entry.mapBit1 != -1)
                {
                    keyboardMap[entry.mapEntry1] |= (1 << entry.mapBit1);
                }
                if (entry.mapEntry2 != -1 && entry.mapBit2 != -1)
                {
                    keyboardMap[entry.mapEntry2] |= (1 << entry.mapBit2);
                }
                break;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::keyboardCheckCapsLockStatus()
{
    // The Caps Lock combination needs to be set for minimum 2 frames for it to be registered by the ROM.
    // Once two frames have passed the key combination can be removed from the keyboard map.
    if (keyboardCapsLockPressed && keyboardCapsLockFrames > 2)
    {
        keyboardMap[0] |= 0x01;
        keyboardMap[3] |= 0x02;
        keyboardCapsLockPressed = false;
        keyboardCapsLockFrames = 0;
    }
    else if (keyboardCapsLockPressed)
    {
        keyboardCapsLockFrames ++;
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::keyboardMapReset()
{
    for (int i = 0; i < 8; i++)
    {
        keyboardMap[i] = 0xbf;
    }
}



