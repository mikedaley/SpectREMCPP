//
//  ZXSpectrum128_2.cpp
//  SpectREM
//
//  Created by Michael Daley on 2020-01-30.
//  Copyright © 2020 Michael Daley. All rights reserved.
//

#include "ZXSpectrum128_2A.hpp"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------
// - Constants

//static const int cROM_SIZE = 16384;

static const char *cDEFAULT_ROM_0 = "plus3-0.rom";
static const char *cDEFAULT_ROM_1 = "plus3-1.rom";
static const char *cDEFAULT_ROM_2 = "plus3-2.rom";
static const char *cDEFAULT_ROM_3 = "plus3-3.rom";
// ------------------------------------------------------------------------------------------------------------
// - Constructor/Destructor

ZXSpectrum128_2A::ZXSpectrum128_2A(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum128_2A::Constructor" << "\n";
    if (t)
    {
        tapePlayer = t;
    }
    else
    {
        tapePlayer = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------------------

ZXSpectrum128_2A::~ZXSpectrum128_2A()
{
    std::cout << "ZXSpectrum128_2::Destructor" << "\n";
    release();
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum128_2A::initialise(std::string romPath)
{
    std::cout << "ZXSpectrum128_2A::initialise(char *rom)" << "\n";
    
    machineInfo = machines[ eZXSpectrum128_2A ];
    ZXSpectrum::initialise(romPath);
    z80Core.setCPUMan(CZ80Core::eCPUMAN_Zilog);
    z80Core.setCPUType(CZ80Core::eCPUTYPE_NMOS);

    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80Core.RegisterOpcodeCallback(opcodeCallback);
    
    loadROM( cDEFAULT_ROM_0, 0 );
    loadROM( cDEFAULT_ROM_1, 1 );
    loadROM( cDEFAULT_ROM_2, 2 );
    loadROM( cDEFAULT_ROM_3, 3 );

    emuROMNumber = 0;
    emuRAMPage = 0;
    emuSpecialPagingMode = true;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPort7FFDValue = 0;

}

// ------------------------------------------------------------------------------------------------------------
// - ULA
// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128_2A::coreIORead(uint16_t address)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging && (memoryPage == 5 || (memoryPage == 3 && (emuRAMPage == 4 || emuRAMPage == 5 || emuRAMPage == 6 || emuRAMPage == 7))))
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
        else if ((address & 0xc002) == 0xc000 && machineInfo.hasAY)
        {
            return audioAYReadData();
        }

        if ( (address & 0xd001) == 0 )
        {
            std::cout << "0x2FFD READ\n";
            return 0;
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
                result &= keyboardMap[i];
            }
        }
    }

    result = static_cast<uint8_t>((result & 191) | (audioEarBit << 6) | (tapePlayer->inputBit << 6));
    
    return result;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128_2A::coreIOWrite(uint16_t address, uint8_t data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging && (memoryPage == 5 || (memoryPage == 3 && (emuRAMPage == 4 || emuRAMPage == 5 || emuRAMPage == 6 || emuRAMPage == 7))))
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
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset);
        audioEarBit = (data & 0x10) ? 1 : 0;
        audioMicBit = (data & 0x08) ? 1 : 0;
        displayBorderColor = data & 0x07;
    }
    
    // AY-3-8912 ports
    if((address & 0xc002) == 0xc000 && machineInfo.hasAY)
    {
        ULAPort7FFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && machineInfo.hasAY)
    {
        audioAYWriteData(data);
    }
    
    // Memory paging port
    if ( (address & 0xc002) == 0x4000 && emuDisablePaging == false)
    {
        updatePort7FFD(data);
    }

    if ( (address & 0xf002) == 0x1000 && emuDisablePaging == false)
    {
        updatePort1FFD(data);
    }
    
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128_2A::updatePort7FFD(uint8_t data)
{
    // Save the last byte set, used when generating a Z80 snapshot
    ULAPort7FFDValue = data;
    
    if (emuDisplayPage != (((data & 0x08ul) == 0x08ul) ? 7ul : 5ul))
    {
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset);
    }
    
    // You should only be able to disable paging once. To enable paging again then a reset is necessary.
    if (data & 0x20 && emuDisablePaging != true)
    {
        emuDisablePaging = true;
    }
    
    emuROMLoBit = (data & 0x10) >> 4;
    updateROMNumber();
    emuRAMPage = (data & 0x07);
    emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
}

// ------------------------------------------------------------------------------------------------------------
// Bit 0: Paging mode. 0=normal, 1=special
// Bit 1: In normal mode, ignored.
// Bit 2: In normal mode, high bit of ROM selection. The four ROMs are:
//         ROM 0: 128k editor, menu system and self-test program
//         ROM 1: 128k syntax checker
//         ROM 2: +3DOS
//         ROM 3: 48 BASIC
// Bit 3: Disk motor; 1=on, 0=off
// Bit 4: Printer port strobe.

void ZXSpectrum128_2A::updatePort1FFD(uint8_t data)
{
    ULAPort1FFDValue = data;
    
    emuSpecialPagingMode = (data & 0x01);
    if (!emuSpecialPagingMode)
    {
        emuROMHiBit = ((data & 0x04) >> 1);
        updateROMNumber();
    }
    else
    {
        emuPagingMode = (data & 0x06) >> 1;
    }
    
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128_2A::updateROMNumber()
{
    emuROMNumber = emuROMHiBit | emuROMLoBit;
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Read/Write

void ZXSpectrum128_2A::coreMemoryWrite(uint16_t address, uint8_t data)
{
    const uint32_t page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    switch (page) {
        case 0:
            return;
            break;
            
        case 1:
            displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.paperDrawingOffset);
            memoryRam[(5 * cMEMORY_PAGE_SIZE) + address] = data;
            break;
            
        case 2:
            memoryRam[(2 * cMEMORY_PAGE_SIZE) + address] = data;
            break;
            
        case 3:
            memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address] = data;
            break;
            
        default:
            break;
    }
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128_2A::coreMemoryRead(uint16_t address)
{
    const uint32_t page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    switch (page) {
        case 0:
            return (memoryRom[(emuROMNumber * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 1:
            return (memoryRam[(5 * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 2:
            return (memoryRam[(2 * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 3:
            return (memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        default:
            return 0;
            break;
    }
}
// ------------------------------------------------------------------------------------------------------------
// - Debug Memory Read/Write

void ZXSpectrum128_2A::coreDebugWrite(uint16_t address, uint8_t byte, void *)
{
    const uint32_t page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;
    
    switch (page) {
        case 0:
            return;
            break;
            
        case 1:
            memoryRam[(5 * cMEMORY_PAGE_SIZE) + address] = byte;
            break;
            
        case 2:
            memoryRam[(2 * cMEMORY_PAGE_SIZE) + address] = byte;
            break;
            
        case 3:
            memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address] = byte;
            break;
            
        default:
            break;
    }
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128_2A::coreDebugRead(uint16_t address, void *)
{
    const uint32_t page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;
    
    switch (page) {
        case 0:
            return (memoryRom[(emuROMNumber * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 1:
            return (memoryRam[(5 * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 2:
            return (memoryRam[(2 * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        case 3:
            return (memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address]);
            break;
            
        default:
            return 0;
            break;
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Contention

void ZXSpectrum128_2A::coreMemoryContention(uint16_t address, uint32_t)
{
    const uint32_t memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (memoryPage == 5 || (memoryPage == 3 &&
          (emuRAMPage == 4 || emuRAMPage == 5 || emuRAMPage == 6 || emuRAMPage == 7)))
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Release/Reset

void ZXSpectrum128_2A::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum128_2A::resetMachine(bool hard)
{
    emuROMNumber = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPort7FFDValue = 0;
    ZXSpectrum::resetMachine(hard);
}

// ------------------------------------------------------------------------------------------------------------
// - Opcode Callback Function

bool ZXSpectrum128_2A::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum128_2A *machine = static_cast<ZXSpectrum128_2A*>(param);
    
    if (machine->emuTapeInstantLoad)
    {
        // Trap ROM tap LOADING
        if (address == 0x056b || address == 0x0111)
        {
            if (opcode == 0xc0)
            {
                machine->emuLoadTrapTriggered = true;
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
            return true;
        }
    }
    
    machine->emuSaveTrapTriggered = false;
    machine->emuLoadTrapTriggered = false;
    
    return false;
}
