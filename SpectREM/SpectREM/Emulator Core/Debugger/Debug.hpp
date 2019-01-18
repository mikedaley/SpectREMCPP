//
//  Debugger.hpp
//  SpectREM
//
//  Created by Mike on 11/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
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
#include "ZXSpectrum.hpp"

using namespace std;

#pragma mark - Debugger

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
    
#pragma mark - Methods
    
public:
    Debug();
    ~Debug();
    
    void                        attachMachine(ZXSpectrum * new_machine);
    
    bool                        checkForBreakpoint(uint16_t address, uint8_t type);
    void                        addBreakpoint(uint16_t address, uint8_t type);
    void                        removeBreakpoint(uint16_t address, uint8_t type);
    unsigned long               numberOfBreakpoints();
    Debug::Breakpoint           breakpoint(unsigned long index);
    uint8_t                     breakpointAtAddress(uint16_t address);
    
    void                        disassemble(uint16_t fromAddress, uint16_t bytes, bool hexFormat);
    Debug::DisassembledOpcode   disassembly(unsigned long index);
    unsigned long               numberOfMnemonics();
    
    unsigned long               numberOfStackEntries();
    void                        stackTableUpdate();
    Stack                       stackAddress(unsigned long index);
    
    bool                        setRegister(string reg, unsigned int value);
    
    void                        fillMemory(uint16_t fromAddress, uint16_t toAddress, uint8_t value);
    
#pragma mark - Members

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
