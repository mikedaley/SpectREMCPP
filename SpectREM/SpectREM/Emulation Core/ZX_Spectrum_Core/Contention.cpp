//
//  Contention.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

const uint32_t ZXSpectrum::ULAConentionValues[] = { 6, 5, 4, 3, 2, 1, 0, 0 };

// - IO Contention

/**
 Calculate the necessary contention based on the Port number being accessed and if the port belongs to the ULA.
 All non-even port numbers below to the ULA. N:x means no contention to be added and just advance the tStates.
 C:x means that contention should be calculated based on the current tState value and then x tStates are to be
 added to the current tState count
 
  in 40 - 7F?| Low bit | Contention pattern
 ------------+---------+-------------------
 No    |  Reset  | N:1, C:3
 No    |   Set   | N:4
 Yes   |  Reset  | C:1, C:3
 Yes   |   Set   | C:1, C:1, C:1, C:1
 **/
void ZXSpectrum::ULAApplyIOContention(uint16_t address, bool contended)
{
    if (contended)
    {
        if ((address & 0x01) == 0)
        {
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(3);
        }
        else
        {
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
        }
    } else {
        if ((address & 0x01) == 0)
        {
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( ULAMemoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(3);
        }
        else
        {
            z80Core.AddTStates(4);
        }
    }
}

// - Build Contention Table

void ZXSpectrum::ULABuildContentionTable()
{
    for (uint32_t i = 0; i < machineInfo.tsPerFrame; i++)
    {
        ULAMemoryContentionTable[i] = 0;
        ULAIOContentionTable[i] = 0;
        
        if (i >= machineInfo.tsToOrigin)
        {
            uint32_t line = (i - machineInfo.tsToOrigin) / machineInfo.tsPerLine;
            uint32_t ts = (i - machineInfo.tsToOrigin) % machineInfo.tsPerLine;
            
            if (line < machineInfo.pxVerticalDisplay && ts < 128)
            {
                ULAMemoryContentionTable[i] = ULAConentionValues[ ts & 0x07 ];
                ULAIOContentionTable[i] = ULAConentionValues[ ts & 0x07 ];
            }
        }
    }
}
