//
//  ZXSpectrum.hpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#ifndef ZXSpectrum_hpp
#define ZXSpectrum_hpp

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "Z80Core.h"
#include "MachineInfo.h"
#include "Tape.hpp"

using namespace std;

#pragma mark - Base ZXSpectrum class

class ZXSpectrum
{

public:
    static const int32_t    cBITMAP_ADDRESS = 16384;
    static const int32_t    cBITMAP_SIZE = 6144;
    static const int32_t    cATTR_SIZE = 768;
    static const int32_t    cMEMORY_PAGE_SIZE = 16384;
    
    enum
    {
        eAYREGISTER_A_FINE = 0,
        eAYREGISTER_A_COARSE,
        eAYREGISTER_B_FINE,
        eAYREGISTER_B_COARSE,
        eAYREGISTER_C_FINE,
        eAYREGISTER_C_COARSE,
        eAYREGISTER_NOISEPER,
        eAYREGISTER_ENABLE,
        eAYREGISTER_A_VOL,
        eAYREGISTER_B_VOL,
        eAYREGISTER_C_VOL,
        eAYREGISTER_E_FINE,
        eAYREGISTER_E_COARSE,
        eAYREGISTER_E_SHAPE,
        eAYREGISTER_PORT_A,
        eAYREGISTER_PORT_B,
        
        // Used to emulate the odd floating behaviour of setting an AY register > 15. The value
        // written to registers > 15 decays over time and this is the value returned when reading
        // a register > 15
        eAYREGISTER_FLOATING,
        
        eAY_MAX_REGISTERS
    };
    
private:    
    
    // Holds details of the host platforms key codes and how they map to the spectrum keyboard matrix
    typedef struct
    {
        int32_t     key;
        int32_t     mapEntry;
        int32_t     mapBit;
    } KEYBOARD_ENTRY;
    
public:
    
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct Snap {
        int32_t     length = 0;
        uint8_t     *data = nullptr;
    };
    
public:
    ZXSpectrum();
    virtual ~ZXSpectrum();

public:
    virtual void            initialise(string romPath);
    virtual void            resetMachine(bool hard = true);
    virtual void            resetToSnapLoad();
    void                    pause();
    void                    resume();
    virtual void            release();

    // Main function that when called, generates an entire frame, which includes processing interrupts, beeper sound and AY Sound
    // On completion, the displayBuffer member variable will contain RGBA formatted image data that can then be used to build a display image
    void                    generateFrame();
    
    void                    keyboardKeyDown(uint16_t key);
    void                    keyboardKeyUp(uint16_t key);
    void                    keyboardFlagsChanged(uint64_t flags, uint16_t key);
    
    
    bool                    snapshotZ80LoadWithPath(const char *path);
    bool                    snapshotSNALoadWithPath(const char *path);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    Snap                    snapshotCreateSNA();
    Snap                    snapshotCreateZ80();
    
protected:
    void                    emuReset();
    void                    loadROM(const char *rom, int page);
    
    void                    displayFrameReset();
    void                    displayUpdateWithTs(int32_t tStates);

    void                    ULAApplyIOContention(uint16_t address, bool contended);
    void                    ULABuildFloatingBusTable();
    uint8_t                 ULAFloatingBus();

    void                    audioAYSetRegister(uint8_t reg);
    void                    audioAYWriteData(uint8_t data);
    uint8_t                 audioAYReadData();
    void                    audioAYUpdate();
    void                    audioReset();
    void                    audioUpdateWithTs(int32_t tStates);
    
private:
    void                    displayBuildTsTable();
    void                    displayBuildLineAddressTable();
    void                    displayBuildCLUT();
    void                    ULABuildContentionTable();
    void                    audioBuildAYVolumesTable();
    void                    keyboardCheckCapsLockStatus();
    void                    keyboardMapReset();
    string                  snapshotHardwareTypeForVersion(uint32_t version, uint32_t hardwareType);
    void                    snapshotExtractMemoryBlock(uint8_t *fileBytes, uint32_t memAddr, uint32_t fileOffset, bool isCompressed, uint32_t unpackedLength);
    void                    displaySetup();
    void                    displayClear();
    void                    audioSetup(float sampleRate, float fps);
    
    // Core memory/IO functions
    static unsigned char    zxSpectrumMemoryRead(unsigned short address, void *param);
    static void             zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param);
    static void             zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param);
    static unsigned char    zxSpectrumDebugRead(unsigned int address, void *param, void *m);
    static void             zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data);
    static unsigned char    zxSpectrumIORead(unsigned short address, void *param);
    static void             zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param);

public:
    virtual unsigned char   coreMemoryRead(unsigned short address) = 0;
    virtual void            coreMemoryWrite(unsigned short address, unsigned char data) = 0;
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates) = 0;
    virtual unsigned char   coreIORead(unsigned short address) = 0;
    virtual void            coreIOWrite(unsigned short address, unsigned char data) = 0;

    virtual unsigned char   coreDebugRead(unsigned int address, void *data) = 0;
    virtual void            coreDebugWrite(unsigned int address, unsigned char byte, void *data) = 0;
        
    // Machine hardware
    CZ80Core                z80Core;
    
    vector<char>            memoryRom;
    vector<char>            memoryRam;
    
    uint8_t                 keyboardMap[8]{0};
    static KEYBOARD_ENTRY   keyboardLookup[];
    uint32_t                keyboardCapsLockFrames = 0;
    
    int16_t                 *audioBuffer = nullptr;
  
public:
    
    // Emulation
    MachineInfo             machineInfo;
    uint32_t                emuCurrentDisplayTs = 0;
    uint32_t                emuFrameCounter = 0;
    bool                    emuPaused = 0;
    uint32_t                emuRAMPage = 0;
    uint32_t                emuROMPage = 0;
    uint32_t                emuDisplayPage = 0;
    bool                    emuDisablePaging = true;
    string                  emuROMPath;
    bool                    emuTapeInstantLoad = 0;
    bool                    emuUseAYSound = 0;
    bool                    emuLoadTrapTriggered = 0;
    bool                    emuSaveTrapTriggered = 0;

    // Display
    uint8_t                 *displayBuffer = nullptr;
    uint32_t                displayBufferIndex = 0;
    uint32_t                screenWidth = 320;
    uint32_t                screenHeight = 256;
    uint32_t                screenBufferSize = 0;
    uint32_t                displayTstateTable[312][228]{0};
    uint32_t                displayLineAddrTable[192]{0};
    uint64_t                *displayCLUT = nullptr;
    uint8_t                 *displayALUT = nullptr;
    uint32_t                displayBorderColor = 0;
    bool                    displayReady = false;

    // Audio
    int32_t                 audioEarBit = 0;
    int32_t                 audioMicBit = 0;
    uint32_t                audioBufferSize = 0;
    uint32_t                audioBufferIndex = 0;
    float                   audioTsCounter = 0;
    float                   audioTsStepCounter = 0;

    float                   audioBeeperTsStep = 0;
    float                   audioBeeperLeft = 0;
    float                   audioBeeperRight = 0;

    float                   audioAYChannelOutput[3]{0};
    uint32_t                audioAYChannelCount[3]{0};
    uint16_t                audioAYVolumes[16]{0};
    uint32_t                audioAYrandom = 0;
    uint32_t                audioAYOutput = 0;
    uint32_t                audioAYNoiseCount = 0;
    uint32_t                audioATaudioAYEnvelopeCount = 0;
    int32_t                 audioAYaudioAYEnvelopeStep = 0;
    uint8_t                 audioAYRegisters[ eAY_MAX_REGISTERS ]{0};
    uint8_t                 audioAYCurrentRegister = 0;
    uint8_t                 audioAYFloatingRegister = 0;
    bool                    audioAYaudioAYaudioAYEnvelopeHolding = 0;
    bool                    audioAYaudioAYEnvelopeHold = 0;
    bool                    audioAYaudioAYEnvelopeAlt = 0;
    bool                    audioAYEnvelope = 0;
    uint32_t                audioAYAttackEndVol = 0;
    float                   audioAYTsStep = 0;
    float                   audioAYTs = 0;
        
    // Keyboard
    bool                    keyboardCapsLockPressed = false;
    
    // ULA
    uint32_t                ULAMemoryContentionTable[80000]{0};
    uint32_t                ULAIOContentionTable[80000]{0};
    uint32_t                ULAFloatingBusTable[80000]{0};
    const static uint32_t   ULAConentionValues[];
    int32_t                 ULAPortFFFDValue = 0;

    // Floating bus
    const static uint32_t   ULAFloatingBusValues[];
    
    // Tape object
    Tape                    *tape = nullptr;
    
    // SPI port
    unsigned short          spiPort = 0xfaf7;

};

#endif /* ZXSpectrum_hpp */






