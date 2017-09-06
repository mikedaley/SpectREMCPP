//
//  MachineDetails.h
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef MachineDetails_h
#define MachineDetails_h

enum
{
    eZXSpectrum48 = 0,
    eZXSpectrum128
};

// Details for each machine type being emulated
typedef struct
{
    int intLength;              // 1
    
    int tsPerFrame;             // 2
    int tsToOrigin;             // 3
    int tsPerLine;              // 4
    int tsTopBorder;            // 5
    int tsVerticalBlank;        // 6
    int tsVerticalDisplay;      // 7
    int tsHorizontalDisplay;    // 8
    int tsPerChar;              // 9
    
    int pxVertBorder;           // 10
    int pxVerticalBlank;        // 11
    int pxHorizontalDisplay;    // 12
    int pxVerticalDisplay;      // 13
    int pxHorizontalTotal;      // 14
    int pxVerticalTotal;        // 15
    int pxEmuBorder;            // 16
    
    bool hasAY;                 // 17
    bool hasPaging;             // 18
    
    // Offsets used during border and screen drawing. Calculated using trial and error!!!
    int borderDrawingOffset;    // 19
    int paperDrawingOffset;     // 20
    
    int romSize;                // 21
    int ramSize;                // 22
    
    const char *machineName ;    // 23
    
    int machineType;            // 24
    
} MachineInfo;

static MachineInfo machines[] = {
    //1   2      3      4    5      6     7      8    9  10 11  12   13   14   15   16  17    18      19  20  21     22      23                  24
    { 32, 69888, 14335, 224, 12544, 1792, 43008, 128, 4, 56, 8, 256, 192, 448, 312, 32, true, false,  10, 16, 16384, 65536,  "48k",  eZXSpectrum48 },
    { 36, 70908, 14362, 228, 12768, 1596, 43776, 128, 4, 56, 7, 256, 192, 448, 311, 32, true,  true,  12, 16, 32768, 131072, "128k", eZXSpectrum128 }
};

#endif /* MachineDetails_h */
