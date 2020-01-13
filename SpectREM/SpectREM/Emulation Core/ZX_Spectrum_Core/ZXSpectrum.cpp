//
//  ZXSpectrum48.cpp
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#include "ZXSpectrum.hpp"
#include <cstring>

// ------------------------------------------------------------------------------------------------------------
// - Constants

static constexpr uint32_t cSAMPLE_RATE = 44100;
static constexpr uint32_t cFPS = 50;
static constexpr uint32_t cROM_SIZE = 16384;

// ------------------------------------------------------------------------------------------------------------
// - Constructor/Deconstructor

ZXSpectrum::ZXSpectrum()
{
	std::cout << "ZXSpectrum::Constructor" << "\n";

	display_clut = new uint64_t[32 * 1024];
	display_alut = new uint8_t[256];
}

ZXSpectrum::~ZXSpectrum()
{
	std::cout << "ZXSpectrum::Destructor" << "\n";

	delete[] display_clut;
	delete[] display_alut;
}

// ------------------------------------------------------------------------------------------------------------
// - Initialise

void ZXSpectrum::initialise(std::string romPath)
{
	std::cout << "ZXSpectrum::initialise(char *romPath)" << "\n";

	z80_core.Initialise(zxSpectrumMemoryRead,
		zxSpectrumMemoryWrite,
		zxSpectrumIORead,
		zxSpectrumIOWrite,
		zxSpectrumMemoryContention,
		zxSpectrumDebugRead,
		zxSpectrumDebugWrite,
		this);

	emu_rom_path = romPath;

	display_screen_width = machine_info.pixel_emulator_border + machine_info.pixel_horizontal_display + machine_info.pixel_emulator_border;
	display_screen_height = machine_info.pixel_emulator_border + machine_info.pixel_vertical_display + machine_info.pixel_emulator_border;
	display_screen_buffer_size = display_screen_height * display_screen_width;

	memory_rom.resize(machine_info.rom_size);
	memory_ram.resize(machine_info.ram_size);

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
	uint32_t current_frame_ts_states = machine_info.ts_per_frame;

	while (current_frame_ts_states > 0 && !emu_paused && !debugger_breakpoint_hit)
	{
		if (debugOpCallbackBlock)
		{
			if (debugOpCallbackBlock(z80_core.GetRegister(CZ80Core::eREG_PC), DebugOperation::EXECUTE) || emu_paused)
			{
				return;
			}
		}

		uint32_t ts = z80_core.Execute(1, machine_info.interrupt_length);

		if (virtual_tape && virtual_tape->playing)
		{
			virtual_tape->updateWithTs(ts);
		}

		if (virtual_tape && emu_save_trap_triggered)
		{
			virtual_tape->saveBlock(this);
		}
		else if (emu_load_trap_triggered && virtual_tape && virtual_tape->loaded)
		{
			virtual_tape->loadBlock(this);
		}
		else
		{
			current_frame_ts_states -= ts;

			audioUpdateWithTs(ts);

			if (z80_core.GetTStates() >= machine_info.ts_per_frame)
			{
				z80_core.ResetTStates(machine_info.ts_per_frame);
				z80_core.SignalInterrupt();

				displayUpdateWithTs(static_cast<int32_t>(machine_info.ts_per_frame - emu_current_display_ts));

				emu_frame_counter++;

				audio_last_index = audio_buffer_index;
				displayFrameReset();
				keyboardCheckCapsLockStatus();
				audioDecayAYFloatingRegister();

				current_frame_ts_states = 0;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// - Debug

void ZXSpectrum::step()
{
	uint32_t ts = z80_core.Execute(1, machine_info.interrupt_length);

	if (virtual_tape && virtual_tape->playing)
	{
		virtual_tape->updateWithTs(ts);
	}

	if (virtual_tape && emu_save_trap_triggered)
	{
		virtual_tape->saveBlock(this);
	}
	else if (emu_load_trap_triggered && virtual_tape && virtual_tape->loaded)
	{
		virtual_tape->loadBlock(this);
	}
	else
	{
		if (z80_core.GetTStates() >= machine_info.ts_per_frame)
		{
			z80_core.ResetTStates(machine_info.ts_per_frame);
			z80_core.SignalInterrupt();

			emu_frame_counter++;

			displayFrameReset();
			keyboardCheckCapsLockStatus();

			audioDecayAYFloatingRegister();
		}
	}

	displayUpdateWithTs(static_cast<int32_t>(machine_info.ts_per_frame - emu_current_display_ts));
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
	emu_paused = true;
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::resume()
{
	emu_paused = false;
}

// ------------------------------------------------------------------------------------------------------------
// - Reset

void ZXSpectrum::resetMachine(bool hard)
{
	if (hard)
	{
		for (size_t i = 0; i < machine_info.ram_size; i++)
		{
			memory_ram[i] = static_cast<char>(rand() % 255);
		}
	}

	delete[] display_buffer;

	displaySetup();

	z80_core.Reset(hard);
	emuReset();
	keyboardMapReset();
	displayFrameReset();
	audioReset();
}

// ------------------------------------------------------------------------------------------------------------

void ZXSpectrum::emuReset()
{
	emu_frame_counter = 0;
	emu_save_trap_triggered = false;
	emu_load_trap_triggered = false;
}

// ------------------------------------------------------------------------------------------------------------
// - ROM Loading

ZXSpectrum::FileResponse ZXSpectrum::loadROM(const std::string rom, uint32_t page)
{
	size_t rom_address = cROM_SIZE * page;

	if (memory_rom.size() < rom_address)
	{
        std::cout << "ZXSpectrum::loadROM - Unable to load into ROM page " << page << "\n";
        return ZXSpectrum::FileResponse{false, std::to_string(page) + " is an invalid ROM page" };
	}

    std::string rom_path = emu_base_path + emu_rom_path;
	rom_path.append(rom);

    std::ifstream rom_file(rom_path, std::ios::binary | std::ios::ate | std::ios::in);
	if (rom_file.good())
	{
		std::streampos file_size = rom_file.tellg();
        rom_file.seekg(0, std::ios::beg);
		rom_file.read(memory_rom.data() + rom_address, file_size);
		rom_file.close();
        return ZXSpectrum::FileResponse{true, "Loaded successfully"};
	}

    char* error_string = strerror(errno);
    std::cout << "ERROR: Could not read from ROM file: " << error_string;
    return ZXSpectrum::FileResponse{false, error_string};
}

// ------------------------------------------------------------------------------------------------------------
// SCR Loading

ZXSpectrum::FileResponse ZXSpectrum::scrLoadWithPath(const std::string path)
{
    std::ifstream scr_file(path, std::ios::binary | std::ios::ate | std::ios::in);
    if (scr_file.good())
    {
        std::streampos file_size = scr_file.tellg();
        scr_file.seekg(0, std::ios::beg);
        
        switch (machine_info.machine_type) {
            case eZXSpectrum48:
                scr_file.read(memory_ram.data() + kDisplayBitmapAddress, file_size);
                break;
            case eZXSpectrum128:
                // Load the image data into memory page 5
                scr_file.read(memory_ram.data() + (kDisplayBitmapAddress * 5), file_size);
                break;
                
            default:
                break;
        }
        scr_file.close();
        return ZXSpectrum::FileResponse{true, "Loaded successfully"};
    }

    char* error_string = strerror(errno);
    std::cout << "ERROR: Could not read from file: " << error_string << "\n";
    
    return ZXSpectrum::FileResponse{false, error_string};
}

// ------------------------------------------------------------------------------------------------------------
// - Getters

void* ZXSpectrum::getScreenBuffer()
{
	return display_buffer;
}

// ------------------------------------------------------------------------------------------------------------
// - Release

void ZXSpectrum::release()
{
    std::cout << "ZXSpectrum::Release" << "\n";
    delete[] display_buffer;
	delete[] audio_buffer;
}





