//
//  ULAFloatingBus.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

// ------------------------------------------------------------------------------------------------------------

/**
 When the Z80 reads from an unattached port, such as 0xFF, it actually reads the data currently on the
 Spectrums ULA data bus. This may happen to be a byte being transferred from screen memory. If the ULA
 is building the border then the bus is idle and the return value is 0xFF, otherwise its possible to
 predict if the ULA is reading a pixel or attribute byte based on the current t-state.
 This routine works out what would be on the ULA bus for a given tstate and returns the result
 **/
uint8_t ZXSpectrum::ULAFloatingBus()
{
    // The Z80 samples the data bus on the last tState of IO, so having already applied contention to the cores tStates
    // we need to reduce the current tStates by 1 to get the correct data bus value
    uint32_t cpu_ts = z80_core.GetTStates() + static_cast<uint32_t>(machine_info.floating_bus_adjustment);
    uint32_t current_display_line = cpu_ts / machine_info.ts_per_line;
    uint32_t current_ts = cpu_ts % machine_info.ts_per_line;
    
    // If the line and tState are within the paper area of the screen then grab the
    // pixel or attribute value which is determined by looking at the current tState
    if (current_display_line >= (machine_info.pixel_vertical_border + machine_info.pixel_vertical_blank)
        && current_display_line < (machine_info.pixel_vertical_border + machine_info.pixel_vertical_blank + machine_info.pixel_vertical_display)
        && current_ts < machine_info.ts_horizontal_display)
    {
        uint32_t y = current_display_line - (machine_info.pixel_vertical_border + machine_info.pixel_vertical_blank);
        uint32_t x = current_ts >> 2;
        
        switch(current_ts % 8)
        {
            case 5: // Attribute read
            case 3:
                return static_cast<uint8_t>(coreMemoryRead(static_cast<uint16_t>(kDisplayBitmapAddress + kDisplayBitmapSize + ((y >> 3) << 5) + x)));
                
            case 4: // Bitmap read
            case 2:
                return coreMemoryRead(static_cast<uint16_t>(kDisplayBitmapAddress + display_line_addr_table[ y ] + x));
            
            case 0: case 1: case 6: case 7: // Everything else
                return 0xff;
        }
    }
    
    return 0xff;
}

// ------------------------------------------------------------------------------------------------------------
// - Build Float bus state table

void ZXSpectrum::ULABuildFloatingBusTable()
{
    for (uint32_t i = 0; i < machine_info.ts_per_frame; i++)
    {
        ula_floating_bus_table[ i ] = 0;
        
        if (i >= machine_info.ts_to_origin)
        {
            uint32_t line = (i - machine_info.ts_to_origin) / machine_info.ts_per_line;
            uint32_t ts = (i - machine_info.ts_to_origin) % machine_info.ts_per_line;
            
            if (line < machine_info.pixel_vertical_display && ts < 128)
            {
                ula_memory_contention_table[i] = ula_contention_values[ ts & 0x07 ];
                ula_io_contention_table[i] = ula_contention_values[ ts & 0x07 ];
            }
        }
    }
    
    
}
