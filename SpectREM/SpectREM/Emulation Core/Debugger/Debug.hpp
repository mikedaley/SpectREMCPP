//
//  Debugger.hpp
//  SpectREM
//
//  Created by Mike on 11/07/2018.
//  Copyright © 2018 Mike Daley Ltd. All rights reserved.
//

#ifndef Debug_hpp
#define Debug_hpp

#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"

using namespace std;

// - Debugger

class Debug
{
public:
    // Holds details of each disassembled instruction
    struct DisassembledOpcode
    {
        uint16_t    address;
        string      bytes;
        string      mnemonic;
    };

    struct Breakpoint
    {
        uint16_t    address;
        uint8_t     type;
    };

    struct Stack
    {
        uint16_t    address;
        uint16_t    value;
    };

// - Methods

public:
    Debug();
    ~Debug();

    void                        attachMachine(ZXSpectrum * new_machine);

    bool                        checkForBreakpoint(uint16_t address, uint8_t type);
    void                        addBreakpoint(uint16_t address, uint8_t type);
    void                        removeBreakpoint(uint16_t address, uint8_t type);
    size_t                      numberOfBreakpoints();
    Debug::Breakpoint           breakpoint(uint32_t index);
    uint8_t                     breakpointAtAddress(uint16_t address);

    void                        disassemble(uint16_t fromAddress, uint16_t bytes, bool hexFormat);
    Debug::DisassembledOpcode   disassembly(uint32_t index);
    size_t                      numberOfMnemonics();

    size_t                      numberOfStackEntries();
    void                        stackTableUpdate();
    Stack                       stackAddress(uint32_t index);

    bool                        setRegister(string reg, uint8_t value);

    void                        fillMemory(uint16_t fromAddress, uint16_t toAddress, uint8_t value);

// - Members

protected:

    vector<DisassembledOpcode>                  m_disassembly;
    vector<Breakpoint>                          m_breakpoints;
    vector<Stack>                               m_stack;
    map<string, CZ80Core::eZ80BYTEREGISTERS>    m_byteRegisters;
    map<string, CZ80Core::eZ80WORDREGISTERS>    m_wordRegisters;

public:
    ZXSpectrum *                machine;

};

#endif /* Debugger_hpp */
