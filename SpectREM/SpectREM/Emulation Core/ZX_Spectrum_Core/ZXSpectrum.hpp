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
    static constexpr uint16_t    kDisplayBitmapAddress    = 16384;
    static constexpr uint16_t    kDisplayBitmapSize       = 6144;
    static constexpr uint16_t    kAttributeMemorySize     = 768;
    static constexpr uint16_t    kMemoryPageSize          = 16384;
    
    enum AyRegister
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
    
//    // ULAPlus mode values
//    enum eULAPLUS
//    {
//        PALLETTEGROUP,
//        MODEGROUP
//    };
    
    // Debug operation type
    enum DebugOperation
    {
        READ = 0x01,
        WRITE = 0x02,
        EXECUTE = 0x04
    };
    
    // Spectrum keyboard
    enum class ZXSpectrumKey
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
        ZXSpectrumKey      key;
        int                 mapEntry1;
        int                 mapBit1;
        int                 mapEntry2;
        int                 mapBit2;
    } KEYBOARD_ENTRY;
    
public:
    // Holds the data returned when creating a Snapshot or Z80 snapshot
    struct snapshot_data {
        int32_t             length = 0;
        uint8_t             *data = nullptr;
    };

    // Response from core when you have asked it to deal with a file
    struct FileResponse {
        bool                success;
        std::string         responseMsg;
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

    // Main function that when called generates an entire frame, which includes processing interrupts, beeper sound and AY Sound.
    // On completion the displayBuffer member variable will contain RGBA formatted image data that can then be used to build a display image
    void                    generateFrame();
    
    void                    keyboardKeyDown(ZXSpectrumKey key);
    void                    keyboardKeyUp(ZXSpectrumKey key);
    void                    keyboardFlagsChanged(uint64_t flags, ZXSpectrumKey key);
    
    FileResponse            snapshotZ80LoadWithPath(const std::string path);
    FileResponse            snapshotZ80LoadWithBuffer(const char *buffer, size_t size);
    FileResponse            snapshotSNALoadWithPath(const std::string path);
    FileResponse            snapshotSNALoadWithBuffer(const char *buffer, size_t size);
    int                     snapshotMachineInSnapshotWithPath(const char *path);
    snapshot_data            snapshotCreateSNA();
    snapshot_data            snapshotCreateZ80();
    
    FileResponse            scrLoadWithPath(const std::string path);
    
    
    void                    step();
    
    void                    registerDebugOpCallback(std::function<bool(uint16_t, uint8_t)> debugOpCallbackBlock);
    std::function<bool(uint16_t, uint8_t)> debugOpCallbackBlock = nullptr;
    
    void                    *getScreenBuffer();
    uint32_t                getLastAudioBufferIndex() { return audio_last_index; }

protected:
    void                    emuReset();
    FileResponse            loadROM(const std::string rom, uint32_t page);
    
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
    void                    ULABuildContentionTable();
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
    
    void                    coreMemoryWriteWithBuffer(const char *buffer, size_t size, uint16_t address);
        
    // Machine hardware
    CZ80Core                z80_core;
    std::vector<char>       memory_rom;
    std::vector<char>       memory_ram;
    uint8_t                 keyboard_map[8]{0};
    static KEYBOARD_ENTRY   keyboard_lookup[];
    uint32_t                keyboard_capslock_frames = 0;
      
    // Emulation
    MachineInfo             machine_info;
    uint32_t                emu_current_display_ts = 0;
    uint32_t                emu_frame_counter = 0;
    bool                    emu_paused = true;
    uint8_t                 emu_ram_page = 0;
    uint8_t                 emu_rom_page = 0;
    uint8_t                 emu_display_page = 0;
    bool                    emu_disable_paging = true;
    std::string             emu_rom_path;
    std::string             emu_base_path;
    bool                    emu_tape_instant_load = false;
    bool                    emu_use_ay_sound = false;
    bool                    emu_load_trap_triggered = false;
    bool                    emu_save_trap_triggered = false;
    bool                    emu_use_specdrum = false;

    // Display
    uint8_t *               display_buffer;
    uint32_t                display_buffer_index = 0;
    uint32_t                display_screen_width = 48 + 256 + 48;
    uint32_t                display_screen_height = 48 + 192 + 48;
    uint32_t                display_screen_buffer_size = 0;
    uint32_t                display_ts_state_table[312][228]{{0}};
    uint16_t                display_line_addr_table[192]{0};
    uint64_t *              display_clut = nullptr;
    uint8_t *               display_alut = nullptr;
    uint32_t                display_border_color = 0;
    bool                    display_ready = false;
    Color                   display_clut_buffer[64];
    
    // Audio
    int16_t *               audio_buffer = nullptr;
    int8_t                  audio_ear_bit = 0;
    int8_t                  audio_mic_bit = 0;
    uint32_t                audio_buffer_size = 0;
    uint32_t                audio_buffer_index = 0;
    float                   audio_ts_counter = 0;
    float                   audio_ts_step_counter = 0;
    uint32_t                audio_last_index = 0;

    float                   audio_beeper_ts_step = 0;
    float                   audio_output_level_left = 0;
    float                   audio_output_level_right = 0;
	float                   audio_ay_level_left = 0;
	float                   audio_ay_level_right = 0;
    
    float                   audio_ay_channel_output[3]{0};
    uint32_t                audio_ay_channel_count[3]{0};
    uint16_t                audio_ay_volumes[16]{0};
    uint32_t                audio_ay_random = 0;
    uint32_t                audio_ay_output = 0;
    uint32_t                audio_ay_noise_count = 0;
    uint16_t                audio_ay_envelope_count = 0;
    
    uint8_t                 audio_ay_registers[ AyRegister::MAX_REGISTERS ]{0};
    uint8_t                 audio_ay_current_register = 0;
    uint8_t                 audio_ay_floating_register = 0;
    bool                    audio_ay_envelope_holding = false;
    bool                    audio_ay_envelope_hold = false;
    bool                    audio_ay_envelope_alternate = false;
    bool                    audio_ay_envelope_continue = false;
    bool                    audio_ay_envelope = false;;
    bool                    audio_ay_one_shot = false;
    bool                    audio_ay_envelope_attack = false;
    uint8_t                 audio_ay_attack_end_volume = 0;
    float                   audio_ay_ts_step = 0;
    float                   audio_ay_ts = 0;

    //Specdrum Peripheral
    int                     audio_specdrum_dac_value = 0;
    
    // Keyboard
    bool                    keyboard_capslock_pressed = false;
    
    // ULA
    uint32_t                ula_memory_contention_table[80000]{0};
    uint32_t                ula_io_contention_table[80000]{0};
    uint32_t                ula_floating_bus_table[80000]{0};
    const static uint32_t   ula_contention_values[];
    uint8_t                 ula_port_nnfd_value = 0;

    // Floating bus
    const static uint32_t   ula_floating_bus_values[];
    
    // Tape object
    Tape *                  virtual_tape = nullptr;
    
    // Debugger
    bool                    debugger_breakpoint_hit = false;

};

#endif /* ZXSpectrum_hpp */






