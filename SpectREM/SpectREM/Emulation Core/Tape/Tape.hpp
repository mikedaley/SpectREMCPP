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

// - TypeDefs


typedef void (*TapeStatusCallback)(int blockIndex, int bytes);


// - Tape Block


class TapeBlock
{
public:
    virtual ~TapeBlock();

public:
    virtual uint8_t   getFlag();
    virtual uint8_t   getDataType();
    virtual uint16_t  getDataLength();
    virtual uint8_t   getChecksum();
    virtual uint16_t  getAutoStartLine();
    virtual uint16_t  getStartAddress();
    virtual std::string getBlockName() = 0;
    virtual std::string getFilename();

public:
    uint16_t          blockLength = 0;
    uint8_t           *blockData = nullptr;
    int               blockType = 0;
    int               currentByte = 0;
};


// - Tape Program Header Block


class ProgramHeader : public TapeBlock
{
public:
    virtual uint16_t  getAutoStartLine();
    virtual uint16_t  getProgramLength();
    virtual uint16_t  getDataLength();
    virtual uint8_t   getChecksum();
    virtual std::string    getBlockName();
};


// - Tape Numeric Data Header Block


class NumericDataHeader : public TapeBlock
{
public:
    virtual std::string          getBlockName();
};


// - Tape Alphanumeric Data Header Block


class AlphanumericDataHeader : public TapeBlock
{
public:
    virtual std::string          getBlockName();
};


// - Tape Byte Header Block Block


class ByteHeader : public TapeBlock
{
public:
    uint16_t          getStartAddress();
    virtual uint8_t   getChecksum();
    virtual std::string    getBlockName();
};


// - Tape Data Block Block


class DataBlock : public TapeBlock
{
public:
    uint8_t           *getDataBlock();
    virtual uint8_t   getDataType();
    virtual uint8_t   getChecksum();
    virtual std::string    getBlockName();
};


// - Main Tape Processing Class


class Tape
{
    // TAPE block types
    enum BlockType
    {
        PROGRAM_HEADER = 0,
        NUMERIC_DATA_HEADER,
        ALPHANUMERIC_DATA_HEADER,
        BYTE_HEADER,
        DATA_BLOCK,
        FRAGMENTED_DATA_BLOCK,
        UNKNOWN_BLOCK = 99
    };

    // TAP Processing states
    enum ProcessingState
    {
        NO_TAPE = 0,
        HEADER_PILOT,
        SYNC1,
        SYNC2,
        DATA_PILOT,
        BLOCK_PAUSE,
        DATA_STREAM,
        HEADER_DATA_STREAM,
        DATA_BIT
    };

public:
    struct TapResponse {
        bool success;
        std::string responseMsg;
    };
    
public:
    Tape(TapeStatusCallback callback);
    virtual ~Tape();

public:
    void                    clearStatusCallback(void); // Removes the callback allocated to the Tape
    // Load a TAP file
    TapResponse             loadWithPath(const std::string path);

    // Loads/Saves the block controlled by performing a ROM load or save
    void                    loadBlock(void *m);
    void                    saveBlock(void *m);

    // Updates the tape to generate the tape output. Tstates passed in should be the tStates used in each opcode executed
    void                    updateWithTs(uint32_t tStates);

    // Functions used to control the state of the currently loaded tape
    void                    startPlaying();
    void                    stopPlaying();
    void                    rewindTape();
    void                    rewindBlock();
    void                    eject();

    // Functions used to get details of the loaded tape that can then be used in a UI to display those details
    void                    updateStatus(); // Called when the internal status of the current tape changes
    size_t                  numberOfTapeBlocks();
    void                    setSelectedBlock(uint32_t blockIndex);

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
    uint32_t                current_block_index = 0;
    bool                    new_block = false;
    std::vector<TapeBlock *> blocks;
    int                     input_bit = 0;

private:
    uint32_t                current_byte_pointer = 0;
    uint32_t                pilot_pulse_ts = 0;             // How many Ts have passed since the start of the pilot pulses
    uint32_t                pilot_pulses = 0;               // How many pilot pulses have been generated
    uint32_t                sync_pulse_ts = 0;              // Sync pulse tStates
    uint32_t                data_pulse_ts = 0;              // How many Ts have passed since the start of the data pulse
    bool                    flip_tape_bit = false;          // Should the tape bit be flipped
    uint32_t                processing_state = 0;           // Current processing state e.g. generating pilot, streaming data
    uint32_t                next_processing_state = 0;      // Next processing state to be used
    uint32_t                current_data_bit = 0;           // Which bit of the current byte in the data stream is being processed
    uint32_t                block_pause_ts = 0;             // How many tStates have passed since starting the pause between data blocks
    uint32_t                data_bit_ts = 0;                // How many tStates to pause when processing data bit pulses
    uint32_t                data_pulse_count = 0;           // How many pulses have been generated for the current data bit;
    TapeBlock *             current_tape_block = nullptr;   // Current tape block object

    // Function called whenever the status of the tape changes e.g. new block, rewind, stop etc
    TapeStatusCallback      updateStatusCallback = nullptr;
};

#endif /* Tape1_hpp */
