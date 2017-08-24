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

void ZXSpectrum::initialise()
{
    cout << "ZXSpectrum::initialise" << endl;
    
    z80Core.Initialise(zxSpectrumMemoryRead,
                       zxSpectrumMemoryWrite,
                       zxSpectrumIORead,
                       zxSpectrumIOWrite,
                       zxSpectrumMemoryContention,
                       zxSpectrumDebugRead,
                       zxSpectrumDebugWrite,
                       this);
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
    size_t tStates = 0;
    
    while (tStates < tstatesPerFrame)
    {
        tStates += z80Core.Execute();
    }
    
    z80Core.SignalInterrupt();
    
    generateScreen();
}

#pragma mark - Generate Screen

void ZXSpectrum::generateScreen()
{
    size_t displayIndex = screenWidth * borderSize;
    
    for (int y = 0; y < 192; y++)
    {
        displayIndex += borderSize;
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
        displayIndex += borderSize;
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
