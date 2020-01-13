//
//  Contention.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

const uint32_t ZXSpectrum::ula_contention_values[] = { 6, 5, 4, 3, 2, 1, 0, 0 };

// ------------------------------------------------------------------------------------------------------------
// - IO Contention

/**
 Calculate the necessary contention based on the Port number being accessed and if the port belongs to the ULA.
 All non-even port numbers below to the ULA. N:x means no contention to be added and just advance the tStates.
 C:x means that contention should be calculated based on the current tState value and then x tStates are to be
 added to the current tState count
 
 in 40 - 7F?| Low bit | Contention pattern
 ------------+---------+-------------------
 No            |  Reset   | N:1, C:3
 No            |   Set      | N:4
 Yes           |  Reset   | C:1, C:3
 Yes           |   Set      | C:1, C:1, C:1, C:1
 **/
void ZXSpectrum::ULAApplyIOContention(uint16_t address, bool contended)
{
    if (contended)
    {
        if ((address & 0x01) == 0)
        {
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(1);
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(3);
        }
        else
        {
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(1);
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(1);
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(1);
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(1);
        }
    } else {
        if ((address & 0x01) == 0)
        {
            z80_core.AddTStates(1);
            z80_core.AddContentionTStates( ula_memory_contention_table[z80_core.GetTStates() % machine_info.ts_per_frame] );
            z80_core.AddTStates(3);
        }
        else
        {
            z80_core.AddTStates(4);
        }
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Build Contention Table

void ZXSpectrum::ULABuildContentionTable()
{
    for (uint32_t i = 0; i < machine_info.ts_per_frame; i++)
    {
        ula_memory_contention_table[i] = 0;
        ula_io_contention_table[i] = 0;
        
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
