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
    return m_breakpoints[ index ];
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
    return m_disassembly[ index ];
}

unsigned long Debug::numberOfMnemonics()
{
    return m_disassembly.size();
}
