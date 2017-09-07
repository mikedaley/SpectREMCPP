//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
    cout << "ZXSpectrum::Constructor" << endl;
    displayFrameReset();
    audioReset();
    tapeReset(true);
}

ZXSpectrum::~ZXSpectrum()
{
    cout << "ZXSpectrum::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum::initialise(string romPath)
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
    
    screenWidth = machineInfo.pxEmuBorder + machineInfo.pxHorizontalDisplay + machineInfo.pxEmuBorder;
    screenHeight = machineInfo.pxEmuBorder + machineInfo.pxVerticalDisplay + machineInfo.pxEmuBorder;
    screenBufferSize = screenHeight * screenWidth;
    emuROMPath = romPath;
    
    
    memoryRom.resize( machineInfo.romSize );
    memoryRam.resize( machineInfo.ramSize );
    
    displaySetup();
    displayBuildLineAddressTable();
    displayBuildTsTable();
    
    ULABuildContentionTable();

    audioSetup(192000, 50);
    audioBuildAYVolumesTable();
    
    emuReset();
    displayFrameReset();
    audioReset();
    keyboardMapReset();
    tapeReset(true);
}

void ZXSpectrum::loadDefaultROM()
{

}

#pragma mark - Generate a frame

void ZXSpectrum::generateFrame()
{
    int currentFrameTstates = machineInfo.tsPerFrame;
    
    while (currentFrameTstates > 0 && !emuPaused)
    {
        int tStates = z80Core.Execute(1, machineInfo.intLength);
        
        if (tapePlaying)
        {
            tapeUpdateWithTs(tStates);
        }
        
        if (emuSaveTrapTriggered)
        {
            
        }
        else if (emuLoadTrapTriggered && tapeLoaded)
        {
            tapeLoadBlock();
        }
        else
        {
            currentFrameTstates -= tStates;
            
            audioUpdateWithTs(tStates);
            
            if (z80Core.GetTStates() >= machineInfo.tsPerFrame)
            {
                z80Core.ResetTStates( machineInfo.tsPerFrame );
                z80Core.SignalInterrupt();
                
                displayUpdateWithTs(machineInfo.tsPerFrame - emuCurrentDisplayTs);
                
                emuFrameCounter++;
                
                displayFrameReset();
                keyboardCheckCapsLockStatus();
                
                currentFrameTstates = 0;
            }
        }
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
    ((ZXSpectrum *) param)->coreMemoryContention(address, tStates);
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
    return 0;
}

void ZXSpectrum::coreMemoryWrite(unsigned short address, unsigned char data) { }

void ZXSpectrum::coreMemoryContention(unsigned short address, unsigned int tStates) { }

unsigned char ZXSpectrum::coreDebugRead(unsigned int address, void *data) { return 0; }

void ZXSpectrum::coreDebugWrite(unsigned int address, unsigned char byte, void *data) { }

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
    // Check to see if the keyboard is being read and if so return any keys currently pressed
    unsigned char result = 0xff;
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
    
    result = (result & 191) | (audioEarBit << 6) | (tapeInputBit << 6);
    
    return result;
}

void ZXSpectrum::coreIOWrite(unsigned short address, unsigned char data)
{
    // Nothing to see here. Most likely implemented in the 
}

#pragma mark - Pause/Resume

void ZXSpectrum::pause()
{
    emuPaused = true;
}

void ZXSpectrum::resume()
{
    emuPaused = false;
}

#pragma mark - Reset

void ZXSpectrum::resetMachine(bool hard)
{
    if (hard)
    {
        for (int i = 0; i < machineInfo.ramSize; i++)
        {
            memoryRam[i] = arc4random_uniform(255);
        }
    }
    
    z80Core.Reset(hard);
    keyboardMapReset();
    displayFrameReset();
    audioReset();
    tapeReset(true);
    emuFrameCounter = 0;
}

void ZXSpectrum::emuReset()
{
    emuSaveTrapTriggered = false;
    emuLoadTrapTriggered = false;
    emuTapeInstantLoad = false;
}

#pragma mark - Release

void ZXSpectrum::release()
{
    delete displayBuffer;
    delete displayBufferCopy;
    delete audioBuffer;
    delete audioQueueBuffer;
}





