//
//  Tape1.hpp
//  SpectREM
//
//  Created by Mike Daley on 11/09/2017.
//  Copyright © 2017 Mike Daley Ltd. All rights reserved.
//

#ifndef Tape_hpp
#define Tape_hpp

#include <vector>
#include <iostream>
#include <fstream>


// - Tape Block


class TapeBlock
{
public:
    virtual ~TapeBlock();

public:
    virtual uint8_t         getFlag();
    virtual uint8_t         getDataType();
    virtual uint16_t        getDataLength();
    virtual uint8_t         getChecksum();
    virtual uint16_t        getAutoStartLine();
    virtual uint16_t        getStartAddress();
    virtual std::string     getBlockName() = 0;
    virtual std::string     getFilename();

public:
    uint16_t                blockLength = 0;
    uint8_t               * blockData = nullptr;
    int                     blockType = 0;
    int                     currentByte = 0;
};


// - Tape Program Header Block


class ProgramHeader : public TapeBlock
{
public:
    virtual uint16_t        getAutoStartLine();
    virtual uint16_t        getProgramLength();
    virtual uint16_t        getDataLength();
    virtual uint8_t         getChecksum();
    virtual std::string     getBlockName();
};


// - Tape Numeric Data Header Block


class NumericDataHeader : public TapeBlock
{
public:
    virtual std::string     getBlockName();
};


// - Tape Alphanumeric Data Header Block


class AlphanumericDataHeader : public TapeBlock
{
public:
    virtual std::string     getBlockName();
};


// - Tape Byte Header Block Block


class ByteHeader : public TapeBlock
{
public:
    uint16_t                getStartAddress();
    virtual uint8_t         getChecksum();
    virtual std::string     getBlockName();
};


// - Tape Data Block Block


class DataBlock : public TapeBlock
{
public:
    uint8_t               * getDataBlock();
    virtual uint8_t         getDataType();
    virtual uint8_t         getChecksum();
    virtual std::string     getBlockName();
};


// - Main Tape Processing Class


class Tape
{
    // TAPE block types
    enum
    {
        E_PROGRAM_HEADER = 0,
        E_NUMERIC_DATA_HEADER,
        E_ALPHANUMERIC_DATA_HEADER,
        E_BYTE_HEADER,
        E_DATA_BLOCK,
        E_FRAGMENTED_DATA_BLOCK,
        E_UNKNOWN_BLOCK = 99
    };

    // TAP Processing states
    enum
    {
        E_NO_TAPE = 0,
        E_HEADER_PILOT,
        E_SYNC1,
        E_SYNC2,
        E_DATA_PILOT,
        E_BLOCK_PAUSE,
        E_DATA_STREAM,
        E_HEADER_DATA_STREAM,
        E_DATA_BIT
    };

    // Tape player actions
    enum TAPEACTION
    {
        E_TAPE_STOP,
        E_TAPE_PLAY,
        E_TAPE_REWIND,
        E_BLOCK_REWIND,
        E_BLOCK_CHANGED,
        E_PROCESSING_BLOCK,
        E_NEW_BLOCK,
        E_INSTA_LOAD_BLOCK,
        E_RESET,
        E_EJECT
    };
    
public:
    struct FileResponse {
        bool        success;
        std::string responseMsg;
    };
    
public:
    Tape(std::function<void(int blockIndex, int bytes, int action)> callback);
    virtual ~Tape();

public:
    void                    clearStatusCallback(void); // Removes the callback allocated to the Tape
    FileResponse            insertTapeWithPath(const std::string path);
    
    // Setup a callback for status changes
    void                    setStatusCallback(std::function<void(int blockIndex, int bytes, int action)>);

    // Loads/Saves the block controlled by performing a ROM load or save
    void                    loadBlockWithMachine(void *m);
    void                    saveBlockWithMachine(void *m);

    // Updates the tape to generate the tape output. Tstates passed in should be the tStates used in each opcode executed
    void                    updateWithTs(uint32_t tStates);

    // Functions used to control the state of the currently loaded tape
    void                    play();
    void                    stop();
    void                    rewindTape();
    void                    rewindBlock();
    void                    eject();

    // Functions used to get details of the loaded tape that can then be used in a UI to display those details
    void                    updateStatus(); // Called when the internal status of the current tape changes and in turn calls any registered callback function
    size_t                  numberOfTapeBlocks();
    void                    setCurrentBlock(uint32_t blockIndex);

    // Returns a vector that contains the current tape data ready to write to disk
    std::vector<uint8_t>    getTapeData();

private:
    void                    resetAndClearBlocks(bool clearBlocks);
    bool                    processData(uint8_t *fileBytes, uint32_t size);
    void                    generateHeaderPilotWithTs(uint32_t tStates);
    void                    generateSync1WithTs(uint32_t tStates);
    void                    generateSync2WithTs(uint32_t tStates);
    void                    generateDataPilotWithTs(uint32_t tStates);
    void                    tapeGenerateDataStreamWithTs(uint32_t tStates);
    void                    generateHeaderDataStreamWithTs(uint32_t tStates);
    void                    generateDataBitWithTs(uint32_t tStates);
    void                    tapeBlockPauseWithTs(uint32_t tStates);

public:
    bool                    loaded = false;
    bool                    playing = false;
    uint32_t                currentBlockIndex = 0;
    bool                    newBlock = false;
    std::vector<TapeBlock *> blocks;
    int                     inputBit = 0;

private:
    uint32_t                currentBytePtr      = 0;
    uint32_t                pilotPulseTStates   = 0;          // How many Ts have passed since the start of the pilot pulses
    uint32_t                pilotPulses         = 0;          // How many pilot pulses have been generated
    uint32_t                syncPulseTStates    = 0;          // Sync pulse tStates
    uint32_t                dataPulseTStates    = 0;          // How many Ts have passed since the start of the data pulse
    bool                    flipTapeBit         = false;      // Should the tape bit be flipped
    uint32_t                processingState     = 0;          // Current processing state e.g. generating pilot, streaming data
    uint32_t                nextProcessingState = 0;          // Next processing state to be used
    uint32_t                currentDataBit      = 0;          // Which bit of the current byte in the data stream is being processed
    uint32_t                blockPauseTStates   = 0;          // How many tStates have passed since starting the pause between data blocks
    uint32_t                dataBitTStates      = 0;          // How many tStates to pause when processing data bit pulses
    uint32_t                dataPulseCount      = 0;          // How many pulses have been generated for the current data bit;
    TapeBlock               *tapeCurrentBlock   = nullptr;    // Current tape block object

    // Function called whenever the status of the tape changes e.g. new block, rewind, stop etc
    std::function<void(int blockIndex, int bytes, int action)> updateStatusCallback = nullptr;
};

#endif /* Tape1_hpp */
