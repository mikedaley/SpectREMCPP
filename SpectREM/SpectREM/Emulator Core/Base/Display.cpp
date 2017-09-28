//
//  Display.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Spectrum displayPalette



enum
{
    eDisplayBorder = 1,
    eDisplayPaper = 2,
    eDisplayRetrace = 3
};

#pragma mark - Setup

void ZXSpectrum::displaySetup()
{
    displayBuffer = new uint8_t[ screenBufferSize ]();
    displayRGBABuffer = new uint32_t[ screenBufferSize ]();
    displayBufferCopy = new ScreenBufferData[ machineInfo.tsPerFrame ]();
}

#pragma mark - Generate Screen

void ZXSpectrum::displayUpdateWithTs(int tStates)
{
    const uint8_t *memoryAddress = reinterpret_cast<uint8_t *>(memoryRam.data() + emuDisplayPage * cBITMAP_ADDRESS);
    const int32_t yAdjust = (machineInfo.pxVerticalBlank + machineInfo.pxVertBorder);
    uint64_t *displayBuffer8 = reinterpret_cast<uint64_t *>(displayBuffer + displayBufferIndex);
    
    while (tStates > 0)
    {
        int line = emuCurrentDisplayTs / machineInfo.tsPerLine;
        int ts = emuCurrentDisplayTs % machineInfo.tsPerLine;

        int action = displayTstateTable[ line ][ ts ];

        switch (action) {
                
            case eDisplayBorder:
            {
                uint64_t *colour8 = displayCLUT + (displayBorderColor * 2048);
                *displayBuffer8++ = *colour8;
                break;
            }

            case eDisplayPaper:
            {
                const int32_t y = line - yAdjust;
                const int32_t x = (ts >> 2) - 4;
                
                const uint32_t pixelAddress = displayLineAddrTable[y] + x;
                const uint32_t attributeAddress = cBITMAP_SIZE + ((y >> 3) << 5) + x;
                
                const unsigned char pixelByte = memoryAddress[ pixelAddress ];
                unsigned char attributeByte = memoryAddress[ attributeAddress ];
                
                if (emuFrameCounter & 16 && attributeByte & 0x80)
                {
                    // Switch ink and paper for the flash
                    attributeByte = (attributeByte * 0xc0) | ((attributeByte >> 3) & 7) | ((attributeByte & 7) << 3);
                }

                uint64_t *colour8 = displayCLUT + ((attributeByte & 0x7f) * 256) + pixelByte;
                *displayBuffer8++ = *colour8;
                break;
            }
                
            default:
                break;
        }
        
        displayBufferIndex += static_cast<uint32_t>(displayBuffer8 - reinterpret_cast<uint64_t *>(displayBuffer + displayBufferIndex)) / sizeof(uint64_t);
        emuCurrentDisplayTs += machineInfo.tsPerChar;
        tStates -= machineInfo.tsPerChar;
    }
}

#pragma mark - Reset Display

void ZXSpectrum::displayFrameReset()
{
    emuCurrentDisplayTs = 0;
    displayBufferIndex = 0;
    
    audioBufferIndex = 0;
    audioTsCounter = 0;
    audioTsStepCounter = 0;
}

void ZXSpectrum::displayClear()
{
    if (displayBuffer)
    {
//        delete[] displayBuffer;
//        delete[] displayBufferCopy;
        displaySetup();
    }
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

void ZXSpectrum::displayBuildCLUT()
{
    int tableIdx = 0;
    uint8_t *displayCLUT8 = reinterpret_cast<uint8_t *>(displayCLUT);
    
    for (int bright = 0; bright < 2; bright++)
    {
        for (int paper = 0; paper < 8; paper++)
        {
            for (int ink = 0; ink < 8; ink++)
            {
                for (int pixels = 0; pixels < 256; pixels++)
                {
                    for (char pixelbit = 7; pixelbit >= 0; pixelbit--)
                    {
                        displayCLUT8[ tableIdx++ ] = (pixels & (1 << pixelbit) ? ink : paper) + (bright * 8);
                    }
                }
            }
        }
    }
}

