//
//  EmulationController.hpp
//  SpectREM
//
//  Created by Michael Daley on 2020-01-14.
//  Copyright © 2020 Michael Daley. All rights reserved.
//

#ifndef EmulationController_hpp
#define EmulationController_hpp

#import "ZXSpectrum.hpp"
#import "Debug.hpp"
#import "Tape.hpp"

class EmulationController
{

public:
    ZXSpectrum                  * machine = nullptr;        // Current instance of a ZXSpectrum
    Tape                        * tapePlayer = nullptr;
    Debug                       * debugger = nullptr;

public:
    EmulationController();
    ~EmulationController();
   
    // Machine
    void                        createMachineOfType(int machineType, std::string romPath);
    void                        pauseMachine()                                                          { if (machine) machine->pause(); };
    void                        resumeMachine()                                                         { machine->resume(); };
    void                        resetMachine(bool hard)                                                 { machine->resetMachine(hard); };
    void                        generateFrame()                                                         { machine->generateFrame(); };
    ZXSpectrum::SnapshotData    snapshotCreateZ80()                                                     { return machine->snapshotCreateZ80(); };
    ZXSpectrum::SnapshotData    snapshotCreateSNA()                                                     { return machine->snapshotCreateSNA(); };
    int                         snapshotMachineInSnapshotWithPath(const char * path)                    { return machine->snapshotMachineInSnapshotWithPath(path); };
    Tape::FileResponse          loadFileWithPath(const std::string path);
    void                        keyboardKeyDown(ZXSpectrum::eZXSpectrumKey key)                         { machine->keyboardKeyDown(key); };
    void                        keyboardKeyUp(ZXSpectrum::eZXSpectrumKey key)                           { machine->keyboardKeyUp(key); };
    void                        keyboardFlagsChanged(uint64_t flags, ZXSpectrum::eZXSpectrumKey key)    { machine->keyboardFlagsChanged(flags, key); };
    
    void                      * getDisplayBuffer()                                                      { return machine->getScreenBuffer(); };
    int16_t                   * getAudioBuffer()                                                        { return machine->audioBuffer; };
    const char                * getMachineName()                                                        { return machine->machineInfo.machineName; };
    int                         getMachineType()                                                        { return machine->machineInfo.machineType; };
    ZXSpectrum                * getMachine()                                                            { return machine; };
    uint32_t                    getFrameCounter()                                                       { return machine->emuFrameCounter; };
    bool                        isDisplayReady()                                                        { return machine->displayReady; };
    bool                        isMachinePaused()                                                       { return machine->emuPaused; };
                            
    void                        setInstantTapeLoad(bool instantTapeLoad)                                { machine->emuTapeInstantLoad = instantTapeLoad; };
    void                        setUseAySound(bool useAy)                                               { machine->emuUseAYSound = useAy; };
    void                        setUseSpecDrum(bool useSpecDrum)                                        { machine->emuUseSpecDRUM = useSpecDrum; };
                                
    // Debugger
    void                      * getDebugger()                                                           { if (debugger) return debugger; else return nullptr; };

    // Callbacks
    void                        setTapeStatusCallback(std::function<void(int blockIndex, int bytes)> tapeStatusCallback);
    void                        setDebugCallback(std::function<bool(uint16_t address, int operationType)> debugCallback);
    
    // Tape player
    void                        playTape()                                                              { tapePlayer->play(); };
    void                        stopTape()                                                              { tapePlayer->stop(); };
    void                        rewindTape()                                                            { tapePlayer->rewindTape(); };
    void                        ejectTape()                                                             { tapePlayer->eject(); };
    void                        setCurrentTapeBlockIndex(int index);
    std::vector<uint8_t>        getTapeData()                                                           { return tapePlayer->getTapeData(); };
    Tape::FileResponse          insertTapeWithPath(const std::string path)                              { return tapePlayer->insertTapeWithPath(path); };
    size_t                      getNumberOfTapeBlocks()                                                 { return tapePlayer->numberOfTapeBlocks(); };
    std::string                 tapeBlockTypeForIndex(int index)                                        { return tapePlayer->blocks[index]->getBlockName(); };
    std::string                 tapeFilenameForIndex(int index)                                         { return tapePlayer->blocks[index]->getFilename(); };
    int                         tapeAutostartLineForIndex(int index)                                    { return tapePlayer->blocks[index]->getAutoStartLine(); };
    uint16_t                    tapeBlockStartAddressForIndex(int index)                                { return tapePlayer->blocks[index]->getStartAddress(); };
    uint16_t                    tapeBlockLengthForIndex(int index)                                      { return tapePlayer->blocks[index]->getDataLength(); };
    int                         getCurrentTapeBlock()                                                   { return tapePlayer->currentBlockIndex; };
    bool                        isTapePlaying()                                                         { return tapePlayer->playing; };
    
private:
    std::string                 getFileExtensionFromPath(const std::string &path);
    std::string                 stringToUpper(std::string str);
};

#endif /* EmulationController_hpp */
