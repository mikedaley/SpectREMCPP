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
    ZXSpectrum128();
    virtual ~ZXSpectrum128();
    
public:
    virtual void            initialise(string romPath);
    virtual void            loadDefaultROM();
    virtual void            release();
    virtual void            resetMachine(bool hard = true);
    virtual void            coreMemoryWrite(unsigned short address, unsigned char data);
    virtual unsigned char   coreMemoryRead(unsigned short address);
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates);
    virtual unsigned char   coreIORead(unsigned short address);
    virtual void            coreIOWrite(unsigned short address, unsigned char data);
    virtual unsigned char   coreDebugRead(unsigned short address, void *data);
    virtual void            coreDebugWrite(unsigned short address, unsigned char byte, void *data);
    static bool             opcodeCallback(unsigned char opcode, unsigned short address, void *param);
};

#endif /* ZXSpectrum128_hpp */
