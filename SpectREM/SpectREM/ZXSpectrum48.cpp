//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum48.hpp"

#include <iostream>
#include <fstream>

using namespace std;

#pragma mark - Constants 

// Size the of the ZX Spectrum ROM in bytes
const size_t ROM_SIZE = 16383;

// Size of the ZX Spectrum RAM in bytes
const size_t RAM_SIZE = 48 * 1024;

// Number of tStates per frame
const size_t TSTATES_PER_FRAME = 69888;

// Screen buffer size
const size_t SCREEN_BUFFER_SIZE = (256 * 192) * 4;

#pragma mark - Constructor/Deconstructor

ZXSpectrum48::ZXSpectrum48()
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
    cout << "ZXSpectrum48::initialise" << endl;
    
    memoryRom.resize(ROM_SIZE);
    memoryRam.resize(RAM_SIZE);

    loadRomWithPath(rom);

    z80Core.Initialise(zxSpectrumMemoryRead,
                       zxSpectrumMemoryWrite,
                       zxSpectrumIORead,
                       zxSpectrumIOWrite,
                       zxSpectrumMemoryContention,
                       zxSpectrumDebugRead,
                       zxSpectrumDebugWrite,
                       this);
    
    display = (unsigned int *)calloc(SCREEN_BUFFER_SIZE, sizeof(unsigned int));
}

void ZXSpectrum48::loadRomWithPath(char *romPath)
{
    std::ifstream romFile(romPath, std::ios::binary|std::ios::ate);
    romFile.seekg(0, std::ios::beg);
    romFile.read(memoryRom.data(), memoryRom.size());
}

#pragma mark - Generate a frame

void ZXSpectrum48::runFrame()
{
    size_t tStates = 0;
    
    while (tStates < TSTATES_PER_FRAME)
    {
        tStates += z80Core.Execute();
    }
    
    z80Core.SignalInterrupt();
    
    generateScreen();
}

#pragma mark - Reset

void ZXSpectrum48::reset()
{
    z80Core.Reset();
}

#pragma mark - Generate Screen

void ZXSpectrum48::generateScreen()
{
    size_t displayIndex = 0;
    
    for (int y = 0; y < 192; y++)
    {
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
    }
}

#pragma mark - Memory Access

unsigned char ZXSpectrum48::zxSpectrumMemoryRead(unsigned short address, void *param)
{
    return ((ZXSpectrum48 *) param)->coreMemoryRead(address);
}

unsigned char ZXSpectrum48::coreMemoryRead(unsigned short address)
{
    if (address < 0x4000)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - ROM_SIZE];
}

void ZXSpectrum48::zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum48 *) param)->coreMemoryWrite(address, data);
}

void ZXSpectrum48::coreMemoryWrite(unsigned short address, unsigned char data)
{
    if (address < 0x4000)
    {
        return;
    }
    
    memoryRam[address - ROM_SIZE] = data;
}

void ZXSpectrum48::zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param)
{
    
}

void ZXSpectrum48::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    
}

unsigned char ZXSpectrum48::zxSpectrumDebugRead(unsigned int address, void *param, void *data)
{
    return ((ZXSpectrum48 *) param)->coreDebugRead(address, data);
}

unsigned char ZXSpectrum48::coreDebugRead(unsigned int address, void *data)
{
    if (address < 0x4000)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address - ROM_SIZE];
}

void ZXSpectrum48::zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data)
{
    ((ZXSpectrum48 *) param)->coreDebugWrite(address, byte, data);
}

void ZXSpectrum48::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
{
    if (address < 0x4000)
    {
        memoryRom[address] = byte;
    }
    else
    {
        memoryRam[address] = byte;
    }
}

#pragma mark - IO Access

unsigned char ZXSpectrum48::zxSpectrumIORead(unsigned short address, void *param)
{
    return ((ZXSpectrum48 *) param)->coreIORead(address);
}

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
{
    return 0xff;
}

void ZXSpectrum48::zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param)
{
    ((ZXSpectrum48 *) param)->coreIOWrite(address, data);
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    
}

#pragma mark - Release

void ZXSpectrum48::release()
{

}
