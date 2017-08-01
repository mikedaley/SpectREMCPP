//
//  ZXSpectrum48.hpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#ifndef ZXSpectrum48_hpp
#define ZXSpectrum48_hpp

#include <vector>

#include "Z80Core.h"

using namespace std;

class ZXSpectrum48
{
    
public:
    ZXSpectrum48();
    ~ZXSpectrum48();

public:
    void                    initialise(char *rom);
    void                    release();
    void                    reset();
    void                    runFrame();
    void                    loadRomWithPath(char *romPath);
    void                    generateScreen();
    
    // Core memory functions
    static unsigned char    zxSpectrumMemoryRead(unsigned short address, void *param);
    static void             zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *m);
    static void             zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *m);
    static unsigned char    zxSpectrumDebugRead(unsigned int address, void *param, void *m);
    static void             zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data);
    static unsigned char    zxSpectrumIORead(unsigned short address, void *m);
    static void             zxSpectrumIOWrite(unsigned short address, unsigned char data, void *m);

    unsigned char           coreMemoryRead(unsigned short address);
    void                    coreMemoryWrite(unsigned short address, unsigned char data);
    void                    coreMemoryContention(unsigned short address, unsigned int tStates);
    unsigned char           coreDebugRead(unsigned int address, void *data);
    void                    coreDebugWrite(unsigned int address, unsigned char byte, void *data);
    unsigned char           coreIORead(unsigned short address);
    void                    coreIOWrite(unsigned short address, unsigned char data);

protected:
    CZ80Core                z80Core;
    vector<char>            memoryRom;
    vector<char>            memoryRam;
    
public:
    unsigned int            *display;

};

#endif /* ZXSpectrum48_hpp */
