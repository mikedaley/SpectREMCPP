//
//  EmulationController.cpp
//  SpectREM
//
//  Created by Michael Daley on 2020-01-14.
//  Copyright © 2020 Michael Daley. All rights reserved.
//

#include "EmulationController.hpp"
#include "ZXSpectrum48.hpp"
#include "ZXSpectrum128.hpp"

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Deconstructor
// ------------------------------------------------------------------------------------------------------------

EmulationController::EmulationController()
{
    std::cout << "EmulationController::Constructor" << "\n";
    tapePlayer = new Tape(nullptr);
    debugger = new Debug();
}

// ------------------------------------------------------------------------------------------------------------

EmulationController::~EmulationController()
{
    std::cout << "EmulationController::Destructor" << "\n";
    delete tapePlayer;
    delete debugger;
}

// ------------------------------------------------------------------------------------------------------------
// - Machine
// ------------------------------------------------------------------------------------------------------------

void EmulationController::createMachineOfType(int machineType, std::string romPath)
{
    if (machine)
    {
        machine->pause();
        delete machine;
    }
    
    switch (machineType) {
        case eZXSpectrum48:
            machine = new ZXSpectrum48(tapePlayer);
            break;
            
        case eZXSpectrum128:
            machine = new ZXSpectrum128(tapePlayer);
            break;
            
        default:
            machine = new ZXSpectrum48(tapePlayer);
            break;
    }
    
    machine->initialise(romPath);
    debugger->attachMachine(machine);
}

// ------------------------------------------------------------------------------------------------------------

Tape::FileResponse EmulationController::loadFileWithPath(const std::string path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate | std::ios::in);
    if (!file.good())
    {
        char * errorString = strerror(errno);
        return Tape::FileResponse{ false, errorString };
    }
    
    std::string fileExtension = stringToUpper( getFileExtensionFromPath(path) );
    
    if (fileExtension == "SNA")
    {
        return machine->snapshotSNALoadWithPath(path);
    }
    else if (fileExtension == "Z80")
    {
        return machine->snapshotZ80LoadWithPath(path);
    }
    else if (fileExtension == "TAP")
    {
        return tapePlayer->insertTapeWithPath(path);
    }
    else if (fileExtension == "SCR")
    {
        return machine->scrLoadWithPath(path);
    }
    
    return Tape::FileResponse{ false, "Unknown file type" };
}

// ------------------------------------------------------------------------------------------------------------
// - Tape player
// ------------------------------------------------------------------------------------------------------------

void EmulationController::setTapeStatusCallback(std::function<void(int blockIndex, int bytes)> tapeStatusCallback)
{
    tapePlayer->setStatusCallback(tapeStatusCallback);
}

// ------------------------------------------------------------------------------------------------------------

void EmulationController::setCurrentTapeBlockIndex(int index)
{
    tapePlayer->setCurrentBlock(index);
    tapePlayer->rewindBlock();
    tapePlayer->stop();
}

// ------------------------------------------------------------------------------------------------------------
// - Debugger
// ------------------------------------------------------------------------------------------------------------

void EmulationController::setDebugCallback(std::function<bool (uint16_t, int)> debugCallback)
{
    machine->registerDebugOpCallback(debugCallback);
}

// ------------------------------------------------------------------------------------------------------------
// - Helpers
// ------------------------------------------------------------------------------------------------------------

std::string EmulationController::getFileExtensionFromPath(const std::string &path)
{
    size_t i = path.rfind('.', path.length());
    if (i != std::string::npos) {
       return(path.substr(i + 1, path.length() - i));
    }

    return("");
}

// ------------------------------------------------------------------------------------------------------------

std::string EmulationController::stringToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::toupper(c); }
                  );
    return str;
}
