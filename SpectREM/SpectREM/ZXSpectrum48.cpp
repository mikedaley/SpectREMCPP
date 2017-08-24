//
//  ZXSpectrum48.cpp
//  SpectREM
//
//  Created by Michael Daley on 23/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum48.hpp"

#include <iostream>
#include <fstream>

#pragma mark - Constructor/Destructor

ZXSpectrum48::ZXSpectrum48() : ZXSpectrum()
{
    cout << "ZXSpectrum48::Constructor" << endl;
}

ZXSpectrum48::~ZXSpectrum48()
{
    cout << "ZXSpectrum48::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum48::initialise(char *romPath)
{
    cout << "ZXSpectrum48::initialise(char *rom)" << endl;
    
    romSize = 16 * 1024;
    ramSize = 48 * 1024;
    tstatesPerFrame = 69888;
    borderSize = 32;
    screenWidth = borderSize + 256 + borderSize;
    screenHeight = borderSize + 192 + borderSize;
    screenBufferSize = screenHeight * screenWidth;
    
    memoryRom.resize(romSize);
    memoryRam.resize(ramSize);
    
    display = new unsigned int[screenBufferSize];

    ZXSpectrum::initialise(romPath);
}

#pragma mark - ULA

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
{
    unsigned char result = 0xff;
    
    // Check to see if the keyboard is being read and if so return any keys currently pressed
    if (address & 0xfe)
    {
        for (int i = 0; i < 8; i++)
        {
            if (!(address & (0x100 << i)))
            {
                result &= keyboardMap[i];
            }
        }
    }

    return result;
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    cout << "IO Write" << endl;
}

#pragma mark - Memory Contention

void ZXSpectrum48::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    
}

#pragma mark - Release/Reset

void ZXSpectrum48::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum48::reset()
{
    ZXSpectrum::reset();
}
