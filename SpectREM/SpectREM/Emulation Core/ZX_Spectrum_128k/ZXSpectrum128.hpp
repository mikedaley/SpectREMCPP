//
//  ZXSpectrum128.hpp
//  SpectREM
//
//  Created by Mike Daley on 04/09/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#ifndef ZXSpectrum128_hpp
#define ZXSpectrum128_hpp

#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"

class ZXSpectrum128 : public ZXSpectrum
{
    
public:
    ZXSpectrum128(Tape *t);
    virtual ~ZXSpectrum128();
    
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

    void                    UpdatePort7FFD(uint8_t data);
    
};

#endif /* ZXSpectrum128_hpp */
