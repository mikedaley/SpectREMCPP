//
//  ZXSpectrum48.cpp
//  SpectREM
//
//  Created by Michael Daley on 23/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum48.hpp"

#include <iostream>
#include <fstream>

#pragma mark - Constants

static const int cROM_SIZE = 16384;
static const char *cROM0 = "48.ROM";

#pragma mark - Constructor/Destructor

ZXSpectrum48::ZXSpectrum48(Tape *t) : ZXSpectrum()
{
    cout << "ZXSpectrum48::Constructor" << endl;
    if (t)
    {
        tape = t;
    }
    else
    {
        tape = NULL;
    }
}

ZXSpectrum48::~ZXSpectrum48()
{
    cout << "ZXSpectrum48::Destructor" << endl;
    release();
}

#pragma mark - Initialise

void ZXSpectrum48::initialise(string romPath)
{
    cout << "ZXSpectrum48::initialise(char *rom)" << endl;
    
    machineInfo = machines[ eZXSpectrum48 ];

    ZXSpectrum::initialise(romPath);
    
    z80Core.RegisterOpcodeCallback(ZXSpectrum48::opcodeCallback);
    
    loadDefaultROM();
    
}

void ZXSpectrum48::loadDefaultROM()
{
    string romPath = emuROMPath.append( cROM0 );
    
    ifstream romFile(romPath, ios::binary|ios::ate);
    romFile.seekg(0, ios::beg);
    romFile.read(memoryRom.data(), cROM_SIZE);
}

#pragma mark - ULA

unsigned char ZXSpectrum48::coreIORead(unsigned short address)
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

        // SPI - HexTank
        if (address == spiPort)
        {
            return ZXSpectrum48::spi_read();
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

void ZXSpectrum48::coreIOWrite(unsigned short address, unsigned char data)
{
    bool contended = false;
    int memoryPage = address / cMEMORY_PAGE_SIZE;
    if (memoryPage == 1)
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
    if(address == 0xfffd && (machineInfo.hasAY || emuUseAYSound))
    {
        ULAPortFFFDValue = data;
        audioAYSetRegister(data);
    }
    
    if ((address & 0xc002) == 0x8000 && (machineInfo.hasAY || emuUseAYSound))
    {
        audioAYWriteData(data);
    }

    // SPI - HexTank
    if (address == spiPort)
    {
        return ZXSpectrum48::spi_write(data);
    }
}

#pragma mark - Memory Read/Write

void ZXSpectrum48::coreMemoryWrite(unsigned short address, unsigned char data)
{
    if (address < cROM_SIZE)
    {
        return;
    }
    
    if (address >= cROM_SIZE && address < cBITMAP_ADDRESS + cBITMAP_SIZE + cATTR_SIZE){
        displayUpdateWithTs((z80Core.GetTStates() - emuCurrentDisplayTs) + machineInfo.paperDrawingOffset);
    }
    
    memoryRam[address] = data;
}

unsigned char ZXSpectrum48::coreMemoryRead(unsigned short address)
{
    if (address < cROM_SIZE)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address];
}

#pragma mark - Debug Memory Read/Write

unsigned char ZXSpectrum48::coreDebugRead(unsigned int address, void *data)
{
    if (address < cROM_SIZE)
    {
        return memoryRom[address];
    }
    
    return memoryRam[address];
}

void ZXSpectrum48::coreDebugWrite(unsigned int address, unsigned char byte, void *data)
{
    if (address < cROM_SIZE)
    {
        memoryRom[address] = byte;
    }
    else
    {
        memoryRam[address] = byte;
    }
}

#pragma mark - Memory Contention

void ZXSpectrum48::coreMemoryContention(unsigned short address, unsigned int tStates)
{
    if (address >= 16384 && address <= 32767)
    {
        z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
    }
}

#pragma mark - Release/Reset

void ZXSpectrum48::release()
{
    ZXSpectrum::release();
}

void ZXSpectrum48::resetMachine(bool hard)
{
    emuDisplayPage = 1;
    ZXSpectrum::resetMachine(hard);
}

#pragma mark - Opcode Callback Function

bool ZXSpectrum48::opcodeCallback(unsigned char opcode, unsigned short address, void *param)
{
    ZXSpectrum48 *machine = static_cast<ZXSpectrum48*>(param);
    
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
    else if (opcode == 0x08 && address == 0x04d0)
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



