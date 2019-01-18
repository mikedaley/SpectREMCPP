//
//  Debugger.cpp
//  SpectREM
//
//  Created by Mike on 11/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#include "Debug.hpp"

#pragma mark - Constructor/Destructor

Debug::Debug()
{
    cout << "Debugger::Constructor" << endl;
    m_byteRegisters = {
        {"A" , CZ80Core::eREG_A},
        {"F" , CZ80Core::eREG_F},
        {"B" , CZ80Core::eREG_B},
        {"C" , CZ80Core::eREG_C},
        {"D" , CZ80Core::eREG_D},
        {"E" , CZ80Core::eREG_E},
        {"H" , CZ80Core::eREG_H},
        {"L" , CZ80Core::eREG_L},
        {"A'" , CZ80Core::eREG_ALT_A},
        {"F'" , CZ80Core::eREG_ALT_F},
        {"B'" , CZ80Core::eREG_ALT_B},
        {"C'" , CZ80Core::eREG_ALT_C},
        {"D'" , CZ80Core::eREG_ALT_D},
        {"E'" , CZ80Core::eREG_ALT_E},
        {"H'" , CZ80Core::eREG_ALT_H},
        {"L'" , CZ80Core::eREG_ALT_L},
        {"I" , CZ80Core::eREG_I},
        {"R" , CZ80Core::eREG_R}
    };
    
    m_wordRegisters = {
        {"AF" , CZ80Core::eREG_AF},
        {"BC" , CZ80Core::eREG_BC},
        {"DE" , CZ80Core::eREG_DE},
        {"HL" , CZ80Core::eREG_HL},
        {"AF'" , CZ80Core::eREG_ALT_AF},
        {"BC'" , CZ80Core::eREG_ALT_BC},
        {"DE'" , CZ80Core::eREG_ALT_DE},
        {"HL'" , CZ80Core::eREG_ALT_HL},
        {"PC" , CZ80Core::eREG_PC},
        {"SP" , CZ80Core::eREG_SP}
    };
}

Debug::~Debug()
{
    cout << "Debugger::Destructor" << endl;
}

#pragma mark - Register machine

void Debug::attachMachine(ZXSpectrum * new_machine)
{
    machine = new_machine;
}

#pragma mark - Breakpoints

bool Debug::checkForBreakpoint(uint16_t address, uint8_t bpType)
{
    for (vector<unsigned long>::size_type i = 0; i != m_breakpoints.size(); i++)
    {
        if (m_breakpoints[ i ].address == address &&  m_breakpoints[ i ].type & bpType)
        {
            cout << "BREAK ON " << to_string(bpType) << " at address " << address << endl;
            return true;
        }
    }
    
    return false;
}

void Debug::addBreakpoint(uint16_t address, uint8_t type)
{
    for (vector<unsigned long>::size_type i = 0; i != m_breakpoints.size(); i++)
    {
        if (m_breakpoints[ i ].address == address)
        {
            m_breakpoints[ i ].type |= type;
            return;
        }
    }
    
    Breakpoint bp;
    bp.address = address;
    bp.type = type;
    m_breakpoints.push_back(bp);
}

void Debug::removeBreakpoint(uint16_t address, uint8_t type)
{
    for (vector<unsigned long>::size_type i = 0; i != m_breakpoints.size(); i++)
    {
        if (m_breakpoints[ i ].address == address && (m_breakpoints[ i ].type - type))
        {
            m_breakpoints[ i ].type = m_breakpoints[ i ].type - type;
        }
        else if (m_breakpoints[ i ].address == address)
        {
            m_breakpoints.erase( m_breakpoints.begin() + i );
            break;
        }
    }
}

unsigned long Debug::numberOfBreakpoints()
{
    return m_breakpoints.size();
}

Debug::Breakpoint Debug::breakpoint(unsigned long index)
{
    if (index < m_breakpoints.size())
    {
        return m_breakpoints[ index ];
    }
    Breakpoint bp;
    bp.type = 0xff;
    return bp;
}

uint8_t Debug::breakpointAtAddress(uint16_t address)
{
    for (int i = 0; i < m_breakpoints.size(); i++)
    {
        if (m_breakpoints[ i ].address == address)
        {
            return m_breakpoints[ i ].type;
        }
    }
    
    return 0;
}

#pragma mark - Disassemble

void Debug::disassemble(uint16_t fromAddress, uint16_t bytes, bool hexFormat)
{
    m_disassembly.clear();
    
    uint  pc = fromAddress;

    while (pc <= fromAddress + bytes)
    {
        DisassembledOpcode dop;
        char opcode[128];
        int length = machine->z80Core.Debug_Disassemble(opcode, 128, pc, hexFormat, NULL);
        
        if (length == 0)
        {
            dop.address = pc;
            dop.bytes = to_string(machine->z80Core.Z80CoreDebugMemRead(pc, NULL));
            dop.mnemonic = "DB " + dop.bytes;
            pc++;
        }
        else
        {
            dop.address = pc;

            for (int i = 0; i < length; i++)
            {
                dop.bytes += to_string(machine->z80Core.Z80CoreDebugMemRead(pc + i, NULL));
                dop.bytes += " ";
            }

            dop.mnemonic = opcode;

            pc += length;
        }


        m_disassembly.push_back(dop);
    }
}

Debug::DisassembledOpcode Debug::disassembly(unsigned long index)
{
    if (index < m_disassembly.size())
    {
        return m_disassembly[ index ];
    }
    
    DisassembledOpcode dop;
    return dop;
}

unsigned long Debug::numberOfMnemonics()
{
    return m_disassembly.size();
}

#pragma mark - Stack

unsigned long Debug::numberOfStackEntries()
{
    return m_stack.size();
}

void Debug::stackTableUpdate()
{
    m_stack.clear();
    for (uint i = machine->z80Core.GetRegister(CZ80Core::eREG_SP); i <= 0xfffe; i += 2)
    {
        uint16_t value = machine->z80Core.Z80CoreDebugMemRead(i + 1, NULL) << 8;
        value |= machine->z80Core.Z80CoreDebugMemRead(i, NULL);

        Stack sp;
        sp.address = i;
        sp.value = value;

        m_stack.push_back(sp);
    }
}

Debug::Stack Debug::stackAddress(unsigned long index)
{
    return m_stack[ index ];
}

#pragma mark - Set Registers

bool Debug::setRegister(string reg, unsigned int value)
{
    map<string, CZ80Core::eZ80BYTEREGISTERS>::iterator byteit;
    for (byteit = m_byteRegisters.begin(); byteit != m_byteRegisters.end(); byteit++)
    {
        if (byteit->first == reg)
        {
            machine->z80Core.SetRegister(byteit->second, value);
            return true;
        }
    }

    map<string, CZ80Core::eZ80WORDREGISTERS>::iterator wordit;
    for (wordit = m_wordRegisters.begin(); wordit != m_wordRegisters.end(); wordit++)
    {
        if (wordit->first == reg)
        {
            machine->z80Core.SetRegister(wordit->second, value);
            return true;
        }
    }
    
    return false;
}

#pragma mark - Fill Memory

void Debug::fillMemory(uint16_t fromAddress, uint16_t toAddress, uint8_t value)
{
    for (uint16_t addr = fromAddress; addr < toAddress; addr++)
    {
        machine->coreDebugWrite(addr, value, NULL);
    }
}
