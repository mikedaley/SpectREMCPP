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
}

Debug::~Debug()
{
    cout << "Debugger::Destructor" << endl;
}

#pragma mark - Register machine

void Debug::registerMachine(ZXSpectrum * new_machine)
{
    machine = new_machine;
}

#pragma mark - Breakpoints

bool Debug::checkForBreakpoint(uint16_t address, uint8_t type)
{
    for (vector<unsigned long>::size_type i = 0; i != m_breakpoints.size(); i++)
    {
        if (m_breakpoints[ i ].type & type)
        {
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
            dop.address = to_string(pc);
            dop.bytes = to_string(machine->z80Core.Z80CoreDebugMemRead(pc, NULL));
            dop.mnemonic = "DB " + dop.bytes;
            pc++;
        }
        else
        {
            dop.address = to_string(pc);

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
    for (unsigned int i = machine->z80Core.GetRegister(CZ80Core::eREG_SP); i <= 0xfffe; i += 2)
    {
        uint16_t address = machine->z80Core.Z80CoreDebugMemRead(i + 1, NULL) << 8;
        address |= machine->z80Core.Z80CoreDebugMemRead(i, NULL);
        m_stack.push_back(address);
    }

}

uint16_t Debug::stackAddress(unsigned long index)
{
    return m_stack[ index ];
}
