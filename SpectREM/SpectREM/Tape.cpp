//
//  Tape.cpp
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "ZXSpectrum.hpp"

#pragma mark - Constants

//static int const cHEADER_LENGTH = 21;

static const int cPILOT_HEADER_PULSES = 8063;
static const int cPILOT_DATA_PULSES = 3223;
static const int cPILOT_PULSE_TSTATE_LENGTH = 2168;
static const int cFIRST_SYNC_PULSE_TSTATE_DELAY = 667;
static const int cSECOND_SYNC_PULSE_TSTATE_DELAY = 735;
static const int cDATA_BIT_ZERO_PULSE_TSTATE_DELAY = 855;
static const int cDATA_BIT_ONE_PULSE_TSTATE_DELAY = 1710;

static const int cHEADER_FLAG_OFFSET = 0;
static const int cHEADER_DATA_TYPE_OFFSET = 1;
static const int cHEADER_FILENAME_OFFSET = 2;
//static int cHEADER_DATA_LENGTH_OFFSET = 12;
static const int cHEADER_CHECKSUM_OFFSET = 17;

static const int cPROGRAM_HEADER_AUTOSTART_LINE_OFFSET = 14;
static const int cPROGRAM_HEADER_PROGRAM_LENGTH_OFFSET = 16;
//static int cPROGRAM_HEADER_CHECKSUM_OFFSET = 18;

//static int cNUMERIC_DATA_HEADER_UNUSED_1_OFFSET = 14;
static const int cNUMERIC_DATA_HEADER_VARIBABLE_NAME_OFFSET = 15;
//static int cNUMERIC_DATA_HEADER_UNUSED_2_OFFSET = 16;

//static int cALPHA_NUMERIC_DATA_HEADER_UNUSED_1_OFFSET = 14;
static const int cALPHA_NUMERIC_DATA_HEADER_VARIABLE_NAME_OFFSET = 15;
//static int cALPHA_NUMERIC_DATA_HEADER_UNUSED_2_OFFSET = 16;

static const int cBYTE_HEADER_START_ADDRESS_OFFSET = 14;
//static int cBYTE_HEADER_UNUSED_1_OFFSET = 16;

static const int cDATA_BLOCK_DATA_LENGTH_OFFSET = 1;

static const int cHEADER_FILENAME_LENGTH = 10;

static const int cHEADER_BLOCK_LENGTH = 19;

#pragma mark - TapeBlock

TapeBlock::~TapeBlock()
{

}

unsigned char TapeBlock::getFlag()
{
    return blockData[ cHEADER_FLAG_OFFSET ];
}

unsigned char TapeBlock::getDataType()
{
    return blockData[ cHEADER_DATA_TYPE_OFFSET ];
}

const char * TapeBlock::getFilename()
{
    const char *filename = (char *)calloc(cHEADER_FILENAME_LENGTH, sizeof(char));
    memcpy(&filename, &blockData[ cHEADER_FILENAME_OFFSET ], cHEADER_FILENAME_LENGTH);
    return filename;
}

unsigned short TapeBlock::getDataLength()
{
    return blockLength;
}

unsigned char TapeBlock::getChecksum()
{
    return blockData[ cHEADER_CHECKSUM_OFFSET ];
}

#pragma mark - Program Header

unsigned short ProgramHeader::getAutoStartLine()
{
    return blockData[ cPROGRAM_HEADER_AUTOSTART_LINE_OFFSET ];
}

unsigned short ProgramHeader::getProgramLength()
{
    return blockData[ cPROGRAM_HEADER_PROGRAM_LENGTH_OFFSET ];
}

unsigned short ProgramHeader::getBlockLength()
{
    return cHEADER_BLOCK_LENGTH;
}

#pragma mark - Numeric Header

unsigned char NumericDataHeader::getVariableName()
{
    return blockData[ cNUMERIC_DATA_HEADER_VARIBABLE_NAME_OFFSET ];
}

unsigned short NumericDataHeader::getDataLength()
{
    return cHEADER_BLOCK_LENGTH - 2;
}

#pragma mark - Alphanumeric Header

unsigned char AlphanumericDataHeader::getVariableName()
{
    return blockData[ cALPHA_NUMERIC_DATA_HEADER_VARIABLE_NAME_OFFSET ];
}

unsigned short AlphanumericDataHeader::getDataLength()
{
    return cHEADER_BLOCK_LENGTH - 2;
}

#pragma mark - Byter Header

unsigned short ByteHeader::getStartAddress()
{
    return ((unsigned char *)&blockData[ cBYTE_HEADER_START_ADDRESS_OFFSET ])[0];
}

unsigned char ByteHeader::getChecksum()
{
    return blockData[ blockLength - 1 ];
}

#pragma mark - Data Block

unsigned char *DataBlock::getDataBlock()
{
    unsigned char *dataBlock = (unsigned char *)calloc(getDataLength(), sizeof(unsigned char));
    memcpy(dataBlock, &blockData[cDATA_BLOCK_DATA_LENGTH_OFFSET], sizeof(unsigned char) * getDataLength());
    return dataBlock;
}

unsigned char DataBlock::getDataType()
{
    return blockData[ cHEADER_FLAG_OFFSET ];
}

unsigned char DataBlock::getChecksum()
{
    return blockData[ blockLength - 1 ];
}

#pragma mark - TAP Processing

void ZXSpectrum::tapeReset(bool clearBlocks)
{
    tapeInputBit = 0;
    tapeCurrentBytePtr = 0;
    tapeCurrentDataBit = 0;
    tapePilotPulseTStates = 0;
    tapePilotPulses = 0;
    tapeDataPulseTStates = 0;
    tapeFlipTapeBit = true;
    tapePlaying = false;
    tapeNewBlock = true;
    tapeCurrentBytePtr = 0;
    tapeCurrentBlockIndex = 0;
    
    if (clearBlocks)
    {
        tapeBlocks.clear();
    }
}

bool ZXSpectrum::tapeLoadWithPath(const char *path)
{
    FILE *fileHandle;
    
    fileHandle = fopen(path, "rb");
    
    if (!fileHandle)
    {
        cout << "ERROR LOADING TAPE: " << path << endl;
        tapeLoaded = false;
        fclose(fileHandle);
        return false;
    }
    
    fseek(fileHandle, 0, SEEK_END);
    long size = ftell(fileHandle);
    fseek(fileHandle, 0, SEEK_SET);
    
    unsigned char fileBytes[size];
    
    fread(&fileBytes, 1, size, fileHandle);

    tapeReset(true);
    
    unsigned short blockLength = 0;
    unsigned char flag = 0;
    unsigned char dataType = 0;
    
    while (tapeCurrentBytePtr < size)
    {
        blockLength = ((unsigned short *)&fileBytes[ tapeCurrentBytePtr ])[0];
        
        // Move the byte pointer to the top of the actual TAP block
        tapeCurrentBytePtr += 2;
        
        flag = fileBytes[ tapeCurrentBytePtr  + cHEADER_FLAG_OFFSET ];
        dataType = fileBytes[ tapeCurrentBytePtr + cHEADER_DATA_TYPE_OFFSET ];
        
        TapeBlock *newTapeBlock;
        
        if (dataType == ePROGRAM_HEADER && flag != 0xff)
        {
            newTapeBlock = new ProgramHeader;
            newTapeBlock->blockType = ePROGRAM_HEADER;
        }
        else if (dataType == eNUMERIC_DATA_HEADER && flag != 0xff)
        {
            newTapeBlock = new NumericDataHeader;
            newTapeBlock->blockType = eNUMERIC_DATA_HEADER;
        }
        else if (dataType == eALPHANUMERIC_DATA_HEADER && flag != 0xff)
        {
            newTapeBlock = new AlphanumericDataHeader;
            newTapeBlock->blockType = eALPHANUMERIC_DATA_HEADER;
        }
        else if (dataType == eBYTE_HEADER && flag != 0xff)
        {
            newTapeBlock = new ByteHeader;
            newTapeBlock->blockType = eBYTE_HEADER;
        }
        else
        {
            newTapeBlock = new DataBlock;
            newTapeBlock->blockType = eDATA_BLOCK;
        }
        
        if (!newTapeBlock)
        {
            cout << "INVALID FLAG FOUND PROCESSING TAP" << endl;
            return false;
        }
        
        newTapeBlock->blockLength = blockLength;
        newTapeBlock->blockData = (unsigned char *)calloc(blockLength, sizeof(unsigned char));
        memcpy(newTapeBlock->blockData, &fileBytes[ tapeCurrentBytePtr ], blockLength);
        
        tapeBlocks.push_back(*newTapeBlock);
        
        tapeCurrentBytePtr += blockLength;
    }
    
    tapeLoaded = true;
    
    return true;
}

void ZXSpectrum::tapeUpdateWithTs(int tStates)
{
    if (tapeCurrentBlockIndex > tapeBlocks.size() - 1)
    {
        cout << "TAPE STOPPED" << endl;
        tapePlaying = false;
        tapeInputBit = 0;
        tapeCurrentBlockIndex = static_cast<int>(tapeBlocks.size()) - 1;
        return;
    }
    
    if (tapeNewBlock)
    {
        tapeNewBlock = false;
        
        tapeCurrentBlock = &tapeBlocks[ tapeCurrentBlockIndex ];
        
        if (tapeCurrentBlock->blockType == ePROGRAM_HEADER ||
            tapeCurrentBlock->blockType == eNUMERIC_DATA_HEADER ||
            tapeCurrentBlock->blockType == eALPHANUMERIC_DATA_HEADER ||
            tapeCurrentBlock->blockType == eBYTE_HEADER)
        {
            tapeProcessingState = eHEADER_PILOT;
            tapeNextProcessingState = eHEADER_DATA_STREAM;
        }
        else if (tapeCurrentBlock->blockType == eDATA_PILOT)
        {
            tapeProcessingState = eDATA_PILOT;
            tapeNextProcessingState = eDATA_STREAM;
        }
        
        tapeCurrentBytePtr = 0;
        tapeCurrentDataBit = 0;
        tapePilotPulseTStates = 0;
        tapePilotPulses = 0;
        tapeDataPulseTStates = 0;
        tapeFlipTapeBit = true;
    }
    
    switch (tapeProcessingState)
    {
        case eHEADER_PILOT:
            tapeGenerateHeaderPilotWithTs(tStates);
            break;
        case eSYNC1:
            tapeGenerateSync1WithTs(tStates);
            break;
        case eSYNC2:
            tapeGenerateSync2WithTs(tStates);
            break;
        case eDATA_PILOT:
            tapeGenerateDataPilotWithTs(tStates);
            break;
        case eDATA_STREAM:
            tapeGenerateDataStreamWithTs(tStates);
            break;
        case eHEADER_DATA_STREAM:
            tapeGenerateHeaderDataStreamWithTs(tStates);
            break;
        case eDATA_BIT:
            tapeGenerateDataBitWithTs(tStates);
            break;
        case eBLOCK_PAUSE:
            tapeBlockPauseWithTs(tStates);
            break;
    }
}

void ZXSpectrum::tapeGenerateHeaderPilotWithTs(int tStates)
{
    if (tapePilotPulses < cPILOT_HEADER_PULSES)
    {
        if (tapeFlipTapeBit)
        {
            tapeInputBit ^= 1;
            tapeFlipTapeBit = false;
        }
        
        if (tapePilotPulseTStates >= cPILOT_PULSE_TSTATE_LENGTH)
        {
            tapePilotPulses += 1;
            tapePilotPulseTStates = 0;
            tapeFlipTapeBit = true;
        }
    }
    else
    {
        tapeSyncPulseTStates = 0;
        tapeProcessingState = eSYNC1;
    }
    
    tapePilotPulseTStates += tStates;
}


void ZXSpectrum::tapeGenerateDataPilotWithTs(int tStates)
{
    if (tapePilotPulses < cPILOT_DATA_PULSES)
    {
        if (tapeFlipTapeBit)
        {
            tapeInputBit ^= 1;
            tapeFlipTapeBit = false;
        }
        
        if (tapePilotPulseTStates >= cPILOT_PULSE_TSTATE_LENGTH)
        {
            tapePilotPulses += 1;
            tapePilotPulseTStates = 0;
            tapeFlipTapeBit = true;;
        }
    }
    else
    {
        tapeSyncPulseTStates = 0;
        tapeProcessingState = eSYNC1;
    }
    
    tapePilotPulseTStates += tStates;
}

void ZXSpectrum::tapeGenerateSync1WithTs(int tStates)
{
    if (tapeFlipTapeBit)
    {
        tapeInputBit ^= 1;
        tapeFlipTapeBit = false;
    }
    
    if (tapeSyncPulseTStates >= cFIRST_SYNC_PULSE_TSTATE_DELAY)
    {
        tapeSyncPulseTStates = 0;
        tapeFlipTapeBit = true;
        tapeProcessingState = eSYNC2;
    }
    else
    {
        tapeSyncPulseTStates += tStates;
    }
}

void ZXSpectrum::tapeGenerateSync2WithTs(int tStates)
{
    if (tapeFlipTapeBit)
    {
        tapeInputBit ^= 1;
        tapeFlipTapeBit = false;
    }
    
    if (tapeSyncPulseTStates >= cSECOND_SYNC_PULSE_TSTATE_DELAY)
    {
        tapeSyncPulseTStates = 0;
        tapeCurrentBytePtr = 0;
        tapeFlipTapeBit = true;
        tapeProcessingState = tapeNextProcessingState;
    }
    else
    {
        tapeSyncPulseTStates += tStates;
    }
}

void ZXSpectrum::tapeGenerateDataStreamWithTs(int tStates)
{
    int currentBlockLength = tapeBlocks[ tapeCurrentBlockIndex ].getDataLength();
    unsigned char byte = tapeBlocks[ tapeCurrentBlockIndex ].blockData[ tapeCurrentBytePtr ];
    unsigned char bit = (byte << tapeCurrentDataBit) & 128;
    
    tapeCurrentDataBit += 1;
    if (tapeCurrentDataBit > 7)
    {
        tapeCurrentDataBit = 0;
        tapeCurrentBytePtr += 1;
        if (tapeCurrentBytePtr > currentBlockLength)
        {
            tapeProcessingState = eBLOCK_PAUSE;
            tapeBlockPauseTStates = 0;
            return;
        }
    }
    
    if (bit)
    {
        tapeDataPulseTStates = cDATA_BIT_ONE_PULSE_TSTATE_DELAY;
    }
    else
    {
        tapeDataPulseTStates = cDATA_BIT_ZERO_PULSE_TSTATE_DELAY;
    }
    tapeFlipTapeBit = true;
    tapeDataBitTStates = 0;
    tapeDataPulseCount = 0;
    tapeProcessingState = eDATA_BIT;
}

void ZXSpectrum::tapeGenerateHeaderDataStreamWithTs(int tStates)
{
    int currentBlockLength = cHEADER_BLOCK_LENGTH;
    unsigned char byte = tapeBlocks[ tapeCurrentBlockIndex ].blockData[ tapeCurrentBytePtr ];
    unsigned char bit = (byte << tapeCurrentDataBit) & 128;
    
    tapeCurrentDataBit += 1;
    if (tapeCurrentDataBit > 7)
    {
        tapeCurrentDataBit = 0;
        tapeCurrentBytePtr += 1;
        tapeCurrentBlock->currentByte += 1;
        if (tapeCurrentBytePtr > currentBlockLength)
        {
            tapeProcessingState = eBLOCK_PAUSE;
            tapeBlockPauseTStates = 0;
            return;
        }
    }
    
    if (bit)
    {
        tapeDataPulseTStates = cDATA_BIT_ONE_PULSE_TSTATE_DELAY;
    }
    else
    {
        tapeDataPulseTStates = cDATA_BIT_ZERO_PULSE_TSTATE_DELAY;
    }
    tapeFlipTapeBit = true;
    tapeDataBitTStates = 0;
    tapeDataPulseCount = 0;
    tapeProcessingState = eDATA_BIT;
}

void ZXSpectrum::tapeGenerateDataBitWithTs(int tStates)
{
    if (tapeFlipTapeBit)
    {
        tapeInputBit ^= 1;
        tapeFlipTapeBit = false;
    }
    
    if (tapeDataBitTStates >= tapeDataPulseTStates)
    {
        tapeDataPulseCount += 1;
        if (tapeDataPulseCount < 2)
        {
            tapeFlipTapeBit = true;
            tapeDataBitTStates = 0;
        }
        else
        {
            tapeProcessingState = tapeNextProcessingState;
        }
    }
    else
    {
        tapeDataBitTStates += tStates;
    }
}

void ZXSpectrum::tapeBlockPauseWithTs(int tStates)
{
    tapeBlockPauseTStates += tStates;
    if (tapeBlockPauseTStates > 3500000 * 2)
    {
        tapeCurrentBlockIndex += 1;
        tapeNewBlock = true;
    }
    
    // Introduce a random crackle in between blocks to produce a similar experience as loading from a real tape
    // on a ZX Spectrum.
    if (arc4random_uniform(200000) == 1)
    {
        tapeInputBit ^= 1;
    }
}

#pragma mark - Tape controls

void ZXSpectrum::tapeStartPlaying()
{
    if (tapeLoaded)
    {
        tapePlaying = true;
    }
}

void ZXSpectrum::tapeStopPlaying()
{
    if (tapeLoaded)
    {
        tapePlaying = false;
        tapeInputBit = 0;
    }
}

void ZXSpectrum::tapeRewind()
{
    if (tapeLoaded)
    {
        tapeReset(false);
    }
}
















