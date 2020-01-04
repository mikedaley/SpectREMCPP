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



// LogType:
//    LOG_INFO is for general info
//    LOG_DEBUG is for debugging info, note that it will also include INFO
enum LogType
{
    LOG_NONE, LOG_INFO, LOG_DEBUG, LOG_FULL
};

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
static bool LogOpenOrCreate(std::string filename);
static bool Log(LogType lType, std::string text);
static bool LogClose();
static void ShowSettingsDialog();
static void RunSlideshow(int secs);
static std::string GetTimeAsString();
static std::string GetApplicationBasePath();
static std::string GetCurrentDirectoryAsString();
static std::vector<std::string> GetFilesInDirectory(std::string folder, std::string filter);
void IterateSCRImages(HWND mWindow, std::vector<std::string> fileList, ZXSpectrum* m_pMachine, int secs);
static void IterateSCRImagesOnTimerCallback();
static void OpenSCR();
static void InsertTape();
static void EjectTape();
static void PlayPauseTape();

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


const UINT PM_UPDATESPECTREM = 7777;
const std::string EXT_Z80 = "z80";
const std::string EXT_SNA = "sna";
const UINT_PTR IDT_SLIDESHOW = 7778;
std::string romPath;
HACCEL hAcc;
bool isResetting = false;
HWND mainWindow;
HMENU mainMenu;
bool TurboMode = false;
bool menuDisplayed = true;
uint8_t zoomLevel = 3;
uint8_t logLevel = LOG_NONE;
const std::string logFilename = "spectrem_win32.log";
std::string logFullFilename = "";
std::ofstream logFileStream;
std::string slideshowDirectory = "\\slideshow\\";
std::vector<std::string> fileList;
uint8_t fileListIndex = 0;
std::thread scrDisplayThread;
bool slideshowTimerRunning = false;
bool slideshowRandom = true;

std::unordered_map<WPARAM, ZXSpectrum::ZXSpectrumKey> KeyMappings
{
    { VK_UP, ZXSpectrum::ZXSpectrumKey::Key_ArrowUp },
    { VK_DOWN, ZXSpectrum::ZXSpectrumKey::Key_ArrowDown },
    { VK_LEFT, ZXSpectrum::ZXSpectrumKey::Key_ArrowLeft },
    { VK_RIGHT, ZXSpectrum::ZXSpectrumKey::Key_ArrowRight },
    { VK_RETURN, ZXSpectrum::ZXSpectrumKey::Key_Enter },
    { VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Shift },
    { VK_RSHIFT, ZXSpectrum::ZXSpectrumKey::Key_Shift },
    { VK_SPACE, ZXSpectrum::ZXSpectrumKey::Key_Space },
    { VK_CONTROL, ZXSpectrum::ZXSpectrumKey::Key_SymbolShift },
    { VK_RCONTROL, ZXSpectrum::ZXSpectrumKey::Key_SymbolShift },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_InvVideo },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_TrueVideo },
    { VK_DELETE, ZXSpectrum::ZXSpectrumKey::Key_Backspace },
    { VK_BACK, ZXSpectrum::ZXSpectrumKey::Key_Backspace },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Quote },
    { VK_OEM_1, ZXSpectrum::ZXSpectrumKey::Key_SemiColon },
    { VK_OEM_COMMA, ZXSpectrum::ZXSpectrumKey::Key_Comma },
    { VK_OEM_MINUS, ZXSpectrum::ZXSpectrumKey::Key_Minus },
    { VK_OEM_PLUS, ZXSpectrum::ZXSpectrumKey::Key_Plus },
    { VK_OEM_PERIOD, ZXSpectrum::ZXSpectrumKey::Key_Period },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Edit },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Graph },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_Break },
    //{ VK_SHIFT, ZXSpectrum::ZXSpectrumKey::Key_ExtendMode },
    { VK_CAPITAL, ZXSpectrum::ZXSpectrumKey::Key_CapsLock },
    // Numbers
    { 0x30, ZXSpectrum::ZXSpectrumKey::Key_0 },
    { 0x31, ZXSpectrum::ZXSpectrumKey::Key_1 },
    { 0x32, ZXSpectrum::ZXSpectrumKey::Key_2 },
    { 0x33, ZXSpectrum::ZXSpectrumKey::Key_3 },
    { 0x34, ZXSpectrum::ZXSpectrumKey::Key_4 },
    { 0x35, ZXSpectrum::ZXSpectrumKey::Key_5 },
    { 0x36, ZXSpectrum::ZXSpectrumKey::Key_6 },
    { 0x37, ZXSpectrum::ZXSpectrumKey::Key_7 },
    { 0x38, ZXSpectrum::ZXSpectrumKey::Key_8 },
    { 0x39, ZXSpectrum::ZXSpectrumKey::Key_9 },
    // Letters
    { 0x41, ZXSpectrum::ZXSpectrumKey::Key_A },
    { 0x42, ZXSpectrum::ZXSpectrumKey::Key_B },
    { 0x43, ZXSpectrum::ZXSpectrumKey::Key_C },
    { 0x44, ZXSpectrum::ZXSpectrumKey::Key_D },
    { 0x45, ZXSpectrum::ZXSpectrumKey::Key_E },
    { 0x46, ZXSpectrum::ZXSpectrumKey::Key_F },
    { 0x47, ZXSpectrum::ZXSpectrumKey::Key_G },
    { 0x48, ZXSpectrum::ZXSpectrumKey::Key_H },
    { 0x49, ZXSpectrum::ZXSpectrumKey::Key_I },
    { 0x4a, ZXSpectrum::ZXSpectrumKey::Key_J },
    { 0x4b, ZXSpectrum::ZXSpectrumKey::Key_K },
    { 0x4c, ZXSpectrum::ZXSpectrumKey::Key_L },
    { 0x4d, ZXSpectrum::ZXSpectrumKey::Key_M },
    { 0x4e, ZXSpectrum::ZXSpectrumKey::Key_N },
    { 0x4f, ZXSpectrum::ZXSpectrumKey::Key_O },
    { 0x50, ZXSpectrum::ZXSpectrumKey::Key_P },
    { 0x51, ZXSpectrum::ZXSpectrumKey::Key_Q },
    { 0x52, ZXSpectrum::ZXSpectrumKey::Key_R },
    { 0x53, ZXSpectrum::ZXSpectrumKey::Key_S },
    { 0x54, ZXSpectrum::ZXSpectrumKey::Key_T },
    { 0x55, ZXSpectrum::ZXSpectrumKey::Key_U },
    { 0x56, ZXSpectrum::ZXSpectrumKey::Key_V },
    { 0x57, ZXSpectrum::ZXSpectrumKey::Key_W },
    { 0x58, ZXSpectrum::ZXSpectrumKey::Key_X },
    { 0x59, ZXSpectrum::ZXSpectrumKey::Key_Y },
    { 0x5a, ZXSpectrum::ZXSpectrumKey::Key_Z },

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
    Log(LOG_INFO, "Zoom level changed to " + std::to_string(zoomLevel));
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
        glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
        break;

    case WM_USER:
        switch (LOWORD(wparam))
        {
        case PM_UPDATESPECTREM:
            Sleep(50);
            m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);
            Log(LOG_DEBUG, "Changed slideshow image");
            break;
        }
        break;

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
    ofn.lpstrFilter = "All\0*.*\0Tape\0*.TAP\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        EjectTape(); // Eject the current tape if inserted
        Tape::TapResponse tR = m_pTape->loadWithPath(szFile);
        if (tR.success)
        {
            Log(LOG_INFO, "Loaded tape - " + std::string(szFile));
        }
        else
        {
            MessageBox(mainWindow, TEXT("Unable to load tape >> "), TEXT("Tape Loader"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            Log(LOG_INFO, "Failed to load tape - " + std::string(szFile) + " > " + tR.responseMsg);
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
            m_pTape->stopPlaying();
        }
        else
        {
            m_pTape->startPlaying();
        }
    }
}

//-----------------------------------------------------------------------------------------

static void EjectTape()
{
    if (m_pTape->loaded)
    {
        m_pTape->stopPlaying();
        m_pTape->eject();
    }
}

//-----------------------------------------------------------------------------------------

static void OpenSCR()
{
    HardReset();
    Sleep(1000);

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
        ZXSpectrum::Response sR = m_pMachine->scrLoadWithPath(szFile);
        if (sR.success)
        {
            Sleep(1);
            m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);
            Log(LOG_INFO, "Loaded .scr file - " + std::string(szFile));
        }
        else
        {
            MessageBox(mainWindow, TEXT("Invalid SCR file"), TEXT("Gimme SCR's !!!"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            Log(LOG_INFO, "Failed to load .scr file - " + std::string(szFile) + " > " + sR.responseMsg);
            return;
        }
    }
    


}

//-----------------------------------------------------------------------------------------

static void RunSlideshow(int secs)
{
    HardReset();
    Sleep(1000);
    Log(LOG_INFO, "Running slideshow (" + std::to_string(secs) + " secs) from " + GetApplicationBasePath() + slideshowDirectory);
    fileList.clear();
    fileList = GetFilesInDirectory(GetApplicationBasePath() + slideshowDirectory, "*.scr");
    Log(LOG_DEBUG, "Found " + std::to_string(fileList.size()) + " matching files");
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
        ZXSpectrum::Response sR = m_pMachine->scrLoadWithPath(GetApplicationBasePath() + slideshowDirectory + fileList[randomIndex]);
        Sleep(1);
        m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);
    }
    else
    {
        ZXSpectrum::Response sR = m_pMachine->scrLoadWithPath(GetApplicationBasePath() + slideshowDirectory + fileList[fileListIndex]);
        Sleep(1);
        m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);
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
            ZXSpectrum::Response sR = machine->scrLoadWithPath(GetApplicationBasePath() + slideshowDirectory + fileList[i]);
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
    Log(LOG_DEBUG, "ShowSettingsDialog()");
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
    Log(LOG_DEBUG, "ShowUI()");
    SetMenu(hWnd, mainMenu);
    menuDisplayed = true;
}

//-----------------------------------------------------------------------------------------

static void HideUI(HWND hWnd = mainWindow)
{
    Log(LOG_DEBUG, "HideUI()");
    SetMenu(hWnd, NULL);
    menuDisplayed = false;
}

//-----------------------------------------------------------------------------------------

static void ShowHelpAbout()
{
    Log(LOG_DEBUG, "ShowHelpAbout()");
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
            Log(LOG_DEBUG, "Flip machine requested");
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

//-----------------------------------------------------------------------------------------

static void SoftReset()
{
    // Soft reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType);
        Log(LOG_INFO, "Soft reset completed");
    }
}

//-----------------------------------------------------------------------------------------

static void HardReset()
{
    // Hard reset
    if (isResetting != true)
    {
        ResetMachineForSnapshot(m_pMachine->machineInfo.machineType);
        Log(LOG_INFO, "Hard reset completed");
    }
}

//-----------------------------------------------------------------------------------------

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
            Log(LOG_INFO, "Loading Z80 Snapshot - " + s);
            ZXSpectrum::Response sR = m_pMachine->snapshotZ80LoadWithPath(szFile);
            if (sR.success)
            {
                Log(LOG_INFO, "Snapshot loaded successfully");
            }
            else
            {
                Log(LOG_INFO, "Snapshot loading failed : " + sR.responseMsg);
            }
        }
        else if (_stricmp(extension.c_str(), EXT_SNA.c_str()) == 0)
        {
            Log(LOG_DEBUG, "Loading SNA Snapshot - " + s);
            ZXSpectrum::Response sR = m_pMachine->snapshotSNALoadWithPath(szFile);
            if (sR.success)
            {
                Log(LOG_INFO, "Snapshot loaded successfully");
            }
            else
            {
                Log(LOG_INFO, "Snapshot loading failed : " + sR.responseMsg);
            }
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
    // Check for logging type if needed, CTRL = LOG_INFO, ALT = LOG_DEBUG
    logLevel = LOG_NONE;
    if (GetAsyncKeyState(VK_MENU))
    {
        // ALT is pressed
        logLevel = LOG_DEBUG;
    }
    else if (GetAsyncKeyState(VK_CONTROL))
    {
        // CTRL is pressed
        logLevel = LOG_INFO;
    }
    if (logLevel != LOG_NONE)
    {
        if (LogOpenOrCreate(GetApplicationBasePath() + "\\" + logFilename))
        {
            Log(LOG_INFO, "Log created");
        }
    }

    // check if under VS/Debugger and set up ROM paths accordingly
    if (IsDebuggerPresent() != 0)
    {
        Log(LOG_INFO, "Running under debugger");
        romPath = "\\ROMS\\";
    }
    else
    {
        Log(LOG_INFO, "Running standalone");
        romPath = "\\ROMS\\";
    }
    unsigned int cThreads = std::thread::hardware_concurrency();
    Log(LOG_INFO, "Maximum available threads = " + std::to_string(cThreads));

    slideshowTimerRunning = false;
    slideshowRandom = true;
    //srand((unsigned int)time(NULL));

    bool exit_emulator = false;
    LARGE_INTEGER  perf_freq, time, last_time;
    MSG	msg;

    OutputDebugString(TEXT("SpectREM startup\r\n"));
    std::string bpath = GetApplicationBasePath();
    Log(LOG_INFO, "Application base path is " + bpath);
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
    RECT wr = { 0, 0, 256 * zoomLevel, 192 * zoomLevel };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    Log(LOG_INFO, "Current zoom level is " + std::to_string(zoomLevel));


    mainWindow = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM"), TEXT("SpectREM"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 0, 0, wr.right - wr.left, wr.bottom - wr.top, 0, 0, inst, 0);
    ShowWindow(mainWindow, ncmd);
    UpdateWindow(mainWindow);
    mainMenu = GetMenu(mainWindow);
    menuDisplayed = true;

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
    m_pMachine->emuBasePath = GetApplicationBasePath();
    Log(LOG_INFO, "ROMs path = " + m_pMachine->emuBasePath + romPath);
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
                if (logLevel != LOG_NONE)
                {
                    Log(LOG_INFO, "Log closed");
                    LogClose();
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

                m_pOpenGLView->UpdateTextureData(m_pMachine->displayBuffer);

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
                if (logLevel == LOG_DEBUG)
                {
                    sprintf_s(lLevel, sizeof(lLevel), "[logging = DEBUG, INFO]");
                }
                else if (logLevel == LOG_INFO)
                {
                    sprintf_s(lLevel, sizeof(lLevel), "[logging = INFO]");
                }
                else
                {
                    sprintf_s(lLevel, sizeof(lLevel), " ");
                }

                char buff[100];
                sprintf_s(buff, sizeof(buff), "SpectREM - %4.1f fps - [%s] - %s %s", 1.0f / delta_time, specType, zoom, lLevel);
                SetWindowTextA(mainWindow, buff);
            }
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

static void ResetMachineForSnapshot(uint8_t mc)
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
        Log(LOG_INFO, "SpectREM changed to 48K Mode");
        m_pMachine = new ZXSpectrum48(m_pTape);
        m_pMachine->emuUseAYSound = false;
        break;
    case ZX128:
        Log(LOG_INFO, "SpectREM changed to 128K Mode");
        m_pMachine = new ZXSpectrum128(m_pTape);
        m_pMachine->emuUseAYSound = true;
        break;
    default:
        // default to 128K
        Log(LOG_INFO, "UNKNOWN MACHINE TYPE, Defaulting to 128K Mode");
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

//-----------------------------------------------------------------------------------------

static std::string GetCurrentDirectoryAsString()
{
    char basePT[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, basePT);
    OutputDebugString(TEXT("Start path = "));
    OutputDebugStringA(basePT);
    OutputDebugString(TEXT("\r\n"));
    return basePT;
}

//-----------------------------------------------------------------------------------------

static std::string GetApplicationBasePath()
{
    char appDirT[MAX_PATH];
    GetModuleFileNameA(NULL, appDirT, MAX_PATH);
    PathRemoveFileSpecA(appDirT);// appDirT);
    return appDirT;
}

//-----------------------------------------------------------------------------------------

static bool LogOpenOrCreate(std::string filename)
{
    logFileStream.open(filename, std::ios::ate | std::ios::app);
    if (logFileStream.is_open())
    {
        logFullFilename = filename;
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------------------

static bool Log(LogType lType, std::string text)
{
    // we will use the file and always append to it
    if (logFileStream.is_open())
    {
        std::string lty = "";
        switch (lType)
        {
        case LOG_INFO:
            lty = "[INFO]   ";
            break;
        case LOG_DEBUG:
            lty = "[DEBUG]  ";
            break;
        default:
            lty = "[UNKNOWN]";
            break;
        }
        logFileStream << GetTimeAsString().c_str() << " : " << lty.c_str() << " : " << text.c_str() << std::endl;
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------------------

static bool LogClose()
{
    if (logFileStream.is_open())
    {
        logFileStream.close();
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------------------

static bool fileExists(const std::string& filename)
{
    struct stat fileBuffer;
    return (stat(filename.c_str(), &fileBuffer) == 0);
}

//-----------------------------------------------------------------------------------------

static std::string GetTimeAsString()
{
    time_t rawtime;
    struct tm timeinfo;
    char buffer[80];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    std::string str(buffer);
    return str;
}

static std::vector<std::string> GetFilesInDirectory(std::string folder, std::string filter)
{
    std::vector<std::string> fileList;
    std::string fullPath = folder + filter;
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(fullPath.c_str(), &data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do
        {
            Log(LOG_DEBUG, std::string(data.cFileName));
            fileList.push_back(data.cFileName);
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
        return fileList;
    }
    else
    {
        // Error finding files, so clear the list and return
        fileList.clear();
        return fileList;
    }
}
