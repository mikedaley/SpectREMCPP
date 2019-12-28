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
    { -1,        0, 0 },    // SHIFT
#ifdef QT_SPECTRUM
    { Qt::Key_Z, 0,	1 },    // Z
    { Qt::Key_X, 0,	2 },    // X
    { Qt::Key_C, 0,	3 },    // C
    { Qt::Key_V, 0,	4 },    // V
    
    { Qt::Key_A, 1,	0 },    // A
    { Qt::Key_S, 1,	1 },    // S
    { Qt::Key_D, 1,	2 },    // D
    { Qt::Key_F, 1,	3 },    // F
    { Qt::Key_G, 1,	4 },    // G
    
    { Qt::Key_Q, 2, 0 },   // Q
    { Qt::Key_W, 2, 1 },   // W
    { Qt::Key_E, 2, 2 },   // E
    { Qt::Key_R, 2, 3 },   // R
    { Qt::Key_T, 2, 4 },   // T
    
    { Qt::Key_1, 3, 0 },   // 1
    { Qt::Key_2, 3, 1 },   // 2
    { Qt::Key_3, 3, 2 },   // 3
    { Qt::Key_4, 3, 3 },   // 4
    { Qt::Key_5, 3, 4 },   // 5
    
    { Qt::Key_0, 4, 0 },   // 0
    { Qt::Key_9, 4, 1 },   // 9
    { Qt::Key_8, 4, 2 },   // 8
    { Qt::Key_7, 4, 3 },   // 7
    { Qt::Key_6, 4, 4 },   // 6
    
    { Qt::Key_P, 5, 0 },   // P
    { Qt::Key_O, 5, 1 },   // O
    { Qt::Key_I, 5, 2 },   // I
    { Qt::Key_U, 5, 3 },   // U
    { Qt::Key_Y, 5, 4 },   // Y
    
    { Qt::Key_Return, 6, 0 },   // ENTER
    { Qt::Key_L, 6, 1 },   // L
    { Qt::Key_K, 6, 2 },   // K
    { Qt::Key_J, 6, 3 },   // J
    { Qt::Key_H,  6, 4 },  // H
    
    { Qt::Key_Space, 7, 0 },   // Space
    { -1,        7, 1 },   // SYMBOL SHIFT
    { Qt::Key_M, 7, 2 },   // M
    { Qt::Key_N, 7, 3 },   // N
    { Qt::Key_B, 7, 4 }    // B
#endif
};

void ZXSpectrum::keyboardKeyDown(int key)
{
    switch (key)
    {
#ifdef QT_SPECTRUM
    case Qt::Key_BracketRight: // Inv Video
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[3] &= ~0x08; // 4
        break;

    case Qt::Key_BracketLeft: // True Video
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[3] &= ~0x04; // 3
        break;

    case Qt::Key_Apostrophe: // "
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[5] &= ~0x01; // P
        break;

    case Qt::Key_Semicolon: // ;
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[5] &= ~0x02; // O
        break;

    case Qt::Key_Comma: // ,
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[7] &= ~0x08; // N
        break;

    case Qt::Key_Minus: // -
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[6] &= ~0x08; // J
        break;

    case Qt::Key_Plus: // +
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[6] &= ~0x04; // K
        break;

    case Qt::Key_Period: // .
        keyboardMap[7] &= ~0x02; // Sym
        keyboardMap[7] &= ~0x04; // M
        break;

    case Qt::Key_Tab: // Edit
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[3] &= ~0x01; // 1
        break;

    case Qt::Key_QuoteLeft: // Graph
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[4] &= ~0x02; // 9
        break;

    case Qt::Key_Escape: // Break
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[7] &= ~0x01; // Space
        break;

    case Qt::Key_Backspace: // Backspace
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[4] &= ~0x01; // 0
        break;

    case Qt::Key_Up: // Arrow up
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[4] &= ~0x08; // 7
        break;

    case Qt::Key_Down: // Arrow down
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[4] &= ~0x10; // 6
        break;

    case Qt::Key_Left: // Arrow left
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[3] &= ~0x10; // 5
        break;

    case Qt::Key_Right: // Arrow right
        keyboardMap[0] &= ~0x01; // Shift
        keyboardMap[4] &= ~0x04; // 8
        break;

    case Qt::Key_Alt:
        keyboardMap[0] &= ~0x01;
        keyboardMap[7] &= ~0x02;
        break;

    case Qt::Key_Shift:
        keyboardMap[0] &= ~0x01;
        break;

    case Qt::Key_Control:
        keyboardMap[7] &= ~0x02;
        break;

    case Qt::Key_CapsLock:
        keyboardMap[0] &= ~0x01;
        keyboardMap[3] &= ~0x02;
        keyboardCapsLockPressed = true;
        break;
#endif    
    default:
        for (int i = 0; i < static_cast<int>(sizeof(keyboardLookup) / sizeof(keyboardLookup[0])); i++)
        {
            if (keyboardLookup[i].key == key)
            {
                keyboardMap[keyboardLookup[i].mapEntry] &= ~(1 << keyboardLookup[i].mapBit);
                break;
            }
        }
        break;
    }
}

void ZXSpectrum::keyboardKeyUp(int key)
{
    switch (key)
    {
#ifdef QT_SPECTRUM
    case Qt::Key_BracketRight: // Inv Video
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[3] |= 0x08; // 4
        break;

    case Qt::Key_BracketLeft: // True Video
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[3] |= 0x04; // 3
        break;

    case Qt::Key_Apostrophe: // "
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[5] |= 0x01; // P
        break;

    case Qt::Key_Semicolon: // ;
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[5] |= 0x02; // O
        break;

    case Qt::Key_Comma: // ,
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[7] |= 0x08; // M
        break;

    case Qt::Key_Plus: // +
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[6] |= 0x04; // K
        break;

    case Qt::Key_Minus: // -
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[6] |= 0x08; // J
        break;

    case Qt::Key_Period: // .
        keyboardMap[7] |= 0x02; // Sym
        keyboardMap[7] |= 0x04; // N
        break;

    case Qt::Key_Tab: // Edit
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[3] |= 0x01; // 1
        break;

    case Qt::Key_QuoteLeft: // Graph
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[4] |= 0x02; // 9
        break;

    case Qt::Key_Escape: // Break
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[7] |= 0x01; // Space
        break;

    case Qt::Key_Backspace: // Backspace
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[4] |= 0x01; // 0
        break;

    case Qt::Key_Up: // Arrow up
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[4] |= 0x08; // 7
        break;

    case Qt::Key_Down: // Arrow down
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[4] |= 0x10; // 6
        break;

    case Qt::Key_Left: // Arrow left
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[3] |= 0x10; // 5
        break;

    case Qt::Key_Right: // Arrow right
        keyboardMap[0] |= 0x01; // Shift
        keyboardMap[4] |= 0x04; // 8
        break;

    case Qt::Key_Alt:
        keyboardMap[0] |= 0x01;
        keyboardMap[7] |= 0x02;
        break;

    case Qt::Key_Shift: // Shift
        keyboardMap[0] |= 0x01;
        break;

    case Qt::Key_Control:
        keyboardMap[7] |= 0x02;
        break;
#endif
    default:
        for (int i = 0; i < static_cast<int>(sizeof(keyboardLookup) / sizeof(keyboardLookup[0])); i++)
        {
            if (keyboardLookup[i].key == key)
            {
                keyboardMap[keyboardLookup[i].mapEntry] |= (1 << keyboardLookup[i].mapBit);
                break;
            }
        }
        break;
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



