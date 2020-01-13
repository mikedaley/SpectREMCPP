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

// ------------------------------------------------------------------------------------------------------------
// - Setup

void ZXSpectrum::displaySetup()
{
    display_buffer = new uint8_t[ display_screen_buffer_size ]();
}

// ------------------------------------------------------------------------------------------------------------
// - Generate Screen

void ZXSpectrum::displayUpdateWithTs(int32_t tStates)
{
    const uint8_t *memoryAddress = reinterpret_cast<uint8_t *>( memory_ram.data() + emu_display_page * kDisplayBitmapAddress );
    const uint32_t yAdjust = ( machine_info.pixel_vertical_blank + machine_info.pixel_vertical_border );
    
    // By creating a new buffer which is interpreting the display buffer as 64bits rather than 8, on 64 bit machines an
    // entire display character is copied in a single assignment
    uint64_t *displayBuffer8 = reinterpret_cast<uint64_t*>( display_buffer ) + display_buffer_index;
    
    const uint8_t flashMask = ( emu_frame_counter & 16 ) ? 0xff : 0x7f;
    
    while (tStates > 0)
    {
        uint32_t line = emu_current_display_ts / machine_info.ts_per_line;
        uint32_t ts = emu_current_display_ts % machine_info.ts_per_line;

        uint32_t action = display_ts_state_table[ line ][ ts ];

        switch ( action ) {
                
            case eDisplayBorder:
            {
                uint64_t *colour8 = display_clut + ( display_border_color * 2048 );
                *displayBuffer8++ = *colour8;
                break;
            }

            case eDisplayPaper:
            {
                const uint32_t y = line - yAdjust;
                const uint32_t x = ( ts >> 2 ) - 4;
                
                uint32_t pixelAddress = display_line_addr_table[ y ] + x;
                uint32_t attributeAddress = kDisplayBitmapSize + ( ( y >> 3 ) << 5 ) + x;
                
                const uint8_t pixelByte = memoryAddress[ pixelAddress ];
                uint8_t attributeByte = display_alut[ memoryAddress[ attributeAddress ] & flashMask ];

                uint64_t *colour8 = display_clut + ( ( attributeByte & 0x7f ) * 256 ) + pixelByte;
                *displayBuffer8++ = *colour8;
                break;
            }
                
            default:
                break;
        }
        
        display_buffer_index += static_cast<uint32_t>( displayBuffer8 - ( reinterpret_cast<uint64_t*>( display_buffer ) + display_buffer_index ) );
        emu_current_display_ts += machine_info.ts_per_char;
        tStates -= machine_info.ts_per_char;
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Reset Display

void ZXSpectrum::displayFrameReset()
{
    emu_current_display_ts = 0;
    display_buffer_index = 0;
    audio_buffer_index = 0;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::displayClear()
{
    if (display_buffer)
    {
        displaySetup();
    }
}

// ------------------------------------------------------------------------------------------------------------
// - Build Display Tables

void ZXSpectrum::displayBuildLineAddressTable()
{
    for(uint32_t i = 0; i < 3; i++)
    {
        for(uint32_t j = 0; j < 8; j++)
        {
            for(uint32_t k = 0; k < 8; k++)
            {
                display_line_addr_table[ ( i << 6 ) + ( j << 3 ) + k ] = static_cast<uint16_t>(( i << 11 ) + ( j << 5 ) + ( k << 8 ));
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::displayBuildTsTable()
{
    uint32_t tsRightBorderStart = ( machine_info.pixel_emulator_border / 2 ) + machine_info.ts_horizontal_display;
    uint32_t tsRightBorderEnd = ( machine_info.pixel_emulator_border / 2 ) + machine_info.ts_horizontal_display + ( machine_info.pixel_emulator_border / 2 );
    uint32_t tsLeftBorderStart = 0;
    uint32_t tsLeftBorderEnd = machine_info.pixel_emulator_border / 2;
    
    uint32_t pxLineTopBorderStart = machine_info.pixel_vertical_blank;
    uint32_t pxLineTopBorderEnd = machine_info.pixel_vertical_blank + machine_info.pixel_vertical_border;
    uint32_t pxLinePaperStart = machine_info.pixel_vertical_blank + machine_info.pixel_vertical_border;
    uint32_t pxLinePaperEnd = machine_info.pixel_vertical_blank + machine_info.pixel_vertical_border + machine_info.pixel_vertical_display;
    uint32_t pxLineBottomBorderEnd = machine_info.pixel_vertical_total - ( machine_info.pixel_vertical_border - machine_info.pixel_emulator_border );
    
    for (uint32_t line = 0; line < machine_info.pixel_vertical_total; line++)
    {
        for (uint32_t ts = 0 ; ts < machine_info.ts_per_line; ts++)
        {
            // Screen Retrace
            if (line < machine_info.pixel_vertical_blank)
            {
                display_ts_state_table[ line ][ ts ] = eDisplayRetrace;
            }
            
            // Top Border
            if (line >= pxLineTopBorderStart && line < pxLineTopBorderEnd)
            {
                if ( ( ts >= tsRightBorderEnd && ts < machine_info.ts_per_line ) || line < pxLinePaperStart - machine_info.pixel_emulator_border )
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayBorder;
                }
            }
            
            // Border + Paper + Border
            if (line >= pxLinePaperStart && line < pxLinePaperEnd)
            {
                if ( ( ts >= tsLeftBorderStart && ts < tsLeftBorderEnd ) || ( ts >= tsRightBorderStart && ts < tsRightBorderEnd ) )
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayBorder;
                }
                else if (ts >= tsRightBorderEnd && ts < machine_info.ts_per_line)
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayPaper;
                }
            }
            
            // Bottom Border
            if (line >= pxLinePaperEnd && line < pxLineBottomBorderEnd)
            {
                if (ts >= tsRightBorderEnd && ts < machine_info.ts_per_line)
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayRetrace;
                }
                else
                {
                    display_ts_state_table[ line ][ ts ] = eDisplayBorder;
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------------------
/**
 Build a table that contains a colour lookup value for every combination of Bright, Paper, Ink and Pixel. This table is then
 used to populate an 8bit display buffer with an index to the colour to be used for each pixel rather than the colour data itself. The actual
 colour to be used is worked out in the Fragment Shader using a 1D lookup texture that contains the actual colour information.
 **/
void ZXSpectrum::displayBuildCLUT()
{
    int32_t tableIdx = 0;
    uint8_t *displayCLUT8 = reinterpret_cast<uint8_t *>( display_clut );
    
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
        display_alut[ alutIdx ] = static_cast<uint8_t>(alutIdx & 0x80 ? ( ( alutIdx & 0xc0 ) | ( ( alutIdx & 0x07 ) << 3 ) | ( ( alutIdx & 0x38) >> 3 ) ) : alutIdx);
    }
}

