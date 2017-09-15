//
//  Display.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Spectrum displayPalette

const unsigned int displayPalette[] =
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

enum
{
    eDisplayBorder = 1,
    eDisplayPaper,
    eDisplayRetrace
};

#pragma mark - Setup

void ZXSpectrum::displaySetup()
{
    displayBuffer = new unsigned int[ screenBufferSize ]();
    displayBufferCopy = new ScreenBufferData[ machineInfo.tsPerFrame ]();
}

#pragma mark - Generate Screen

void ZXSpectrum::displayUpdateWithTs(int tStates)
{
    const int memoryAddress = emuDisplayPage * cBITMAP_ADDRESS;
    
    while (tStates > 0)
    {
        int line = emuCurrentDisplayTs / machineInfo.tsPerLine;
        int ts = emuCurrentDisplayTs % machineInfo.tsPerLine;

        int action = displayTstateTable[ line ][ ts ];

        switch (action) {
                
            case eDisplayBorder:
            {
                if (displayBufferCopy[ emuCurrentDisplayTs ].attribute != displayBorderColor)
                {
                    displayBufferCopy[ emuCurrentDisplayTs ].attribute = displayBorderColor;
                    
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                    displayBuffer[ displayBufferIndex++ ] = displayPalette[ displayBorderColor ];
                }
                else
                {
                    displayBufferIndex += 8;
                }
                break;
            }
                
            case eDisplayPaper:
            {
                const int y = line - (machineInfo.pxVerticalBlank + machineInfo.pxVertBorder);
                const int x = (ts >> 2) - 4;
                
                const uint pixelAddress = displayLineAddrTable[y] + x;
                const uint attributeAddress = cBITMAP_SIZE + ((y >> 3) << 5) + x;
                
                const int pixelByte = memoryRam[ memoryAddress + pixelAddress ];
                const int attributeByte = memoryRam[ memoryAddress + attributeAddress ];
                
                // Only draw the bitmap if the bitmap data has changed
                if (displayBufferCopy[ emuCurrentDisplayTs ].pixels != pixelByte ||
                    displayBufferCopy[ emuCurrentDisplayTs ].attribute != attributeByte ||
                    (attributeByte & 0x80))
                {
                    displayBufferCopy[ emuCurrentDisplayTs ].pixels = pixelByte;
                    displayBufferCopy[ emuCurrentDisplayTs ].attribute = attributeByte;
                    
                    // Extract the ink and paper colours from the attribute byte read in
                    int ink = (attributeByte & 0x07) + ((attributeByte & 0x40) >> 3);
                    int paper = ((attributeByte >> 3) & 0x07) + ((attributeByte & 0x40) >> 3);
                    
                    // Switch ink and paper if the flash phase has changed
                    if ((emuFrameCounter & 16) && (attributeByte & 0x80))
                    {
                        const int tempPaper = paper;
                        paper = ink;
                        ink = tempPaper;
                    }
                    
                    for (int i = 0x80; i; i >>= 1)
                    {
                        if (pixelByte & i)
                        {
                            displayBuffer[ displayBufferIndex++ ] = displayPalette[ink];
                        }
                        else
                        {
                            displayBuffer[ displayBufferIndex++ ] = displayPalette[paper];
                        }
                    }
                }
                else
                {
                    displayBufferIndex += 8;
                }
                break;
            }
                
            default:
                break;
        }
        
        emuCurrentDisplayTs += machineInfo.tsPerChar;
        tStates -= machineInfo.tsPerChar;
    }
}

#pragma mark - Reset Frame

void ZXSpectrum::displayFrameReset()
{
    emuCurrentDisplayTs = 0;
    displayBufferIndex = 0;
    
    audioBufferIndex = 0;
    audioTsCounter = 0;
    audioTsStepCounter = 0;
}

#pragma mark - Build Display Tables

void ZXSpectrum::displayBuildLineAddressTable()
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

void ZXSpectrum::displayBuildTsTable()
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

