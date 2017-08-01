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

#include "Z80Core.h"

using namespace std;

ZXSpectrum48::~ZXSpectrum48()
{
    cout << "ZXSpectrum48::Destructor" << endl;
    
    release();
}

ZXSpectrum48::ZXSpectrum48()
{
    cout << "ZXSpectrum48::Constructor" << endl;
    
    reset();
}

#pragma mark - Initialise

void ZXSpectrum48::initialise(char *rom)
{
    cout << "ZXSpectrum48::initialise" << endl;

    memory.resize(64 * 1024);
    
    std::ifstream romFile(rom, std::ios::binary|std::ios::ate);
    romFile.seekg(0, std::ios::beg);
    romFile.read(memory.data(), memory.size());
    
    core.Initialise(zxSpectrumMemoryRead,
                    zxSpectrumMemoryWrite,
                    zxSpectrumIORead,
                    zxSpectrumIOWrite,
                    zxSpectrumMemoryContention,
                    zxSpectrumDebugRead,
                    zxSpectrumDebugWrite,
                    this);
}

#pragma mark - Generate a frame

void ZXSpectrum48::runFrame()
{
    int tStates = 0;
    
    while (tStates < 69888)
    {
        tStates += core.Execute();
    }
    
    core.SignalInterrupt();
}

#pragma mark - Reset

void ZXSpectrum48::reset()
{
    core.Reset();
}

#pragma mark - Memory Access

unsigned char ZXSpectrum48::zxSpectrumMemoryRead(unsigned short address, void *param)
{
    return ((ZXSpectrum48 *) param)->coreMemoryRead(address);
}

unsigned char ZXSpectrum48::coreMemoryRead(unsigned short address)
{
    return memory[address];
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
    
    memory[address] = data;
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
    return memory[address];
}

void ZXSpectrum48::zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data)
{
    ((ZXSpectrum48 *) param)->coreDebugWrite(address, byte, data);
}

void ZXSpectrum48::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
{
    
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
