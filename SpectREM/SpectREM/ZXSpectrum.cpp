//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"

#include <iostream>
#include <fstream>

#include "Z80Core.h"

using namespace std;

#pragma mark - Constants

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

#pragma mark - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
    cout << "ZXSpectrum::Constructor" << endl;
}

ZXSpectrum::~ZXSpectrum()
{
    cout << "ZXSpectrum::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum::initialise(char *romPath)
{
    cout << "ZXSpectrum::initialise(char *romPath)" << endl;
    
    z80Core.Initialise(zxSpectrumMemoryRead,
                       zxSpectrumMemoryWrite,
                       zxSpectrumIORead,
                       zxSpectrumIOWrite,
                       zxSpectrumMemoryContention,
                       zxSpectrumDebugRead,
                       zxSpectrumDebugWrite,
                       this);
    
    resetKeyboardMap();
    loadRomWithPath(romPath);
}

void ZXSpectrum::loadRomWithPath(char *romPath)
{
    std::ifstream romFile(romPath, std::ios::binary|std::ios::ate);
    romFile.seekg(0, std::ios::beg);
    romFile.read(memoryRom.data(), memoryRom.size());
}

#pragma mark - Generate a frame

void ZXSpectrum::runFrame()
{
    size_t count = tstatesPerFrame;
    
    while (count > 0)
    {
        int tStates = z80Core.Execute(1, 32);
        count -= tStates;
        
        if (z80Core.GetTStates() >= tstatesPerFrame)
        {
            count = 0;

            z80Core.ResetTStates( (unsigned int)tstatesPerFrame );
            z80Core.SignalInterrupt();
            generateScreen();
        }
    }
}

#pragma mark - Generate Screen

void ZXSpectrum::generateScreen()
{
    size_t displayIndex = 0;
    
    for (int x = 0; x < screenWidth * borderSize; x++)
    {
        display[displayIndex++] = 0xffbbbbbb;
    }
    
    for (int y = 0; y < 192; y++)
    {
        for (int x = 0; x < borderSize; x++)
        {
            display[displayIndex++] = 0xffbbbbbb;
        }
        
        for (int x = 0; x < 256; x++)
        {
            int address = (x >> 3) + ((y & 0x07) << 8) + ((y & 0x38) << 2) + ((y & 0xc0) << 5);
            unsigned char byte = memoryRam[address];
            
            if (byte & (0x80 >> (x & 7)))
            {
                display[displayIndex++] = 0xff000000;
            }
            else
            {
                display[displayIndex++] = 0xffbbbbbb;
            }
        }

        for (int x = 0; x < borderSize; x++)
        {
            display[displayIndex++] = 0xffbbbbbb;
        }
    }

    for (int x = 0; x < screenWidth * borderSize; x++)
    {
        display[displayIndex++] = 0xffbbbbbb;
    }

}

#pragma mark - Memory Access

unsigned char ZXSpectrum::zxSpectrumMemoryRead(unsigned short address, void *param)
{
    return ((ZXSpectrum *) param)->coreMemoryRead(address);
}

void ZXSpectrum::zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum *) param)->coreMemoryWrite(address, data);
}

void ZXSpectrum::zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param)
{
    // Nothing to see here
}

unsigned char ZXSpectrum::zxSpectrumDebugRead(unsigned int address, void *param, void *data)
{
    return ((ZXSpectrum *) param)->coreDebugRead(address, data);
}

void ZXSpectrum::zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data)
{
    ((ZXSpectrum *) param)->coreDebugWrite(address, byte, data);
}

unsigned char ZXSpectrum::coreMemoryRead(unsigned short address)
{
    if (address < romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - romSize];
}

void ZXSpectrum::coreMemoryWrite(unsigned short address, unsigned char data)
{
    if (address < romSize)
    {
        return;
    }
    
    memoryRam[address - romSize] = data;
}

void ZXSpectrum::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    // Nothing to see here
}

unsigned char ZXSpectrum::coreDebugRead(unsigned int address, void *data)
{
    if (address < romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - romSize];
}

void ZXSpectrum::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
{
    if (address < romSize)
    {
        memoryRom[address] = byte;
    }
    else
    {
        memoryRam[address] = byte;
    }
}

#pragma mark - IO Access

unsigned char ZXSpectrum::zxSpectrumIORead(unsigned short address, void *param)
{
    return ((ZXSpectrum *) param)->coreIORead(address);
}

void ZXSpectrum::zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum *) param)->coreIOWrite(address, data);
}

unsigned char ZXSpectrum::coreIORead(unsigned short address)
{
    return 0xff;
}

void ZXSpectrum::coreIOWrite(unsigned short address, unsigned char data)
{
    // Nothing to see here
}

#pragma mark - Reset

void ZXSpectrum::reset()
{
    z80Core.Reset();
}

#pragma mark - Release

void ZXSpectrum::release()
{
    delete display;
}

#pragma mark - Keyboard

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
                if (keyboardLookup[i].vkey == key)
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
                if (keyboardLookup[i].vkey == key)
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




