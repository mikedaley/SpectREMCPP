//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"

const float cSAMPLE_RATE = 44100;
const float cFPS = 50;
const float cROM_SIZE = 16384;
const char *cSMART_ROM = "smartload.v31";


#pragma mark - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
    cout << "ZXSpectrum::Constructor" << endl;
    
    displayCLUT = new uint64_t[32 * 1024];
    displayALUT = new uint8_t[256];
}

ZXSpectrum::~ZXSpectrum()
{
    cout << "ZXSpectrum::Destructor" << endl;
    
    delete [] displayCLUT;
    delete [] displayALUT; 
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

    audioSetup(cSAMPLE_RATE, cFPS);
    audioBuildAYVolumesTable();
    
    resetMachine(true);
}

void ZXSpectrum::registerDebugOpCallback(DebugOpCallbackBlock debugOpCallbackBlock)
{
    this->debugOpCallbackBlock = debugOpCallbackBlock;
}

#pragma mark - Generate a frame

void ZXSpectrum::generateFrame()
{
    uint32_t currentFrameTstates = machineInfo.tsPerFrame;
    
    while (currentFrameTstates > 0 && !emuPaused && !breakpointHit)
    {
        if (debugOpCallbackBlock)
        {
            if( debugOpCallbackBlock( z80Core.GetRegister(CZ80Core::eREG_PC), eDebugExecuteOp ) || emuPaused)
            {
                return;
            }
        }
        
        int tStates = z80Core.Execute(1, machineInfo.intLength);
                
        if (tape && tape->playing)
        {
            tape->updateWithTs(tStates);
        }
        
        if (tape && emuSaveTrapTriggered)
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
                
                audioDecayAYFloatingRegister();
            }
        }
    }
}

#pragma mark - Debug

void ZXSpectrum::step()
{
    int tStates = z80Core.Execute(1, machineInfo.intLength);
    
    if (tape && tape->playing)
    {
        tape->updateWithTs(tStates);
    }
    
    if (tape && emuSaveTrapTriggered)
    {
        tape->saveBlock(this);
    }
    else if (emuLoadTrapTriggered && tape && tape->loaded)
    {
        tape->loadBlock(this);
    }
    else
    {
        if (z80Core.GetTStates() >= machineInfo.tsPerFrame)
        {
            z80Core.ResetTStates( machineInfo.tsPerFrame );
            z80Core.SignalInterrupt();
            
            emuFrameCounter++;
            
            displayFrameReset();
            keyboardCheckCapsLockStatus();
            
            audioDecayAYFloatingRegister();
        }
    }

    displayUpdateWithTs(machineInfo.tsPerFrame - emuCurrentDisplayTs);
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
        for (int i = 0; i < (int)machineInfo.ramSize; i++)
        {
            memoryRam[i] = rand() % 255;
        }
    }
    
    delete [] displayBuffer;
    
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

#pragma mark - ROM Loading

void ZXSpectrum::loadROM(const char *rom, int page)
{
    size_t romAddress = cROM_SIZE * page;
    
    if (memoryRom.size() < romAddress)
    {
        cout << "ZXSpectrum::loadROM - Unable to load into ROM page " << page << endl;
        exit(1);
    }
    
    string romPath = emuROMPath;
    romPath.append( rom );
    
    ifstream romFile(romPath, ios::binary|ios::ate);
    size_t fileSize = romFile.tellg();
    romFile.seekg(0, ios::beg);
    romFile.read(memoryRom.data() + romAddress, fileSize);
    romFile.close();
}

#pragma mark - Release

void ZXSpectrum::release()
{
    delete[] displayBuffer;
    delete[] audioBuffer;
}





