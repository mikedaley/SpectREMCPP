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
    ZXSpectrum48(Tape *t);
    virtual ~ZXSpectrum48();
    
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
    
    // SPI
    virtual void            spi_write(uint8_t data);
    virtual uint8_t         spi_read();
};

#endif /* ZXSpectrum48_h */
