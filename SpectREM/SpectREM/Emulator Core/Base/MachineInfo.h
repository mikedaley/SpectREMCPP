//
//  MachineDetails.h
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef MachineInfo_h
#define MachineInfo_h

enum
{
    eZXSpectrum48 = 0,
    eZXSpectrum128 = 1,
    eZXSpectrum128_2 = 2
};

// Details for each machine type being emulated
typedef struct
{
    int intLength;                      // 1
    
    unsigned int tsPerFrame;            // 2
    unsigned int tsToOrigin;            // 3
    unsigned int tsPerLine;             // 4
    unsigned int tsTopBorder;           // 5
    unsigned int tsVerticalBlank;       // 6
    unsigned int tsVerticalDisplay;     // 7
    unsigned int tsHorizontalDisplay;   // 8
    unsigned int tsPerChar;             // 9
    
    unsigned int pxVertBorder;          // 10
    unsigned int pxVerticalBlank;       // 11
    unsigned int pxHorizontalDisplay;   // 12
    unsigned int pxVerticalDisplay;     // 13
    unsigned int pxHorizontalTotal;     // 14
    unsigned int pxVerticalTotal;       // 15
    unsigned int pxEmuBorder;           // 16
    
    bool hasAY;                         // 17
    bool hasPaging;                     // 18
    
    // Offsets used during border and screen drawing. Calculated using trial and error!!!
    unsigned int borderDrawingOffset;   // 19
    unsigned int paperDrawingOffset;    // 20
    
    unsigned int romSize;               // 21
    unsigned int ramSize;               // 22
    
    const char *machineName ;           // 23
    
    unsigned int machineType;           // 24
    
} MachineInfo;

static MachineInfo machines[] = {
    //1   2      3      4    5      6     7      8    9  10 11  12   13   14   15   16  17     18      19  20  21     22      23         24
    { 32, 69888, 14335, 224, 12544, 1792, 43008, 128, 4, 56, 8, 256, 192, 448, 312, 32, false, false,  10, 16, 16384, 65536,  "48k",     eZXSpectrum48 },
    { 36, 70908, 14362, 228, 12768, 1596, 43776, 128, 4, 56, 7, 256, 192, 448, 311, 32, true,   true,  12, 16, 32768, 131072, "128k",    eZXSpectrum128 },
    { 36, 70908, 14362, 228, 12768, 1596, 43776, 128, 4, 56, 7, 256, 192, 448, 311, 32, true,   true,  12, 16, 32768, 131072, "128k +2", eZXSpectrum128_2 }
};

#endif /* MachineInfo_h */
