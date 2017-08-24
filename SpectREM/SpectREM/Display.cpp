//
//  Display.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Generate Screen

void ZXSpectrum::generateScreen()
{
    int displayIndex = 0;
    
    for (int x = 0; x < screenWidth * machineInfo.pxEmuBorder; x++)
    {
        displayBuffer[displayIndex++] = 0xffbbbbbb;
    }
    
    for (int y = 0; y < 192; y++)
    {
        for (int x = 0; x < machineInfo.pxEmuBorder; x++)
        {
            displayBuffer[displayIndex++] = 0xffbbbbbb;
        }
        
        for (int x = 0; x < 256; x++)
        {
            int address = (x >> 3) + ((y & 0x07) << 8) + ((y & 0x38) << 2) + ((y & 0xc0) << 5);
            unsigned char byte = memoryRam[address];
            
            if (byte & (0x80 >> (x & 7)))
            {
                displayBuffer[displayIndex++] = 0xff000000;
            }
            else
            {
                displayBuffer[displayIndex++] = 0xffbbbbbb;
            }
        }
        
        for (int x = 0; x < machineInfo.pxEmuBorder; x++)
        {
            displayBuffer[displayIndex++] = 0xffbbbbbb;
        }
    }
    
    for (int x = 0; x < screenWidth * machineInfo.pxEmuBorder; x++)
    {
        displayBuffer[displayIndex++] = 0xffbbbbbb;
    }
    
}

#pragma mark - Build Display Tables

void ZXSpectrum::buildScreenLineAddressTable()
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            for(int k = 0; k < 8; k++)
            {
                displayLineAddrTable[(i << 6) + (j << 3) + k] = (i << 11) + (j << 5) + (k << 8);
            }
        }
    }
}

void ZXSpectrum::buildDisplayTstateTable()
{
    int tsRightBorderStart = (machineInfo.pxEmuBorder / 2) + machineInfo.tsHorizontalDisplay;
    int tsRightBorderEnd = (machineInfo.pxEmuBorder / 2) + machineInfo.tsHorizontalDisplay + (machineInfo.pxEmuBorder / 2);
    int tsLeftBorderStart = 0;
    int tsLeftBorderEnd = machineInfo.pxEmuBorder / 2;
    int pxLineTopBorderStart = machineInfo.pxVerticalBlank;
    int pxLineTopBorderEnd = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder;
    int pxLinePaperStart = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder;
    int pxLinePaperEnd = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder + machineInfo.pxVerticalDisplay;
    int pxLineBottomBorderEnd = machineInfo.pxVerticalTotal - (machineInfo.pxVertBorder - machineInfo.pxEmuBorder);
    
    for(int line = 0; line < machineInfo.pxVerticalTotal; line++)
    {
        for(int ts = 0 ; ts < machineInfo.tsPerLine; ts++)
        {
            // Screen Retrace
            if (line >= 0  && line < machineInfo.pxVerticalBlank)
            {
                displayTstateTable[line][ts] = eDisplayRetrace;
            }
            
            // Top Border
            if (line >= pxLineTopBorderStart && line < pxLineTopBorderEnd)
            {
                if ((ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine) || line < pxLinePaperStart - machineInfo.pxVertBorder)
                {
                    displayTstateTable[line][ts] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[line][ts] = eDisplayBorder;
                }
            }
            
            // Border + Paper + Border
            if (line >= pxLinePaperStart && line < pxLinePaperEnd)
            {
                if ((ts >= tsLeftBorderStart && ts < tsLeftBorderEnd) || (ts >= tsRightBorderStart && ts < tsRightBorderEnd))
                {
                    displayTstateTable[line][ts] = eDisplayBorder;
                }
                else if (ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine)
                {
                    displayTstateTable[line][ts] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[line][ts] = eDisplayPaper;
                }
            }
            
            // Bottom Border
            if (line >= pxLinePaperEnd && line < pxLineBottomBorderEnd)
            {
                if (ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine)
                {
                    displayTstateTable[line][ts] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[line][ts] = eDisplayBorder;
                }
            }
        }
    }
}

