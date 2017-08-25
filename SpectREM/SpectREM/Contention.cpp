//
//  Contention.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

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
