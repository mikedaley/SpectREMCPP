//
//  Tape1.hpp
//  SpectREM
//
//  Created by Mike Daley on 11/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef Tape_hpp
#define Tape_hpp

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;


#pragma mark - TypeDefs


typedef void (*TapeStatusCallback)(int blockIndex, int bytes);


#pragma mark - Tape Block


class TapeBlock
{
public:
    virtual ~TapeBlock();
    
public:
    virtual unsigned char   getFlag();
    virtual unsigned char   getDataType();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual unsigned short  getAutoStartLine();
    virtual unsigned short  getStartAddress();
    virtual string          getBlockName() = 0;
    virtual string          getFilename();
    
public:
    unsigned short          blockLength;
    unsigned char           *blockData;
    int                     blockType;
    int                     currentByte;
};


#pragma mark - Tape Program Header Block


class ProgramHeader : public TapeBlock
{
public:
    virtual unsigned short  getAutoStartLine();
    virtual unsigned short  getProgramLength();
    virtual unsigned short  getDataLength();
    virtual unsigned char   getChecksum();
    virtual string          getBlockName();
};


#pragma mark - Tape Numeric Data Header Block


class NumericDataHeader : public TapeBlock
{
public:
    virtual string          getBlockName();
};


#pragma mark - Tape Alphanumeric Data Header Block


class AlphanumericDataHeader : public TapeBlock
{
public:
    virtual string          getBlockName();
};


#pragma mark - Tape Byte Header Block Block


class ByteHeader : public TapeBlock
{
public:
    unsigned short          getStartAddress();
    virtual unsigned char   getChecksum();
//    virtual unsigned short  getDataLength();
    virtual string          getBlockName();
};


#pragma mark - Tape Data Block Block


class DataBlock : public TapeBlock
{
public:
    unsigned char           *getDataBlock();
    virtual unsigned char   getDataType();
    virtual unsigned char   getChecksum();
    virtual string          getBlockName();
};


#pragma mark - Main Tape Processing Class


class Tape
{
    // TAPE block types
    enum
    {
        ePROGRAM_HEADER = 0,
        eNUMERIC_DATA_HEADER,
        eALPHANUMERIC_DATA_HEADER,
        eBYTE_HEADER,
        eDATA_BLOCK,
        eFRAGMENTED_DATA_BLOCK,
        eUNKNOWN_BLOCK = 99
    };
    
    // TAP Processing states
    enum
    {
        eNO_TAPE = 0,
        eHEADER_PILOT,
        eSYNC1,
        eSYNC2,
        eDATA_PILOT,
        eBLOCK_PAUSE,
        eDATA_STREAM,
        eHEADER_DATA_STREAM,
        eDATA_BIT
    };

public:
    Tape(TapeStatusCallback callback);
    virtual ~Tape();
    
public:
    bool                    loadWithPath(const char *);
    void                    loadBlock(void *m);
    void                    saveBlock(void *m);
    void                    updateWithTs(int tStates);
    void                    startPlaying();
    void                    stopPlaying();
    void                    rewindTape();
    void                    rewindBlock();
    void                    eject();
    void                    reset(bool clearBlocks);
    void                    updateStatus();
    unsigned long           numberOfTapeBlocks();
    void                    setSelectedBlock(int blockIndex);
    vector<unsigned char>   getTapeData();
    
private:
    bool                    processData(unsigned char *fileBytes, long size);
    void                    generateHeaderPilotWithTs(int tStates);
    void                    generateSync1WithTs(int tStates);
    void                    generateSync2WithTs(int tStates);
    void                    generateDataPilotWithTs(int tStates);
    void                    tapeGenerateDataStreamWithTs(int tStates);
    void                    generateHeaderDataStreamWithTs(int tStates);
    void                    generateDataBitWithTs(int tStates);
    void                    tapeBlockPauseWithTs(int tStates);
    
public:
    bool                    loaded;
    bool                    playing;
    int                     currentBytePtr;
    int                     currentBlockIndex;
    int                     newBlock;
    vector<TapeBlock *>     blocks;
    int                     inputBit;
    
private:
    int                     pilotPulseTStates;          // How many Ts have passed since the start of the pilot pulses
    int                     pilotPulses;                // How many pilot pulses have been generated
    int                     syncPulseTStates;           // Sync pulse tStates
    int                     dataPulseTStates;           // How many Ts have passed since the start of the data pulse
    bool                    flipTapeBit;                // Should the tape bit be flipped
    int                     processingState;            // Current processing state e.g. generating pilot, streaming data
    int                     nextProcessingState;        // Next processing state to be used
    int                     currentDataBit;             // Which bit of the current byte in the data stream is being processed
    int                     blockPauseTStates;          // How many tStates have passed since starting the pause between data blocks
    int                     dataBitTStates;             // How many tStates to pause when processing data bit pulses
    int                     dataPulseCount;             // How many pulses have been generated for the current data bit;
    TapeBlock               *tapeCurrentBlock;          // Current tape block
    
    // Function called whenever the status of the tape changes e.g. new block, rewind, stop etc
    TapeStatusCallback      updateStatusCallback;
};

#endif /* Tape1_hpp */
