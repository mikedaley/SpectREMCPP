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
    uint32_t interrupt_length;             // 1
    
    uint32_t ts_per_frame;            // 2
    uint32_t ts_to_origin;            // 3
    uint32_t ts_per_line;             // 4
    uint32_t ts_top_border;           // 5
    uint32_t ts_vertical_blank;       // 6
    uint32_t ts_vertical_display;     // 7
    uint32_t ts_horizontal_display;   // 8
    uint32_t ts_per_char;             // 9
    
    uint32_t pixel_vertical_border;          // 10
    uint32_t pixel_vertical_blank;       // 11
    uint32_t pixel_horizontal_display;   // 12
    uint32_t pixel_vertical_display;     // 13
    uint32_t pixel_horizontal_total;     // 14
    uint32_t pixel_vertical_total;       // 15
    uint32_t pixel_emulator_border;           // 16
    
    bool has_ay;                     // 17
    bool has_paging;                 // 18
    
    // Offsets used during border and screen drawing. Calculated using trial and error!!!
    uint32_t border_drawing_offset;   // 19
    uint32_t paper_drawing_offset;    // 20
    
    uint32_t rom_size;               // 21
    uint32_t ram_size;               // 22
    int32_t floating_bus_adjustment;         // 23
    
    const char *machine_name ;       // 24
    
    uint32_t machine_type;           // 25
    
} MachineInfo;

static const MachineInfo machines[] = {
    //1   2      3      4    5      6     7      8    9  10 11  12   13   14   15   16  17     18      19  20  21     22      23  24        25
    { 32, 69888, 14335, 224, 12544, 1792, 43008, 128, 4, 56, 8, 256, 192, 448, 312, 32, false, false,  10, 16, 16384, 65536,  -1, "48k",    eZXSpectrum48 },
    { 36, 70908, 14361, 228, 12768, 1596, 43776, 128, 4, 56, 7, 256, 192, 448, 311, 32,  true,  true,  12, 16, 32768, 131072,  1, "128k",   eZXSpectrum128 }
};

#endif /* MachineInfo_h */
