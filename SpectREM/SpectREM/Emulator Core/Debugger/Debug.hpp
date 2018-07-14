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
        string      address;
        string      bytes;
        string      mnemonic;
    };
    
    struct Breakpoint
    {
        uint16_t    address;
        uint8_t     type;
    };
    
#pragma mark - Methods
    
public:
    Debug();
    ~Debug();
    
    void                        registerMachine(ZXSpectrum * new_machine);
    
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
    uint16_t                    stackAddress(unsigned long index);
    
#pragma mark - Members

protected:
    
    vector<DisassembledOpcode>  m_disassembly;
    vector<Breakpoint>          m_breakpoints;
    vector<uint16_t>            m_stack;
    
public:
    ZXSpectrum *                machine;
        
};

#endif /* Debugger_hpp */
