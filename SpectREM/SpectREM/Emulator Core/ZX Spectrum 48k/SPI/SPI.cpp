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

void ZXSpectrum48::spi_write( uint8_t data ) {
    buffered_spi_data[buffered_spi_data_flipflop] = data;
    buffered_spi_data_flipflop = 1 - buffered_spi_data_flipflop;
}

uint8_t ZXSpectrum48::spi_read()
{
    return buffered_spi_data[buffered_spi_data_flipflop];
}
