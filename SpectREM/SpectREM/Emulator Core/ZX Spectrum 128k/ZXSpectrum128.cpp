//
//  ZXSpectrum128.cpp
//  SpectREM
//
//  Created by Mike Daley on 04/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum128.hpp"

#include <iostream>
#include <fstream>

#pragma mark - Constants

static const int cROM_SIZE = 16384;
static const char *cROM0 = "128-0.ROM";
static const char *cROM1 = "128-1.ROM";

#pragma mark - Constructor/Destructor

ZXSpectrum128::ZXSpectrum128(Tape *t) : ZXSpectrum()
{
    cout << "ZXSpectrum128::Constructor" << endl;
    if (t)
    {
        tape = t;
    }
    else
    {
        tape = NULL;
    }
}

ZXSpectrum128::~ZXSpectrum128()
{
    cout << "ZXSpectrum128::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum128::initialise(string romPath)
{
    cout << "ZXSpectrum128::initialise(char *rom)" << endl;
    
    machineInfo = machines[ eZXSpectrum128 ];
    
    ZXSpectrum::initialise(romPath);
    
    z80Core.RegisterOpcodeCallback(opcodeCallback);
    
    loadDefaultROM();
    
    emuROMPage = 0;
    emuRAMPage = 0;
    emuDisplayPage = 5;
    emuDisablePaging = false;
    ULAPortFFFDValue = 0;

}

void ZXSpectrum128::loadDefaultROM()
{
    string romPath = emuROMPath;
    romPath.append( cROM0 );
    
    ifstream romFile0(romPath, ios::binary|ios::ate);
    romFile0.seekg(0, ios::beg);
    romFile0.read(memoryRom.data(), cROM_SIZE);
    romFile0.close();
    
    string romPath1 = emuROMPath;
    romPath1.append( cROM1 );
    
    ifstream romFile1(romPath1, ios::binary|ios::ate);
    romFile1.seekg(0, ios::beg);
    romFile1.read(memoryRom.data() + cROM_SIZE, cROM_SIZE);
    romFile1.close();
}

#pragma mark - ULA

unsigned char ZXSpectrum128::coreIORead(unsigned short address)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging &&
        (memoryPage == 1 ||
         (memoryPage == 3 && (emuRAMPage == 1 ||
                              emuRAMPage == 3 ||
                              emuRAMPage == 5 ||
                              emuRAMPage == 7))))
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
        
        // Getting here means that nothing has handled that port read so based on a real Spectrum
        // return the floating bus value
        return ULAFloatingBus();
    }
    
    // Check to see if the keyboard is being read and if so return any keys currently pressed
    unsigned char result = 0xff;
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
    
    result = (result & 191) | (audioEarBit << 6) | (tape->inputBit << 6);
    
    return result;
}

void ZXSpectrum128::coreIOWrite(unsigned short address, unsigned char data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (machineInfo.hasPaging &&
        (memoryPage == 1 ||
         (memoryPage == 3 && (emuRAMPage == 1 ||
                              emuRAMPage == 3 ||
                              emuRAMPage == 5 ||
                              emuRAMPage == 7))))
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
        audioEarBit = (data & 0x10) >> 4;
        audioMicBit = (data & 0x08) >> 3;
        displayBorderColor = data & 0x07;
    }
    
    // AY-3-8912 ports
    if(address == 0xfffd && machineInfo.hasAY)
    {
        ULAPortFFFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && machineInfo.hasAY)
    {
        audioAYWriteData(data);
    }
    
    // Memory paging port
    if ( address == 0x7ffd && emuDisablePaging == false)
    {
        // Save the last byte set, used when generating a Z80 snapshot
        ULAPortFFFDValue = data;
        
        if (emuDisplayPage != ((data & 0x08) == 0x08) ? 7 : 5)
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
}

#pragma mark - Memory Read/Write

void ZXSpectrum128::coreMemoryWrite(unsigned short address, unsigned char data)
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

unsigned char ZXSpectrum128::coreMemoryRead(unsigned short address)
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

#pragma mark - Debug Memory Read/Write

void ZXSpectrum128::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
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

unsigned char ZXSpectrum128::coreDebugRead(unsigned int address, void *data)
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

#pragma mark - Memory Contention

void ZXSpectrum128::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    
    if (memoryPage == 1 ||
        (memoryPage == 3 &&
          (emuRAMPage == 1 || emuRAMPage == 3 || emuRAMPage == 5 || emuRAMPage == 7)))
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

#pragma mark - Release/Reset

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
    ULAPortFFFDValue = 0;
    ZXSpectrum::resetMachine(hard);
}

#pragma mark - Opcode Callback Function

bool ZXSpectrum128::opcodeCallback(unsigned char opcode, unsigned short address, void *param)
{
    ZXSpectrum128 *machine = static_cast<ZXSpectrum128*>(param);
    
    // Trap ROM tape SAVING
    if (opcode == 0x08 && address == 0x04d0)
    {
        machine->emuSaveTrapTriggered = true;
        machine->tape->updateStatus();
        return true;
    }
    else if (machine->emuSaveTrapTriggered)
    {
        machine->emuSaveTrapTriggered = false;
        machine->tape->updateStatus();
    }
    
    // Trap ROM tap LOADING
    if (machine->emuTapeInstantLoad)
    {
        if (opcode == 0xc0 && (address == 0x056b || address == 0x0111))
        {
            machine->emuLoadTrapTriggered = true;
            machine->tape->updateStatus();
            return true;
        }
        else if (machine->emuLoadTrapTriggered)
        {
            machine->emuLoadTrapTriggered = false;
            machine->tape->updateStatus();
        }
    }

    return false;
}




