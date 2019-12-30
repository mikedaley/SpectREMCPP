//
//  Keyboard.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

// - Keyboard

ZXSpectrum::KEYBOARD_ENTRY ZXSpectrum::keyboardLookup[] =
{
    { ZXSpectrumKey::Key_Shift,         0,  0, -1, -1 },    // CAPS SHIFT
    { ZXSpectrumKey::Key_Z,             0,	1, -1, -1 },    // Z
    { ZXSpectrumKey::Key_X,             0,	2, -1, -1 },    // X
    { ZXSpectrumKey::Key_C,             0,	3, -1, -1 },    // C
    { ZXSpectrumKey::Key_V,             0,	4, -1, -1 },    // V
    
    { ZXSpectrumKey::Key_A,             1,	0, -1, -1 },    // A
    { ZXSpectrumKey::Key_S,             1,	1, -1, -1 },    // S
    { ZXSpectrumKey::Key_D,             1,	2, -1, -1 },    // D
    { ZXSpectrumKey::Key_F,             1,	3, -1, -1 },    // F
    { ZXSpectrumKey::Key_G,             1,	4, -1, -1 },    // G
    
    { ZXSpectrumKey::Key_Q,             2, 0, -1, -1 },     // Q
    { ZXSpectrumKey::Key_W,             2, 1, -1, -1 },     // W
    { ZXSpectrumKey::Key_E,             2, 2, -1, -1 },     // E
    { ZXSpectrumKey::Key_R,             2, 3, -1, -1 },     // R
    { ZXSpectrumKey::Key_T,             2, 4, -1, -1 },     // T
    
    { ZXSpectrumKey::Key_1,             3, 0, -1, -1 },     // 1
    { ZXSpectrumKey::Key_2,             3, 1, -1, -1 },     // 2
    { ZXSpectrumKey::Key_3,             3, 2, -1, -1 },     // 3
    { ZXSpectrumKey::Key_4,             3, 3, -1, -1 },     // 4
    { ZXSpectrumKey::Key_5,             3, 4, -1, -1 },     // 5
    
    { ZXSpectrumKey::Key_0,             4, 0, -1, -1 },     // 0
    { ZXSpectrumKey::Key_9,             4, 1, -1, -1 },     // 9
    { ZXSpectrumKey::Key_8,             4, 2, -1, -1 },     // 8
    { ZXSpectrumKey::Key_7,             4, 3, -1, -1 },     // 7
    { ZXSpectrumKey::Key_6,             4, 4, -1, -1 },     // 6
    
    { ZXSpectrumKey::Key_P,             5, 0, -1, -1 },     // P
    { ZXSpectrumKey::Key_O,             5, 1, -1, -1 },     // O
    { ZXSpectrumKey::Key_I,             5, 2, -1, -1 },     // I
    { ZXSpectrumKey::Key_U,             5, 3, -1, -1 },     // U
    { ZXSpectrumKey::Key_Y,             5, 4, -1, -1 },     // Y
    
    { ZXSpectrumKey::Key_Enter,         6, 0, -1, -1 },     // ENTER
    { ZXSpectrumKey::Key_L,             6, 1, -1, -1 },     // L
    { ZXSpectrumKey::Key_K,             6, 2, -1, -1 },     // K
    { ZXSpectrumKey::Key_J,             6, 3, -1, -1 },     // J
    { ZXSpectrumKey::Key_H,             6, 4, -1, -1 },     // H
    
    { ZXSpectrumKey::Key_Space,         7, 0, -1, -1 },     // Space
    { ZXSpectrumKey::Key_SymbolShift,   7, 1, -1, -1 },     // SYMBOL SHIFT
    { ZXSpectrumKey::Key_M,             7, 2, -1, -1 },     // M
    { ZXSpectrumKey::Key_N,             7, 3, -1, -1 },     // N
    { ZXSpectrumKey::Key_B,             7, 4, -1, -1 },     // B

    { ZXSpectrumKey::Key_InvVideo,      0, 0, 3, 4 },       // Inv Video
    { ZXSpectrumKey::Key_TrueVideo,     0, 0, 3, 3 },       // True Video
    { ZXSpectrumKey::Key_Quote,         7, 1, 5, 0 },       // "
    { ZXSpectrumKey::Key_SemiColon,     7, 1, 5, 1 },       // ;
    { ZXSpectrumKey::Key_Comma,         7, 1, 7, 3 },       // ,
    { ZXSpectrumKey::Key_Minus,         7, 1, 6, 3 },       // -
    { ZXSpectrumKey::Key_Plus,          7, 1, 6, 2 },       // +
    { ZXSpectrumKey::Key_Period,        7, 1, 7, 2 },       // .
    { ZXSpectrumKey::Key_Edit,          0, 0, 3, 0 },       // Edit
    { ZXSpectrumKey::Key_Graph,         0, 0, 4, 1 },       // Graph
    { ZXSpectrumKey::Key_Break,         0, 0, 7, 0 },       // Break
    { ZXSpectrumKey::Key_Backspace,     0, 0, 4, 0 },       // Backspace
    { ZXSpectrumKey::Key_ArrowUp,       0, 0, 4, 3 },       // Arrow up
    { ZXSpectrumKey::Key_ArrowDown,     0, 0, 4, 4 },       // Arrow down
    { ZXSpectrumKey::Key_ArrowLeft,     0, 0, 3, 4 },       // Arrow left
    { ZXSpectrumKey::Key_ArrowRight,    0, 0, 4, 2 },       // Arrow right
    { ZXSpectrumKey::Key_ExtendMode,    0, 0, 7, 1 },       // Extend Mode
    { ZXSpectrumKey::Key_CapsLock,      0, 0, 3, 1 },       // Caps lock
};

void ZXSpectrum::keyboardKeyDown(ZXSpectrumKey key)
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
            if (key == ZXSpectrumKey::Key_CapsLock)
            {
                keyboardCapsLockPressed = true;
            }
            break;
        }
    }
}

void ZXSpectrum::keyboardKeyUp(ZXSpectrumKey key)
{
    if (key != ZXSpectrumKey::Key_CapsLock)
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

void ZXSpectrum::keyboardMapReset()
{
    for (int i = 0; i < 8; i++)
    {
        keyboardMap[i] = 0xbf;
    }
}



