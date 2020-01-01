//
//  WinMain.cpp
//  SpectREM
//
//  Created by Mike Daley on 26/10/2016.
//  Copyright � 2016 71Squared Ltd. All rights reserved.
//
//  25/07/17 - Adrian Brown - Added initial Win32 platform code

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <unordered_map>
#include <Shlwapi.h>
#include "AudioCore.hpp"
#include "..\Emulation Core\ZX_Spectrum_Core\ZXSpectrum.hpp"
#include "..\Emulation Core\ZX_Spectrum_48k\ZXSpectrum48.hpp"
#include "..\Emulation Core\ZX_Spectrum_128k\ZXSpectrum128.hpp"
#include "..\Emulation Core\Tape\Tape.hpp"
#include "..\OSX\AudioQueue.hpp"
#include "OpenGLView.hpp"
#include "../../resource.h"

#define WIN32API_GUI

static void audio_callback(uint32_t nNumSamples, uint8_t* pBuffer);
static void tapeStatusCallback(int blockIndex, int bytes);
static void LoadSnapshot();
static void HardReset();
static void SoftReset();
static void SwitchMachines();
static void ShowHelpAbout();
static void ShowHideUI(HWND hWnd);
static void ShowUI(HWND hWnd);
static void HideUI(HWND hWnd);
static void ResetMachineForSnapshot(uint8_t mc);
static void Log(std::string text);
static std::string GetApplicationBasePath();
static std::string GetCurrentDirectoryAsString();

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
HACCEL hAcc;
bool isResetting = false;
HWND mainWindow;

std::unordered_map<WPARAM, ZXSpectrum::ZXSpectrumKey> KeyMappings
{
    { VK_UP, ZXSpectrum::ZXSpectrumKey::Key_ArrowUp },
    { VK_DOWN, ZXSpectrum::ZXSpectrumKey::Key_ArrowDown },
    { VK_LEFT, ZXSpectrum::ZXSpectrumKey::Key_ArrowLeft },
    { VK_RIGHT, ZXSpectrum::ZXSpectrumKey::Key_ArrowRight },
    { VK_RETURN, ZXSpectrum::ZXSpectrumKey::Key_Enter },
};

//-----------------------------------------------------------------------------------------

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {

#ifdef WIN32API_GUI
    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case ID_FILE_EXIT:
            PostQuitMessage(0);
            break;
        case ID_FILE_OPENSNAPSHOT:
            LoadSnapshot();
            break;
        case ID_EMULATION_FULLSPEED:
            //
            break;
        case ID_RESET_HARD:
            HardReset();
            break;
        case ID_RESET_SOFT:
            SoftReset();
            break;
        case ID_SWITCH_TO48K:
            ResetMachineForSnapshot(ZX48);
            break;
        case ID_SWITCH_TO128K:
            ResetMachineForSnapshot(ZX128);
            break;
        case ID_SWITCH_FLIP:
            SwitchMachines();
            break;
        case ID_HELP_ABOUT:
            ShowHelpAbout();
            break;
        case ID_SHOWUI:
            //
            ShowHideUI(mainWindow);
            break;
        default:
            break;
        }
        break;
#endif

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (m_pMachine != nullptr)
        {
            if (wparam == VK_F1)
            {
                LoadSnapshot();
            }
            else if (wparam == VK_F3)
            {
                HardReset();
            }
            else if (wparam == VK_F4)
            {
                SoftReset();
            }
            else if (wparam == VK_F5)
            {
                /// TODO: Fails sometimes 48 -> 128
                SwitchMachines();
            }
            else if (wparam == VK_F10)
            {
                ShowHideUI(mainWindow);
            }
            // See if we asked to stop
            else if (wparam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            else
            {
                auto iter = KeyMappings.find(wparam);
                if (iter != KeyMappings.end())
                {
                    m_pMachine->keyboardKeyDown((*iter).second);
                }
            }
        }
        break;

    case WM_KEYUP:
        if (m_pMachine != nullptr)
        {
            auto iter = KeyMappings.find(wparam);
            if (iter != KeyMappings.end())
            {
                m_pMachine->keyboardKeyUp((*iter).second);
            }
        }

        break;

    case WM_SIZE:
        glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
        break;

    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

static void ShowHideUI(HWND hWnd = mainWindow)
{
    // Get the current state of the menu item checkbox
    MENUITEMINFO lpmi;
    HMENU mainMenu = GetMenu(hWnd);
    if (mainMenu != NULL)
    {
        if (GetMenuItemInfo(mainMenu, ID_SHOWUI, false, &lpmi))
        {
            // got the menuitem data, now check/uncheck and do the UI thing
            if (lpmi.fState & MFS_CHECKED)
            {
                // item is currently checked (UI visible)
                // uncheck it and remove the UI	
                lpmi.fState |= MFS_CHECKED;
                if (SetMenuItemInfo(mainMenu, ID_SHOWUI, false, &lpmi))
                {
                    // changed the check so do the deed...
                    HideUI(hWnd);
                }
            }
            else
            {
                // item is currently unchecked (UI invisible)
                // check it and show the UI
                lpmi.fState &= ~MFS_CHECKED;
                if (SetMenuItemInfo(mainMenu, ID_SHOWUI, false, &lpmi))
                {
                    // changed the check so do the deed...
                    ShowUI(hWnd);
                }
            }
        }
        DWORD lastError = GetLastError();
        if (lastError != 0)
        {
            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            std::string message(messageBuffer, size);
            cout << "ERROR: Could not change menu item..." << message;
        }
    }
}

static void ShowUI(HWND hWnd = mainWindow)
{

}

static void HideUI(HWND hWnd = mainWindow)
{

}


static void ShowHelpAbout()
{
    // 

}

static void SwitchMachines()
{
    // Switch machine (128<>48)
    if (m_pMachine->machineInfo.machineType == 1) // is it 128?
    {
        if (isResetting != true)
        {
            ResetMachineForSnapshot(ZX48);
        }
    }
    else
    {
        if (isResetting != true)
        {
            ResetMachineForSnapshot(ZX128);
        }
    }
}

static void SoftReset()
{
    // Soft reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType);
    }
}

static void HardReset()
{
    // Hard reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType);
    }
}

static void LoadSnapshot()
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
            ResetMachineForSnapshot(ZX48);
            Sleep(500);
        }
        else
        {
            // 128 based
            ResetMachineForSnapshot(ZX128);
            Sleep(500);
        }

        if (_stricmp(extension.c_str(), EXT_Z80.c_str()) == 0)
        {
            m_pMachine->snapshotZ80LoadWithPath(szFile);
        }
        else if (_stricmp(extension.c_str(), EXT_SNA.c_str()) == 0)
        {
            m_pMachine->snapshotSNALoadWithPath(szFile);
        }
    }
}

//-----------------------------------------------------------------------------------------

static void tapeStatusCallback(int blockIndex, int bytes)
{
}

//-----------------------------------------------------------------------------------------

static void audio_callback(uint32_t nNumSamples, uint8_t* pBuffer)
{
    if (isResetting != true)
    {
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
}

//-----------------------------------------------------------------------------------------

int __stdcall WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int ncmd)
{
    // check if under VS/Debugger and set up ROM paths accordingly
    if (IsDebuggerPresent() != 0)
    {
        romPath = "\\ROMS\\";
    }
    else
    {
        romPath = "\\ROMS\\";
    }

    bool exit_emulator = false;
    LARGE_INTEGER  perf_freq, time, last_time;
    MSG	msg;

    OutputDebugString(TEXT("SpectREM startup\r\n"));
    std::string bpath = GetApplicationBasePath();
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
    wcex.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
    wcex.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
    hAcc = nullptr;
#ifdef WIN32API_GUI
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    hAcc = LoadAccelerators(inst, MAKEINTRESOURCE(IDR_MENUACCELERATORS));
#endif

    RegisterClassEx(&wcex);

    // Make sure the client size is correct
    RECT wr = { 0, 0, 256 * 3, 192 * 3 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);


    mainWindow = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM"), TEXT("SpectREM"), WS_OVERLAPPEDWINDOW, 0, 0, wr.right - wr.left, wr.bottom - wr.top, 0, 0, inst, 0);
    ShowWindow(mainWindow, ncmd);
    UpdateWindow(mainWindow);

    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&last_time);

    m_pOpenGLView = new OpenGLView(GetApplicationBasePath());
    m_pOpenGLView->Init(mainWindow, 256 * 3, 192 * 3);
    m_pAudioQueue = new AudioQueue();
    m_pAudioCore = new AudioCore();
    m_pAudioCore->Init(44100, 50, audio_callback);
    m_pTape = new Tape(tapeStatusCallback);
    m_pMachine = new ZXSpectrum128(m_pTape);
    m_pMachine->emuUseAYSound = true;
    m_pMachine->emuBasePath = GetApplicationBasePath();
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
                if (!TranslateAccelerator(msg.hwnd, hAcc, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        else
        {
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
                char specType[20];
                if (m_pMachine->machineInfo.machineType == eZXSpectrum48)
                {
                    sprintf_s(specType, sizeof(specType), "ZX Spectrum 48K");
                }
                else
                {
                    sprintf_s(specType, sizeof(specType), "ZX Spectrum 128K");
                }
                char buff[64];
                sprintf_s(buff, sizeof(buff), "SpectREM - %4.1f fps - [%s]", 1.0f / delta_time, specType);
                SetWindowTextA(mainWindow, buff);
            }
        }
    }
    return 0;
}


static void ResetMachineForSnapshot(uint8_t mc)
{
    isResetting = true;

    m_pMachine->pause();
    m_pAudioCore->Stop();
    delete m_pAudioCore;
    m_pAudioCore = new AudioCore();
    m_pAudioCore->Init(44100, 50, audio_callback);
    delete m_pMachine;
    m_pMachine = nullptr;

    switch (mc)
    {
    case ZX48:
        OutputDebugString(TEXT("SpectREM changed to 48K Mode\r\n"));
        m_pMachine = new ZXSpectrum48(m_pTape);
        m_pMachine->emuUseAYSound = false;
        break;
    case ZX128:
        OutputDebugString(TEXT("SpectREM changed to 128K Mode\r\n"));
        m_pMachine = new ZXSpectrum128(m_pTape);
        m_pMachine->emuUseAYSound = true;
        break;
    default:
        // default to 128K
        OutputDebugString(TEXT("UNKNOWN MACHINE TYPE, Defaulting to 128K Mode\r\n"));
        m_pMachine = new ZXSpectrum128(m_pTape);
        m_pMachine->emuUseAYSound = true;
        break;
    }

    m_pMachine->emuBasePath = GetApplicationBasePath();
    m_pMachine->initialise(romPath);
    m_pAudioCore->Start();
    m_pMachine->resume();

    isResetting = false;
}

static std::string GetCurrentDirectoryAsString()
{
    TCHAR basePT[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, basePT);
    OutputDebugString(TEXT("Start path = "));
    OutputDebugString(basePT);
    OutputDebugString(TEXT("\r\n"));
    wstring test(&basePT[0]); // first convert it to WSTR
    std::string baseP(test.begin(), test.end()); // then finally get into a std::string
    return baseP;
}

static std::string GetApplicationBasePath()
{
    TCHAR appDirT[MAX_PATH];
    GetModuleFileName(NULL, appDirT, MAX_PATH);
    PathRemoveFileSpec(appDirT);
    wstring test(&appDirT[0]); // first convert it to WSTR
    std::string appDir(test.begin(), test.end()); // then finally get into a std::string
    return appDir;
}
//-----------------------------------------------------------------------------------------
