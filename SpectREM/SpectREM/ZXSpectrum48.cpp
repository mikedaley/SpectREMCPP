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

void ZXSpectrum48::initialise(char *rom)
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

    ZXSpectrum::loadRomWithPath(rom);
    ZXSpectrum::initialise();
}

#pragma mark - ULA

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
{
    return 0xff;
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    
}

#pragma mark - Memory

void ZXSpectrum48::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    
}

#pragma mark - Release

void ZXSpectrum48::release()
{
    ZXSpectrum::release();
}

#pragma mark - Reset

void ZXSpectrum48::reset()
{
    ZXSpectrum::reset();
}
