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
#include <functional>

#include "../Z80_Core/Z80Core.h"
#include "MachineInfo.h"
#include "../Tape/Tape.hpp"

// - Base ZXSpectrum class

class ZXSpectrum
{

public:
    static const uint16_t    cBITMAP_ADDRESS   = 16384;
    static const uint16_t    cBITMAP_SIZE      = 6144;
    static const uint16_t    cATTR_SIZE        = 768;
    static const uint16_t    cMEMORY_PAGE_SIZE = 16384;
    
    enum E_FILETYPE
    {
        SNA = 0,
        Z80,
        TAP,
        SCR
    };
    
    enum E_AYREGISTER
    {
        A_FINE = 0,
        A_COARSE,
        B_FINE,
        B_COARSE,
        C_FINE,
        C_COARSE,
        NOISEPER,
        ENABLE,
        A_VOL,
        B_VOL,
        C_VOL,
        E_FINE,
        E_COARSE,
        E_SHAPE,
        PORT_A,
        PORT_B,
        
        // Used to emulate the odd floating behaviour of setting an AY register > 15. The value
        // written to registers > 15 decays over time and this is the value returned when reading
        // a register > 15
        FLOATING,
        
        MAX_REGISTERS
    };
    
    // Debug operation type
    enum E_DEBUGOPERATION
    {
        READ = 0x01,
        WRITE = 0x02,
        EXECUTE = 0x04
    };
    
    // Spectrum keyboard
    enum class eZXSpectrumKey
    {
        __NoKey,
        Key_0,
        Key_1,
        Key_2,
        Key_3,
        Key_4,
        Key_5,
        Key_6,
        Key_7,
        Key_8,
        Key_9,
        Key_A,
        Key_B,
        Key_C,
        Key_D,
        Key_E,
        Key_F,
        Key_G,
        Key_H,
        Key_I,
        Key_J,
        Key_K,
        Key_L,
        Key_M,
        Key_N,
        Key_O,
        Key_P,
        Key_Q,
        Key_R,
        Key_S,
        Key_T,
        Key_U,
        Key_V,
        Key_W,
        Key_X,
        Key_Y,
        Key_Z,
        Key_Shift,
        Key_Enter,
        Key_Space,
        Key_SymbolShift,
        Key_InvVideo,
        Key_TrueVideo,
        Key_Quote,
        Key_SemiColon,
        Key_Comma,
        Key_Minus,
        Key_Plus,
        Key_Period,
        Key_Edit,
        Key_Graph,
        Key_Break,
        Key_Backspace,
        Key_ArrowUp,
        Key_ArrowDown,
        Key_ArrowLeft,
        Key_ArrowRight,
        Key_ExtendMode,
        Key_CapsLock,
    };

private:
    // Holds details of the host platforms key codes and how they map to the spectrum keyboard matrix
    typedef struct
    {
        eZXSpectrumKey      key;
        int                 mapEntry1;
        int                 mapBit1;
        int                 mapEntry2;
        int                 mapBit2;
    } KEYBOARD_ENTRY;
    
public:
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct SnapshotData {
        int32_t             length = 0;
        uint8_t             *data = nullptr;
    };

    // Breakpoint information
    struct DebugBreakpoint {
        uint16_t            address;
        bool                breakPoint;
    };
    
    typedef struct
    {
        float r;
        float g;
        float b;
        float a;
    } Color;

    
public:
    ZXSpectrum();
    virtual ~ZXSpectrum();

public:
    virtual void            initialise(std::string romPath);
    virtual void            resetMachine(bool hard = true);
    void                    pause();
    void                    resume();
    virtual void            release();
    virtual void            attachTapePlayer(Tape *tapePlayer);

    // Main function that when called generates an entire frame, which includes processing interrupts, beeper sound and AY Sound.
    // On completion the displayBuffer member variable will contain RGBA formatted image data that can then be used to build a display image
    void                    generateFrame();
    
    void                    keyboardKeyDown(eZXSpectrumKey key);
    void                    keyboardKeyUp(eZXSpectrumKey key);
    void                    keyboardFlagsChanged(uint64_t flags, eZXSpectrumKey key);
    
    Tape::FileResponse      snapshotZ80LoadWithPath(const std::string path);
    Tape::FileResponse      snapshotZ80LoadWithBuffer(const char *buffer, size_t size);
    Tape::FileResponse      snapshotSNALoadWithPath(const std::string path);
    Tape::FileResponse      snapshotSNALoadWithBuffer(const char *buffer, size_t size);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    SnapshotData            snapshotCreateSNA();
    SnapshotData            snapshotCreateZ80();
    
    Tape::FileResponse      scrLoadWithPath(const std::string path);
    
    void                    step();
    
    void                    registerDebugOpCallback(std::function<bool(uint16_t, uint8_t)> debugOpCallbackBlock);
    std::function<bool(uint16_t, uint8_t)> debugOpCallbackBlock = nullptr;
    
    void                    *getScreenBuffer();
    uint32_t                getLastAudioBufferIndex() { return audioLastIndex; }

protected:
    void                    emuReset();
    Tape::FileResponse      loadROM(const std::string rom, uint32_t page);
    
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
    void                    audioDecayAYFloatingRegister();
    
private:
    void                    displayBuildTsTable();
    void                    displayBuildLineAddressTable();
    void                    displayBuildCLUT();
    void                    ULABuildContentionTable(bool alt);
    void                    audioBuildAYVolumesTable();
    void                    keyboardCheckCapsLockStatus();
    void                    keyboardMapReset();
    std::string             snapshotHardwareTypeForVersion(uint32_t version, uint32_t hardwareType);
    void                    snapshotExtractMemoryBlock(const char *buffer, size_t bufferSize, uint32_t memAddr, uint32_t fileOffset, bool isCompressed, uint32_t unpackedLength);
    void                    displaySetup();
    void                    displayClear();
    void                    audioSetup(double sampleRate, double fps);
    
    // Core memory/IO functions
    static uint8_t          zxSpectrumMemoryRead(uint16_t address, void *param);
    static void             zxSpectrumMemoryWrite(uint16_t address, uint8_t data, void *param);
    static void             zxSpectrumMemoryContention(uint16_t address, uint32_t tStates, void *param);
    static uint8_t          zxSpectrumDebugRead(uint16_t address, void *param, void *m);
    static void             zxSpectrumDebugWrite(uint16_t address, uint8_t byte, void *param, void *data);
    static uint8_t          zxSpectrumIORead(uint16_t address, void *param);
    static void             zxSpectrumIOWrite(uint16_t address, uint8_t data, void *param);

public:
    virtual uint8_t         coreMemoryRead(uint16_t address) = 0;
    virtual void            coreMemoryWrite(uint16_t address, uint8_t data) = 0;
    virtual void            coreMemoryContention(uint16_t address, uint32_t tStates) = 0;
    virtual uint8_t         coreIORead(uint16_t address) = 0;
    virtual void            coreIOWrite(uint16_t address, uint8_t data) = 0;

    virtual uint8_t         coreDebugRead(uint16_t address, void *data) = 0;
    virtual void            coreDebugWrite(uint16_t address, uint8_t byte, void *data) = 0;
    
    void                    coreMemoryWriteWithBuffer(const char *buffer, size_t size, uint16_t address, void *param);
        
    // Machine hardware
    CZ80Core                z80Core;
    std::vector<char>       memoryRom;
    std::vector<char>       memoryRam;
    uint8_t                 keyboardMap[8]{0};
    static KEYBOARD_ENTRY   keyboardLookup[];
    uint32_t                keyboardCapsLockFrames  = 0;
      
    // Emulation
    MachineInfo             machineInfo;
    uint32_t                emuCurrentDisplayTs     = 0;
    uint32_t                emuFrameCounter         = 0;
    bool                    emuPaused               = true;
    uint8_t                 emuRAMPage              = 0;
    uint8_t                 emuROMNumber            = 0;
    uint8_t                 emuDisplayPage          = 0;
    bool                    emuDisablePaging        = true;
    std::string             emuROMPath;
    std::string             emuBasePath;
    bool                    emuTapeInstantLoad      = false;
    bool                    emuUseAYSound           = false;
    bool                    emuLoadTrapTriggered    = false;
    bool                    emuSaveTrapTriggered    = false;
    bool                    emuUseSpecDRUM          = false;
    bool                    emuSpecialPagingMode    = false;
    uint8_t                 emuPagingMode           = 0;
    uint8_t                 emuROMHiBit             = 0;
    uint8_t                 emuROMLoBit             = 0;

    // Display
    uint8_t                 *displayBuffer;
    uint32_t                displayBufferIndex      = 0;
    uint32_t                screenWidth             = 48 + 256 + 48;
    uint32_t                screenHeight            = 48 + 192 + 48;
    uint32_t                screenBufferSize        = 0;
    uint32_t                displayTstateTable[312][228]{{0}};
    uint16_t                displayLineAddrTable[192]{0};
    uint64_t                *displayCLUT            = nullptr;
    uint8_t                 *displayALUT            = nullptr;
    uint32_t                displayBorderColor      = 0;
    bool                    displayReady            = false;
    Color                   clutBuffer[64];
    
    // Audio
    int16_t                 *audioBuffer            = nullptr;
    int8_t                  audioEarBit             = 0;
    int8_t                  audioMicBit             = 0;
    uint32_t                audioBufferSize         = 0;
    uint32_t                audioBufferIndex        = 0;
    float                   audioTsCounter          = 0;
    float                   audioTsStepCounter      = 0;
    uint32_t                audioLastIndex          = 0;

    float                   audioBeeperTsStep       = 0;
    float                   audioOutputLevelLeft    = 0;
    float                   audioOutputLevelRight   = 0;
    float                   audioAYLevelLeft           = 0;
    float                   audioAYLevelRight          = 0;
    
    float                   audioAYChannelOutput[3]{0};
    uint32_t                audioAYChannelCount[3]{0};
    uint16_t                audioAYVolumes[16]{0};
    uint32_t                audioAYrandom           = 0;
    uint32_t                audioAYOutput           = 0;
    uint32_t                audioAYNoiseCount       = 0;
    uint16_t                audioAYEnvelopeCount    = 0;
    
    uint8_t                 audioAYRegisters[ E_AYREGISTER::MAX_REGISTERS ]{0};
    uint8_t                 audioAYCurrentRegister  = 0;
    uint8_t                 audioAYFloatingRegister = 0;
    bool                    audioAYEnvelopeHolding  = false;
    bool                    audioAYEnvelopeHold     = false;
    bool                    audioAYEnvelopeAlt      = false;
    bool                    audioAYEnvelopeContinue = false;
    bool                    audioAYEnvelope         = false;;
    bool                    audioAYOneShot          = false;
    bool                    audioAYEnvelopeAttack   = false;
    uint8_t                 audioAYAttackEndVol     = 0;
    float                   audioAYTsStep           = 0;
    float                   audioAYTs               = 0;

    //Specdrum Peripheral
    int                     specdrumDACValue        = 0;
    
    // Keyboard
    bool                    keyboardCapsLockPressed = false;
    
    // ULA
    uint32_t                ULAMemoryContentionTable[80000]{0};
    uint32_t                ULAIOContentionTable[80000]{0};
    uint32_t                ULAFloatingBusTable[80000]{0};
    const static uint32_t   ULAContentionValues[];
    const static uint32_t   ULAAltContentionValues[];
    uint8_t                 ULAPort7FFDValue        = 0;
    uint8_t                 ULAPort1FFDValue        = 0;

    // Floating bus
    const static uint32_t   ULAFloatingBusValues[];
    
    // Tape object
    Tape                    *tapePlayer              = nullptr;

    // Debugger
    bool                    breakpointHit           = false;

};

#endif /* ZXSpectrum_hpp */






