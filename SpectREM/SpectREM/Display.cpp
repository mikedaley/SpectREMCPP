//
//  Display.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Spectrum Palette

unsigned int ZXSpectrum::palette[] =
{
    // Normal Colours
    0xff000000, // Black
    0xffc80000, // Blue
    0xff0000c8, // Red
    0xffc800c8, // Green
    0xff00c800, // Magenta
    0xffc8c800, // Cyan
    0xff00c8c8, // Yellow
    0xffc8c8c8, // White
    
    // Bright Colours
    0xff000000,
    0xffff0000,
    0xff0000ff,
    0xffff00ff,
    0xff00ff00,
    0xffffff00,
    0xff00ffff,
    0xffffffff
};

#pragma mark - Generate Screen

void ZXSpectrum::updateScreenWithTstates(int tStates)
{
    int memoryAddress = (displayPage * cBITMAP_ADDRESS) - machineInfo.romSize;
    
    while (tStates > 0)
    {
        int line = currentDisplayTstates / machineInfo.tsPerLine;
        int ts = currentDisplayTstates % machineInfo.tsPerLine;

        int action = displayTstateTable[line][ts];

        if (action == eDisplayBorder)
        {
            for (int i = 0; i < 8; i++)
            {
                displayBuffer[displayBufferIndex++] = palette[borderColor];
            }
        }
        else if (action == eDisplayPaper)
        {
            int y = line - (machineInfo.pxVerticalBlank + machineInfo.pxVertBorder);
            int x = (ts >> 2) - 4;
            
            uint pixelAddress = displayLineAddrTable[y] + x;
            uint attributeAddress = cBITMAP_SIZE + ((y >> 3) << 5) + x;
            
            int pixelByte = memoryRam[memoryAddress + pixelAddress];
            int attributeByte = memoryRam[memoryAddress + attributeAddress];

            // Extract the ink and paper colours from the attribute byte read in
            int ink = (attributeByte & 0x07) + ((attributeByte & 0x40) >> 3);
            int paper = ((attributeByte >> 3) & 0x07) + ((attributeByte & 0x40) >> 3);
            
            // Switch ink and paper if the flash phase has changed
            if ((frameCounter & 16) && (attributeByte & 0x80))
            {
                int tempPaper = paper;
                paper = ink;
                ink = tempPaper;
            }
            
            for (int i = 0x80; i; i >>= 1)
            {
                if (pixelByte & i)
                {
                    displayBuffer[displayBufferIndex++] = palette[ink];
                }
                else
                {
                    displayBuffer[displayBufferIndex++] = palette[paper];
                }
            }
        }
        
        currentDisplayTstates += machineInfo.tsPerChar;
        tStates -= machineInfo.tsPerChar;
    }
}

void ZXSpectrum::resetFrame()
{
    currentDisplayTstates = 0;
    displayBufferIndex = 0;
}

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
                if ((ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine) || line < pxLinePaperStart - machineInfo.pxEmuBorder)
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

