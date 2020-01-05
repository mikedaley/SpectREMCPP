//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"
#include <cstring>

const uint32_t cSAMPLE_RATE = 44100;
const uint32_t cFPS = 50;
const uint32_t cROM_SIZE = 16384;

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
	std::cout << "ZXSpectrum::Constructor" << std::endl;

	displayCLUT = new uint64_t[32 * 1024];
	displayALUT = new uint8_t[256];
}

ZXSpectrum::~ZXSpectrum()
{
	std::cout << "ZXSpectrum::Destructor" << std::endl;

	delete[] displayCLUT;
	delete[] displayALUT;
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum::initialise(std::string romPath)
{
	std::cout << "ZXSpectrum::initialise(char *romPath)" << std::endl;

	z80Core.Initialise(zxSpectrumMemoryRead,
		zxSpectrumMemoryWrite,
		zxSpectrumIORead,
		zxSpectrumIOWrite,
		zxSpectrumMemoryContention,
		zxSpectrumDebugRead,
		zxSpectrumDebugWrite,
		this);

	emuROMPath = romPath;

	screenWidth = machineInfo.pxEmuBorder + machineInfo.pxHorizontalDisplay + machineInfo.pxEmuBorder;
	screenHeight = machineInfo.pxEmuBorder + machineInfo.pxVerticalDisplay + machineInfo.pxEmuBorder;
	screenBufferSize = screenHeight * screenWidth;

	memoryRom.resize(machineInfo.romSize);
	memoryRam.resize(machineInfo.ramSize);

	displaySetup();
	displayBuildLineAddressTable();
	displayBuildTsTable();
	displayBuildCLUT();

	ULABuildContentionTable();

	audioSetup(cSAMPLE_RATE, cFPS);
	audioBuildAYVolumesTable();

	resetMachine(true);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::registerDebugOpCallback(std::function<bool(uint16_t, uint8_t)> debugOpCallbackBlock)
{
	this->debugOpCallbackBlock = debugOpCallbackBlock;
}

// ------------------------------------------------------------------------------------------------------------
// - Generate a frame

void ZXSpectrum::generateFrame()
{
	uint32_t currentFrameTstates = machineInfo.tsPerFrame;

	while (currentFrameTstates > 0 && !emuPaused && !breakpointHit)
	{
		if (debugOpCallbackBlock)
		{
			if (debugOpCallbackBlock(z80Core.GetRegister(CZ80Core::eREG_PC), eDebugExecuteOp) || emuPaused)
			{
				return;
			}
		}

		uint32_t tStates = z80Core.Execute(1, machineInfo.intLength);

		if (tape && tape->playing)
		{
			tape->updateWithTs(tStates);
		}

		if (tape && emuSaveTrapTriggered)
		{
			tape->saveBlock(this);
		}
		else if (emuLoadTrapTriggered && tape && tape->loaded)
		{
			tape->loadBlock(this);
		}
		else
		{
			currentFrameTstates -= tStates;

			audioUpdateWithTs(tStates);

			if (z80Core.GetTStates() >= machineInfo.tsPerFrame)
			{
				z80Core.ResetTStates(machineInfo.tsPerFrame);
				z80Core.SignalInterrupt();

				displayUpdateWithTs(static_cast<int32_t>(machineInfo.tsPerFrame - emuCurrentDisplayTs));

				emuFrameCounter++;

				audioLastIndex = audioBufferIndex;
				displayFrameReset();
				keyboardCheckCapsLockStatus();
				audioDecayAYFloatingRegister();

				currentFrameTstates = 0;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// - Debug

void ZXSpectrum::step()
{
	uint32_t tStates = z80Core.Execute(1, machineInfo.intLength);

	if (tape && tape->playing)
	{
		tape->updateWithTs(tStates);
	}

	if (tape && emuSaveTrapTriggered)
	{
		tape->saveBlock(this);
	}
	else if (emuLoadTrapTriggered && tape && tape->loaded)
	{
		tape->loadBlock(this);
	}
	else
	{
		if (z80Core.GetTStates() >= machineInfo.tsPerFrame)
		{
			z80Core.ResetTStates(machineInfo.tsPerFrame);
			z80Core.SignalInterrupt();

			emuFrameCounter++;

			displayFrameReset();
			keyboardCheckCapsLockStatus();

			audioDecayAYFloatingRegister();
		}
	}

	displayUpdateWithTs(static_cast<int32_t>(machineInfo.tsPerFrame - emuCurrentDisplayTs));
}

// ------------------------------------------------------------------------------------------------------------
// - Memory Access

uint8_t ZXSpectrum::zxSpectrumMemoryRead(uint16_t address, void* param)
{
	return static_cast<ZXSpectrum*>(param)->coreMemoryRead(address);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::zxSpectrumMemoryWrite(uint16_t address, uint8_t data, void* param)
{
	static_cast<ZXSpectrum*>(param)->coreMemoryWrite(address, data);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::zxSpectrumMemoryContention(uint16_t address, uint32_t tStates, void* param)
{
	static_cast<ZXSpectrum*>(param)->coreMemoryContention(address, tStates);
}

// ------------------------------------------------------------------------------------------------------------

uint8_t ZXSpectrum::zxSpectrumDebugRead(uint16_t address, void* param, void* data)
{
	return static_cast<ZXSpectrum*>(param)->coreDebugRead(address, data);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::zxSpectrumDebugWrite(uint16_t address, uint8_t byte, void* param, void*)
{
	static_cast<ZXSpectrum*>(param)->coreMemoryWrite(address, byte);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::coreMemoryWriteWithBuffer(const char *buffer, size_t size, uint16_t address)
{
    std::vector<uint8_t> bufferData(buffer, buffer + size);
    for (size_t i = 0; i < size; i++)
    {
        ZXSpectrum::zxSpectrumDebugWrite(address++, bufferData[i], nullptr, nullptr);
    }
}

// ------------------------------------------------------------------------------------------------------------
// - IO Access

uint8_t ZXSpectrum::zxSpectrumIORead(uint16_t address, void* param)
{
	return static_cast<ZXSpectrum*>(param)->coreIORead(address);
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::zxSpectrumIOWrite(uint16_t address, uint8_t data, void* param)
{
	static_cast<ZXSpectrum*>(param)->coreIOWrite(address, data);
}

// ------------------------------------------------------------------------------------------------------------
// - Pause/Resume

void ZXSpectrum::pause()
{
	emuPaused = true;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::resume()
{
	emuPaused = false;
}

// ------------------------------------------------------------------------------------------------------------
// - Reset

void ZXSpectrum::resetMachine(bool hard)
{
	if (hard)
	{
		for (uint32_t i = 0; i < machineInfo.ramSize; i++)
		{
			memoryRam[i] = static_cast<char>(rand() % 255);
		}
	}

	delete[] displayBuffer;

	displaySetup();

	z80Core.Reset(hard);
	emuReset();
	keyboardMapReset();
	displayFrameReset();
	audioReset();
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::emuReset()
{
	emuFrameCounter = 0;
	emuSaveTrapTriggered = false;
	emuLoadTrapTriggered = false;
}

// ------------------------------------------------------------------------------------------------------------
// - ROM Loading

ZXSpectrum::Response ZXSpectrum::loadROM(const std::string rom, uint32_t page)
{
	size_t romAddress = cROM_SIZE * page;

	if (memoryRom.size() < romAddress)
	{
        std::cout << "ZXSpectrum::loadROM - Unable to load into ROM page " << page << std::endl;
        return ZXSpectrum::Response{false, std::to_string(page) + " is an invalid ROM page" };
	}

    std::string romPath = emuBasePath + emuROMPath;
	romPath.append(rom);

    std::ifstream romFile(romPath, std::ios::binary | std::ios::ate | std::ios::in);
	if (romFile.good())
	{
		std::streampos fileSize = romFile.tellg();
        romFile.seekg(0, std::ios::beg);
		romFile.read(memoryRom.data() + romAddress, fileSize);
		romFile.close();
        return ZXSpectrum::Response{true, "Loaded successfully"};
	}

    char* errorstring = strerror(errno);
    std::cout << "ERROR: Could not read from ROM file: " << errorstring;
    return ZXSpectrum::Response{false, errorstring};
}

// ------------------------------------------------------------------------------------------------------------
// SCR Loading

ZXSpectrum::Response ZXSpectrum::scrLoadWithPath(const std::string path)
{
    std::ifstream scrFile(path, std::ios::binary | std::ios::ate | std::ios::in);
    if (scrFile.good())
    {
        std::streampos fileSize = scrFile.tellg();
        scrFile.seekg(0, std::ios::beg);
        
        switch (machineInfo.machineType) {
            case eZXSpectrum48:
                scrFile.read(memoryRam.data() + cBITMAP_ADDRESS, fileSize);
                break;
            case eZXSpectrum128:
                // Load the image data into memory page 5
                scrFile.read(memoryRam.data() + (cBITMAP_ADDRESS * 5), fileSize);
                break;
                
            default:
                break;
        }
        scrFile.close();
        return ZXSpectrum::Response{true, "Loaded successfully"};
    }

    char* errorstring = strerror(errno);
    std::cout << "ERROR: Could not read from file: " << errorstring << std::endl;
    
    return ZXSpectrum::Response{false, errorstring};
}

// ------------------------------------------------------------------------------------------------------------
// - Getters

void* ZXSpectrum::getScreenBuffer()
{
	return displayBuffer;
}

// ------------------------------------------------------------------------------------------------------------
// - Release

void ZXSpectrum::release()
{
	delete[] displayBuffer;
	delete[] audioBuffer;
}





