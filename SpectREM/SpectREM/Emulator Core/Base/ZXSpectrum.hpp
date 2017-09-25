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
    static const int    cBITMAP_ADDRESS = 16384;
    static const int    cBITMAP_SIZE = 6144;
    static const int    cATTR_SIZE = 768;
    static const int    cMEMORY_PAGE_SIZE = 16384;

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
        int                 key;
        int                 mapEntry;
        int                 mapBit;
    } KEYBOARD_ENTRY;
    
public:
    
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct Snap {
        int length;
        unsigned char *data;
    };
    
    struct ScreenBufferData {
        unsigned int pixels;
        unsigned int attribute;
    };
        
public:
    ZXSpectrum();
    virtual ~ZXSpectrum();

public:
    virtual void            initialise(string romPath);
    virtual void            resetMachine(bool hard = true);
    void                    pause();
    void                    resume();
    virtual void            release();

    void                    generateFrame();
    
    void                    keyboardKeyDown(unsigned short key);
    void                    keyboardKeyUp(unsigned short key);
    void                    keyboardFlagsChanged(unsigned long flags, unsigned short key);
    
    
    bool                    snapshotZ80LoadWithPath(const char *path);
    bool                    snapshotSNALoadWithPath(const char *path);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    Snap                    snapshotCreateSNA();
    Snap                    snapshotCreateZ80();
    
protected:
    void                    emuReset();
    virtual void            loadDefaultROM() = 0;
    
    void                    displayFrameReset();
    void                    displayUpdateWithTs(int tStates);

    void                    ULAApplyIOContention(unsigned short address, bool contended);
    unsigned char           ULAFloatingBus();

    void                    audioAYSetRegister(unsigned char reg);
    void                    audioAYWriteData(unsigned char data);
    unsigned char           audioAYReadData();
    void                    audioAYUpdate();
    void                    audioReset();
    void                    audioUpdateWithTs(int tStates);
    
private:
    void                    displayBuildTsTable();
    void                    displayBuildLineAddressTable();
    void                    displayBuildCLUT();
    void                    ULABuildContentionTable();
    void                    audioBuildAYVolumesTable();
    void                    keyboardCheckCapsLockStatus();
    void                    keyboardMapReset();
    string                  snapshotHardwareTypeForVersion(int version, int hardwareType);
    void                    snapshotExtractMemoryBlock(unsigned char *fileBytes, int memAddr, int fileOffset, bool isCompressed, int unpackedLength);
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
    
    unsigned char           keyboardMap[8]{0};
    static KEYBOARD_ENTRY   keyboardLookup[];
    int                     keyboardCapsLockFrames = 0;
    
    unsigned short          *audioBuffer = nullptr;
  
public:
    
    // Emulation
    MachineInfo             machineInfo;
    int                     emuCurrentDisplayTs = 0;
    int                     emuFrameCounter = 0;
    bool                    emuPaused = 0;
    int                     emuRAMPage = 0;
    int                     emuROMPage = 0;
    int                     emuDisplayPage = 0;
    bool                    emuDisablePaging = true;
    string                  emuROMPath = "";
    bool                    emuTapeInstantLoad = 0;
    bool                    emuUseAYSound = 0;
    bool                    emuLoadTrapTriggered = 0;
    bool                    emuSaveTrapTriggered = 0;

    // Display
    unsigned int            *displayBuffer = nullptr;
    ScreenBufferData        *displayBufferCopy = nullptr;
    unsigned int            displayBufferIndex = 0;
    int                     screenWidth = 320;
    int                     screenHeight = 256;
    int                     screenBufferSize = 0;
    int                     displayTstateTable[312][228]{0};
    int                     displayLineAddrTable[192]{0};
    unsigned int            displayCLUT[ 256 * 1024 ]{0};
    int                     displayBorderColor = 0;

    // Audio
    int                     audioEarBit = 0;
    int                     audioMicBit = 0;
    int                     audioBufferSize = 0;
    int                     audioBufferIndex = 0;
    int                     audioTsCounter = 0;
    float                   audioTsStepCounter = 0;

    float                   audioBeeperTsStep = 0;
    float                   audioBeeperLeft = 0;
    float                   audioBeeperRight = 0;

    int                     audioAYChannelOutput[3]{0};
    unsigned int            audioAYChannelCount[3]{0};
    unsigned short          audioAYVolumes[16]{0};
    unsigned int            audioAYrandom = 0;
    unsigned int            audioAYOutput = 0;
    unsigned int            audioAYNoiseCount = 0;
    unsigned int            audioATaudioAYEnvelopeCount = 0;
    int                     audioAYaudioAYEnvelopeStep = 0;
    unsigned char           audioAYRegisters[ eAY_MAX_REGISTERS ]{0};
    unsigned char           audioAYCurrentRegister = 0;
    unsigned char           audioAYFloatingRegister = 0;
    bool                    audioAYaudioAYaudioAYEnvelopeHolding = 0;
    bool                    audioAYaudioAYEnvelopeHold = 0;
    bool                    audioAYaudioAYEnvelopeAlt = 0;
    bool                    audioAYEnvelope = 0;
    unsigned int            audioAYAttackEndVol = 0;
    float                   audioAYTsStep = 0;
    int                     audioAYTs = 0;
        
    // Keyboard
    bool                    keyboardCapsLockPressed = false;
    
    // ULA
    unsigned int            ULAMemoryContentionTable[80000]{0};
    unsigned int            ULAIOContentionTable[80000]{0};
    const static unsigned int ULAConentionValues[];
    int                     ULAPortFFFDValue = 0;

    // Floating bus
    const static unsigned int ULAFloatingBusValues[];
    
    // Tape object
    Tape                    *tape = nullptr;

};


#endif /* ZXSpectrum_hpp */






