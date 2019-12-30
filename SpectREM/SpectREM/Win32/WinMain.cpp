//
//  WinMain.cpp
//  SpectREM
//
//  Created by Mike Daley on 26/10/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//
//  25/07/17 - Adrian Brown - Added initial Win32 platform code

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "AudioCore.hpp"
#include "..\Emulation Core\ZX_Spectrum_Core\ZXSpectrum.hpp"
#include "..\Emulation Core\ZX_Spectrum_48k\ZXSpectrum48.hpp"
#include "..\Emulation Core\ZX_Spectrum_128k\ZXSpectrum128.hpp"
#include "..\Emulation Core\Tape\Tape.hpp"
#include "..\AudioQueue.hpp"
#include "OpenGLView.hpp"

static void audio_callback(uint32_t nNumSamples, uint8_t* pBuffer);
static void tapeStatusCallback(int blockIndex, int bytes);
static void ResetMachineForSnapshot(uint8_t mc);
static void Log(std::string text);

ZXSpectrum* m_pMachine;
Tape* m_pTape;
AudioCore* m_pAudioCore;
AudioQueue* m_pAudioQueue;
OpenGLView* m_pOpenGLView;

enum MachineType
{
	ZX48, ZX128, PLUS2, PLUS3, UNKNOWN
} mType;

enum SnapType
{
	SNA, Z80
};

const std::string EXT_Z80 = "z80";
const std::string EXT_SNA = "sna";
std::string romPath;

bool isResetting = false;

//-----------------------------------------------------------------------------------------

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (m_pMachine != NULL)
		{
			if (wparam == VK_F1)
			{
				OPENFILENAMEA ofn;
				char szFile[_MAX_PATH];

				// Setup the ofn structure
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = "All\0*.*\0Snapshot\0*.SNA\0Z80\0*.Z80\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (GetOpenFileNameA(&ofn))
				{
					int32_t mType = m_pMachine->snapshotMachineInSnapshotWithPath(szFile);
					std::string s(szFile, sizeof(szFile));
					std::string extension = s.substr(s.find_last_of(".") + 1, s.find_last_of(".") + 4);

					// Check the machine type returned from the user supplied snapshot
					if (mType <= ZX48)
					{
						// 48 based
						OutputDebugString(TEXT("Snapshot changed SpectREM to 48K mode"));
						ResetMachineForSnapshot(ZX48);
						Sleep(500);
					}
					else
					{
						// 128 based
						OutputDebugString(TEXT("Snapshot changed SpectREM to 128K Mode"));
						ResetMachineForSnapshot(ZX128);
						Sleep(500);
					}

					if (stricmp(extension.c_str(), EXT_Z80.c_str()) == 0)
					{
						m_pMachine->snapshotZ80LoadWithPath(szFile);
					}
					else if (stricmp(extension.c_str(), EXT_SNA.c_str()) == 0)
					{
						m_pMachine->snapshotSNALoadWithPath(szFile);
					}
				}
			}

			else if (wparam == VK_F3)
			{
				m_pMachine->resetMachine(true);
			}
			else if (wparam == VK_F4)
			{
				m_pMachine->resetMachine(false);
			}

			// See if we asked to stop
			else if (wparam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			else
			{
				m_pMachine->keyboardKeyDown(static_cast<uint16_t>(wparam));
			}
		}

	case WM_KEYUP:
		if (m_pMachine != NULL)
		{
			m_pMachine->keyboardKeyUp(static_cast<uint16_t>(wparam));
		}

		break;

		// 	case WM_PAINT:
		// 	{
		// 		PAINTSTRUCT ps;
		// 
		// 		HDC hdc = BeginPaint(hwnd, &ps);
		// 
		// 		// Draw the display
		// 
		// 		if (m_pMachine != NULL)
		// 		{
		// 
		// 		}
		// 
		// 
		// 		EndPaint(hwnd, &ps);
		// 	}
		// 	return 0L;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
		break;

	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------

static void tapeStatusCallback(int blockIndex, int bytes)
{
}

//-----------------------------------------------------------------------------------------

static void audio_callback(uint32_t nNumSamples, uint8_t* pBuffer)
{
	if (isResetting == true) return;
	if (m_pMachine)
	{
		m_pAudioQueue->read((int16_t*)pBuffer, nNumSamples);

		// Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
		if (m_pAudioQueue->bufferUsed() < ((44100 * 2) / 50))
		{
			m_pMachine->generateFrame();

			//			m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);

			m_pAudioQueue->write(m_pMachine->audioBuffer, ((44100 * 2) / 50));
		}
	}
}

//-----------------------------------------------------------------------------------------

int __stdcall WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int ncmd)
{
	// check if under VS/Debugger and set up ROM paths accordingly
	if (IsDebuggerPresent() != 0)
	{
		romPath = "SpectREM\\Emulation Core\\ROMS\\";
	}
	else
	{
		romPath = "ROMS\\";
	}


	bool exit_emulator = false;
	LARGE_INTEGER  perf_freq, time, last_time;
	MSG	msg;

	OutputDebugString(TEXT("SpectREM startup"));
	// Create our window
	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = inst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszClassName = TEXT("SpectREM");
	RegisterClassEx(&wcex);

	// Make sure the client size is correct
	RECT wr = { 0, 0, 256 * 3, 192 * 3 };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND window = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM"), TEXT("SpectREM"), WS_OVERLAPPEDWINDOW, 0, 0, wr.right - wr.left, wr.bottom - wr.top, 0, 0, inst, 0);
	ShowWindow(window, ncmd);
	UpdateWindow(window);

	QueryPerformanceFrequency(&perf_freq);
	QueryPerformanceCounter(&last_time);

	m_pOpenGLView = new OpenGLView();
	m_pOpenGLView->Init(window, 256 * 3, 192 * 3);
	m_pAudioQueue = new AudioQueue();
	m_pAudioCore = new AudioCore();
	m_pAudioCore->Init(44100, 50, audio_callback);
	m_pTape = new Tape(tapeStatusCallback);
	m_pMachine = new ZXSpectrum128(m_pTape);
	m_pMachine->emuUseAYSound = true;
	m_pMachine->initialise(romPath);
	m_pAudioCore->Start();
	m_pMachine->resume();

	// Do the main message loop
	while (!exit_emulator)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				exit_emulator = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			//Sleep(10);
			if (isResetting == true) break;
			// Get the current time counter
			LARGE_INTEGER old_time = last_time;
			QueryPerformanceCounter(&time);
			const float delta_time = (time.QuadPart - old_time.QuadPart) / (float)perf_freq.QuadPart;

			// See if we need to update
			if (delta_time > 1.0f / 50.0f || GetAsyncKeyState(VK_F2))
			{
				last_time = time;

				m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);

				// Force the window to redraw
				//InvalidateRect(window, NULL, true);

				// Set the time
				char buff[64];
				sprintf_s(buff, 64, "SpectREM - %4.1f fps", 1.0f / delta_time);
				SetWindowTextA(window, buff);
			}
			//g_pMachine->RunFrame();
		}
	}
	return 0;
}


static void ResetMachineForSnapshot(uint8_t mc)
{
	isResetting = true;
	delete m_pAudioQueue;
	m_pAudioQueue = nullptr;
	m_pAudioQueue = new AudioQueue();

	m_pAudioCore->Deinit();
	delete m_pAudioCore;
	m_pAudioCore = nullptr;
	m_pAudioCore = new AudioCore();
	m_pAudioCore->Init(44100, 50, audio_callback);

	delete m_pMachine;
	m_pMachine = nullptr;

	switch (mc)
	{
	case ZX48:
		m_pMachine = new ZXSpectrum48(m_pTape);
		m_pMachine->emuUseAYSound = false;
		break;
	case ZX128:
		m_pMachine = new ZXSpectrum128(m_pTape);
		m_pMachine->emuUseAYSound = true;
		break;
	}

	m_pMachine->initialise(romPath);
	//m_pMachine->resetMachine(true);
	m_pAudioCore->Start();
	//m_pMachine->resume();
	isResetting = false;
}

//-----------------------------------------------------------------------------------------
