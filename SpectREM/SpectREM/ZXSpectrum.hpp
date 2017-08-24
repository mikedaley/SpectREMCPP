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
#include <iostream>
#include <fstream>

#include "Z80Core.h"
#include "MachineDetails.h"

using namespace std;

class ZXSpectrum
{

private:
    enum
    {
        eDisplayRetrace,
        eDisplayPaper,
        eDisplayBorder
    };
    
    // Holds details of the host platforms key codes and how they map to the spectrum keyboard matrix
    typedef struct
    {
        int                 key;
        int                 mapEntry;
        int                 mapBit;
    } KEYBOARD_ENTRY;
    
public:
    ZXSpectrum();
    ~ZXSpectrum();

public:
    virtual void            initialise(char *romPath);
    void                    loadRomWithPath(char *romPath);
    void                    runFrame();
    virtual void            reset();
    virtual void            release();
    void                    keyDown(unsigned short key);
    void                    keyUp(unsigned short key);
    void                    keyFlagsChanged(unsigned short key);
    void                    resetKeyboardMap();
    
private:
    void                    generateScreen();
    void                    buildDisplayTstateTable();
    void                    buildScreenLineAddressTable();
    
    // Core memory/IO functions
    static unsigned char    zxSpectrumMemoryRead(unsigned short address, void *param);
    static void             zxSpectrumMemoryWrite(unsigned short address, unsigned char data, void *param);
    static void             zxSpectrumMemoryContention(unsigned short address, unsigned int tStates, void *param);
    static unsigned char    zxSpectrumDebugRead(unsigned int address, void *param, void *m);
    static void             zxSpectrumDebugWrite(unsigned int address, unsigned char byte, void *param, void *data);
    static unsigned char    zxSpectrumIORead(unsigned short address, void *param);
    static void             zxSpectrumIOWrite(unsigned short address, unsigned char data, void *param);

public:
    virtual unsigned char   coreMemoryRead(unsigned short address);
    virtual void            coreMemoryWrite(unsigned short address, unsigned char data);
    virtual void            coreMemoryContention(unsigned short address, unsigned int tStates);
    virtual unsigned char   coreDebugRead(unsigned int address, void *data);
    virtual void            coreDebugWrite(unsigned int address, unsigned char byte, void *data);
    virtual unsigned char   coreIORead(unsigned short address);
    virtual void            coreIOWrite(unsigned short address, unsigned char data);

protected:
    CZ80Core                z80Core;
    vector<char>            memoryRom;
    vector<char>            memoryRam;
    unsigned char           keyboardMap[8];
    static KEYBOARD_ENTRY   keyboardLookup[];
    
public:
    unsigned int            *displayBuffer;

    int                     screenWidth;
    int                     screenHeight;
    int                     screenBufferSize;
    int                     displayTstateTable[312][224];
    int                     displayLineAddrTable[192];
    
    MachineInfo             machineInfo;

};

#endif /* ZXSpectrum_hpp */






