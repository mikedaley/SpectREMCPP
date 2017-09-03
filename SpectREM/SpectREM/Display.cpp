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
    // Normal Colours in AABBGGRR format
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

#pragma mark - Setup

void ZXSpectrum::setupDisplay()
{
    displayBuffer = new unsigned int[ screenBufferSize ];
    displayBufferCopy = new ScreenBufferData[ machineInfo.tsPerFrame ];
}

#pragma mark - Generate Screen

void ZXSpectrum::updateScreenWithTstates(int tStates)
{
    // ROM and RAM are held in separate arrays, so we need to reduce the memory location by the size of the machines ROM
    int memoryAddress = (displayPage * cBITMAP_ADDRESS) - machineInfo.romSize;
    
    while (tStates > 0)
    {
        int line = currentDisplayTstates / machineInfo.tsPerLine;
        int ts = currentDisplayTstates % machineInfo.tsPerLine;

        int action = displayTstateTable[line][ts];

        if (action == eDisplayBorder)
        {
            // Only draw the border of the border data has changed
            if (displayBufferCopy[ currentDisplayTstates ].attribute != borderColor)
            {
                displayBufferCopy[ currentDisplayTstates ].attribute = borderColor;
                displayBufferCopy[ currentDisplayTstates ].changed = true;

                for (int i = 0; i < 8; i++)
                {
                    displayBuffer[displayBufferIndex++] = palette[borderColor];
                }
            }
            else
            {
                displayBufferCopy [ currentDisplayTstates ].changed = false;
                displayBufferIndex += 8;
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

            // Only draw the bitmap if the bitmap data has changed
            if (displayBufferCopy[ currentDisplayTstates ].pixels != pixelByte ||
                displayBufferCopy[ currentDisplayTstates ].attribute != attributeByte ||
                (attributeByte & 0x80))
            {
                displayBufferCopy[ currentDisplayTstates ].pixels = pixelByte;
                displayBufferCopy[ currentDisplayTstates ].attribute = attributeByte;
                displayBufferCopy[ currentDisplayTstates ].changed = true;
                

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
            else
            {
                displayBufferCopy[ currentDisplayTstates ].changed = false;
                displayBufferIndex += 8;
            }
        }
        
        currentDisplayTstates += machineInfo.tsPerChar;
        tStates -= machineInfo.tsPerChar;
    }
}

#pragma mark - Reset Frame

void ZXSpectrum::resetFrame()
{
    currentDisplayTstates = 0;
    displayBufferIndex = 0;
    
    audioBufferIndex = 0;
    audioTsCounter = 0;
    audioTsStepCounter = 0;
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
    
    for (int line = 0; line < machineInfo.pxVerticalTotal; line++)
    {
        for (int ts = 0 ; ts < machineInfo.tsPerLine; ts++)
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

