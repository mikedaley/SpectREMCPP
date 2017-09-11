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
#include "MachineDetails.h"

using namespace std;

#pragma mark - Base TapeBlock class

class TapeBlock
{
public:
    virtual ~TapeBlock();
    
public:
    virtual unsigned char   getFlag();
    virtual unsigned char   getDataType();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual const char *    getBlockName() = 0;
    virtual unsigned short  getAutolineStart();
    virtual char *          getFilename();
    
public:
    unsigned short          blockLength;
    unsigned char           *blockData;
    int                     blockType;
    int                     currentByte;
};

#pragma mark - TypeDefs

typedef void (*SpectrumTapeCallback)(int blockIndex, int bytes);

#pragma mark - Base ZXSpectrum class

class ZXSpectrum
{

public:
    static const int        cBITMAP_ADDRESS = 16384;
    static const int        cBITMAP_SIZE = 6144;
    static const int        cATTR_SIZE = 768;
    static const int        cMEMORY_PAGE_SIZE = 16384;

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
    struct Snap {
        int length;
        unsigned char *data;
    };
    
    // Previous frames screen buffer contents
    struct ScreenBufferData {
        unsigned char pixels;
        unsigned char attribute;
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
    
    // AY chip envelope flag type
    enum
    {
        eENVFLAG_HOLD = 0x01,
        eENVFLAG_ALTERNATE = 0x02,
        eENVFLAG_ATTACK = 0x04,
        eENVFLAG_CONTINUE = 0x08
    };
    
    // TAPE block types
    enum
    {
        ePROGRAM_HEADER = 0,
        eNUMERIC_DATA_HEADER,
        eALPHANUMERIC_DATA_HEADER,
        eBYTE_HEADER,
        eDATA_BLOCK,
        eFRAGMENTED_DATA_BLOCK,
        eUNKNOWN_BLOCK = 99
    };
    
    // TAP Processing states
    enum
    {
        eNO_TAPE = 0,
        eHEADER_PILOT,
        eSYNC1,
        eSYNC2,
        eDATA_PILOT,
        eBLOCK_PAUSE,
        eDATA_STREAM,
        eHEADER_DATA_STREAM,
        eDATA_BIT
    };
    
public:
    ZXSpectrum();
    virtual ~ZXSpectrum() = 0;

public:
    virtual void            initialise(string romPath);
    virtual void            resetMachine(bool hard = true);
    virtual void            release();

    virtual void            loadDefaultROM() = 0;

    void                    generateFrame();
    void                    displayFrameReset();
    void                    emuReset();
    void                    displayUpdateWithTs(int tStates);
    void                    keyboardKeyDown(unsigned short key);
    void                    keyboardKeyUp(unsigned short key);
    void                    keyboardFlagsChanged(unsigned long flags, unsigned short key);
    void                    pause();
    void                    resume();
    
    void                    ULAApplyIOContention(unsigned short address, bool contended);
    unsigned char           ULAFloatingBus();
    
    bool                    snapshotZ80LoadWithPath(const char *path);
    bool                    snapshotSNALoadWithPath(const char *path);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    Snap                    snapshotCreateSNA();
    Snap                    snapshotCreateZ80();
    
    bool                    tapeLoadWithPath(const char *);
    void                    tapeLoadBlock();
    void                    tapeUpdateWithTs(int tStates);
    void                    tapeStartPlaying();
    void                    tapeStopPlaying();
    void                    tapeRewind();
    void                    tapeReset(bool clearBlocks);
private:
    void                    tapeGenerateHeaderPilotWithTs(int tStates);
    void                    tapeGenerateSync1WithTs(int tStates);
    void                    tapeGenerateSync2WithTs(int tStates);
    void                    tapeGenerateDataPilotWithTs(int tStates);
    void                    tapeGenerateDataStreamWithTs(int tStates);
    void                    tapeGenerateHeaderDataStreamWithTs(int tStates);
    void                    tapeGenerateDataBitWithTs(int tStates);
    void                    tapeBlockPauseWithTs(int tStates);

public:
    void                    audioAYSetRegister(unsigned char reg);
    void                    audioAYWriteData(unsigned char data);
    unsigned char           audioAYReadData();
    void                    audioAYUpdate(int audioSteps);
    void                    audioReset();
    int                     audioQueueWrite(signed short *buffer, int count);
    int                     audioQueueRead(signed short *buffer, int count);
    int                     audioQueueBufferUsed();
    void                    audioUpdateWithTs(int tStates);
    
private:
    void                    displayBuildTsTable();
    void                    displayBuildLineAddressTable();
    void                    ULABuildContentionTable();
    void                    audioBuildAYVolumesTable();
    void                    keyboardCheckCapsLockStatus();
    void                    keyboardMapReset();
    string                  snapshotHardwareTypeForVersion(int version, int hardwareType);
    void                    snapshotExtractMemoryBlock(unsigned char *fileBytes, int memAddr, int fileOffset, bool isCompressed, int unpackedLength);
    void                    displaySetup();
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
//    const unsigned int      displayPalette[];
    int                     displayBorderColor;

    // Emulation
    int                     emuCurrentDisplayTs;
    int                     emuFrameCounter;
    bool                    emuPaused;
    int                     emuRAMPage;
    int                     emuROMPage;
    int                     emuDisplayPage;
    bool                    emuDisablePaging;
    string                  emuROMPath;
    bool                    emuTapeInstantLoad;
    bool                    emuLoadTrapTriggered;
    bool                    emuSaveTrapTriggered;
    
    // Tape Processing
    bool                    tapeLoaded;
    bool                    tapePlaying;
    int                     tapeCurrentBytePtr;
    int                     tapeCurrentBlockIndex;
    int                     tapeNewBlock;
    vector<TapeBlock *>     tapeBlocks;
    int                     tapeInputBit;
    
    int                     tapePilotPulseTStates;          // How many Ts have passed since the start of the pilot pulses
    int                     tapePilotPulses;                // How many pilot pulses have been generated
    int                     tapeSyncPulseTStates;           // Sync pulse tStates
    int                     tapeDataPulseTStates;           // How many Ts have passed since the start of the data pulse
    bool                    tapeFlipTapeBit;                // Should the tape bit be flipped
    int                     tapeProcessingState;            // Current processing state e.g. generating pilot, streaming data
    int                     tapeNextProcessingState;        // Next processing state to be used
    int                     tapeCurrentDataBit;             // Which bit of the current byte in the data stream is being processed
    int                     tapeBlockPauseTStates;          // How many tStates have passed since starting the pause between data blocks
    int                     tapeDataBitTStates;             // How many tStates to pause when processing data bit pulses
    int                     tapeDataPulseCount;             // How many pulses have been generated for the current data bit;
    TapeBlock               *tapeCurrentBlock;              // Current tape block
    
    // Audio
    int                     audioEarBit;
    int                     audioMicBit;
    short                   *audioBuffer;
    int                     audioBufferSize;
    int                     audioBufferIndex;
    int                     audioTsCounter;
    float                   audioTsStepCounter;

    float                   audioBeeperTsStep;
    float                   audioBeeperLeft;
    float                   audioBeeperRight;

    int                     audioAYChannelOutput[3];
    unsigned int            audioAYChannelCount[3];
    unsigned short          audioAYVolumes[16];
    unsigned int            audioAYrandom;
    unsigned int            audioAYOutput;
    unsigned int            audioAYNoiseCount;
    unsigned int            audioATaudioAYEnvelopeCount;
    int                     audioAYaudioAYEnvelopeStep;
    unsigned char           audioAYRegisters[ eAY_MAX_REGISTERS ];
    unsigned char           audioAYCurrentRegister;
    unsigned char           audioAYFloatingRegister;
    bool                    audioAYaudioAYaudioAYEnvelopeHolding;
    bool                    audioAYaudioAYEnvelopeHold;
    bool                    audioAYaudioAYEnvelopeAlt;
    bool                    audioAYEnvelope;
    unsigned int            audioAYAttackEndVol;
    float                   audioAYTsStep;
    int                     audioAYTs;
    
    short                   *audioQueueBuffer;
    int                     audioQueueBufferRead;
    int                     audioQueueBufferWritten;
    int                     audioQueueBufferCapacity;
    
    // Keyboard
    bool                    keyboardCapsLockPressed;
    
    // ULA
    unsigned int            ULAMemoryContentionTable[80000];
    unsigned int            ULAIOContentionTable[80000];
    const static unsigned int     ULAConentionValues[];
    int                     ULAPortFFFDValue;

    // Floating bus
    const static unsigned int ULAFloatingBusValues[];
    
    SpectrumTapeCallback    tapeCallback;

};

#pragma mark - Program Header

class ProgramHeader : public TapeBlock
{
public:
    unsigned short          getAutoStartLine();
    unsigned short          getProgramLength();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual const char *    getBlockName();
    virtual char *          getFilename();
};

#pragma mark - Numeric Data Header

class NumericDataHeader : public TapeBlock
{
public:
    unsigned char           getVariableName();
    virtual unsigned short  getDataLength();
    virtual const char *    getBlockName();
    virtual char *          getFilename();
};

#pragma mark - Alphanumeric Data Header

class AlphanumericDataHeader : public TapeBlock
{
public:
    unsigned char           getVariableName();
    virtual unsigned short  getDataLength();
    virtual const char *    getBlockName();
    virtual char *          getFilename();
};

#pragma mark - Byte Header Block

class ByteHeader : public TapeBlock
{
public:
    unsigned short          getStartAddress();
    virtual unsigned char   getChecksum();
    virtual unsigned short  getDataLength();
    virtual const char *    getBlockName();
    virtual char *          getFilename();
};

#pragma mark - Data Block

class DataBlock : public TapeBlock
{
public:
    unsigned char           *getDataBlock();
    virtual unsigned char   getDataType();
    virtual unsigned char   getChecksum();
    virtual const char *    getBlockName();
    virtual char *          getFilename();
};

#endif /* ZXSpectrum_hpp */






