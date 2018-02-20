//
//  ULAFloatingBus.cpp
//  SpectREM
//
//  Created by Michael Daley on 25/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

/**
 When the Z80 reads from an unattached port, such as 0xFF, it actually reads the data currently on the
 Spectrums ULA data bus. This may happen to be a byte being transferred from screen memory. If the ULA
 is building the border then the bus is idle and the return value is 0xFF, otherwise its possible to
 predict if the ULA is reading a pixel or attribute byte based on the current t-state.
 This routine works out what would be on the ULA bus for a given tstate and returns the result
 **/
uint8_t ZXSpectrum::ULAFloatingBus()
{
    int32_t cpuTs = z80Core.GetTStates() + machineInfo.floatBusAdjust;
    int32_t currentDisplayLine = cpuTs / machineInfo.tsPerLine;
    int32_t currentTs = cpuTs % machineInfo.tsPerLine;
    
    // If the line and tState are within the paper area of the screen then grab the
    // pixel or attribute value which is determined by looking at the current tState
    if (currentDisplayLine >= (machineInfo.pxVertBorder + machineInfo.pxVerticalBlank)
        && currentDisplayLine < (machineInfo.pxVertBorder + machineInfo.pxVerticalBlank + machineInfo.pxVerticalDisplay)
        && currentTs < machineInfo.tsHorizontalDisplay)
    {
        
        int32_t y = currentDisplayLine - (machineInfo.pxVertBorder + machineInfo.pxVerticalBlank);
        int32_t x = currentTs >> 2;
        
        switch(currentTs % 8)
        {
            case 5:
            case 3:
                return coreMemoryRead(cBITMAP_ADDRESS + cBITMAP_SIZE + ((y >> 3) << 5) + x);
                
            case 4:
            case 2:
                return coreMemoryRead(cBITMAP_ADDRESS + displayLineAddrTable[ y ] + x);
            
            case 0: case 1: case 6: case 7:
                return 0xff;
        }
        
    }
    
    return 0xff;
}
