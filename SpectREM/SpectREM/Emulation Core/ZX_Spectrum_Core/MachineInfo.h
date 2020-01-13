//
//  MachineDetails.h
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
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
    uint32_t intLength;             // 1
    
    uint32_t tsPerFrame;            // 2
    uint32_t tsToOrigin;            // 3
    uint32_t tsPerLine;             // 4
    uint32_t tsTopBorder;           // 5
    uint32_t tsVerticalBlank;       // 6
    uint32_t tsVerticalDisplay;     // 7
    uint32_t tsHorizontalDisplay;   // 8
    uint32_t tsPerChar;             // 9
    
    uint32_t pxVertBorder;          // 10
    uint32_t pxVerticalBlank;       // 11
    uint32_t pxHorizontalDisplay;   // 12
    uint32_t pxVerticalDisplay;     // 13
    uint32_t pxHorizontalTotal;     // 14
    uint32_t pxVerticalTotal;       // 15
    uint32_t pxEmuBorder;           // 16
    
    bool hasAY;                     // 17
    bool hasPaging;                 // 18
    
    // Offsets used during border and screen drawing. Calculated using trial and error!!!
    uint32_t borderDrawingOffset;   // 19
    uint32_t paperDrawingOffset;    // 20
    
    uint32_t romSize;               // 21
    uint32_t ramSize;               // 22
    int32_t floatBusAdjust;         // 23
    
    const char *machineName ;       // 24
    
    uint32_t machineType;           // 25
    
} MachineInfo;

static const MachineInfo machines[] = {
    //1   2      3      4    5      6     7      8    9  10 11  12   13   14   15   16  17     18      19  20  21     22      23  24        25
    { 32, 69888, 14335, 224, 12544, 1792, 43008, 128, 4, 56, 8, 256, 192, 448, 312, 32, false, false,  10, 16, 16384, 65536,  -1, "48k",    eZXSpectrum48 },
    { 36, 70908, 14361, 228, 12768, 1596, 43776, 128, 4, 56, 7, 256, 192, 448, 311, 32,  true,  true,  12, 16, 32768, 131072,  1, "128k",   eZXSpectrum128 }
};

#endif /* MachineInfo_h */
