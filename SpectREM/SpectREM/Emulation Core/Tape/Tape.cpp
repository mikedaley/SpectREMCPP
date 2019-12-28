//
//  Tape.cpp
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#include "Tape.hpp"
#include "../ZX_Spectrum_Core/ZXSpectrum.hpp"

// - Constants

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


// - TapeBlock


TapeBlock::~TapeBlock()
{
   delete blockData;
}

uint8_t TapeBlock::getFlag()
{
   return blockData[ cHEADER_FLAG_OFFSET ];
}

uint8_t TapeBlock::getDataType()
{
   return blockData[ cHEADER_DATA_TYPE_OFFSET ];
}

uint16_t TapeBlock::getDataLength()
{
   return blockLength;
}

uint8_t TapeBlock::getChecksum()
{
   return blockData[ cHEADER_CHECKSUM_OFFSET ];
}

uint16_t TapeBlock::getAutoStartLine()
{
   return 0;
}

uint16_t TapeBlock::getStartAddress()
{
   return 0;
}

string TapeBlock::getFilename()
{
   string filename(&blockData[ cHEADER_FILENAME_OFFSET ], &blockData[ cHEADER_FILENAME_OFFSET ] + cHEADER_FILENAME_LENGTH);
   return filename;
}


// - Program Header


string ProgramHeader::getBlockName()
{
   string blockName = "Program Header";
   return blockName;
}

uint16_t ProgramHeader::getAutoStartLine()
{
   return (reinterpret_cast<uint16_t*>(&blockData[ cPROGRAM_HEADER_AUTOSTART_LINE_OFFSET ])[0]);
}

uint16_t ProgramHeader::getProgramLength()
{
   return (reinterpret_cast<uint16_t*>(&blockData[ cPROGRAM_HEADER_PROGRAM_LENGTH_OFFSET ])[0]);
}

uint8_t ProgramHeader::getChecksum()
{
   return blockData[ cPROGRAM_HEADER_CHECKSUM_OFFSET ];
}

uint16_t ProgramHeader::getDataLength()
{
   return cHEADER_BLOCK_LENGTH;
}


// - Numeric Header


string NumericDataHeader::getBlockName()
{
   return "Numeric Data Header";
}


// - Alphanumeric Header


string AlphanumericDataHeader::getBlockName()
{
   return "Alphanumeric Data Header";
}


// - Byter Header


string ByteHeader::getBlockName()
{
   return "Byte Header";
}

uint16_t ByteHeader::getStartAddress()
{
   return (reinterpret_cast<uint16_t*>(&blockData[ cBYTE_HEADER_START_ADDRESS_OFFSET ])[0]);
}

uint8_t ByteHeader::getChecksum()
{
   return blockData[ blockLength - 1 ];
}


// - Data Block


string DataBlock::getBlockName()
{
   return "Data Block";
}

uint8_t *DataBlock::getDataBlock()
{
   uint8_t *dataBlock = new uint8_t[ getDataLength() ];
   memcpy(dataBlock, &blockData[cDATA_BLOCK_DATA_LENGTH_OFFSET], sizeof(uint8_t) * getDataLength());
   return dataBlock;
}

uint8_t DataBlock::getDataType()
{
   return blockData[ cHEADER_FLAG_OFFSET ];
}

uint8_t DataBlock::getChecksum()
{
   return blockData[ blockLength - 1 ];
}


// - TAP Processing


Tape::Tape(TapeStatusCallback callback)
{
   if (callback)
   {
       updateStatusCallback = callback;
   }
   else
   {
       updateStatusCallback = nullptr;
   }
}

Tape::~Tape()
{

}

void Tape::resetAndClearBlocks(bool clearBlocks)
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
       updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
   }
}

bool Tape::loadWithPath(const char *path)
{
    bool success = false;

    ifstream tapeFile(path, ios::binary | ios::ate);
    if (tapeFile.good())
    {
        vector<uint8_t> snapshotData(static_cast<size_t>(tapeFile.tellg()));
        tapeFile.seekg(0, ios::beg);
        tapeFile.read(reinterpret_cast<char*>(snapshotData.data()), snapshotData.size());

        resetAndClearBlocks(true);
        if (processData(snapshotData.data(), static_cast<unsigned int>(snapshotData.size())))
        {
            success = true;
        }
    }
    else
    {
        std::cout << "ERROR LOADING TAPE: " << path << std::endl;
    }
    loaded = success;
    return success;
}

void Tape::updateWithTs(uint32_t tStates)
{
   if (currentBlockIndex > static_cast<uint32_t>(blocks.size() - 1))
   {
       std::cout << "TAPE STOPPED" << std::endl;
       playing = false;
       inputBit = 0;
       rewindTape();

       if (updateStatusCallback)
       {
           updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
       }

       return;
   }

   if (newBlock)
   {
       if (updateStatusCallback)
       {
           updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
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

void Tape::generateHeaderPilotWithTs(uint32_t tStates)
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


void Tape::generateDataPilotWithTs(uint32_t tStates)
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

void Tape::generateSync1WithTs(uint32_t tStates)
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

void Tape::generateSync2WithTs(uint32_t tStates)
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

void Tape::tapeGenerateDataStreamWithTs(uint32_t)
{
   size_t currentBlockLength = blocks[ currentBlockIndex ]->getDataLength();
   uint8_t byte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
   uint8_t bit = (byte << currentDataBit) & 128;

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

void Tape::generateHeaderDataStreamWithTs(uint32_t)
{
   size_t currentBlockLength = cHEADER_BLOCK_LENGTH;
   uint8_t byte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
   uint8_t bit = (byte << currentDataBit) & 128;

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

void Tape::generateDataBitWithTs(uint32_t tStates)
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

void Tape::tapeBlockPauseWithTs(uint32_t tStates)
{
   blockPauseTStates += tStates;
   if (blockPauseTStates > 3500000 * 3)
   {
       currentBlockIndex += 1;
       newBlock = true;
   }

   // Introduce a random crackle in between blocks to produce a similar experience as loading from a real tape
   // on a ZX Spectrum.
   if (std::rand() == 1)
   {
       inputBit ^= 1;
   }
}

// - Process Tape Data

bool Tape::processData(uint8_t *dataBytes, uint32_t size)
{
   uint16_t blockLength = 0;
   uint8_t flag = 0;
   uint8_t dataType = 0;
   currentBytePtr = 0;

   while (currentBytePtr < size)
   {
       blockLength = (reinterpret_cast<uint16_t*>(&dataBytes[ currentBytePtr ])[0]);

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
           std::cout << "INVALID FLAG FOUND PROCESSING TAP" << std::endl;
           return false;
       }

       newTapeBlock->blockLength = blockLength;
       newTapeBlock->blockData = new uint8_t[blockLength];
       memcpy(newTapeBlock->blockData, &dataBytes[ currentBytePtr ], blockLength);

       blocks.push_back(newTapeBlock);

       currentBytePtr += blockLength;
   }

   return true;
}


// - Instant Tape Load


void Tape::loadBlock(void *m)
{
   ZXSpectrum *machine = static_cast<ZXSpectrum *>(m);

   // Stops us trying to read past the avaiable blocks. This is a hack and should be fixed properly at source
   if (currentBlockIndex >= blocks.size())
   {
       currentBlockIndex = 0;
   }

   uint32_t expectedBlockType = machine->z80Core.GetRegister(CZ80Core::eREG_ALT_A);
   uint16_t startAddress = machine->z80Core.GetRegister(CZ80Core::eREG_IX);

   // Some TAP files have blocks which are shorter than what is expected in DE (Chuckie Egg 2)
   // so just take the smallest value
   uint32_t blockLength = machine->z80Core.GetRegister(CZ80Core::eREG_DE);
   uint32_t tapBlockLength = blocks[ currentBlockIndex ]->getDataLength();
   blockLength = (blockLength < tapBlockLength) ? blockLength : tapBlockLength;
   uint32_t success = 1;

   if (blocks[ currentBlockIndex ]->getFlag() == expectedBlockType)
   {
       if (machine->z80Core.GetRegister(CZ80Core::eREG_ALT_F) & CZ80Core::FLAG_C)
       {
           currentBytePtr = cHEADER_DATA_TYPE_OFFSET;
           uint32_t checksum = expectedBlockType;

           for (uint16_t i = 0; i < blockLength; i++)
           {
               uint8_t tapByte = blocks[ currentBlockIndex ]->blockData[ currentBytePtr ];
               machine->z80Core.Z80CoreDebugMemWrite(startAddress + i, tapByte, nullptr);
               checksum ^= tapByte;
               currentBytePtr++;
           }

           size_t expectedChecksum = blocks[ currentBlockIndex ]->getChecksum();
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
       updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
   }
}

// - ROM Save

void Tape::saveBlock(void *m)
{
   ZXSpectrum *machine = static_cast<ZXSpectrum *>(m);

   uint8_t parity = 0;
   uint16_t length = machine->z80Core.GetRegister(CZ80Core::eREG_DE) + 2;
   uint16_t dataIndex = 0;
   loaded = true;

   uint8_t *pData = new uint8_t[ length ];

   if (pData != nullptr)
   {
       pData[dataIndex++] = length & 255;
       pData[dataIndex++] = length >> 8;

       parity = machine->z80Core.GetRegister(CZ80Core::eREG_A);

       pData[dataIndex++] = parity;

       for (uint16_t i = 0; i < machine->z80Core.GetRegister(CZ80Core::eREG_DE); i++)
       {
           // Read memory using the debug read from the core which takes into account any paging
           // on the 128k Spectrum
           uint8_t byte = static_cast<uint8_t>(machine->z80Core.Z80CoreDebugMemRead(machine->z80Core.GetRegister(CZ80Core::eREG_IX) + i, nullptr));
           parity ^= byte;
           pData[dataIndex++] = byte;
       }

       pData[dataIndex++] = parity;

       processData(pData, length);

       // Once a block has been saved this is the RET address
       machine->z80Core.SetRegister(CZ80Core::eREG_PC, 0x053e);

       delete[] pData;
   }

   newBlock = true;
}


// - Tape controls


void Tape::startPlaying()
{
   if (loaded)
   {
       playing = true;
       if (updateStatusCallback)
       {
           updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
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
           updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
       }
   }
}

void Tape::rewindTape()
{
   if (loaded)
   {
       resetAndClearBlocks(false);
   }
   if (updateStatusCallback)
   {
       updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
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
   resetAndClearBlocks(true);
   loaded = false;
}

vector<uint8_t> Tape::getTapeData()
{
   vector<uint8_t> tapeData;
   for (size_t i = 0; i < blocks.size(); i++)
   {
       uint16_t blockLength = blocks[ i ]->getDataLength();
       tapeData.push_back(blockLength & 0xff);
       tapeData.push_back(blockLength >> 8);
       for (int j = 0; j < blockLength; j++)
       {
           tapeData.push_back(blocks[ i ]->blockData[ j ]);
       }
   }

   return tapeData;
}

size_t Tape::numberOfTapeBlocks()
{
   return blocks.size();
}

void  Tape::setSelectedBlock(uint32_t blockIndex)
{
   currentBlockIndex = blockIndex;
}


// - Tape Callback


void Tape::updateStatus()
{
   if (updateStatusCallback)
   {
       updateStatusCallback(static_cast<int>(currentBlockIndex), 0);
   }
}














