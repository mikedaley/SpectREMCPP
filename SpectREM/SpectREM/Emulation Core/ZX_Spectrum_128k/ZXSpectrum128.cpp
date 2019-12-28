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

// - Constants

//static const int cROM_SIZE = 16384;
static const char *cDEFAULT_ROM_0 = "128-0.ROM";
static const char *cDEFAULT_ROM_1 = "128-1.ROM";

// - Constructor/Destructor

ZXSpectrum128::ZXSpectrum128(Tape *t) : ZXSpectrum()
{
    std::cout << "ZXSpectrum128::Constructor" << std::endl;
    if (t)
    {
        tape = t;
    }
    else
    {
        tape = nullptr;
    }
}

ZXSpectrum128::~ZXSpectrum128()
{
    std::cout << "ZXSpectrum128::Destructor" << std::endl;
    release();
}

// - Initialise

void ZXSpectrum128::initialise(string romPath)
{
    std::cout << "ZXSpectrum128::initialise(char *rom)" << std::endl;
    
    machineInfo = machines[ eZXSpectrum128 ];
    ZXSpectrum::initialise(romPath);
    
    // Register an opcode callback function with the Z80 core so that opcodes can be intercepted
    // when handling things like ROM saving and loading
    z80Core.RegisterOpcodeCallback(opcodeCallback);
    
    loadROM( cDEFAULT_ROM_0, 0 );
    loadROM( cDEFAULT_ROM_1, 1 );

    emuROMPage = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortnnFDValue = 0;

}

// - ULA

uint8_t ZXSpectrum128::coreIORead(uint16_t address)
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
            UpdatePort7FFD(floatingBusData);
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

void ZXSpectrum128::coreIOWrite(uint16_t address, uint8_t data)
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
    if(address == 0xfffd && machineInfo.hasAY)
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
        UpdatePort7FFD(data);
    }
}

void ZXSpectrum128::UpdatePort7FFD(uint8_t data)
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
    
    emuROMPage = ((data & 0x10) == 0x10) ? 1 : 0;
    emuRAMPage = (data & 0x07);
    emuDisplayPage = ((data & 0x08) == 0x08) ? 7 : 5;
    
}

// - Memory Read/Write

void ZXSpectrum128::coreMemoryWrite(uint16_t address, uint8_t data)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    if (memoryPage == 0)
    {
        return;
    }
    else if (memoryPage == 1)
    {
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.paperDrawingOffset);
        memoryRam[(5 * cMEMORY_PAGE_SIZE) + address] = data;
    }
    else if (memoryPage == 2)
    {
        memoryRam[(2 * cMEMORY_PAGE_SIZE) + address] = data;
    }
    else if (memoryPage == 3)
    {
        memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address] = data;
    }
}

uint8_t ZXSpectrum128::coreMemoryRead(uint16_t address)
{
    int page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;

    if (page == 0)
    {
        return (memoryRom[(emuROMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 1)
    {
        return (memoryRam[(5 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 2)
    {
        return (memoryRam[(2 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 3)
    {
        return (memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    
    return 0;
}

// - Debug Memory Read/Write

void ZXSpectrum128::coreDebugWrite(uint16_t address, uint8_t byte, void *)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    address &= 16383;
    
    if (memoryPage == 0)
    {
        return;
    }
    else if (memoryPage == 1)
    {
        memoryRam[(5 * cMEMORY_PAGE_SIZE) + address] = byte;
    }
    else if (memoryPage == 2)
    {
        memoryRam[(2 * cMEMORY_PAGE_SIZE) + address] = byte;
    }
    else if (memoryPage == 3)
    {
        memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address] = byte;
    }
}

uint8_t ZXSpectrum128::coreDebugRead(uint16_t address, void *)
{
    int page = address / cMEMORY_PAGE_SIZE;
    address &= 16383;
    
    if (page == 0)
    {
        return (memoryRom[(emuROMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 1)
    {
        return (memoryRam[(5 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 2)
    {
        return (memoryRam[(2 * cMEMORY_PAGE_SIZE) + address]);
    }
    else if (page == 3)
    {
        return (memoryRam[(emuRAMPage * cMEMORY_PAGE_SIZE) + address]);
    }
    
    return 0;
}

// - Memory Contention

void ZXSpectrum128::coreMemoryContention(uint16_t address, uint32_t)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (memoryPage == 1 ||
        (memoryPage == 3 &&
          (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7)))
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

// - Release/Reset

void ZXSpectrum128::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum128::resetMachine(bool hard)
{
    emuROMPage = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortnnFDValue = 0;
    ZXSpectrum::resetMachine(hard);
}

void ZXSpectrum128::resetToSnapLoad()
{
    // Not implemented yet
}

// - Opcode Callback Function

bool ZXSpectrum128::opcodeCallback(uint8_t opcode, uint16_t address, void *param)
{
    ZXSpectrum128 *machine = static_cast<ZXSpectrum128*>(param);
    
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




