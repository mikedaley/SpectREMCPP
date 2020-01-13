//
//  Debugger.cpp
//  SpectREM
//
//  Created by Mike on 11/07/2018.
//  Copyright © 2018 Mike Daley Ltd. All rights reserved.
//

#include "Debug.hpp"

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Destructor

Debug::Debug()
{
    std::cout << "Debugger::Constructor" << "\n";
    byteRegisters_ = {
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

    wordRegisters_ = {
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

// ------------------------------------------------------------------------------------------------------------

Debug::~Debug()
{
    std::cout << "Debugger::Destructor" << "\n";
}

// ------------------------------------------------------------------------------------------------------------
// - Register machine

void Debug::attachMachine(ZXSpectrum * new_machine)
{
    machine = new_machine;
}

// ------------------------------------------------------------------------------------------------------------
// - Breakpoints

bool Debug::checkForBreakpoint(uint16_t address, uint8_t bpType)
{
    for (std::vector<uint32_t>::size_type i = 0; i != breakpoints_.size(); i++)
    {
        if (breakpoints_[ i ].address == address &&  breakpoints_[ i ].type & bpType)
        {
            std::cout << "BREAK ON " << bpType << " at address " << address << "\n";
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------------------------------------------------------

void Debug::addBreakpoint(uint16_t address, uint8_t type)
{
    for (std::vector<uint32_t>::size_type i = 0; i != breakpoints_.size(); i++)
    {
        if (breakpoints_[ i ].address == address)
        {
            breakpoints_[ i ].type |= type;
            return;
        }
    }

    Breakpoint bp;
    bp.address = address;
    bp.type = type;
    breakpoints_.push_back(bp);
}

// ------------------------------------------------------------------------------------------------------------

void Debug::removeBreakpoint(uint16_t address, uint8_t type)
{
    for (std::vector<long>::size_type i = 0; i != breakpoints_.size(); i++)
    {
        if (breakpoints_[ i ].address == address && (breakpoints_[ i ].type - type))
        {
            breakpoints_[ i ].type = breakpoints_[ i ].type - type;
        }
        else if (breakpoints_[ i ].address == address)
        {
            breakpoints_.erase( breakpoints_.begin() + static_cast<long>(i) );
            break;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

size_t Debug::numberOfBreakpoints()
{
    return breakpoints_.size();
}

// ------------------------------------------------------------------------------------------------------------

Debug::Breakpoint Debug::breakpoint(uint32_t index)
{
    if (index < breakpoints_.size())
    {
        return breakpoints_[ index ];
    }
    Breakpoint bp;
    bp.type = 0xff;
    return bp;
}

// ------------------------------------------------------------------------------------------------------------

uint8_t Debug::breakpointAtAddress(uint16_t address)
{
    for (size_t i = 0; i < breakpoints_.size(); i++)
    {
        if (breakpoints_[ i ].address == address)
        {
            return breakpoints_[ i ].type;
        }
    }

    return 0;
}

// ------------------------------------------------------------------------------------------------------------
// - Disassemble

void Debug::disassemble(uint16_t fromAddress, uint16_t bytes, bool hexFormat)
{
    disassembly_.clear();

    uint32_t  pc = fromAddress;

    while (pc <= fromAddress + bytes)
    {
        DisassembledOpcode dop;
        char opcode[128];
        uint16_t length = machine->z80Core.Debug_Disassemble(opcode, 128, pc, hexFormat, nullptr);

        if (length == 0)
        {
            dop.address = pc;
            dop.bytes = std::to_string(machine->z80Core.Z80CoreDebugMemRead(pc, nullptr));
            dop.mnemonic = "DB " + dop.bytes;
            pc++;
        }
        else
        {
            dop.address = pc;

            for (uint32_t i = 0; i < length; i++)
            {
                dop.bytes += std::to_string(machine->z80Core.Z80CoreDebugMemRead(static_cast<uint16_t>(pc + i), nullptr));
                dop.bytes += " ";
            }

            dop.mnemonic = opcode;

            pc += static_cast<uint16_t>(length);
        }


        disassembly_.push_back(dop);
    }
}

// ------------------------------------------------------------------------------------------------------------

Debug::DisassembledOpcode Debug::disassembly(uint32_t index)
{
    if (index < disassembly_.size())
    {
        return disassembly_[ index ];
    }

    DisassembledOpcode dop;
    return dop;
}

// ------------------------------------------------------------------------------------------------------------

size_t Debug::numberOfMnemonics()
{
    return disassembly_.size();
}

// ------------------------------------------------------------------------------------------------------------
// - Stack

size_t Debug::numberOfStackEntries()
{
    return stack_.size();
}

// ------------------------------------------------------------------------------------------------------------

void Debug::stackTableUpdate()
{
    stack_.clear();
    for (uint32_t i = machine->z80Core.GetRegister(CZ80Core::eREG_SP); i <= 0xfffe; i += 2)
    {
        uint16_t value = static_cast<uint16_t>(machine->z80Core.Z80CoreDebugMemRead(i + 1, nullptr) << 8);
        value |= machine->z80Core.Z80CoreDebugMemRead(i, nullptr);

        Stack sp;
        sp.address = i;
        sp.value = value;

        stack_.push_back(sp);
    }
}

// ------------------------------------------------------------------------------------------------------------

Debug::Stack Debug::stackAddress(uint32_t index)
{
    return stack_[ index ];
}

// ------------------------------------------------------------------------------------------------------------
// - Set Registers

bool Debug::setRegister(std::string reg, uint8_t value)
{
    std::map<std::string, CZ80Core::eZ80BYTEREGISTERS>::iterator byteit;
    for (byteit = byteRegisters_.begin(); byteit != byteRegisters_.end(); byteit++)
    {
        if (byteit->first == reg)
        {
            machine->z80Core.SetRegister(byteit->second, value);
            return true;
        }
    }

    std::map<std::string, CZ80Core::eZ80WORDREGISTERS>::iterator wordit;
    for (wordit = wordRegisters_.begin(); wordit != wordRegisters_.end(); wordit++)
    {
        if (wordit->first == reg)
        {
            machine->z80Core.SetRegister(wordit->second, value);
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------------------------------------------------------
// - Fill Memory

void Debug::fillMemory(uint16_t fromAddress, uint16_t toAddress, uint8_t value)
{
    for (uint16_t addr = fromAddress; addr < toAddress; addr++)
    {
        machine->coreDebugWrite(addr, value, nullptr);
    }
}
