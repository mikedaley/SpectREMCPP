//
//  Contention.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

unsigned int ZXSpectrum::contentionValues[] = { 6, 5, 4, 3, 2, 1, 0, 0 };

#pragma mark - IO Contention

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
void ZXSpectrum::applyIOContention(unsigned short address, bool contended)
{
    if (contended)
    {
        if ((address & 1) == 0)
        {
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(3);
        }
        else
        {
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(1);
        }
    } else {
        if ((address & 1) == 0)
        {
            z80Core.AddTStates(1);
            z80Core.AddContentionTStates( memoryContentionTable[z80Core.GetTStates() % machineInfo.tsPerFrame] );
            z80Core.AddTStates(3);
        }
        else
        {
            z80Core.AddTStates(4);
        }
    }
}

#pragma mark - Build Contention Table

void ZXSpectrum::buildContentionTable()
{
    for (int i = 0; i < machineInfo.tsPerFrame; i++)
    {
        memoryContentionTable[i] = 0;
        ioContentionTable[i] = 0;
        
        if (i >= machineInfo.tsToOrigin)
        {
            int line = (i - machineInfo.tsToOrigin) / machineInfo.tsPerLine;
            int ts = (i - machineInfo.tsToOrigin) % machineInfo.tsPerLine;
            
            if (line < machineInfo.pxVerticalDisplay && ts < 128)
            {
                memoryContentionTable[i] = contentionValues[ ts & 0x07 ];
                ioContentionTable[i] = contentionValues[ ts & 0x07 ];
            }
        }
    }
}
