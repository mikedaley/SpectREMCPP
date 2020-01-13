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

// - Debugger

class Debug
{
public:
    // Holds details of each disassembled instruction
    struct DisassembledOpcode
    {
        uint16_t    address;
        std::string bytes;
        std::string mnemonic;
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

    bool                        setRegister(std::string reg, uint8_t value);

    void                        fillMemory(uint16_t fromAddress, uint16_t toAddress, uint8_t value);

// - Members

protected:

    std::vector<DisassembledOpcode>                     disassembly_;
    std::vector<Breakpoint>                             breakpoints_;
    std::vector<Stack>                                  stack_;
    std::map<std::string, CZ80Core::eZ80BYTEREGISTERS>  byteRegisters_;
    std::map<std::string, CZ80Core::eZ80WORDREGISTERS>  wordRegisters_;

public:
    ZXSpectrum *                machine;

};

#endif /* Debugger_hpp */
