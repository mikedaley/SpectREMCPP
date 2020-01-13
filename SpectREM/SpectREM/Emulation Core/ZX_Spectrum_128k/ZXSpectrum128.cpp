//
//  ZXSpectrum128.cpp
//  SpectREM
//
//  Created by Mike Daley on 04/09/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum128.hpp"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------
// - Constants

//static const int cROM_SIZE = 16384;
static const char *cDEFAULT_ROM_0 = "128-0.ROM";
static const char *cDEFAULT_ROM_1 = "128-1.ROM";

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Destructor

ZXSpectrum128::ZXSpectrum128(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum128::Constructor" << "\n";
    if (t)
    {
        virtual_tape = t;
    }
    else
    {
        virtual_tape = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum128::~ZXSpectrum128()
{
    std::cout << "ZXSpectrum128::Destructor" << "\n";
    release();
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum128::initialise(std::string romPath)
{
    std::cout << "ZXSpectrum128::initialise(char *rom)" << "\n";
    
    machine_info = machines[ eZXSpectrum128 ];
    ZXSpectrum::initialise(romPath);
    
    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80_core.RegisterOpcodeCallback(opcodeCallback);
    
    loadROM( cDEFAULT_ROM_0, 0 );
    loadROM( cDEFAULT_ROM_1, 1 );

    emu_rom_page = 0;
    emu_ram_page = 0;
    emu_display_page = 5;
    emu_disable_paging = false;
    ula_port_nnfd_value = 0;

}

// ------------------------------------------------------------------------------------------------------------
// - ULA

uint8_t ZXSpectrum128::coreIORead(uint16_t address)
{
    bool contended = false;
    int memory_page = address / kMemoryPageSize;
    
    if (machine_info.has_paging && (memory_page == 1 || (memory_page == 3 && (emu_ram_page == 1 || emu_ram_page == 3 || emu_ram_page == 5 || emu_ram_page == 7))))
    {
        contended = true;
    }
    
    ZXSpectrum::ULAApplyIOContention(address, contended);
    
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
        
        // AY-3-8912 ports
        else if ((address & 0xc002) == 0xc000 && machine_info.has_ay)
        {
            return audioAYReadData();
        }        
        
        // port 7FFD memory port read bug on the 128k. When reading from 0x7FFD, it actually performs a right to the port
        // with what is on the floating bus.
        if ( (address & 0x8002) == 0)
        {
            uint8_t floatingBusData = ULAFloatingBus();
            uint32_t currentTStates = z80_core.GetTStates();
            UpdatePort7FFD(floatingBusData);
            z80_core.ResetTStates(z80_core.GetTStates() - currentTStates);
        }

        // Getting here means that nothing has handled that port read so based on a real Spectrum
        // return the floating bus value
        return ULAFloatingBus();
    }
    
    // Check to see if the keyboard is being read and if so return any keys currently pressed
    uint8_t result = 0xff;
    if (address & 0xfe)
    {
        for (uint32_t i = 0; i < 8; i++)
        {
            if (!(address & (0x100 << i)))
            {
                result &= keyboard_map[i];
            }
        }
    }

    result = static_cast<uint8_t>((result & 191) | (audio_ear_bit << 6) | (virtual_tape->input_bit << 6));
    
    return result;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128::coreIOWrite(uint16_t address, uint8_t data)
{
    bool contended = false;
    int memory_page = address / kMemoryPageSize;
    
    if (machine_info.has_paging && (memory_page == 1 || (memory_page == 3 && (emu_ram_page == 1 || emu_ram_page == 3 || emu_ram_page == 5 || emu_ram_page == 7))))
    {
        contended = true;
    }

    ZXSpectrum::ULAApplyIOContention(address, contended);
    
    // Port: 0xFE
    //   7   6   5   4   3   2   1   0
    // +---+---+---+---+---+-----------+
    // |   |   |   | E | M |  BORDER   |
    // +---+---+---+---+---+-----------+
    if (!(address & 0x01))
    {
        displayUpdateWithTs((z80_core.GetTStates() - emu_current_display_ts) + machine_info.border_drawing_offset);
        audio_ear_bit = (data & 0x10) ? 1 : 0;
        audio_mic_bit = (data & 0x08) ? 1 : 0;
        display_border_color = data & 0x07;
    }
    
    // AY-3-8912 ports
    if((address & 0xc002) == 0xc000 && machine_info.has_ay)
    {
        ula_port_nnfd_value = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && machine_info.has_ay)
    {
        audioAYWriteData(data);
    }
    
    // Memory paging port
    if ( (address & 0x8002) == 0 && emu_disable_paging == false)
    {
        UpdatePort7FFD(data);
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128::UpdatePort7FFD(uint8_t data)
{
    // Save the last byte set, used when generating a Z80 snapshot
    ula_port_nnfd_value = data;
    
    if (emu_display_page != (((data & 0x08ul) == 0x08ul) ? 7ul : 5ul))
    {
        displayUpdateWithTs((z80_core.GetTStates() - emu_current_display_ts) + machine_info.border_drawing_offset);
    }
    
    // You should only be able to disable paging once. To enable paging again then a reset is necessary.
    if (data & 0x20 && emu_disable_paging != true)
    {
        emu_disable_paging = true;
    }
    
    emu_rom_page = ((data & 0x10) == 0x10) ? 1 : 0;
    emu_ram_page = (data & 0x07);
    emu_display_page = ((data & 0x08) == 0x08) ? 7 : 5;
    
}

// - Memory Read/Write

void ZXSpectrum128::coreMemoryWrite(uint16_t address, uint8_t data)
{
    const int page = address / kMemoryPageSize;
    address &= 16383;

    if (page == 0)
    {
        return;
    }
    else if (page == 1)
    {
        displayUpdateWithTs((z80_core.GetTStates() - emu_current_display_ts) + machine_info.paper_drawing_offset);
        memory_ram[(5 * kMemoryPageSize) + address] = data;
    }
    else if (page == 2)
    {
        memory_ram[(2 * kMemoryPageSize) + address] = data;
    }
    else if (page == 3)
    {
        memory_ram[(emu_ram_page * kMemoryPageSize) + address] = data;
    }
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128::coreMemoryRead(uint16_t address)
{
    const int page = address / kMemoryPageSize;
    address &= 16383;

    if (page == 0)
    {
        return (memory_rom[(emu_rom_page * kMemoryPageSize) + address]);
    }
    else if (page == 1)
    {
        return (memory_ram[(5 * kMemoryPageSize) + address]);
    }
    else if (page == 2)
    {
        return (memory_ram[(2 * kMemoryPageSize) + address]);
    }
    else if (page == 3)
    {
        return (memory_ram[(emu_ram_page * kMemoryPageSize) + address]);
    }
    
    return 0;
}

// ------------------------------------------------------------------------------------------------------------
// - Debug Memory Read/Write

void ZXSpectrum128::coreDebugWrite(uint16_t address, uint8_t byte, void *)
{
    const int page = address / kMemoryPageSize;
    address &= 16383;
    
    if (page == 0)
    {
        return;
    }
    else if (page == 1)
    {
        memory_ram[(5 * kMemoryPageSize) + address] = byte;
    }
    else if (page == 2)
    {
        memory_ram[(2 * kMemoryPageSize) + address] = byte;
    }
    else if (page == 3)
    {
        memory_ram[(emu_ram_page * kMemoryPageSize) + address] = byte;
    }
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128::coreDebugRead(uint16_t address, void *)
{
    const int page = address / kMemoryPageSize;
    address &= 16383;
    
    if (page == 0)
    {
        return (memory_rom[(emu_rom_page * kMemoryPageSize) + address]);
    }
    else if (page == 1)
    {
        return (memory_ram[(5 * kMemoryPageSize) + address]);
    }
    else if (page == 2)
    {
        return (memory_ram[(2 * kMemoryPageSize) + address]);
    }
    else if (page == 3)
    {
        return (memory_ram[(emu_ram_page * kMemoryPageSize) + address]);
    }
    
    return 0;
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Contention

void ZXSpectrum128::coreMemoryContention(uint16_t address, uint32_t)
{
    const int page = address / kMemoryPageSize;
    
    if (page == 1 ||
        (page == 3 &&
          (emu_ram_page == 1 || emu_ram_page == 3 || emu_ram_page == 5 || emu_ram_page == 7)))
    {
        z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Release/Reset

void ZXSpectrum128::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum128::resetMachine(bool hard)
{
    emu_rom_page = 0;
    emu_ram_page = 0;
    emu_display_page = 5;
    emu_disable_paging = false;
    ula_port_nnfd_value = 0;
    ZXSpectrum::resetMachine(hard);
}

// ------------------------------------------------------------------------------------------------------------
// - Opcode Callback Function

bool ZXSpectrum128::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum128 *machine = static_cast<ZXSpectrum128*>(param);
    
    if (machine->emu_tape_instant_load)
    {
        // Trap ROM tap LOADING
        if (address == 0x056b || address == 0x0111)
        {
            if (opcode == 0xc0)
            {
                machine->emu_load_trap_triggered = true;
                machine->virtual_tape->updateStatus();
                return true;
            }
        }
    }
    
    // Trap ROM tape SAVING
    if (opcode == 0x08 && address == 0x04d0)
    {
        if (opcode == 0x08)
        {
            machine->emu_save_trap_triggered = true;
            machine->virtual_tape->updateStatus();
            return true;
        }
    }
    
    machine->emu_save_trap_triggered = false;
    machine->emu_load_trap_triggered = false;
    
    return false;
}




