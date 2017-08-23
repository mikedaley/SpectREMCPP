//
//  ZXSpectrum.hpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#ifndef ZXSpectrum_hpp
#define ZXSpectrum_hpp

#include <vector>

#include "Z80Core.h"

using namespace std;

class ZXSpectrum
{

private:
    typedef struct
    {
        int                 vkey;
        int                 mapEntry;
        int                 mapBit;
    } KEYBOARD_ENTRY;
    
public:
    ZXSpectrum();
    ~ZXSpectrum();

public:
    void                    initialise();
    void                    reset();
    void                    runFrame();
    void                    release();
    void                    loadRomWithPath(char *romPath);
    
private:
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
    unsigned char           keyboardMap[8];
    static KEYBOARD_ENTRY   keyboardLookupp[];
    
public:
    unsigned int            *display;

    // Machine details setup in each specific machine class
    size_t                  romSize;
    size_t                  ramSize;
    size_t                  tstatesPerFrame;
    size_t                  borderSize;
    size_t                  screenWidth;
    size_t                  screenHeight;
    size_t                  screenBufferSize;
    
};

#endif /* ZXSpectrum_hpp */
