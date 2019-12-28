//
//  ZXSpectrum48.hpp
//  SpectREM
//
//  Created by Michael Daley on 23/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#ifndef ZXSpectrum48_h
#define ZXSpectrum48_h

#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"
class ZXSpectrum48 : public ZXSpectrum
{
    
public:
    ZXSpectrum48(Tape *t);
    virtual ~ZXSpectrum48() override;
    
public:
    virtual void            initialise(string romPath) override;
    virtual void            release() override;
    virtual void            resetMachine(bool hard = true) override;
    virtual void            resetToSnapLoad() override;
    
    virtual void            coreMemoryWrite(uint16_t address, uint8_t data) override;
    virtual uint8_t         coreMemoryRead(uint16_t address) override;
    virtual void            coreMemoryContention(uint16_t address, uint32_t tStates) override;
    virtual uint8_t         coreIORead(uint16_t address) override;
    virtual void            coreIOWrite(uint16_t address, uint8_t data) override;
    
    virtual uint8_t         coreDebugRead(uint16_t address, void *data) override;
    virtual void            coreDebugWrite(uint16_t address, uint8_t byte, void *data) override;
    
    static bool             opcodeCallback(uint8_t opcode, uint16_t address, void *param);
};

#endif /* ZXSpectrum48_h */
