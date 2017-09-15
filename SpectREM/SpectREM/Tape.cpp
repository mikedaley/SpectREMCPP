//
//  Tape.cpp
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "Tape.hpp"
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
static int cPROGRAM_HEADER_CHECKSUM_OFFSET = 18;

//static int cNUMERIC_DATA_HEADER_UNUSED_1_OFFSET = 14;
//static const int cNUMERIC_DATA_HEADER_VARIBABLE_NAME_OFFSET = 15;
//static int cNUMERIC_DATA_HEADER_UNUSED_2_OFFSET = 16;

//static int cALPHA_NUMERIC_DATA_HEADER_UNUSED_1_OFFSET = 14;
//static const int cALPHA_NUMERIC_DATA_HEADER_VARIABLE_NAME_OFFSET = 15;
//static int cALPHA_NUMERIC_DATA_HEADER_UNUSED_2_OFFSET = 16;

static const int cBYTE_HEADER_START_ADDRESS_OFFSET = 14;
//static int cBYTE_HEADER_UNUSED_1_OFFSET = 16;

static const int cDATA_BLOCK_DATA_LENGTH_OFFSET = 1;

static const int cHEADER_FILENAME_LENGTH = 10;

static const int cHEADER_BLOCK_LENGTH = 19;


#pragma mark - TapeBlock


TapeBlock::~TapeBlock()
{
    delete blockData;
}

unsigned char TapeBlock::getFlag()
{
    return blockData[ cHEADER_FLAG_OFFSET ];
}

unsigned char TapeBlock::getDataType()
{
    return blockData[ cHEADER_DATA_TYPE_OFFSET ];
}

unsigned short TapeBlock::getDataLength()
{
    return blockLength;
}

unsigned char TapeBlock::getChecksum()
{
    return blockData[ cHEADER_CHECKSUM_OFFSET ];
}

unsigned short TapeBlock::getAutoStartLine()
{
    return 0;
}

unsigned short TapeBlock::getStartAddress()
{
    return 0;
}

string TapeBlock::getFilename()
{
    string filename(&blockData[ cHEADER_FILENAME_OFFSET ], &blockData[ cHEADER_FILENAME_OFFSET ] + cHEADER_FILENAME_LENGTH);
    return filename;
}


#pragma mark - Program Header


string ProgramHeader::getBlockName()
{
    string blockName = "Program Header";
    return blockName;
}

unsigned short ProgramHeader::getAutoStartLine()
{
    return ((unsigned short *)&blockData[ cPROGRAM_HEADER_AUTOSTART_LINE_OFFSET ])[0];
}

unsigned short ProgramHeader::getProgramLength()
{
    return ((unsigned short *)&blockData[ cPROGRAM_HEADER_PROGRAM_LENGTH_OFFSET ])[0];
}

unsigned char ProgramHeader::getChecksum()
{
    return blockData[ cPROGRAM_HEADER_CHECKSUM_OFFSET ];
}

unsigned short ProgramHeader::getDataLength()
{
    return cHEADER_BLOCK_LENGTH;
}


#pragma mark - Numeric Header


string NumericDataHeader::getBlockName()
{
    return "Numeric Data Header";
}


#pragma mark - Alphanumeric Header


string AlphanumericDataHeader::getBlockName()
{
    return "Alphanumeric Data Header";
}


#pragma mark - Byter Header


string ByteHeader::getBlockName()
{
    return "Byte Header";
}

unsigned short ByteHeader::getStartAddress()
{
    return ((unsigned short *)&blockData[ cBYTE_HEADER_START_ADDRESS_OFFSET ])[0];
}

unsigned char ByteHeader::getChecksum()
{
    return blockData[ blockLength - 1 ];
}


#pragma mark - Data Block


string DataBlock::getBlockName()
{
    return "Data Block";
}

unsigned char *DataBlock::getDataBlock()
{
    unsigned char *dataBlock = new unsigned char[ getDataLength() ];
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


Tape::Tape(TapeStatusCallback callback)
{
    if (callback)
    {
        updateStatusCallback = callback;
    }
    else
    {
        updateStatusCallback = NULL;
    }
}

Tape::~Tape()
{

}

void Tape::reset(bool clearBlocks)
{
    inputBit = 0;
    currentBytePtr = 0;
    currentDataBit = 0;
    pilotPulseTStates = 0;
    pilotPulses = 0;
    dataPulseTStates = 0;
    flipTapeBit = true;
    playing = false;
    newBlock = true;
    currentBytePtr = 0;
    currentBlockIndex = 0;
    
    if (clearBlocks)
    {
        blocks.clear();
    }
    
    if (updateStatusCallback)
    {
        updateStatusCallback(currentBlockIndex, 0);
    }
}

bool Tape::loadWithPath(const char *path)
{
    FILE *fileHandle;
    
    fileHandle = fopen(path, "rb");
    
    if (!fileHandle)
    {
        cout << "ERROR LOADING TAPE: " << path << endl;
        loaded = false;
        fclose(fileHandle);
        return false;
    }
    
    fseek(fileHandle, 0, SEEK_END);
    long size = ftell(fileHandle);
    fseek(fileHandle, 0, SEEK_SET);
    
    unsigned char fileBytes[size];
    
    fread(&fileBytes, 1, size, fileHandle);

    reset(true);
    
    if (processData(fileBytes, size))
    {
        loaded = true;
        return true;
    }
    
    return false;
}

void Tape::updateWithTs(int tStates)
{
    if (currentBlockIndex > blocks.size() - 1)
    {
        cout << "TAPE STOPPED" << endl;
        playing = false;
        inputBit = 0;
        currentBlockIndex = static_cast<int>(blocks.size()) - 1;

        if (updateStatusCallback)
        {
            updateStatusCallback(currentBlockIndex, 0);
        }

        return;
    }
    
    if (newBlock)
    {
        if (updateStatusCallback)
        {
            updateStatusCallback(currentBlockIndex, 0);
        }

        newBlock = false;
        
        tapeCurrentBlock = blocks[ currentBlockIndex ];
        
        if (tapeCurrentBlock->blockType == ePROGRAM_HEADER ||
            tapeCurrentBlock->blockType == eNUMERIC_DATA_HEADER ||
            tapeCurrentBlock->blockType == eALPHANUMERIC_DATA_HEADER ||
            tapeCurrentBlock->blockType == eBYTE_HEADER)
        {
            processingState = eHEADER_PILOT;
            nextProcessingState = eHEADER_DATA_STREAM;
        }
        else if (tapeCurrentBlock->blockType == eDATA_PILOT)
        {
            processingState = eDATA_PILOT;
            nextProcessingState = eDATA_STREAM;
        }
        
        currentBytePtr = 0;
        currentDataBit = 0;
        pilotPulseTStates = 0;
        pilotPulses = 0;
        dataPulseTStates = 0;
        flipTapeBit = true;
    }
    
    switch (processingState)
    {
        case eHEADER_PILOT:
            generateHeaderPilotWithTs(tStates);
            break;
        case eSYNC1:
            generateSync1WithTs(tStates);
            break;
        case eSYNC2:
            generateSync2WithTs(tStates);
            break;
        case eDATA_PILOT:
            generateDataPilotWithTs(tStates);
            break;
        case eDATA_STREAM:
            tapeGenerateDataStreamWithTs(tStates);
            break;
        case eHEADER_DATA_STREAM:
            generateHeaderDataStreamWithTs(tStates);
            break;
        case eDATA_BIT:
            generateDataBitWithTs(tStates);
            break;
        case eBLOCK_PAUSE:
            tapeBlockPauseWithTs(tStates);
            break;
    }
    
}

void Tape::generateHeaderPilotWithTs(int tStates)
{
    if (pilotPulses < cPILOT_HEADER_PULSES)
    {
        if (flipTapeBit)
        {
            inputBit ^= 1;
            flipTapeBit = false;
        }
        
        if (pilotPulseTStates >= cPILOT_PULSE_TSTATE_LENGTH)
        {
            pilotPulses += 1;
            pilotPulseTStates = 0;
            flipTapeBit = true;
        }
    }
    else
    {
        syncPulseTStates = 0;
        processingState = eSYNC1;
    }
    
    pilotPulseTStates += tStates;
}


void Tape::generateDataPilotWithTs(int tStates)
{
    if (pilotPulses < cPILOT_DATA_PULSES)
    {
        if (flipTapeBit)
        {
            inputBit ^= 1;
            flipTapeBit = false;
        }
        
        if (pilotPulseTStates >= cPILOT_PULSE_TSTATE_LENGTH)
        {
            pilotPulses += 1;
            pilotPulseTStates = 0;
            flipTapeBit = true;;
        }
    }
    else
    {
        syncPulseTStates = 0;
        processingState = eSYNC1;
    }
    
    pilotPulseTStates += tStates;
}

void Tape::generateSync1WithTs(int tStates)
{
    if (flipTapeBit)
    {
        inputBit ^= 1;
        flipTapeBit = false;
    }
    
    if (syncPulseTStates >= cFIRST_SYNC_PULSE_TSTATE_DELAY)
    {
        syncPulseTStates = 0;
        flipTapeBit = true;
        processingState = eSYNC2;
    }
    else
    {
        syncPulseTStates += tStates;
    }
}

void Tape::generateSync2WithTs(int tStates)
{
    if (flipTapeBit)
    {
        inputBit ^= 1;
        flipTapeBit = false;
    }
    
    if (syncPulseTStates >= cSECOND_SYNC_PULSE_TSTATE_DELAY)
    {
        syncPulseTStates = 0;
        currentBytePtr = 0;
        flipTapeBit = true;
        processingState = nextProcessingState;
    }
    else
    {
        syncPulseTStates += tStates;
    }
}

void Tape::tapeGenerateDataStreamWithTs(int tStates)
{
    int currentBlockLength = blocks[ currentBlockIndex ]->getDataLength();
    unsigned char byte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
    unsigned char bit = (byte << currentDataBit) & 128;
    
    currentDataBit += 1;
    if (currentDataBit > 7)
    {
        currentDataBit = 0;
        currentBytePtr += 1;
        if (currentBytePtr > currentBlockLength)
        {
            processingState = eBLOCK_PAUSE;
            blockPauseTStates = 0;
            return;
        }
    }
    
    if (bit)
    {
        dataPulseTStates = cDATA_BIT_ONE_PULSE_TSTATE_DELAY;
    }
    else
    {
        dataPulseTStates = cDATA_BIT_ZERO_PULSE_TSTATE_DELAY;
    }
    flipTapeBit = true;
    dataBitTStates = 0;
    dataPulseCount = 0;
    processingState = eDATA_BIT;
}

void Tape::generateHeaderDataStreamWithTs(int tStates)
{
    int currentBlockLength = cHEADER_BLOCK_LENGTH;
    unsigned char byte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
    unsigned char bit = (byte << currentDataBit) & 128;
    
    currentDataBit += 1;
    if (currentDataBit > 7)
    {
        currentDataBit = 0;
        currentBytePtr += 1;
        tapeCurrentBlock->currentByte += 1;
        if (currentBytePtr > currentBlockLength)
        {
            processingState = eBLOCK_PAUSE;
            blockPauseTStates = 0;
            return;
        }
    }
    
    if (bit)
    {
        dataPulseTStates = cDATA_BIT_ONE_PULSE_TSTATE_DELAY;
    }
    else
    {
        dataPulseTStates = cDATA_BIT_ZERO_PULSE_TSTATE_DELAY;
    }
    flipTapeBit = true;
    dataBitTStates = 0;
    dataPulseCount = 0;
    processingState = eDATA_BIT;
}

void Tape::generateDataBitWithTs(int tStates)
{
    if (flipTapeBit)
    {
        inputBit ^= 1;
        flipTapeBit = false;
    }
    
    if (dataBitTStates >= dataPulseTStates)
    {
        dataPulseCount += 1;
        if (dataPulseCount < 2)
        {
            flipTapeBit = true;
            dataBitTStates = 0;
        }
        else
        {
            processingState = nextProcessingState;
        }
    }
    else
    {
        dataBitTStates += tStates;
    }
}

void Tape::tapeBlockPauseWithTs(int tStates)
{
    blockPauseTStates += tStates;
    if (blockPauseTStates > 3500000 * 3)
    {
        currentBlockIndex += 1;
        newBlock = true;
    }
    
    // Introduce a random crackle in between blocks to produce a similar experience as loading from a real tape
    // on a ZX Spectrum.
    if (arc4random_uniform(200000) == 1)
    {
        inputBit ^= 1;
    }
}

#pragma mark - Process Tape Data

bool Tape::processData(unsigned char *dataBytes, long size)
{
    unsigned short blockLength = 0;
    unsigned char flag = 0;
    unsigned char dataType = 0;
    currentBytePtr = 0;
    
    while (currentBytePtr < size)
    {
        blockLength = ((unsigned short *)&dataBytes[ currentBytePtr ])[0];
        
        // Move the byte pointer to the top of the actual TAP block
        currentBytePtr += 2;
        
        flag = dataBytes[ currentBytePtr  + cHEADER_FLAG_OFFSET ];
        dataType = dataBytes[ currentBytePtr + cHEADER_DATA_TYPE_OFFSET ];
        
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
        newTapeBlock->blockData = new unsigned char[blockLength];
        memcpy(newTapeBlock->blockData, &dataBytes[ currentBytePtr ], blockLength);
        
        blocks.push_back(newTapeBlock);
        
        currentBytePtr += blockLength;
    }
    
    return true;
}

#pragma mark - Instant Tape Load


void Tape::loadBlock(void *m)
{
    ZXSpectrum *machine = static_cast<ZXSpectrum *>(m);
    
    int expectedBlockType = machine->z80Core.GetRegister(CZ80Core::eREG_ALT_A);
    int startAddress = machine->z80Core.GetRegister(CZ80Core::eREG_IX);
    
    // Some TAP files have blocks which are shorter that what is expected in DE (Chuckie Egg 2)
    // so just take the smallest value
    int blockLength = machine->z80Core.GetRegister(CZ80Core::eREG_DE);
    int tapBlockLength = blocks[ currentBlockIndex ]->getDataLength();
    blockLength = (blockLength < tapBlockLength) ? blockLength : tapBlockLength;
    int success = 1;
    
    if (blocks[ currentBlockIndex ]->getFlag() == expectedBlockType)
    {
        if (machine->z80Core.GetRegister(CZ80Core::eREG_ALT_F) & CZ80Core::FLAG_C)
        {
            currentBytePtr = cHEADER_DATA_TYPE_OFFSET;
            int checksum = expectedBlockType;
            
            for (int i = 0; i < blockLength; i++)
            {
                unsigned char tapByte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
                machine->z80Core.Z80CoreDebugMemWrite(startAddress + i, tapByte, NULL);
                checksum ^= tapByte;
                currentBytePtr++;
            }
            
            int expectedChecksum = blocks[ currentBlockIndex ]->getChecksum();
            if (expectedChecksum != checksum)
            {
                success = 0;
            }
        }
        else
        {
            success = 1;
        }
    }
    
    if (success)
    {
        machine->z80Core.SetRegister(CZ80Core::eREG_F, (machine->z80Core.GetRegister(CZ80Core::eREG_F) | CZ80Core::FLAG_C));
    }
    else
    {
        machine->z80Core.SetRegister(CZ80Core::eREG_F, (machine->z80Core.GetRegister(CZ80Core::eREG_F) & ~CZ80Core::FLAG_C));
    }
    
    currentBlockIndex++;
    machine->z80Core.SetRegister(CZ80Core::eREG_PC, 0x05e2);
    
    if (updateStatusCallback)
    {
        updateStatusCallback(currentBlockIndex, 0);
    }
}

#pragma mark - ROM Save

void Tape::saveBlock(void *m)
{
    ZXSpectrum *machine = static_cast<ZXSpectrum *>(m);

    char parity = 0;
    short length = machine->z80Core.GetRegister(CZ80Core::eREG_DE) + 2;
    short dataIndex = 0;
    loaded = true;
    
    unsigned char data[ length ];
    
    data[ dataIndex++ ] = length & 255;
    data[ dataIndex++ ] = length >> 8;

    parity = machine->z80Core.GetRegister(CZ80Core::eREG_A);

    data[ dataIndex++ ] = parity;
    
    for (int i = 0; i < machine->z80Core.GetRegister(CZ80Core::eREG_DE); i++)
    {
        // Read memory using the debug read from the core which takes into account any paging
        // on the 128k Spectrum
        char byte = machine->z80Core.Z80CoreDebugMemRead(machine->z80Core.GetRegister(CZ80Core::eREG_IX) + i, NULL);
        parity ^= byte;
        data[ dataIndex++ ] = byte;
    }
    
    data[ dataIndex++ ] = parity;
    
    processData(data, length);
    
    // Once a block has been saved this is the RET address
    machine->z80Core.SetRegister(CZ80Core::eREG_PC, 0x053e);
    
    newBlock = true;
}


#pragma mark - Tape controls


void Tape::startPlaying()
{
    if (loaded)
    {
        playing = true;
        if (updateStatusCallback)
        {
            updateStatusCallback(currentBlockIndex, 0);
        }
    }
}

void Tape::stopPlaying()
{
    if (loaded)
    {
        playing = false;
        inputBit = 0;
        if (updateStatusCallback)
        {
            updateStatusCallback(currentBlockIndex, 0);
        }
    }
}

void Tape::rewindTape()
{
    if (loaded)
    {
        reset(false);
    }
    if (updateStatusCallback)
    {
        updateStatusCallback(currentBlockIndex, 0);
    }
}

void Tape::rewindBlock()
{
    if (loaded)
    {
        inputBit = 0;
        currentBytePtr = 0;
        currentDataBit = 0;
        pilotPulseTStates = 0;
        pilotPulses = 0;
        dataPulseTStates = 0;
        flipTapeBit = true;
    }
}

void Tape::eject()
{
    reset(true);
    loaded = false;
}

vector<unsigned char> Tape::getTapeData()
{
    vector<unsigned char> tapeData;
    for (int i = 0; i < blocks.size(); i++)
    {
        unsigned short blockLength = blocks[ i ]->getDataLength();
        tapeData.push_back(blockLength & 0xff);
        tapeData.push_back(blockLength >> 8);
        for (int j = 0; j < blockLength; j++)
        {
            tapeData.push_back(blocks[ i ]->blockData[ j ]);
        }
    }
    
    return tapeData;
}

unsigned long Tape::numberOfTapeBlocks()
{
    return blocks.size();
}

void  Tape::setSelectedBlock(int blockIndex)
{
    currentBlockIndex = blockIndex;
}


#pragma mark - Tape Callback


void Tape::updateStatus()
{
    if (updateStatusCallback)
    {
        updateStatusCallback(currentBlockIndex, 0);
    }
}














