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
    
    displayCLUT = new uint64_t [32 * 1024];
}

ZXSpectrum::~ZXSpectrum()
{
    cout << "ZXSpectrum::Destructor" << endl;
    
    delete [] displayCLUT;
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
    
    emuROMPath = romPath;

    screenWidth = machineInfo.pxEmuBorder + machineInfo.pxHorizontalDisplay + machineInfo.pxEmuBorder;
    screenHeight = machineInfo.pxEmuBorder + machineInfo.pxVerticalDisplay + machineInfo.pxEmuBorder;
    screenBufferSize = screenHeight * screenWidth;
    
    memoryRom.resize( machineInfo.romSize );
    memoryRam.resize( machineInfo.ramSize );

    displaySetup();
    displayBuildLineAddressTable();
    displayBuildTsTable();
    displayBuildCLUT();
    
    ULABuildContentionTable();

    audioSetup(192000, 50);
    audioBuildAYVolumesTable();
    
    resetMachine(true);
}

#pragma mark - Generate a frame

void ZXSpectrum::generateFrame()
{
    
    int currentFrameTstates = machineInfo.tsPerFrame;
    
    while (currentFrameTstates > 0 && !emuPaused)
    {
        int tStates = z80Core.Execute(1, machineInfo.intLength);
        
        if (tape && tape->playing)
        {
            tape->updateWithTs(tStates);
        }
        
        if (emuSaveTrapTriggered)
        {
            tape->saveBlock(this);
        }
        else if (emuLoadTrapTriggered && tape && tape->loaded)
        {
            tape->loadBlock(this);
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
        
        // Decay the floating AY value provided when registers > 15 are read
        audioAYRegisters[ eAYREGISTER_FLOATING ] >>= 1;
    }
}

#pragma mark - Memory Access

unsigned char ZXSpectrum::zxSpectrumMemoryRead(unsigned short address, void *param)
{
    return static_cast<ZXSpectrum *>(param)->coreMemoryRead(address);
}

void ZXSpectrum::zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param)
{
    static_cast<ZXSpectrum *>(param)->coreMemoryWrite(address, data);
}

void ZXSpectrum::zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param)
{
    static_cast<ZXSpectrum *>(param)->coreMemoryContention(address, tStates);
}

unsigned char ZXSpectrum::zxSpectrumDebugRead(unsigned int address, void *param, void *data)
{
    return static_cast<ZXSpectrum *>(param)->coreDebugRead(address, data);
}

void ZXSpectrum::zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data)
{
    static_cast<ZXSpectrum *>(param)->coreMemoryWrite(address, byte);
}

#pragma mark - IO Access

unsigned char ZXSpectrum::zxSpectrumIORead(unsigned short address, void *param)
{
    return static_cast<ZXSpectrum *>(param)->coreIORead(address);
}

void ZXSpectrum::zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param)
{
    static_cast<ZXSpectrum *>(param)->coreIOWrite(address, data);
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
            memoryRam[i] = rand() % 255;
        }
    }
    
    delete [] displayBuffer;
    delete [] displayBufferCopy;
    
    displaySetup();
    
    z80Core.Reset(hard);
    emuReset();
    keyboardMapReset();
    displayFrameReset();
    audioReset();
}

void ZXSpectrum::emuReset()
{
    emuFrameCounter = 0;
    emuSaveTrapTriggered = false;
    emuLoadTrapTriggered = false;
}

#pragma mark - Release

void ZXSpectrum::release()
{
    delete[] displayBuffer;
    delete[] displayBufferCopy;
    delete[] audioBuffer;
}





