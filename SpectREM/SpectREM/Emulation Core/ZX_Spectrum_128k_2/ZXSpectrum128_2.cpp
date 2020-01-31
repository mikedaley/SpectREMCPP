//
//  ZXSpectrum128_2.cpp
//  SpectREM
//
//  Created by Michael Daley on 2020-01-30.
//  Copyright © 2020 Michael Daley. All rights reserved.
//

#include "ZXSpectrum128_2.hpp"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------
// - Constants

//static const int cROM_SIZE = 16384;
static const char *cDEFAULT_ROM_0 = "plus2-0.ROM";
static const char *cDEFAULT_ROM_1 = "plus2-1.ROM";
// ------------------------------------------------------------------------------------------------------------
// - Constructor/Destructor

ZXSpectrum128_2::ZXSpectrum128_2(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum128_2::Constructor" << "\n";
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

ZXSpectrum128_2::~ZXSpectrum128_2()
{
    std::cout << "ZXSpectrum128_2::Destructor" << "\n";
    release();
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum128_2::initialise(std::string romPath)
{
    std::cout << "ZXSpectrum128_2::initialise(char *rom)" << "\n";
    
    machineInfo = machines[ eZXSpectrum128_2 ];
    ZXSpectrum::initialise(romPath);
    z80Core.setCPUMan(CZ80Core::eCPUMAN_Zilog);
    z80Core.setCPUType(CZ80Core::eCPUTYPE_CMOS);

    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80Core.RegisterOpcodeCallback(opcodeCallback);
    
    loadROM( cDEFAULT_ROM_0, 0 );
    loadROM( cDEFAULT_ROM_1, 1 );

    emuROMNumber = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortnnFDValue = 0;

}

// ------------------------------------------------------------------------------------------------------------
// - ULA
// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum128_2::coreIORead(uint16_t address)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging && (memoryPage == 1 || (memoryPage == 3 && (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7))))
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
        
        // port 7FFD memory port read bug on the 128k. When reading from 0x7FFD, it actually performs a right to the port
        // with what is on the floating bus.
        if ( (address & 0x8002) == 0)
        {
            uint8_t floatingBusData = ULAFloatingBus();
            uint32_t currentTStates = z80Core.GetTStates();
            updatePort7FFD(floatingBusData);
            z80Core.ResetTStates(z80Core.GetTStates() - currentTStates);
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

void ZXSpectrum128_2::coreIOWrite(uint16_t address, uint8_t data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging && (memoryPage == 1 || (memoryPage == 3 && (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7))))
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
        ULAPortnnFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && machineInfo.hasAY)
    {
        audioAYWriteData(data);
    }
    
    // Memory paging port
    if ( (address & 0x8002) == 0 && emuDisablePaging == false)
    {
        updatePort7FFD(data);
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum128_2::updatePort7FFD(uint8_t data)
{
    // Save the last byte set, used when generating a Z80 snapshot
    ULAPortnnFDValue = data;
    
    if (emuDisplayPage != (((data & 0x08ul) == 0x08ul) ? 7ul : 5ul))
    {
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.borderDrawingOffset);
    }
    
    // You should only be able to disable paging once. To enable paging again then a reset is necessary.
    if (data & 0x20 && emuDisablePaging != true)
    {
        emuDisablePaging = true;
    }
    
    emuROMNumber = ((data & 0x10) == 0x10) ? 1 : 0;
    emuRAMPage = (data & 0x07);
    emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
    
}
    
// ------------------------------------------------------------------------------------------------------------
// - Memory Read/Write

void ZXSpectrum128_2::coreMemoryWrite(uint16_t address, uint8_t data)
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

uint8_t ZXSpectrum128_2::coreMemoryRead(uint16_t address)
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

void ZXSpectrum128_2::coreDebugWrite(uint16_t address, uint8_t byte, void *)
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

uint8_t ZXSpectrum128_2::coreDebugRead(uint16_t address, void *)
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

void ZXSpectrum128_2::coreMemoryContention(uint16_t address, uint32_t)
{
    const uint32_t memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (memoryPage == 1 ||
        (memoryPage == 3 &&
          (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7)))
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Release/Reset

void ZXSpectrum128_2::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum128_2::resetMachine(bool hard)
{
    emuROMNumber = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortnnFDValue = 0;
    ZXSpectrum::resetMachine(hard);
}

// ------------------------------------------------------------------------------------------------------------
// - Opcode Callback Function

bool ZXSpectrum128_2::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum128_2 *machine = static_cast<ZXSpectrum128_2*>(param);
    
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
