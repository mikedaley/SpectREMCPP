//
//  WinMain.cpp
//  SpectREM
//
//  Created by Mike Daley on 26/10/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//
//  25/07/17 - Adrian Brown - Added initial Win32 platform code

#define _CRT_RAND_S
#define WIN32API_GUI

#define PM_TAPEDATA_FULL            77
#define PM_TAPE_VIEWER_CLOSED       78
#define PM_TAPE_COMMAND             79
#define PM_TAPE_EJECTED             80
#define PM_TAPE_ACTIVEBLOCK         81
#define PM_TAPE_PLAY                82
#define PM_TAPE_PAUSE               83
#define PM_TAPE_REWIND              84
#define PM_TAPE_INSERT              85
#define PM_TAPE_EJECT               86
#define PM_TAPE_SAVE                87
#define PM_TAPE_UPDATE_PLAYPAUSEETC 88

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unordered_map>
#include <Shlwapi.h>
#include <thread>
#include <filesystem>
#include "AudioCore.hpp"
#include <iostream>
#include <fstream>
#include <future>
#include "..\Emulation Core\ZX_Spectrum_Core\ZXSpectrum.hpp"
#include "..\Emulation Core\ZX_Spectrum_48k\ZXSpectrum48.hpp"
#include "..\Emulation Core\ZX_Spectrum_128k\ZXSpectrum128.hpp"
#include "..\Emulation Core\Tape\Tape.hpp"
#include "..\OSX\AudioQueue.hpp"
#include "OpenGLView.hpp"
#include "../../resource.h"
#include "PMDawn.hpp"
#include <CommCtrl.h>
#include "TapeViewerWindow.hpp"
#include <process.h>

#pragma comment(lib, "comctl32.lib")


// WinMain.cpp
static void audio_callback(uint32_t nNumSamples, uint8_t* pBuffer);
static void tapeStatusCallback(int blockIndex, int bytes, int action);
static void LoadSnapshot();
static void HardReset();
static void SoftReset();
static void SwitchMachines();
static void ShowHelpAbout();
static void ShowHideUI(HWND hWnd);
static void ShowUI(HWND hWnd);
static void HideUI(HWND hWnd);
static void ResetMachineForSnapshot(uint8_t mc, bool ayEnabled);
static void ShowSettingsDialog();
static void RunSlideshow(int secs);
void IterateSCRImages(HWND mWindow, std::vector<std::string> fileList, ZXSpectrum* m_pMachine, int secs);
static void IterateSCRImagesOnTimerCallback();
static void OpenSCR();
static void InsertTape();
static void EjectTape();
static void PlayPauseTape();
static void RewindTape();
static void OpenTapeViewer();
static void SetOutputVolume(float vol);
static void IncreaseApplicationVolume();
static void DecreaseApplicationVolume();
unsigned int __stdcall mythread(void* data);
static void SendTapeBlockDataToViewer();
static void GetTapeViewerHwnd();
static void SetupThreadLocalStorageForTapeData();
RECT GetWindowResizeWithUI(HWND mWin, HWND sWin, HMENU menuWin, bool visible);

ZXSpectrum* m_pMachine;
Tape* m_pTape;
AudioCore* m_pAudioCore;
AudioQueue* m_pAudioQueue;
OpenGLView* m_pOpenGLView;
static TapeViewer* tvWindow;
std::string loadedFile;

enum MachineType
{
    ZX48, ZX128, PLUS2, PLUS3, UNKNOWN
} mType;

enum SnapType
{
    SNA, Z80
};

// Status / BlockType / Filename / AutostartLine / Address / Length

static DWORD dwTlsIndex;
static int cxClient, cyClient;
const UINT PM_UPDATESPECTREM = 7777;
const std::string EXT_Z80 = "z80";
const std::string EXT_SNA = "sna";
const std::string EXT_TAP = "tap";
const UINT_PTR IDT_SLIDESHOW = 7778;
std::string romPath;
HACCEL hAcc;
bool isResetting = false;
HWND mainWindow;
HWND statusWindow;
HMENU mainMenu;
bool TurboMode = false;
bool menuDisplayed = true;
bool statusDisplayed = true;
uint8_t zoomLevel = 3;
std::string slideshowDirectory = "\\slideshow\\";
std::vector<std::string> fileList;
uint8_t fileListIndex = 0;
std::thread scrDisplayThread;
bool slideshowTimerRunning = false;
bool slideshowRandom = true;
const float volumeStep = 0.1f;
float applicationVolume = 0.75f;
GLint viewportX;
GLint viewportY;
HANDLE tapeViewerThread;
HWND tvHwnd;

std::unordered_map<WPARAM, ZXSpectrum::eZXSpectrumKey> KeyMappings
{
    { VK_UP, ZXSpectrum::eZXSpectrumKey::Key_ArrowUp },
    { VK_DOWN, ZXSpectrum::eZXSpectrumKey::Key_ArrowDown },
    { VK_LEFT, ZXSpectrum::eZXSpectrumKey::Key_ArrowLeft },
    { VK_RIGHT, ZXSpectrum::eZXSpectrumKey::Key_ArrowRight },
    { VK_RETURN, ZXSpectrum::eZXSpectrumKey::Key_Enter },
    { VK_SHIFT, ZXSpectrum::eZXSpectrumKey::Key_Shift },
    { VK_RSHIFT, ZXSpectrum::eZXSpectrumKey::Key_Shift },
    { VK_SPACE, ZXSpectrum::eZXSpectrumKey::Key_Space },
    { VK_CONTROL, ZXSpectrum::eZXSpectrumKey::Key_SymbolShift },
    { VK_RCONTROL, ZXSpectrum::eZXSpectrumKey::Key_SymbolShift },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_InvVideo },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_TrueVideo },
    { VK_DELETE, ZXSpectrum::eZXSpectrumKey::Key_Backspace },
    { VK_BACK, ZXSpectrum::eZXSpectrumKey::Key_Backspace },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Quote },
    { VK_OEM_1, ZXSpectrum::eZXSpectrumKey::Key_SemiColon },
    { VK_OEM_COMMA, ZXSpectrum::eZXSpectrumKey::Key_Comma },
    { VK_OEM_MINUS, ZXSpectrum::eZXSpectrumKey::Key_Minus },
    { VK_OEM_PLUS, ZXSpectrum::eZXSpectrumKey::Key_Plus },
    { VK_OEM_PERIOD, ZXSpectrum::eZXSpectrumKey::Key_Period },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Edit },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Graph },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Break },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_ExtendMode },
    { VK_CAPITAL, ZXSpectrum::eZXSpectrumKey::Key_CapsLock },
    // Numbers
    { 0x30, ZXSpectrum::eZXSpectrumKey::Key_0 },
    { 0x31, ZXSpectrum::eZXSpectrumKey::Key_1 },
    { 0x32, ZXSpectrum::eZXSpectrumKey::Key_2 },
    { 0x33, ZXSpectrum::eZXSpectrumKey::Key_3 },
    { 0x34, ZXSpectrum::eZXSpectrumKey::Key_4 },
    { 0x35, ZXSpectrum::eZXSpectrumKey::Key_5 },
    { 0x36, ZXSpectrum::eZXSpectrumKey::Key_6 },
    { 0x37, ZXSpectrum::eZXSpectrumKey::Key_7 },
    { 0x38, ZXSpectrum::eZXSpectrumKey::Key_8 },
    { 0x39, ZXSpectrum::eZXSpectrumKey::Key_9 },
    // Letters
    { 0x41, ZXSpectrum::eZXSpectrumKey::Key_A },
    { 0x42, ZXSpectrum::eZXSpectrumKey::Key_B },
    { 0x43, ZXSpectrum::eZXSpectrumKey::Key_C },
    { 0x44, ZXSpectrum::eZXSpectrumKey::Key_D },
    { 0x45, ZXSpectrum::eZXSpectrumKey::Key_E },
    { 0x46, ZXSpectrum::eZXSpectrumKey::Key_F },
    { 0x47, ZXSpectrum::eZXSpectrumKey::Key_G },
    { 0x48, ZXSpectrum::eZXSpectrumKey::Key_H },
    { 0x49, ZXSpectrum::eZXSpectrumKey::Key_I },
    { 0x4a, ZXSpectrum::eZXSpectrumKey::Key_J },
    { 0x4b, ZXSpectrum::eZXSpectrumKey::Key_K },
    { 0x4c, ZXSpectrum::eZXSpectrumKey::Key_L },
    { 0x4d, ZXSpectrum::eZXSpectrumKey::Key_M },
    { 0x4e, ZXSpectrum::eZXSpectrumKey::Key_N },
    { 0x4f, ZXSpectrum::eZXSpectrumKey::Key_O },
    { 0x50, ZXSpectrum::eZXSpectrumKey::Key_P },
    { 0x51, ZXSpectrum::eZXSpectrumKey::Key_Q },
    { 0x52, ZXSpectrum::eZXSpectrumKey::Key_R },
    { 0x53, ZXSpectrum::eZXSpectrumKey::Key_S },
    { 0x54, ZXSpectrum::eZXSpectrumKey::Key_T },
    { 0x55, ZXSpectrum::eZXSpectrumKey::Key_U },
    { 0x56, ZXSpectrum::eZXSpectrumKey::Key_V },
    { 0x57, ZXSpectrum::eZXSpectrumKey::Key_W },
    { 0x58, ZXSpectrum::eZXSpectrumKey::Key_X },
    { 0x59, ZXSpectrum::eZXSpectrumKey::Key_Y },
    { 0x5a, ZXSpectrum::eZXSpectrumKey::Key_Z },

};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

void ZoomWindow(uint8_t zLevel)
{
    zoomLevel = zLevel;
    RECT wr = { 0, 0, 256 * zoomLevel, 192 * zoomLevel };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    SetWindowPos(mainWindow, HWND_TOP, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE | SWP_SHOWWINDOW);
    //glViewport(0, 0, 256 * zoomLevel, 192 * zoomLevel);
    m_pOpenGLView->Resize(256 * zoomLevel, 192 * zoomLevel);
    PMDawn::Log(PMDawn::LOG_INFO, "Zoom level changed to " + std::to_string(zoomLevel));
}

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
            TurboMode = !TurboMode;
            break;
        case ID_RESET_HARD:
            HardReset();
            break;
        case ID_RESET_SOFT:
            SoftReset();
            break;
        case ID_SWITCH_TO48K:
            ResetMachineForSnapshot(ZX48, m_pMachine->ayEnabledSnapshot);
            break;
        case ID_SWITCH_TO128K:
            ResetMachineForSnapshot(ZX128, true);
            break;
        case ID_SWITCH_FLIP:
            SwitchMachines();
            break;
        case ID_HELP_ABOUT:
            ShowHelpAbout();
            break;
        case ID_SHOWUI:
            ShowHideUI(mainWindow);
            break;
        case ID_APPLICATION_SETTINGS:
            ShowSettingsDialog();
            break;
        case ID_ZOOM_100:
            ZoomWindow(1);
            break;
        case ID_ZOOM_200:
            ZoomWindow(2);
            break;
        case ID_ZOOM_300:
            ZoomWindow(3);
            break;
        case ID_ZOOM_400:
            ZoomWindow(4);
            break;
        case ID_SCRSLIDESHOW_DELAY1SECOND:
            RunSlideshow(1);
            break;
        case ID_SCRSLIDESHOW_DELAY3SECONDS:
            RunSlideshow(3);
            break;
        case ID_SCRSLIDESHOW_DELAY6SECONDS:
            RunSlideshow(6);
            break;
        case ID_SCRSLIDESHOW_DELAY10SECONDS:
            RunSlideshow(10);
            break;
        case ID_VIEW_OPENSCR:
            OpenSCR();
            break;
        case ID_TAPE_INSERTTAPE:
            InsertTape();
            break;
        case ID_TAPE_EJECTTAPE:
            EjectTape();
            break;
        case ID_TAPE_START:
            PlayPauseTape();
            break;
        case ID_TAPE_REWINDTAPE:
            RewindTape();
            break;
        case ID_TAPE_TAPEVIEWER:
            OpenTapeViewer();
            break;
        case ID_VOLUME_INCREASE:
            IncreaseApplicationVolume();
            break;
        case ID_VOLUME_DECREASE:
            DecreaseApplicationVolume();
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
            else if (wparam == VK_F2)
            {
                TurboMode = !TurboMode;
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
                SwitchMachines();
            }
            else if (wparam == VK_F10)
            {
                ShowHideUI(mainWindow);
            }
            else if (wparam == VK_F12)
            {
                ShowSettingsDialog();
            }
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
        cxClient = LOWORD(lparam);
        cyClient = HIWORD(lparam);
        //glViewport(viewportX, viewportY, 256 * zoomLevel, 192 * zoomLevel);
        return 0;
        break;

    case WM_USER:
        switch (LOWORD(wparam))
        {
        case PM_UPDATESPECTREM:
            Sleep(50);
            m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);
            PMDawn::Log(PMDawn::LOG_DEBUG, "Changed slideshow image");
            break;
        }
        break;

    case WM_USER + 1:
        switch (LOWORD(wparam))
        {
        case 1:
            GetTapeViewerHwnd();
            if (m_pTape->numberOfTapeBlocks() > 0)
            {
                SendTapeBlockDataToViewer();
                PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_ACTIVEBLOCK, (LPARAM)m_pTape->currentBlockIndex);
                if (m_pTape->playing)
                {
                    PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_UPDATE_PLAYPAUSEETC, 1);  // Indicate tape is playing
                }
                else
                {
                    PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_UPDATE_PLAYPAUSEETC, 0);  // Indicate tape is paused
                }
            }
            break;
        }
        break;

    case WM_USER + 2:
        switch (LOWORD(wparam))
        {
        case PM_TAPE_VIEWER_CLOSED:
            if (tapeViewerThread)
            {
                CloseHandle(tapeViewerThread);
                tapeViewerThread = nullptr;
            }
            break;
        case PM_TAPE_COMMAND:
            // The tape window is sending a command to do something with the tape
            switch (lparam)
            {
            case PM_TAPE_PLAY:
                m_pTape->play();
                break;

            case PM_TAPE_PAUSE:
                m_pTape->stop();// stopPlaying();
                break;

            case PM_TAPE_REWIND:
                RewindTape();
                break;

            case PM_TAPE_INSERT:
                InsertTape();
                break;

            case PM_TAPE_EJECT:
                EjectTape();
                break;

            }
            break;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(mainWindow, &ps);
        //if (m_pMachine)
        //{
        //    m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);
        //}
        EndPaint(mainWindow, &ps);
        return 0;
        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

//-----------------------------------------------------------------------------------------

static void InsertTape()
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
    ofn.lpstrFilter = "All\0*.*\0Tapes\0*.TAP\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        EjectTape(); // Eject the current tape if inserted
        Tape::FileResponse tR = m_pTape->insertTapeWithPath(szFile);
        if (tR.success)
        {
            PMDawn::Log(PMDawn::LOG_INFO, "Loaded tape - " + std::string(szFile));
            PathStripPathA(szFile);
            loadedFile = "TAPE: " + std::string(szFile);
            SendTapeBlockDataToViewer();
        }
        else
        {
            MessageBox(mainWindow, TEXT("Unable to load tape >> "), TEXT("Tape Loader"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            PMDawn::Log(PMDawn::LOG_INFO, "Failed to load tape - " + std::string(szFile) + " > " + tR.responseMsg);
            loadedFile = "-empty-";
            return;
        }
    }
}

//-----------------------------------------------------------------------------------------

static void PlayPauseTape()
{
    if (m_pTape->loaded)
    {
        if (m_pTape->playing)
        {
            m_pTape->stop();
            PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_UPDATE_PLAYPAUSEETC, 0);
        }
        else
        {
            m_pTape->play();
            PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_UPDATE_PLAYPAUSEETC, 1);
        }
    }
}

//-----------------------------------------------------------------------------------------

static void EjectTape()
{
    if (m_pTape->loaded)
    {
        m_pTape->stop();
        m_pTape->eject();
        PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_EJECTED, (LPARAM)0);
    }
}

//-----------------------------------------------------------------------------------------

static void RewindTape()
{
    if (m_pTape->loaded)
    {
        m_pTape->stop();
        m_pTape->rewindTape();
        PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_ACTIVEBLOCK, (LPARAM)0); // Let the tape viewer know we are on block number 0
        PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_UPDATE_PLAYPAUSEETC, 0); // Let the tape viewer know that we are paused
    }
}
//-----------------------------------------------------------------------------------------

static void OpenSCR()
{
    HardReset();
    Sleep(1000);

    std::string scrPath = PMDawn::GetFilenameUsingDialog("");

    if (scrPath != "")
    {
        Tape::FileResponse sR = m_pMachine->scrLoadWithPath(scrPath);
        if (sR.success)
        {
            Sleep(1);
            m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);
            PMDawn::Log(PMDawn::LOG_INFO, "Loaded .scr file - " + std::string(scrPath));
        }
        else
        {
            MessageBox(mainWindow, TEXT("Invalid SCR file"), TEXT("Gimme SCR's !!!"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            PMDawn::Log(PMDawn::LOG_INFO, "Failed to load .scr file - " + std::string(scrPath) + " > " + sR.responseMsg);
            return;
        }
    }
}

//-----------------------------------------------------------------------------------------

static void RunSlideshow(int secs)
{
    HardReset();
    Sleep(1000);
    PMDawn::Log(PMDawn::LOG_INFO, "Running slideshow (" + std::to_string(secs) + " secs) from " + PMDawn::GetApplicationBasePath() + slideshowDirectory);
    fileList.clear();
    std::string fpath = PMDawn::GetFolderUsingDialog("");
    fileList = PMDawn::GetFilesInDirectory(fpath + "\\", "*.scr");
    PMDawn::Log(PMDawn::LOG_DEBUG, "Found " + std::to_string(fileList.size()) + " matching files");
    // iterate (randomly maybe) through the list of files as long as there is at least one file :)
    if (fileList.size() < 1)
    {
        MessageBox(mainWindow, TEXT("No files found for the slideshow"), TEXT("Gimme SCR's !!!"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
        return;
    }
    else
    {
        fileListIndex = 0;
        // setup timer
        SetTimer(mainWindow, IDT_SLIDESHOW, secs * 1000, (TIMERPROC)IterateSCRImagesOnTimerCallback);
        slideshowTimerRunning = true;

        ////auto scrDisplayThread = std::async(IterateSCRImages, std::ref(mainWindow), std::ref(fileList), std::ref(m_pMachine), std::ref(secs));
        //try 
        //{
        //	std::thread scrDisplayThread(IterateSCRImages, std::ref(mainWindow), std::ref(fileList), std::ref(m_pMachine), std::ref(secs));
        //    //scrDisplayThread = std::thread(IterateSCRImages, std::ref(mainWindow), std::ref(fileList), std::ref(m_pMachine), std::ref(secs));
        //    //scrDisplayThread.join();
        //}
        //catch (std::exception & r)
        //{
        //    Log(LOG_DEBUG, "EXCEPTION");
        //    std::string errormsg = r.what();
        //}
    }
}

//-----------------------------------------------------------------------------------------

static void IterateSCRImagesOnTimerCallback()
{
    if (slideshowRandom)
    {
        int randomIndex = (int)rand() % fileList.size();
        Tape::FileResponse sR = m_pMachine->scrLoadWithPath(PMDawn::GetApplicationBasePath() + slideshowDirectory + fileList[randomIndex]);
        Sleep(1);
        m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);
    }
    else
    {
        Tape::FileResponse sR = m_pMachine->scrLoadWithPath(PMDawn::GetApplicationBasePath() + slideshowDirectory + fileList[fileListIndex]);
        Sleep(1);
        m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);
        fileListIndex++;
        if (fileListIndex >= fileList.size())
        {
            fileListIndex = 0;
            KillTimer(mainWindow, IDT_SLIDESHOW);
            slideshowTimerRunning = false;
            return;
        }
    }
}

//-----------------------------------------------------------------------------------------

static void IterateSCRImages(HWND mWindow, std::vector<std::string> fileList, ZXSpectrum* machine, int delaysecs)
{
    // THREAD
    try
    {
        std::chrono::milliseconds msecs(delaysecs * 1000);
        for (std::size_t i = 0; i < 10; i++)//< fileList.size(); i++)
        {
            Tape::FileResponse sR = machine->scrLoadWithPath(PMDawn::GetApplicationBasePath() + slideshowDirectory + fileList[i]);
            //SendMessageCallback(mWindow, WM_USER, PM_UPDATESPECTREM, PM_UPDATESPECTREM, nullptr, 0);
            PostMessage(mWindow, WM_USER, PM_UPDATESPECTREM, PM_UPDATESPECTREM);
            std::this_thread::sleep_for(msecs);
        }
    }
    catch (std::exception & r)
    {
        std::string errormsg = r.what();
    }
}

//-----------------------------------------------------------------------------------------

static void ShowSettingsDialog()
{
    PMDawn::Log(PMDawn::LOG_DEBUG, "ShowSettingsDialog()");
    MessageBoxA(mainWindow, "NOT IMPLEMENTED YET.", "Oh oh...", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
}

//-----------------------------------------------------------------------------------------

static void ShowHideUI(HWND hWnd = mainWindow)
{
    // Get the current state of the menu item check box
    if (menuDisplayed == true)
    {
        HideUI(hWnd);
    }
    else
    {
        ShowUI(hWnd);
    }
}

//-----------------------------------------------------------------------------------------

static void ShowUI(HWND hWnd = mainWindow)
{
    PMDawn::Log(PMDawn::LOG_DEBUG, "ShowUI()");
    menuDisplayed = true;
    statusDisplayed = true;
    RECT newSize = GetWindowResizeWithUI(mainWindow, statusWindow, mainMenu, true);
    SetMenu(hWnd, mainMenu);
    SetWindowPos(mainWindow, HWND_NOTOPMOST, newSize.left, newSize.top, newSize.right, newSize.bottom, 0);
    m_pOpenGLView->Resize(256 * zoomLevel, 192 * zoomLevel);
    ShowWindow(statusWindow, SW_SHOW);
}

//-----------------------------------------------------------------------------------------

static void HideUI(HWND hWnd = mainWindow)
{
    PMDawn::Log(PMDawn::LOG_DEBUG, "HideUI()");
    menuDisplayed = false;
    statusDisplayed = false;
    RECT newSize = GetWindowResizeWithUI(mainWindow, statusWindow, mainMenu, false);
    SetMenu(hWnd, NULL);
    SetWindowPos(mainWindow, HWND_NOTOPMOST, newSize.left, newSize.top, newSize.right, newSize.bottom, 0);
    m_pOpenGLView->Resize(256 * zoomLevel, 192 * zoomLevel);
    ShowWindow(statusWindow, SW_HIDE);
}

//-----------------------------------------------------------------------------------------

RECT GetWindowResizeWithUI(HWND mWin, HWND sWin, HMENU menu, bool visible)
{
    // Get the new window size needed for the status bar to be below the GLView...
    if (visible)
    {
        Log(PMDawn::LOG_INFO, "Resizing with UI");
        Log(PMDawn::LOG_INFO, "Menu height == " + GetSystemMetrics(SM_CYMENU));
    }
    else
    {
        Log(PMDawn::LOG_INFO, "Resizing without UI");
        Log(PMDawn::LOG_INFO, "Menu height == " + GetSystemMetrics(SM_CYMENU));
    }
    RECT m, s;
    GetWindowRect(mWin, &m);
    GetWindowRect(sWin, &s);
   
    Log(PMDawn::LOG_INFO, "Main window RECT = t" +
        std::to_string(m.top) + " l" +
        std::to_string(m.left) + " b" +
        std::to_string(m.bottom) + " r" +
        std::to_string(m.right));
    Log(PMDawn::LOG_INFO, "Status bar RECT = t" +
        std::to_string(s.top) + " l" +
        std::to_string(s.left) + " b" +
        std::to_string(s.bottom) + " r" +
        std::to_string(s.right));

    if (visible)
    {
        RECT nWin = {
            m.left,
            m.top,
            (m.right - m.left),
            (m.bottom - m.top) + (s.bottom - s.top) + GetSystemMetrics(SM_CYMENU)
        };
        Log(PMDawn::LOG_INFO, "Output RECT = t" +
            std::to_string(nWin.top) + " l" +
            std::to_string(nWin.left) + " b" +
            std::to_string(nWin.bottom) + " r" +
            std::to_string(nWin.right));
        return nWin;
    }
    else
    {
        RECT nWin = {
            m.left,
            m.top,
            (m.right - m.left),
            (m.bottom - m.top) - (s.bottom - s.top) - GetSystemMetrics(SM_CYMENU)
        };
        Log(PMDawn::LOG_INFO, "Output RECT = t" +
            std::to_string(nWin.top) + " l" +
            std::to_string(nWin.left) + " b" +
            std::to_string(nWin.bottom) + " r" +
            std::to_string(nWin.right));
        return nWin;
    }
}

//-----------------------------------------------------------------------------------------

static void ShowHelpAbout()
{
    PMDawn::Log(PMDawn::LOG_DEBUG, "ShowHelpAbout()");
    MessageBoxA(mainWindow, "NOT IMPLEMENTED YET.", "Oh oh...", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
}

//-----------------------------------------------------------------------------------------

static void SwitchMachines()
{
    // Switch machine (128<>48)
    if (m_pMachine->machineInfo.machineType == 1) // is it 128?
    {
        if (isResetting != true)
        {
            PMDawn::Log(PMDawn::LOG_DEBUG, "Flip machine requested");
            ResetMachineForSnapshot(ZX48, m_pMachine->ayEnabledSnapshot);
        }
    }
    else
    {
        if (isResetting != true)
        {
            ResetMachineForSnapshot(ZX128, true);
        }
    }
}

//-----------------------------------------------------------------------------------------

static void SoftReset()
{
    // Soft reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType, m_pMachine->ayEnabledSnapshot);
        PMDawn::Log(PMDawn::LOG_INFO, "Soft reset completed");
    }
}

//-----------------------------------------------------------------------------------------

static void HardReset()
{
    // Hard reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType, m_pMachine->ayEnabledSnapshot);
        PMDawn::Log(PMDawn::LOG_INFO, "Hard reset completed");
    }
}

//-----------------------------------------------------------------------------------------

static void LoadSnapshot()
{
    std::string filePath = PMDawn::GetFilenameUsingDialog("");

    if (filePath != "")
    {
        int32_t mType = m_pMachine->snapshotMachineInSnapshotWithPath(filePath.c_str());
        size_t sizeOfFile = sizeof(filePath) - 1;
        std::string s(filePath.c_str(), sizeOfFile);
        std::string extension = filePath.substr(filePath.find_last_of(".") + 1, filePath.find_last_of(".") + 4);

        // Check the machine type returned from the user supplied snapshot if not a tape file
        if (_stricmp(extension.c_str(), EXT_TAP.c_str()) == 0)
        {
            EjectTape(); // Eject the current tape if inserted
            Tape::FileResponse tR = m_pTape->insertTapeWithPath(filePath);
            if (tR.success)
            {
                PMDawn::Log(PMDawn::LOG_INFO, "Loaded tape - " + std::string(filePath));
                PathStripPathA(const_cast<char*>(filePath.c_str()));
                loadedFile = "TAPE: " + filePath;
                SendTapeBlockDataToViewer();
            }
            else
            {
                MessageBox(mainWindow, TEXT("Unable to load tape >> "), TEXT("Tape Loader"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
                loadedFile = "-empty-";
                PMDawn::Log(PMDawn::LOG_INFO, "Failed to load tape - " + std::string(filePath) + " > " + tR.responseMsg);
            }
        }
        else
        {
            EjectTape();
            if (mType <= ZX48)
            {
                // 48 based
                ResetMachineForSnapshot(ZX48, m_pMachine->ayEnabledSnapshot);
                Sleep(500);
            }
            else
            {
                // 128 based
                ResetMachineForSnapshot(ZX128, true);
                Sleep(500);
            }
            if (_stricmp(extension.c_str(), EXT_Z80.c_str()) == 0)
            {
                PMDawn::Log(PMDawn::LOG_INFO, "Loading Z80 Snapshot - " + s);
                Tape::FileResponse sR = m_pMachine->snapshotZ80LoadWithPath(filePath);
                if (sR.success)
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Snapshot loaded successfully");
                    PMDawn::Log(PMDawn::LOG_INFO, "Loaded snapshot - " + std::string(filePath));
                    PathStripPathA(const_cast<char*>(filePath.c_str()));
                    loadedFile = ".Z80: " + filePath;
                }
                else
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Snapshot loading failed : " + sR.responseMsg);
                    loadedFile = "-empty-";
                }
            }
            else if (_stricmp(extension.c_str(), EXT_SNA.c_str()) == 0)
            {
                PMDawn::Log(PMDawn::LOG_DEBUG, "Loading SNA Snapshot - " + s);
                Tape::FileResponse sR = m_pMachine->snapshotSNALoadWithPath(filePath);
                if (sR.success)
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Snapshot loaded successfully");
                    PMDawn::Log(PMDawn::LOG_INFO, "Loaded snapshot - " + std::string(filePath));
                    PathStripPathA(const_cast<char*>(filePath.c_str()));
                    loadedFile = ".SNA: " + filePath;
                }
                else
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Snapshot loading failed : " + sR.responseMsg);
                    loadedFile = "-empty";
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------------------

static void tapeStatusCallback(int blockIndex, int bytes, int action)
{
    if (blockIndex < 1 && m_pTape->playing == false) return;
    PostMessage(tvHwnd, WM_USER + 2, PM_TAPE_ACTIVEBLOCK, (LPARAM)blockIndex);
    //SendTapeBlockDataToViewer();
    //TapeBlock* currentTBI = m_pTape->blocks[blockIndex];
    //PMDawn::Log(PMDawn::LOG_DEBUG, "Tape block       : " + std::to_string(blockIndex));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Block name     : " + currentTBI->getBlockName());
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Checksum       : " + std::to_string(currentTBI->getChecksum()));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Data length    : " + std::to_string(currentTBI->getDataLength()));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Data Type      : " + std::to_string(currentTBI->getDataType()));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Autostart line : " + std::to_string(currentTBI->getAutoStartLine()));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Filename       : " + currentTBI->getFilename());
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Start address  : " + std::to_string(currentTBI->getStartAddress()));
    //PMDawn::Log(PMDawn::LOG_DEBUG, "  Flag           : " + std::to_string(currentTBI->getFlag()));
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
    // Check for logging type if needed, CTRL = LOG_INFO, ALT = LOG_DEBUG
    PMDawn::logLevel = PMDawn::LOG_NONE;
    if (GetAsyncKeyState(VK_MENU))
    {
        // ALT is pressed
        PMDawn::logLevel = PMDawn::LOG_DEBUG;
    }
    else if (GetAsyncKeyState(VK_CONTROL))
    {
        // CTRL is pressed
        PMDawn::logLevel = PMDawn::LOG_INFO;
    }
    if (PMDawn::logLevel != PMDawn::LOG_NONE)
    {
        if (PMDawn::LogOpenOrCreate(PMDawn::GetApplicationBasePath() + "\\" + PMDawn::logFilename))
        {
            PMDawn::Log(PMDawn::LOG_INFO, "Log created");
        }
    }

    // check if under VS/Debugger and set up ROM paths accordingly
    if (IsDebuggerPresent() != 0)
    {
        PMDawn::Log(PMDawn::LOG_INFO, "Running under debugger");
        romPath = "\\ROMS\\";
    }
    else
    {
        PMDawn::Log(PMDawn::LOG_INFO, "Running standalone");
        romPath = "\\ROMS\\";
    }
    unsigned int cThreads = std::thread::hardware_concurrency();
    PMDawn::Log(PMDawn::LOG_INFO, "Maximum available threads = " + std::to_string(cThreads));

    SetupThreadLocalStorageForTapeData();

    loadedFile = "-empty-";
    slideshowTimerRunning = false;
    slideshowRandom = true;
    //srand((unsigned int)time(NULL));
    viewportX = 0;
    viewportY = 0;

    bool exit_emulator = false;
    LARGE_INTEGER  perf_freq, time, last_time;
    MSG	msg;

    OutputDebugString(TEXT("SpectREM startup\r\n"));
    std::string bpath = PMDawn::GetApplicationBasePath();
    PMDawn::Log(PMDawn::LOG_INFO, "Application base path is " + bpath);
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
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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
    RECT wr = { 0, 0, 256 * zoomLevel, 192 * zoomLevel };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    Log(PMDawn::LOG_INFO, "Current zoom level is " + std::to_string(zoomLevel));


    mainWindow = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM"), TEXT("SpectREM"),
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, 0, 0, inst, 0);
#ifdef WIN32API_GUI
    //statusWindow = CreateStatusWindow(WS_CHILD | WS_VISIBLE, TEXT("Welcome to SpectREM for Windows"), mainWindow, 9000);
    statusWindow = CreateWindow(STATUSCLASSNAME, TEXT("Welcome to SpectREM for Windows"), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, mainWindow, NULL, inst, NULL);
#endif
    ShowWindow(mainWindow, ncmd);
    UpdateWindow(mainWindow);
    mainMenu = GetMenu(mainWindow);
    menuDisplayed = true;
    statusDisplayed = true;
    //ShowHideUI(mainWindow);
#ifdef WIN32API_GUI
    RECT newSize = GetWindowResizeWithUI(mainWindow, statusWindow, mainMenu, true);
    SetWindowPos(mainWindow, HWND_NOTOPMOST, newSize.left, newSize.top, newSize.right, newSize.bottom, 0);
#endif

    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&last_time);

    m_pOpenGLView = new OpenGLView();
    m_pOpenGLView->Init(mainWindow, 256 * zoomLevel, 192 * zoomLevel, ID_SHADER_CLUT_VERT, ID_SHADER_CLUT_FRAG, ID_SHADER_DISPLAY_VERT, ID_SHADER_DISPLAY_FRAG, RT_RCDATA);
    m_pAudioQueue = new AudioQueue();
    m_pAudioCore = new AudioCore();
    m_pAudioCore->Init(44100, 50, audio_callback);
    m_pTape = new Tape(tapeStatusCallback);
    m_pMachine = new ZXSpectrum128(m_pTape);
    m_pMachine->emuUseAYSound = true;
    m_pMachine->emuBasePath = PMDawn::GetApplicationBasePath();
    PMDawn::Log(PMDawn::LOG_INFO, "ROMs path = " + m_pMachine->emuBasePath + romPath);
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
                if (PMDawn::logLevel != PMDawn::LOG_NONE)
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Log closed");
                    PMDawn::LogClose();
                }
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
            if (delta_time > 1.0f / 50.0f || GetAsyncKeyState(VK_F2)) // TURBO !! :D
            {
                last_time = time;

                m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer, viewportX, viewportY);


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
                char zoom[10];
                switch (zoomLevel)
                {
                case 1:
                    sprintf_s(zoom, sizeof(zoom), "[100%%]");
                    break;
                case 2:
                    sprintf_s(zoom, sizeof(zoom), "[200%%]");
                    break;
                case 3:
                    sprintf_s(zoom, sizeof(zoom), "[300%%]");
                    break;
                case 4:
                    sprintf_s(zoom, sizeof(zoom), "[400%%]");
                    break;
                default:
                    break;
                }
                char lLevel[30];
                if (PMDawn::logLevel == PMDawn::LOG_DEBUG)
                {
                    sprintf_s(lLevel, sizeof(lLevel), "[logging = DEBUG, INFO]");
                }
                else if (PMDawn::logLevel == PMDawn::LOG_INFO)
                {
                    sprintf_s(lLevel, sizeof(lLevel), "[logging = INFO]");
                }
                else
                {
                    sprintf_s(lLevel, sizeof(lLevel), " ");
                }

                char lfBuff[300];
                if (m_pTape->playing)
                {
                    sprintf_s(lfBuff, sizeof(lfBuff), "%s > Playing", loadedFile.c_str());
                }
                else
                {
                    if (m_pTape->loaded)
                    {
                        sprintf_s(lfBuff, sizeof(lfBuff), "%s > Paused", loadedFile.c_str());
                    }
                    else
                    {
                        sprintf_s(lfBuff, sizeof(lfBuff), "%s", loadedFile.c_str());
                    }
                }


                char buff[512];
                sprintf_s(buff, sizeof(buff), "SpectREM - %4.1f fps - [%s] - %s - [%s] %s", 1.0f / delta_time, specType, zoom, lfBuff, lLevel);
                SetWindowTextA(mainWindow, buff);
            }
        }
    }
    // if tape viewer is running on it's thread then wait before closing
    if (tapeViewerThread != nullptr)
    {
        WaitForSingleObject(tapeViewerThread, INFINITE);
        CloseHandle(tapeViewerThread);
    }
    return 0;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

static void ResetMachineForSnapshot(uint8_t mc, bool ayEnabled)
{
    isResetting = true;

    // check if slideshow is running
    if (slideshowTimerRunning)
    {
        KillTimer(mainWindow, IDT_SLIDESHOW);
        slideshowTimerRunning = false;
    }

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
        PMDawn::Log(PMDawn::LOG_INFO, "SpectREM changed to 48K Mode");
        m_pMachine = new ZXSpectrum48(m_pTape);
        m_pMachine->emuUseAYSound = ayEnabled;
        break;
    case ZX128:
        PMDawn::Log(PMDawn::LOG_INFO, "SpectREM changed to 128K Mode");
        m_pMachine = new ZXSpectrum128(m_pTape);
        m_pMachine->emuUseAYSound = true;
        break;
    default:
        // default to 128K
        PMDawn::Log(PMDawn::LOG_INFO, "UNKNOWN MACHINE TYPE, Defaulting to 128K Mode");
        m_pMachine = new ZXSpectrum128(m_pTape);
        m_pMachine->emuUseAYSound = true;
        break;
    }

    m_pMachine->emuBasePath = PMDawn::GetApplicationBasePath();
    m_pMachine->initialise(romPath);
    m_pAudioCore->Start();
    m_pMachine->resume();

    isResetting = false;
}

//-----------------------------------------------------------------------------------------

static void SetOutputVolume(float vol)
{
    // check volume range first
    if (vol < 0.0f || vol > 1.0f) { return; }
    // now set the volume
    m_pAudioCore->SetOutputVolume(vol);
}

//-----------------------------------------------------------------------------------------

static void IncreaseApplicationVolume()
{
    if (applicationVolume + volumeStep > 1.0f)
    {
        SetOutputVolume(1.0f);
        applicationVolume = 1.0f;
    }
    else
    {
        SetOutputVolume(applicationVolume + volumeStep);
        applicationVolume = applicationVolume + volumeStep;
    }
}

//-----------------------------------------------------------------------------------------

static void DecreaseApplicationVolume()
{
    if (applicationVolume - volumeStep < 0.0f)
    {
        SetOutputVolume(0.0f);
        applicationVolume = 0.0f;
    }
    else
    {
        SetOutputVolume(applicationVolume - volumeStep);
        applicationVolume = applicationVolume - volumeStep;
    }
}

//-----------------------------------------------------------------------------------------

static void OpenTapeViewer()
{
    if (tapeViewerThread) return;

    tapeViewerThread = (HANDLE)_beginthreadex(0, 0, &mythread, 0, 0, 0);
}

unsigned int __stdcall mythread(void* data)
{

    tvWindow = new TapeViewer(GetModuleHandle(NULL), mainWindow, dwTlsIndex);
    tvWindow = nullptr;
    PostMessage(mainWindow, WM_USER + 2, PM_TAPE_VIEWER_CLOSED, (LPARAM)0);
    return 0;
}
//-----------------------------------------------------------------------------------------
static void SendTapeBlockDataToViewer()
{
    size_t numBlocks = m_pTape->numberOfTapeBlocks();

    PMDawn::pData.clear();
    for (int i = 0; i < numBlocks; i++)
    {
        PMDawn::gTAPEBLOCK gT;

        gT.status = " ";
        switch (m_pTape->blocks[i]->getDataType())
        {
        case 0: // ePROGRAM_HEADER
            gT.blocktype = "PROGRAM:";
            break;

        case 1: // eNUMERIC_DATA_HEADER
            gT.blocktype = "DATA():";
            break;

        case 2: // eALPHANUMERIC_DATA_HEADER
            gT.blocktype = "STRING():";
            break;

        case 3: // eBYTE_HEADER
            gT.blocktype = "CODE:";
            break;

        case 4: // eDATA_BLOCK
            gT.blocktype = "DATA:";
            break;

        case 5: // eFRAGMENTED_DATA_BLOCK
            gT.blocktype = "FRAGMENTED:";
            break;

        case 99: // eUNKNOWN_BLOCK
            gT.blocktype = "UNKNOWN:";
            break;

        default: // Uh oh...
            gT.blocktype = "  DATA:";
            break;
        }

        if (m_pTape->blocks[i]->getDataType() < 4)
        {
            gT.filename = m_pTape->blocks[i]->getFilename();
            gT.autostartline = m_pTape->blocks[i]->getAutoStartLine();
        }
        else
        {
            gT.filename = "";
            gT.autostartline = 0;
        }
        gT.address = m_pTape->blocks[i]->getStartAddress();
        gT.length = m_pTape->blocks[i]->getDataLength();

        PMDawn::pData.push_back(gT);
    }

    size_t num = PMDawn::pData.size();

    if (tvHwnd != nullptr)
    {
        PostMessage(tvHwnd, WM_USER + 2, PM_TAPEDATA_FULL, (LPARAM)&PMDawn::pData);
    }
}

//-----------------------------------------------------------------------------------------

static void GetTapeViewerHwnd()
{
    tvHwnd = TapeViewer::tapeViewerWindowInternal;
}

//-----------------------------------------------------------------------------------------

static void SetupThreadLocalStorageForTapeData()
{
    return;
    //// Setup the thread local storage
    //dwTlsIndex = TlsAlloc();
    //TlsSetValue(dwTlsIndex, GlobalAlloc(GPTR, sizeof(PMDawn::THREADDATA)));
    //PMDawn::pData = (PMDawn::PTHREADDATA)TlsGetValue(dwTlsIndex);
    //PMDawn::pData->filename = "POLO2";
}

//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

