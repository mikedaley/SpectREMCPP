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

private:
    ZXSpectrum                  * machine_      = nullptr;        // Current instance of a ZXSpectrum
    Tape                        * tapePlayer_   = nullptr;
    Debug                       * debugger_     = nullptr;

public:
    EmulationController();
    ~EmulationController();
   
    // Machine
    void                        createMachineOfType(int machineType, std::string romPath);
    void                        pauseMachine()                                                          { if (machine_) machine_->pause(); };
    void                        resumeMachine()                                                         { machine_->resume(); };
    void                        resetMachine(bool hard)                                                 { machine_->resetMachine(hard); };
    void                        generateFrame()                                                         { machine_->generateFrame(); };
    ZXSpectrum::SnapshotData    snapshotCreateZ80()                                                     { return machine_->snapshotCreateZ80(); };
    ZXSpectrum::SnapshotData    snapshotCreateSNA()                                                     { return machine_->snapshotCreateSNA(); };
    int                         snapshotMachineInSnapshotWithPath(const char * path)                    { return machine_->snapshotMachineInSnapshotWithPath(path); };
    Tape::FileResponse          loadFileWithPath(const std::string path);
    void                        keyboardKeyDown(ZXSpectrum::eZXSpectrumKey key)                         { machine_->keyboardKeyDown(key); };
    void                        keyboardKeyUp(ZXSpectrum::eZXSpectrumKey key)                           { machine_->keyboardKeyUp(key); };
    void                        keyboardFlagsChanged(uint64_t flags, ZXSpectrum::eZXSpectrumKey key)    { machine_->keyboardFlagsChanged(flags, key); };
    
    void                      * getDisplayBuffer()                                                      { return machine_->getScreenBuffer(); };
    int16_t                   * getAudioBuffer()                                                        { return machine_->audioBuffer; };
    const char                * getMachineName()                                                        { return machine_->machineInfo.machineName; };
    int                         getMachineType()                                                        { return machine_->machineInfo.machineType; };
    ZXSpectrum                * getMachine()                                                            { return machine_; };
    uint32_t                    getFrameCounter()                                                       { return machine_->emuFrameCounter; };
    bool                        isDisplayReady()                                                        { return machine_->displayReady; };
    bool                        isMachinePaused()                                                       { return machine_->emuPaused; };
                            
    void                        setInstantTapeLoad(bool instantTapeLoad)                                { machine_->emuTapeInstantLoad = instantTapeLoad; };
    void                        setUseAySound(bool useAy)                                               { machine_->emuUseAYSound = useAy; };
    void                        setUseSpecDrum(bool useSpecDrum)                                        { machine_->emuUseSpecDRUM = useSpecDrum; };
                                
    // Debugger
    Debug                     * getDebugger()                                                           { if (debugger_) return debugger_; else return nullptr; };
    void                        debugStep()                                                             { machine_->step(); };

    // Callbacks
    void                        setTapeStatusCallback(std::function<void(int blockIndex, int bytes, int action)>    tapeStatusCallback);
    void                        setDebugCallback(std::function<bool(uint16_t address, int operationType)>           debugCallback);
    
    // Tape player
    void                        playTape()                                                              { tapePlayer_->play(); };
    void                        stopTape()                                                              { tapePlayer_->stop(); };
    void                        rewindTape()                                                            { tapePlayer_->rewindTape(); };
    void                        ejectTape()                                                             { tapePlayer_->eject(); };
    void                        setCurrentTapeBlockIndex(int index);
    std::vector<uint8_t>        getTapeData()                                                           { return tapePlayer_->getTapeData(); };
    Tape::FileResponse          insertTapeWithPath(const std::string path)                              { return tapePlayer_->insertTapeWithPath(path); };
    size_t                      getNumberOfTapeBlocks()                                                 { return tapePlayer_->numberOfTapeBlocks(); };
    std::string                 tapeBlockTypeForIndex(int index)                                        { return tapePlayer_->blocks[index]->getBlockName(); };
    std::string                 tapeFilenameForIndex(int index)                                         { return tapePlayer_->blocks[index]->getFilename(); };
    int                         tapeAutostartLineForIndex(int index)                                    { return tapePlayer_->blocks[index]->getAutoStartLine(); };
    uint16_t                    tapeBlockStartAddressForIndex(int index)                                { return tapePlayer_->blocks[index]->getStartAddress(); };
    uint16_t                    tapeBlockLengthForIndex(int index)                                      { return tapePlayer_->blocks[index]->getDataLength(); };
    int                         getCurrentTapeBlock()                                                   { return tapePlayer_->currentBlockIndex; };
    bool                        isTapePlaying()                                                         { return tapePlayer_->playing; };
    
private:
    std::string                 getFileExtensionFromPath(const std::string &path);
    std::string                 stringToUpper(std::string str);
};

#endif /* EmulationController_hpp */
