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

#include "Z80Core.h"
#include "MachineDetails.h"

using namespace std;

class ZXSpectrum
{

public:
    static const int        cBITMAP_ADDRESS = 16384;
    static const int        cBITMAP_SIZE = 6144;
    static const int        cATTR_SIZE = 768;

private:
    enum
    {
        eDisplayBorder = 1,
        eDisplayPaper,
        eDisplayRetrace
    };
    
    enum
    {
        eFloatBusTypeValuePixel = 1,
        eFloatBusTypeValueAttribute
    };
    
    // Holds details of the host platforms key codes and how they map to the spectrum keyboard matrix
    typedef struct
    {
        int                 key;
        int                 mapEntry;
        int                 mapBit;
    } KEYBOARD_ENTRY;
    
public:
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct snap {
        int length;
        unsigned char *data;
    };
    
    // Previous frames screen buffer contents
    struct ScreenBufferData {
        unsigned char pixels;
        unsigned char attribute;
        bool changed;
    };
    
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
    
    enum
    {
        eENVFLAG_HOLD = 0x01,
        eENVFLAG_ALTERNATE = 0x02,
        eENVFLAG_ATTACK = 0x04,
        eENVFLAG_CONTINUE = 0x08
    };

public:
    ZXSpectrum();
    ~ZXSpectrum();

public:
    virtual void            initialise(char *romPath);
    virtual void            resetMachine();
    virtual void            release();

    void                    loadRomWithPath(char *romPath);
    void                    runFrame();
    void                    resetFrame();
    void                    keyDown(unsigned short key);
    void                    keyUp(unsigned short key);
    void                    keyFlagsChanged(unsigned long flags, unsigned short key);
    void                    updateScreenWithTstates(int tStates);
    void                    applyIOContention(unsigned short address, bool contended);
    unsigned char           floatingBus();
    bool                    loadZ80SnapshotWithPath(const char *path);
    bool                    loadSnapshotWithPath(const char *path);
    snap                    createSnapshot();
    snap                    createZ80Snapshot();
    void                    setAYRegister(unsigned char reg);
    void                    writeAYData(unsigned char data);
    unsigned char           readAYData();
    void                    updateAY(int audioSteps);
    void                    resetAudio();
    int                     audioQueueWrite(signed short *buffer, int count);
    int                     audioQueueRead(signed short *buffer, int count);
    int                     audioQueueBufferUsed();
    void                    audioUpdateWithTstates(int tStates);
    
private:
    void                    generateScreen();
    void                    buildDisplayTstateTable();
    void                    buildScreenLineAddressTable();
    void                    buildContentionTable();
    void                    buildAYVolumesTable();
    void                    checkCapsLockStatus();
    void                    resetKeyboardMap();
    string                  hardwareTypeForVersion(int version, int hardwareType);
    void                    extractMemoryBlock(unsigned char *fileBytes, int memAddr, int fileOffset, bool isCompressed, int unpackedLength);
    void                    setupDisplay();
    void                    setupAudio(float sampleRate, float fps);
    
    
    // Core memory/IO functions
    static unsigned char    zxSpectrumMemoryRead(unsigned short address, void *param);
    static void             zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param);
    static void             zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param);
    static unsigned char    zxSpectrumDebugRead(unsigned int address, void *param, void *m);
    static void             zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data);
    static unsigned char    zxSpectrumIORead(unsigned short address, void *param);
    static void             zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param);

public:
    virtual unsigned char   coreMemoryRead(unsigned short address);
    virtual void            coreMemoryWrite(unsigned short address, unsigned char data);
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates);
    virtual unsigned char   coreDebugRead(unsigned int address, void *data);
    virtual void            coreDebugWrite(unsigned int address, unsigned char byte, void *data);
    virtual unsigned char   coreIORead(unsigned short address);
    virtual void            coreIOWrite(unsigned short address, unsigned char data);

protected:
    CZ80Core                z80Core;
    vector<char>            memoryRom;
    vector<char>            memoryRam;
    unsigned char           keyboardMap[8];
    static KEYBOARD_ENTRY   keyboardLookup[];
    int                     keyboardCapsLockFrames;
    
public:
    MachineInfo             machineInfo;

    // Display
    unsigned int            *displayBuffer;
    ScreenBufferData        *displayBufferCopy;
    unsigned int            displayBufferIndex;
    int                     screenWidth;
    int                     screenHeight;
    int                     screenBufferSize;
    int                     displayTstateTable[312][228];
    int                     displayLineAddrTable[192];
    int                     displayPage;
    int                     currentDisplayTstates;
    static unsigned int     palette[];
    int                     borderColor;

    // Emulation
    int                     frameCounter;
    bool                    paused;
    
    // Audio
    int                     audioEarBit;
    int                     audioMicBit;
    int                     channelOutput[3];
    unsigned int            AYChannelCount[3];
    unsigned short          AYVolumes[16];
    short                   *audioBuffer;
    unsigned int            random;
    unsigned int            AYOutput;
    unsigned int            noiseCount;
    unsigned int            envelopeCount;
    int                     envelopeStep;
    unsigned char           AYRegisters[ eAY_MAX_REGISTERS ];
    unsigned char           currentAYRegister;
    unsigned char           floatingAYRegister;
    bool                    envelopeHolding;
    bool                    envelopeHold;
    bool                    envelopeAlt;
    bool                    envelope;
    unsigned int            attackEndVol;
    int                     audioBufferSize;
    double                  audioTsStep;
    int                     audioTsCounter;
    double                  audioAYTStatesStep;
    double                  audioTsStepCounter;
    int                     audioBufferIndex;
    double                  audioBeeperLeft;
    double                  audioBeeperRight;
    
    short                   *audioQueueBuffer;
    int                     audioQueueBufferRead;
    int                     audioQueueBufferWritten;
    int                     audioQueueBufferCapacity;
    
    // Keyboard
    bool                    keyboardCapsLockPressed;
    
    // Contention
    unsigned int            memoryContentionTable[80000];
    unsigned int            ioContentionTable[80000];
    static unsigned int     contentionValues[];

    // Floating bus
    static unsigned int     floatingBusValues[];
    

};

#endif /* ZXSpectrum_hpp */






