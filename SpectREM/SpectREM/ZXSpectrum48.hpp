//
//  ZXSpectrum48.hpp
//  SpectREM
//
//  Created by Michael Daley on 23/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef ZXSpectrum48_h
#define ZXSpectrum48_h

#include "ZXSpectrum.hpp"

class ZXSpectrum48 : public ZXSpectrum
{
    
public:
    ZXSpectrum48();
    virtual ~ZXSpectrum48();
    
public:
    void                    initialise(char *rom);
    virtual void            release();
    virtual void            reset();
    
private:
    void                    loadRomWithPath(char *romPath);
    
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates);
    virtual unsigned char   coreIORead(unsigned short address);
    virtual void            coreIOWrite(unsigned short address, unsigned char data);
    
};

#endif /* ZXSpectrum48_h */
