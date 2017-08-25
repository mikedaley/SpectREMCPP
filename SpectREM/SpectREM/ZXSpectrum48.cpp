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
    
    machineInfo = machines[ eZXSpectrum48 ];

    ZXSpectrum::initialise(romPath);
    
    displayPage = 1;
}

#pragma mark - ULA

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
{
    unsigned char result = ZXSpectrum::coreIORead(address);
    
    return result;
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    // Port: 0xFE
    //   7   6   5   4   3   2   1   0
    // +---+---+---+---+---+-----------+
    // |   |   |   | E | M |  BORDER   |
    // +---+---+---+---+---+-----------+
    if (!(address & 0x01))
    {
        updateScreenWithTstates((z80Core.GetTStates() - currentDisplayTstates) + machineInfo.borderDrawingOffset);
        audioEarBit = (data & 0x10) >> 4;
        audioMicBit = (data & 0x08) >> 3;
        borderColor = data & 0x07;
    }

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



