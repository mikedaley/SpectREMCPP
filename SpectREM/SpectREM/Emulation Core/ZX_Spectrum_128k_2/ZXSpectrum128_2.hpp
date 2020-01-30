//
//  ZXSpectrum128_2.hpp
//  SpectREM
//
//  Created by Michael Daley on 2020-01-30.
//  Copyright © 2020 Michael Daley. All rights reserved.
//

#ifndef ZXSpectrum128_2_hpp
#define ZXSpectrum128_2_hpp

#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"

class ZXSpectrum128_2 : public ZXSpectrum
{
    
public:
    ZXSpectrum128_2(Tape *t);
    virtual ~ZXSpectrum128_2();
    
public:
    virtual void            initialise(std::string romPath) override;
    virtual void            release() override;
    virtual void            resetMachine(bool hard = true) override;

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

#endif /* ZXSpectrum128_2_hpp */
