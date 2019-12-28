//
//  Display.cpp
//  SpectREM
//
//  Created by Michael Daley on 24/08/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

// - Spectrum displayPalette

enum
{
    eDisplayBorder = 1,
    eDisplayPaper = 2,
    eDisplayRetrace = 3
};

// - Setup

void ZXSpectrum::displaySetup()
{
    displayBuffer = new uint8_t[ screenBufferSize ]();
}

// - Generate Screen

void ZXSpectrum::displayUpdateWithTs(int32_t tStates)
{
    const uint8_t *memoryAddress = reinterpret_cast<uint8_t *>( memoryRam.data() + emuDisplayPage * cBITMAP_ADDRESS );
    const uint32_t yAdjust = ( machineInfo.pxVerticalBlank + machineInfo.pxVertBorder );
    
    // By creating a new buffer which is interpreting the display buffer as 64bits rather than 8, on 64 bit machines an
    // entire display character is copied in a single assignment
    uint64_t *displayBuffer8 = reinterpret_cast<uint64_t*>( displayBuffer ) + displayBufferIndex;
    
    const uint8_t flashMask = ( emuFrameCounter & 16 ) ? 0xff : 0x7f;
    
    while (tStates > 0)
    {
        uint32_t line = emuCurrentDisplayTs / machineInfo.tsPerLine;
        uint32_t ts = emuCurrentDisplayTs % machineInfo.tsPerLine;

        uint32_t action = displayTstateTable[ line ][ ts ];

        switch ( action ) {
                
            case eDisplayBorder:
            {
                uint64_t *colour8 = displayCLUT + ( displayBorderColor * 2048 );
                *displayBuffer8++ = *colour8;
                break;
            }

            case eDisplayPaper:
            {
                const uint32_t y = line - yAdjust;
                const uint32_t x = ( ts >> 2 ) - 4;
                
                uint32_t pixelAddress = displayLineAddrTable[ y ] + x;
                uint32_t attributeAddress = cBITMAP_SIZE + ( ( y >> 3 ) << 5 ) + x;
                
                const uint8_t pixelByte = memoryAddress[ pixelAddress ];
                uint8_t attributeByte = displayALUT[ memoryAddress[ attributeAddress ] & flashMask ];

                uint64_t *colour8 = displayCLUT + ( ( attributeByte & 0x7f ) * 256 ) + pixelByte;
                *displayBuffer8++ = *colour8;
                break;
            }
                
            default:
                break;
        }
        
        displayBufferIndex += static_cast<uint32_t>( displayBuffer8 - ( reinterpret_cast<uint64_t*>( displayBuffer ) + displayBufferIndex ) );
        emuCurrentDisplayTs += machineInfo.tsPerChar;
        tStates -= machineInfo.tsPerChar;
    }
}

// - Reset Display

void ZXSpectrum::displayFrameReset()
{
    emuCurrentDisplayTs = 0;
    displayBufferIndex = 0;
    audioBufferIndex = 0;
}

void ZXSpectrum::displayClear()
{
    if (displayBuffer)
    {
        displaySetup();
    }
}

// - Build Display Tables

void ZXSpectrum::displayBuildLineAddressTable()
{
    for(uint32_t i = 0; i < 3; i++)
    {
        for(uint32_t j = 0; j < 8; j++)
        {
            for(uint32_t k = 0; k < 8; k++)
            {
                displayLineAddrTable[ ( i << 6 ) + ( j << 3 ) + k ] = static_cast<uint16_t>(( i << 11 ) + ( j << 5 ) + ( k << 8 ));
            }
        }
    }
}

void ZXSpectrum::displayBuildTsTable()
{
    uint32_t tsRightBorderStart = ( machineInfo.pxEmuBorder / 2 ) + machineInfo.tsHorizontalDisplay;
    uint32_t tsRightBorderEnd = ( machineInfo.pxEmuBorder / 2 ) + machineInfo.tsHorizontalDisplay + ( machineInfo.pxEmuBorder / 2 );
    uint32_t tsLeftBorderStart = 0;
    uint32_t tsLeftBorderEnd = machineInfo.pxEmuBorder / 2;
    
    uint32_t pxLineTopBorderStart = machineInfo.pxVerticalBlank;
    uint32_t pxLineTopBorderEnd = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder;
    uint32_t pxLinePaperStart = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder;
    uint32_t pxLinePaperEnd = machineInfo.pxVerticalBlank + machineInfo.pxVertBorder + machineInfo.pxVerticalDisplay;
    uint32_t pxLineBottomBorderEnd = machineInfo.pxVerticalTotal - ( machineInfo.pxVertBorder - machineInfo.pxEmuBorder );
    
    for (uint32_t line = 0; line < machineInfo.pxVerticalTotal; line++)
    {
        for (uint32_t ts = 0 ; ts < machineInfo.tsPerLine; ts++)
        {
            // Screen Retrace
            if (line < machineInfo.pxVerticalBlank)
            {
                displayTstateTable[ line ][ ts ] = eDisplayRetrace;
            }
            
            // Top Border
            if (line >= pxLineTopBorderStart && line < pxLineTopBorderEnd)
            {
                if ( ( ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine ) || line < pxLinePaperStart - machineInfo.pxEmuBorder )
                {
                    displayTstateTable[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[ line ][ ts ] = eDisplayBorder;
                }
            }
            
            // Border + Paper + Border
            if (line >= pxLinePaperStart && line < pxLinePaperEnd)
            {
                if ( ( ts >= tsLeftBorderStart && ts < tsLeftBorderEnd ) || ( ts >= tsRightBorderStart && ts < tsRightBorderEnd ) )
                {
                    displayTstateTable[ line ][ ts ] = eDisplayBorder;
                }
                else if (ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine)
                {
                    displayTstateTable[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[ line ][ ts ] = eDisplayPaper;
                }
            }
            
            // Bottom Border
            if (line >= pxLinePaperEnd && line < pxLineBottomBorderEnd)
            {
                if (ts >= tsRightBorderEnd && ts < machineInfo.tsPerLine)
                {
                    displayTstateTable[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    displayTstateTable[ line ][ ts ] = eDisplayBorder;
                }
            }
        }
    }
}

/**
 Build a table that contains a colour lookup value for every combination of Bright, Paper, Ink and Pixel. This table is then
 used to populate an 8bit display buffer with an index to the colour to be used for each pixel rather than the colour data itself. The actual
 colour to be used is worked out in the Fragment Shader using a 1D lookup texture that contains the actual colour information.
 **/
void ZXSpectrum::displayBuildCLUT()
{
    int32_t tableIdx = 0;
    uint8_t *displayCLUT8 = reinterpret_cast<uint8_t *>( displayCLUT );
    
    // Bitmap LUT
    for (uint32_t bright = 0; bright < 2; bright++)
    {
        for (uint32_t paper = 0; paper < 8; paper++)
        {
            for (uint32_t ink = 0; ink < 8; ink++)
            {
                for (uint32_t pixels = 0; pixels < 256; pixels++)
                {
                    for (int8_t pixelbit = 7; pixelbit >= 0; pixelbit--)
                    {
                        displayCLUT8[ tableIdx++ ] = static_cast<uint8_t>(( pixels & ( 1 << pixelbit ) ? ink : paper ) + ( bright * 8 ));
                    }
                }
            }
        }
    }
    
    // Attribute LUT
    for (uint32_t alutIdx = 0; alutIdx < 256; ++alutIdx)
    {
        displayALUT[ alutIdx ] = static_cast<uint8_t>(alutIdx & 0x80 ? ( ( alutIdx & 0xc0 ) | ( ( alutIdx & 0x07 ) << 3 ) | ( ( alutIdx & 0x38) >> 3 ) ) : alutIdx);
    }
}

