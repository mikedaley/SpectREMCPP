//
//  Keyboard.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Keyboard

ZXSpectrum::KEYBOARD_ENTRY ZXSpectrum::keyboardLookup[] =
{
    {-1, 0, 0 },    // SHIFT
    { 6, 0,	1 },    // Z
    { 7, 0,	2 },    // X
    { 8, 0,	3 },    // C
    { 9, 0,	4 },    // V
    
    { 0, 1,	0 },    // A
    { 1, 1,	1 },    // S
    { 2, 1,	2 },    // D
    { 3, 1,	3 },    // F
    { 5, 1,	4 },    // G
    
    { 12, 2, 0 },   // Q
    { 13, 2, 1 },   // W
    { 14, 2, 2 },   // E
    { 15, 2, 3 },   // R
    { 17, 2, 4 },   // T
    
    { 18, 3, 0 },   // 1
    { 19, 3, 1 },   // 2
    { 20, 3, 2 },   // 3
    { 21, 3, 3 },   // 4
    { 23, 3, 4 },   // 5
    
    { 29, 4, 0 },   // 0
    { 25, 4, 1 },   // 9
    { 28, 4, 2 },   // 8
    { 26, 4, 3 },   // 7
    { 22, 4, 4 },   // 6
    
    { 35, 5, 0 },   // P
    { 31, 5, 1 },   // O
    { 34, 5, 2 },   // I
    { 32, 5, 3 },   // U
    { 16, 5, 4 },   // Y
    
    { 36, 6, 0 },   // ENTER
    { 37, 6, 1 },   // L
    { 40, 6, 2 },   // K
    { 38, 6, 3 },   // J
    { 4,  6, 4 },   // H
    
    { 49, 7, 0 },   // Space
    { -1, 7, 1 },   // SYMBOL SHIFT
    { 46, 7, 2 },   // M
    { 45, 7, 3 },   // N
    { 11, 7, 4 }    // B
};

void ZXSpectrum::keyDown(unsigned short key)
{
    switch (key)
    {
        case 30: // Inv Video
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[3] &= ~0x08; // 4
            break;
            
        case 33: // True Video
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[3] &= ~0x04; // 3
            break;
            
        case 39: // "
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[5] &= ~0x01; // P
            break;
            
        case 41: // ;
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[5] &= ~0x02; // O
            break;
            
        case 43: // ,
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[7] &= ~0x08; // N
            break;
            
        case 27: // -
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[6] &= ~0x08; // J
            break;
            
        case 24: // +
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[6] &= ~0x04; // K
            break;
            
        case 47: // .
            keyboardMap[7] &= ~0x02; // Sym
            keyboardMap[7] &= ~0x04; // M
            break;
            
        case 48: // Edit
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[3] &= ~0x01; // 1
            break;
            
        case 50: // Graph
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[4] &= ~0x02; // 9
            break;
            
        case 53: // Break
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[7] &= ~0x01; // Space
            break;
            
        case 51: // Backspace
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[4] &= ~0x01; // 0
            break;
            
        case 126: // Arrow up
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[4] &= ~0x08; // 7
            break;
            
        case 125: // Arrow down
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[4] &= ~0x10; // 6
            break;
            
        case 123: // Arrow left
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[3] &= ~0x10; // 5
            break;
            
        case 124: // Arrow right
            keyboardMap[0] &= ~0x01; // Shift
            keyboardMap[4] &= ~0x04; // 8
            break;
            
        default:
            for (int i = 0; i < sizeof(keyboardLookup) / sizeof(keyboardLookup[0]); i++)
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

void ZXSpectrum::keyUp(unsigned short key)
{
    switch (key)
    {
        case 30: // Inv Video
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[3] |= 0x08; // 4
            break;
            
        case 33: // True Video
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[3] |= 0x04; // 3
            break;
            
        case 39: // "
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[5] |= 0x01; // P
            break;
            
        case 41: // "
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[5] |= 0x02; // O
            break;
            
        case 43: // ,
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[7] |= 0x08; // M
            break;
            
        case 24: // +
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[6] |= 0x04; // K
            break;
            
        case 27: // -
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[6] |= 0x08; // J
            break;
            
        case 47: // .
            keyboardMap[7] |= 0x02; // Sym
            keyboardMap[7] |= 0x04; // N
            break;
            
        case 48: // Edit
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[3] |= 0x01; // 1
            break;
            
        case 50: // Graph
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[4] |= 0x02; // 9
            break;
            
        case 53: // Break
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[7] |= 0x01; // Space
            break;
            
        case 51: // Backspace
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[4] |= 0x01; // 0
            break;
            
        case 126: // Arrow up
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[4] |= 0x08; // 7
            break;
            
        case 125: // Arrow down
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[4] |= 0x10; // 6
            break;
            
        case 123: // Arrow left
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[3] |= 0x10; // 5
            break;
            
        case 124: // Arrow right
            keyboardMap[0] |= 0x01; // Shift
            keyboardMap[4] |= 0x04; // 8
            break;
            
        default:
            for (int i = 0; i < sizeof(keyboardLookup) / sizeof(keyboardLookup[0]); i++)
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

void ZXSpectrum::keyFlagsChanged(unsigned short key)
{
    
}

void ZXSpectrum::resetKeyboardMap()
{
    for (int i = 0; i < 8; i++)
    {
        keyboardMap[i] = 0xbf;
    }
}



