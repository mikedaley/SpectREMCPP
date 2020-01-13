//
//  ZXSpectrum48.cpp
//  SpectREM
//
//  Created by Michael Daley on 23/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum48.hpp"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------
// - Constants

static const int kROM_SIZE = 16384;
static const char *kDEFAULT_ROM = "48.ROM";
//static const char *cSMART_ROM = "snapload.v31";

// SmartCard ROM and sundries
static const uint8_t kFAFB_ROM_SWITCHOUT = 0x40;
static const uint8_t kFAF3_SRAM_ENABLE = 0x80;

static uint8_t smart_card_port_faf3 = 0;
static uint8_t smart_card_port_fafb = 0;
static uint8_t smart_card_sram[8 * 64 * 1024];		// 8 * 8k banks, mapped @ $2000-$3FFF

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Destructor

ZXSpectrum48::ZXSpectrum48(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum48::Constructor" << "\n";
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

ZXSpectrum48::~ZXSpectrum48()
{
    std::cout << "ZXSpectrum48::Destructor" << "\n";
    release();
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum48::initialise(std::string romPath)
{
    std::cout << "ZXSpectrum48::initialise(char *rom)" << "\n";
    
    machine_info = machines[ eZXSpectrum48 ];
    ZXSpectrum::initialise( romPath );

    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80_core.RegisterOpcodeCallback( ZXSpectrum48::opcodeCallback );
    
    loadROM( kDEFAULT_ROM, 0 );
}

// ------------------------------------------------------------------------------------------------------------
// - ULA

uint8_t ZXSpectrum48::coreIORead(uint16_t address)
{
    bool contended = false;
    int memoryPage = address / kMemoryPageSize;
    if (memoryPage == 1)
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
        else if ((address & 0xc002) == 0xc000 && (machine_info.has_ay || emu_use_ay_sound) )
        {
            return audioAYReadData();
        }
       
		// Retroleum Smart Card - HexTank
		else if ((address & 0xfff1) == 0xfaf1)
		{
			if(address == 0xfaf3)
			{
				return smart_card_port_faf3;
			}
			else if(address == 0xfafb)
			{
				return smart_card_port_fafb & 0x7f;
			}
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

void ZXSpectrum48::coreIOWrite(uint16_t address, uint8_t data)
{
    bool contended = false;
    int memoryPage = address / kMemoryPageSize;
    if (memoryPage == 1)
    {
        contended = true;
    }
    
    ZXSpectrum::ULAApplyIOContention(address, contended);

    // ULA owned ports
    if (!(address & 0x01))
    {
        displayUpdateWithTs(static_cast<int32_t>((z80_core.GetTStates() - emu_current_display_ts) + machine_info.border_drawing_offset));

        // Port: 0xFE
        //   7   6   5   4   3   2   1   0
        // +---+---+---+---+---+-----------+
        // |   |   |   | E | M |  BORDER   |
        // +---+---+---+---+---+-----------+
        audio_ear_bit = (data & 0x10) ? 1 : 0;
        audio_mic_bit = (data & 0x08) ? 1 : 0;
        display_border_color = data & 0x07;
    }
    
    // AY-3-8912 ports
    if(address == 0xfffd && (machine_info.has_ay || emu_use_ay_sound))
    {
        ula_port_nnfd_value = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && (machine_info.has_ay || emu_use_ay_sound))
    {
        audioAYWriteData(data);
    }

    // SPECDRUM port, all ports ending in 0xdf
    if ((address & 0xff) == 0xdf && emu_use_specdrum)
    {
        // Adjust the output from SpecDrum to get the right volume
        audio_specdrum_dac_value = (data * 256) - 32768;
    }
    
	// Retroleum Smart Card - HexTank
	else if ((address & 0xfff1) == 0xfaf1)
	{
		if(address == 0xfaf3)
		{
			smart_card_port_faf3 = data;
		}
		else if(address == 0xfafb)
		{
			smart_card_port_fafb = data;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Read/Write

void ZXSpectrum48::coreMemoryWrite(uint16_t address, uint8_t data)
{
    if (address < kROM_SIZE)
    {
		if ((smart_card_port_faf3 & 0x80) && address >= 8192 && address < 16384)
		{
			smart_card_sram[ (address - 8192) + ((smart_card_port_faf3 & 0x07) * 8192) ] = data;
		}
		
        return;
    }
    
    if (address >= kROM_SIZE && address < kDisplayBitmapAddress + kDisplayBitmapSize + kAttributeMemorySize){
        displayUpdateWithTs(static_cast<int32_t>((z80_core.GetTStates() - emu_current_display_ts) + machine_info.paper_drawing_offset));
    }

    if (debugOpCallbackBlock != nullptr)
    {
        if (debugOpCallbackBlock( address, DebugOperation::WRITE ))
        {
            debugger_breakpoint_hit = true;
        }
    }
    
    debugger_breakpoint_hit = false;

    memory_ram[ address ] = static_cast<char>(data);
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum48::coreMemoryRead(uint16_t address)
{
    if (address < kROM_SIZE)
    {
		if ((smart_card_port_faf3 & 0x80) && address >= 8192 && address < 16384)
		{
			return smart_card_sram[  (address - 8192) + ((smart_card_port_faf3 & 0x07) * 8192) ];
		}
		if((address & 0xff) == 0x72)
		{
			if (smart_card_port_fafb & 0x40)
			{
                smart_card_port_fafb &= ~kFAFB_ROM_SWITCHOUT;
                smart_card_port_faf3 &= ~kFAF3_SRAM_ENABLE;
                uint8_t retOpCode = static_cast<uint8_t>(memory_rom[ address ]);
                loadROM( kDEFAULT_ROM, 0 );
				return retOpCode;
			}
		}
		
        if (debugOpCallbackBlock != nullptr)
        {
            if (debugOpCallbackBlock( address, DebugOperation::READ ))
            {
                debugger_breakpoint_hit = true;
            }
        }

        debugger_breakpoint_hit = false;
        return static_cast<uint8_t>(memory_rom[address]);
    }

    if (debugOpCallbackBlock != nullptr)
    {
        debugOpCallbackBlock( address, DebugOperation::READ );
    }

    return static_cast<uint8_t>(memory_ram[ address ]);
}

// ------------------------------------------------------------------------------------------------------------
// - Debug Memory Read/Write

uint8_t ZXSpectrum48::coreDebugRead(uint16_t address, void *)
{
    if (address < kROM_SIZE)
    {
        return static_cast<uint8_t>(memory_rom[address]);
    }
    
    return static_cast<uint8_t>(memory_ram[address]);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum48::coreDebugWrite(uint16_t address, uint8_t byte, void *)
{
    if (address < kROM_SIZE)
    {
        memory_rom[address] = static_cast<char>(byte);
    }
    else
    {
        memory_ram[address] = static_cast<char>(byte);
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Contention

void ZXSpectrum48::coreMemoryContention(uint16_t address, uint32_t)
{
    if (address >= 16384 && address <= 32767)
    {
        z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Release/Reset

void ZXSpectrum48::release()
{
    ZXSpectrum::release();
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum48::resetMachine(bool hard)
{
    if (hard)
    {
        // If a hard reset is requested, reload the default ROM and make sure that the smart card
        // ROM switch is disabled along with the smart card SRAM
        loadROM( kDEFAULT_ROM, 0 );
        smart_card_port_fafb &= ~kFAFB_ROM_SWITCHOUT;
        smart_card_port_faf3 &= ~kFAF3_SRAM_ENABLE;
    }

    emu_display_page = 1;
    ZXSpectrum::resetMachine(hard);
}

// ------------------------------------------------------------------------------------------------------------
// - Opcode Callback Function

bool ZXSpectrum48::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum48 *machine = static_cast<ZXSpectrum48*>(param);
    
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



