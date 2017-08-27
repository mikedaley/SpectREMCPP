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
    bool contended = false;
    int memoryPage = address / 16384;
    if (memoryPage == 1)
    {
        contended = true;
    }
    
    ZXSpectrum::applyIOContention(address, contended);
        
    // ULA Un-owned ports
    if (address & 0x01)
    {
        // Add Kemptston joystick support. Until then return 0. Byte returned by a Kempston joystick is in the
        // format: 000FDULR. F = Fire, D = Down, U = Up, L = Left, R = Right
        // Joystick is read first as it takes priority if you read from a port that activates the keyboard as well on a
        // real machine.
        if ((address & 0xff) == 0x1f)
        {
            return 0x0;
        }
        
        // Getting here means that nothing has handled that port read so based on a real Spectrum
        // return the floating bus value
        return floatingBus();
    }

    // The base classes virtual function deals with owned ULA ports such as the keyboard ports
    unsigned char result = ZXSpectrum::coreIORead(address);
    
    return (result & 191);
}

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    bool contended = false;
    int memoryPage = address / 16384;
    if (memoryPage == 1)
    {
        contended = true;
    }
    
    ZXSpectrum::applyIOContention(address, contended);

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
    if (address >= 16384 && address <= 32767)
    {
        z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
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



