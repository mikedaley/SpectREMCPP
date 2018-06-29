//
//  SPI.cpp
//  SpectREM
//
//  Created by Mike Daley on 27/12/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum48.hpp"

static uint8_t buffered_spi_data[2] = {0xff,0xff};
static size_t buffered_spi_data_flipflop = 0;

#pragma mark - SD

#include "testsd.h"

uint8_t cmdtoken[]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr size_t  cmdtoken_size   = std::extent<decltype(cmdtoken)>::value;

const uint8_t *command_buffer = cmdtoken;
size_t command_buffer_size = cmdtoken_size;
size_t command_buffer_index = 0;
bool command_application_specific = false;

// CMD0         - GO_IDLE_STATE   - 0x40 - R1
constexpr uint8_t cmd0[]      = {0x01};
constexpr size_t  cmd0_size   = std::extent<decltype(cmd0)>::value;
// CMD1       - SEND_OP_COND    - 0x41 - R1
constexpr uint8_t cmd1[]      = {0x00};
constexpr size_t  cmd1_size   = std::extent<decltype(cmd1)>::value;
// CMD8         - END_IF_COND     - 0x48 - R7
constexpr uint8_t cmd8[]      = {0x00, 0x00, 0x00, 0x01, 0xAA};
constexpr size_t  cmd8_size   = std::extent<decltype(cmd8)>::value;
// CMD9         - SEND_CSD      - 0x49 - R1
constexpr uint8_t cmd9[]      = {0x00, 0xfe, 0x00, 0x7f, 0x00, 0x32, 0x5b, 0x5a, 0x83, 0xbf, 0xff, 0xff, 0xcf, 0x80, 0x16, 0x80, 0x00, 0x37};
constexpr size_t  cmd9_size   = std::extent<decltype(cmd9)>::value;
// CMD10        - SEND_CID      - 0x4a - R1
constexpr uint8_t cmd10[]     = {0x00, 0xfe, 0x88, 0x03, 0x02, 0x31, 0x32, 0x33, 0x32, 0x20, 0x10, 0x00, 0x00, 0xce, 0xa4, 0x00, 0x71, 0xb9};
constexpr size_t  cmd10_size  = std::extent<decltype(cmd10)>::value;
// CMD17        - READ_SINGLE_BLOCK   - 0x51 - R1
constexpr uint8_t cmd17[]     = {0x00, 0xfe};
constexpr size_t  cmd17_size  = std::extent<decltype(cmd17)>::value;
// CMD55        - APP_CMD       - 0x77 - R1
constexpr uint8_t cmd55[]     = {0x00};
constexpr size_t  cmd55_size  = std::extent<decltype(cmd55)>::value;
// CMD58        - READ_OCR      - 0x7a - R3
constexpr uint8_t cmd58[]     = {0x01, 0x00, 0xff, 0x80, 0x00};
constexpr size_t  cmd58_size  = std::extent<decltype(cmd58)>::value;
// ACMD41       - SD_SEND_OP_COND   - 0x69 - R1
constexpr uint8_t acmd41[]    = {0x00};
constexpr size_t  acmd41_size = std::extent<decltype(acmd41)>::value;

// Not supported
constexpr uint8_t cmdNoSupport[]    = {0x05};
constexpr size_t  cmdNoSupport_size = std::extent<decltype(acmd41)>::value;


void ZXSpectrum48::spi_write( uint8_t data ) {
	
    if(command_buffer==cmdtoken)
	{
		if(command_buffer_index > 0 || (command_buffer_index==0 && (data & 0xc0) == 0x40)) {
			cmdtoken[command_buffer_index++] = data;
		}
		if(command_buffer_index == command_buffer_size) {
			switch ((command_buffer[0]&0x3f) | (command_application_specific ? 0x40 : 0)) {
				case 0:
					command_buffer = cmd0;
					command_buffer_size = cmd0_size;
					break;
				case 1:
					command_buffer = cmd1;
					command_buffer_size = cmd1_size;
					break;
				case 8:
					command_buffer = cmd8;
					command_buffer_size = cmd8_size;
					break;
				case 9:
					command_buffer = cmd9;
					command_buffer_size = cmd9_size;
					break;
				case 10:
					command_buffer = cmd10;
					command_buffer_size = cmd10_size;
					break;
				case 17:
					command_buffer = cmd17;
					command_buffer_size = cmd17_size;
					break;
				case 55:
					command_buffer = cmd55;
					command_buffer_size = cmd55_size;
					break;
				case 58:
					command_buffer = cmd58;
					command_buffer_size = cmd58_size;
					break;
				case 0x40|41:
					command_buffer = acmd41;
					command_buffer_size = acmd41_size;
					command_application_specific = false;
					break;
				default:
					command_buffer = cmdNoSupport;
					command_buffer_size = cmdNoSupport_size;
					break;
			}
			command_buffer_index = 0;
		}
		data=0xff;
	} else {
		data = command_buffer[command_buffer_index++];
		if(command_buffer_index==command_buffer_size) {
			if(command_buffer==cmd17) {
				
                FILE *file = fopen("/Users/mike/Desktop/smart.dmg", "rb");
                
                unsigned int address = (cmdtoken[1] << 24) | (cmdtoken[2] << 16) | (cmdtoken[3] << 8) | (cmdtoken[4]);
                uint8_t *fileData = (uint8_t *)malloc(512);
                fseek(file, address, SEEK_CUR);
                fread(fileData, 512, 1, file);
                command_buffer = fileData;
				command_buffer_size = 512;
				command_buffer_index = 0;
                
                fclose(file);
                
			} else {
				command_buffer = cmdtoken;
				command_buffer_size = cmdtoken_size;
				command_buffer_index = 0;
			}
		}
	}
	
    buffered_spi_data[buffered_spi_data_flipflop] = data;
    buffered_spi_data_flipflop = 1 - buffered_spi_data_flipflop;
}

uint8_t ZXSpectrum48::spi_read()
{
    return buffered_spi_data[buffered_spi_data_flipflop];
}
