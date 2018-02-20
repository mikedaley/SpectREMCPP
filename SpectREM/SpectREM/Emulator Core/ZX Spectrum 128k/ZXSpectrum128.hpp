//
//  ZXSpectrum128.hpp
//  SpectREM
//
//  Created by Mike Daley on 04/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef ZXSpectrum128_hpp
#define ZXSpectrum128_hpp

#include "ZXSpectrum.hpp"

class ZXSpectrum128 : public ZXSpectrum
{
    
public:
    ZXSpectrum128(Tape *t);
    virtual ~ZXSpectrum128();
    
public:
    virtual void            initialise(string romPath) override;
    virtual void            loadROM(const char *rom) override;
    virtual void            release() override;
    virtual void            resetMachine(bool hard = true) override;
    virtual void            resetToSnapLoad() override;

    virtual void            coreMemoryWrite(unsigned short address, unsigned char data) override;
    virtual unsigned char   coreMemoryRead(unsigned short address) override;
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates) override;
    virtual unsigned char   coreIORead(unsigned short address) override;
    virtual void            coreIOWrite(unsigned short address, unsigned char data) override;
    
    virtual unsigned char   coreDebugRead(unsigned int address, void *data) override;
    virtual void            coreDebugWrite(unsigned int address, unsigned char byte, void *data) override;
    
    static bool             opcodeCallback(unsigned char opcode, unsigned short address, void *param);
    
};

#endif /* ZXSpectrum128_hpp */
