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
    ZXSpectrum::release();
}

#pragma mark - Initialise

void ZXSpectrum48::initialise(char *rom)
{
    cout << "ZXSpectrum48::initialise" << endl;
    
    romSize = 16 * 1024;
    ramSize = 48 * 1024;
    tstatesPerFrame = 69888;
    borderSize = 32;
    screenWidth = borderSize + 256 + borderSize;
    screenHeight = borderSize + 192 + borderSize;
    screenBufferSize = screenHeight * screenWidth;
    
    memoryRom.resize(romSize);
    memoryRam.resize(ramSize);
    
    ZXSpectrum::loadRomWithPath(rom);
        
    display = new unsigned int[screenBufferSize];
    
    ZXSpectrum::initialise();
}

#pragma mark - Memory

unsigned char ZXSpectrum48::coreMemoryRead(unsigned short address)
{
    if (address < romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - romSize];
}

void ZXSpectrum48::coreMemoryWrite(unsigned short address, unsigned char data)
{
    if (address < romSize)
    {
        return;
    }
    
    memoryRam[address - romSize] = data;
}

void ZXSpectrum48::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    
}

#pragma mark - Memory Debug

unsigned char ZXSpectrum48::coreDebugRead(unsigned int address, void *data)
{
    if (address < romSize)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - romSize];
}

void ZXSpectrum48::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
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

#pragma mark - IO

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
{
    return 0xff;
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
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
