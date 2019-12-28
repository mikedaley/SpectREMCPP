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

// - Constants

static const int cROM_SIZE = 16384;
static const char *cDEFAULT_ROM = "48.ROM";
static const char *cSMART_ROM = "snapload.v31";

// SmartCard ROM and sundries
static const uint8_t cFAFB_ROM_SWITCHOUT = 0x40;
static const uint8_t cFAF3_SRAM_ENABLE = 0x80;

static uint8_t smartCardPortFAF3 = 0;
static uint8_t smartCardPortFAFB = 0;
static uint8_t smartCardSRAM[8 * 64 * 1024];		// 8 * 8k banks, mapped @ $2000-$3FFF

// - Constructor/Destructor

ZXSpectrum48::ZXSpectrum48(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum48::Constructor" << std::endl;
    if (t)
    {
        tape = t;
    }
    else
    {
        tape = nullptr;
    }
}

ZXSpectrum48::~ZXSpectrum48()
{
    std::cout << "ZXSpectrum48::Destructor" << std::endl;
    release();
}

// - Initialise

void ZXSpectrum48::initialise(string romPath)
{
    std::cout << "ZXSpectrum48::initialise(char *rom)" << std::endl;
    
    machineInfo = machines[ eZXSpectrum48 ];
    ZXSpectrum::initialise( romPath );

    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80Core.RegisterOpcodeCallback( ZXSpectrum48::opcodeCallback );
    
    loadROM( cDEFAULT_ROM, 0 );
}

// - ULA

uint8_t ZXSpectrum48::coreIORead(uint16_t address)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
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
        else if ((address & 0xc002) == 0xc000 && (machineInfo.hasAY || emuUseAYSound) )
        {
            return audioAYReadData();
        }
       
		// Retroleum Smart Card - HexTank
		else if ((address & 0xfff1) == 0xfaf1)
		{
			if(address == 0xfaf3)
			{
				return smartCardPortFAF3;
			}
			else if(address == 0xfafb)
			{
				return smartCardPortFAFB & 0x7f;
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
        for (int i = 0; i < 8; i++)
        {
            if (!(address & (0x100 << i)))
            {
                result &= keyboardMap[i];
            }
        }
    }
    
    result = static_cast<uint8_t>((result & 191) | (audioEarBit << 6) | (tape->inputBit << 6));
    
    return result;
}

void ZXSpectrum48::coreIOWrite(uint16_t address, uint8_t data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    if (memoryPage == 1)
    {
        contended = true;
    }
    
    ZXSpectrum::ULAApplyIOContention(address, contended);

    // ULA owned ports
    if (!(address & 0x01))
    {
        displayUpdateWithTs(static_cast<int32_t>((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset));

        // Port: 0xFE
        //   7   6   5   4   3   2   1   0
        // +---+---+---+---+---+-----------+
        // |   |   |   | E | M |  BORDER   |
        // +---+---+---+---+---+-----------+
        audioEarBit = (data & 0x10) ? 1 : 0;
        audioMicBit = (data & 0x08) ? 1 : 0;
        displayBorderColor = data & 0x07;

        //        qDebug() << static_cast<int>(audioEarBit)  ;
    }
    
    // AY-3-8912 ports
    if(address == 0xfffd && (machineInfo.hasAY || emuUseAYSound))
    {
        ULAPortnnFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && (machineInfo.hasAY || emuUseAYSound))
    {
        audioAYWriteData(data);
    }

    // SPECDRUM port, all ports ending in 0xdf
    if ((address & 0xff) == 0xdf && emuUseSpecDRUM)
    {
        // Adjust the output from SpecDrum to get the right volume
        specdrumDACValue = (data * 256) - 32768;
    }
    
	// Retroleum Smart Card - HexTank
	else if ((address & 0xfff1) == 0xfaf1)
	{
		if(address == 0xfaf3)
		{
			smartCardPortFAF3 = data;
		}
		else if(address == 0xfafb)
		{
			smartCardPortFAFB = data;
		}
	}
}

// - Memory Read/Write

void ZXSpectrum48::coreMemoryWrite(uint16_t address, uint8_t data)
{
    if (address < cROM_SIZE)
    {
		if ((smartCardPortFAF3 & 0x80) && address >= 8192 && address < 16384)
		{
			smartCardSRAM[ (address - 8192) + ((smartCardPortFAF3 & 0x07) * 8192) ] = data;
		}
		
        return;
    }
    
    if (address >= cROM_SIZE && address < cBITMAP_ADDRESS + cBITMAP_SIZE + cATTR_SIZE){
        displayUpdateWithTs(static_cast<int32_t>((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.paperDrawingOffset));
    }

    if (debugOpCallbackBlock != nullptr)
    {
        if (debugOpCallbackBlock( address, eDebugWriteOp ))
        {
            breakpointHit = true;
        }
    }
    
    breakpointHit = false;

    memoryRam[ address ] = static_cast<char>(data);
}

uint8_t ZXSpectrum48::coreMemoryRead(uint16_t address)
{
    if (address < cROM_SIZE)
    {
		if ((smartCardPortFAF3 & 0x80) && address >= 8192 && address < 16384)
		{
			return smartCardSRAM[  (address - 8192) + ((smartCardPortFAF3 & 0x07) * 8192) ];
		}
		if((address & 0xff) == 0x72)
		{
			if (smartCardPortFAFB & 0x40)
			{
                smartCardPortFAFB &= ~cFAFB_ROM_SWITCHOUT;
                smartCardPortFAF3 &= ~cFAF3_SRAM_ENABLE;
                uint8_t retOpCode = static_cast<uint8_t>(memoryRom[ address ]);
                loadROM( cDEFAULT_ROM, 0 );
				return retOpCode;
			}
		}
		
        if (debugOpCallbackBlock != nullptr)
        {
            if (debugOpCallbackBlock( address, eDebugReadOp ))
            {
                breakpointHit = true;
            }
        }

        breakpointHit = false;
        return static_cast<uint8_t>(memoryRom[address]);
    }

    if (debugOpCallbackBlock != nullptr)
    {
        debugOpCallbackBlock( address, eDebugReadOp );
    }

    return static_cast<uint8_t>(memoryRam[ address ]);
}

// - Debug Memory Read/Write

uint8_t ZXSpectrum48::coreDebugRead(uint16_t address, void *)
{
    if (address < cROM_SIZE)
    {
        return static_cast<uint8_t>(memoryRom[address]);
    }
    
    return static_cast<uint8_t>(memoryRam[address]);
}

void ZXSpectrum48::coreDebugWrite(uint16_t address, uint8_t byte, void *)
{
    if (address < cROM_SIZE)
    {
        memoryRom[address] = static_cast<char>(byte);
    }
    else
    {
        memoryRam[address] = static_cast<char>(byte);
    }
}

// - Memory Contention

void ZXSpectrum48::coreMemoryContention(uint16_t address, uint32_t)
{
    if (address >= 16384 && address <= 32767)
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

// - Release/Reset

void ZXSpectrum48::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum48::resetMachine(bool hard)
{
    if (hard)
    {
        // If a hard reset is requested, reload the default ROM and make sure that the smart card
        // ROM switch is disabled along with the smart card SRAM
        loadROM( cDEFAULT_ROM, 0 );
        smartCardPortFAFB &= ~cFAFB_ROM_SWITCHOUT;
        smartCardPortFAF3 &= ~cFAF3_SRAM_ENABLE;
    }

    emuDisplayPage = 1;
    ZXSpectrum::resetMachine(hard);
}

void ZXSpectrum48::resetToSnapLoad()
{
    loadROM( cSMART_ROM, 0 );
    resetMachine(false);
}

// - Opcode Callback Function

bool ZXSpectrum48::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum48 *machine = static_cast<ZXSpectrum48*>(param);
    CZ80Core core = machine->z80Core;
    
    if (machine->emuTapeInstantLoad)
    {
        // Trap ROM tap LOADING
        if (address == 0x056b || address == 0x0111)
        {
            if (opcode == 0xc0)
            {
                machine->emuLoadTrapTriggered = true;
                machine->tape->updateStatus();
                return true;
            }
        }
    }
    
    // Trap ROM tape SAVING
    if (opcode == 0x08 && address == 0x04d0)
    {
        if (opcode == 0x08)
        {
            machine->emuSaveTrapTriggered = true;
            machine->tape->updateStatus();
            return true;
        }
    }

    machine->emuSaveTrapTriggered = false;
    machine->emuLoadTrapTriggered = false;

    return false;
}



