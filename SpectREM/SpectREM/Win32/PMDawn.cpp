//
//  PMDawn.cpp
//    Useful routines
//
//  Created by John Young on 05-01-2020
//  
//

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <time.h>
#include <Shlwapi.h>
#include <shobjidl.h> 
#include <winuser.h>
#include "PMDawn.hpp"

namespace PMDawn
{
    //-----------------------------------------------------------------------------------------
    
    //-----------------------------------------------------------------------------------------

    bool PMDawn::fileExists(const std::string& filename)
    {
        struct stat fileBuffer;
        return (stat(filename.c_str(), &fileBuffer) == 0);
    }

    //-----------------------------------------------------------------------------------------

    std::string PMDawn::GetTimeAsString()
    {
        time_t rawtime;
        struct tm timeinfo;
        char buffer[80];
        time(&rawtime);
        localtime_s(&timeinfo, &rawtime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        std::string str(buffer);
        std::string str(buffer);

        std::string str(buffer);

        return str;
    }

    //-----------------------------------------------------------------------------------------

    std::string PMDawn::GetCurrentDirectoryAsString()
    {
        char basePT[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, basePT);
        OutputDebugString(TEXT("Start path = "));
        OutputDebugStringA(basePT);
        OutputDebugString(TEXT("\r\n"));
        return basePT;
    }

    //-----------------------------------------------------------------------------------------

    std::string PMDawn::GetApplicationBasePath()
    {
        char appDirT[MAX_PATH];
        GetModuleFileNameA(NULL, appDirT, MAX_PATH);
        PathRemoveFileSpecA(appDirT);// appDirT);
        return appDirT;
    }

    //-----------------------------------------------------------------------------------------

    bool PMDawn::LogOpenOrCreate(std::string filename)
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

    bool PMDawn::Log(LogType lType, std::string text)
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
            logFileStream << GetTimeAsString().c_str() << " : " << lty.c_str() << " : " << text.c_str() << "\n";
            return true;
        }
        else
        {
            return false;
        }
    }

    //-----------------------------------------------------------------------------------------

    bool PMDawn::LogClose()
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

    std::vector<std::string> PMDawn::GetFilesInDirectory(std::string folder, std::string filter)
    {
        std::vector<std::string> fileList;
        std::string fullPath = folder + filter;
        WIN32_FIND_DATAA data;
        HANDLE hFind = FindFirstFileA(fullPath.c_str(), &data);

        if (hFind != INVALID_HANDLE_VALUE) {
            do
            {
                PMDawn::Log(PMDawn::LOG_DEBUG, std::string(data.cFileName));
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

    //-----------------------------------------------------------------------------------------

    std::string GetFolderUsingDialog(std::string initialFolder = "")
    {
        wchar_t* fold;
        std::string fl;
        bool error = true;

        IFileDialog* pfd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_PICKFOLDERS); // FOS_FORCEFILESYSTEM
            }
            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &fold)))
                    {
                        MessageBoxA(NULL, "Failed to get folder path", "Error", NULL);

                    }
                    else
                    {
                        // If we get here then we should have a path :)
                        error = false;
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }
        if (!error)
        {
            std::wstring fstr(fold);
            std::string fs(fstr.begin(), fstr.end());

            return fs;
        }
        else
        {
            return "";
        }
    }

    //-----------------------------------------------------------------------------------------

    std::string PMDawn::GetFilenameUsingDialog(std::string initialFolder)
    {
        wchar_t* fold;
        std::string fl;
        bool error = true;

        IFileDialog* pfd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions); // FOS_FORCEFILESYSTEM
            }
            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    if (!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &fold)))
                    {
                        MessageBoxA(NULL, "Failed to get file path", "Error", NULL);

                    }
                    else
                    {
                        // If we get here then we should have a path :)
                        error = false;
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }
        if (!error)
        {
            std::wstring fstr(fold);
            std::string fs(fstr.begin(), fstr.end());

            return fs;
        }
        else
        {
            return "";
        }
    }

    //-----------------------------------------------------------------------------------------

    //HWND CreateButton(HWND owner, std::string buttonText, int x, int y, int w, int h)
    //{
    //    //LPCWSTR lpS = stdStringToLpwstr(buttonText);
    //    HWND btn = CreateWindow(
    //        L"BUTTON",                             // Class
    //        L"Ok",//buttonText.c_str(),                   // Button text 
    //        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,   // Styles 
    //        x,          // x position 
    //        y,          // y position 
    //        w,          // Button width
    //        h,          // Button height
    //        owner,      // Parent window
    //        NULL,       // No menu.
    //        NULL,       // Instance
    //        NULL);      // Pointer not needed.
    //    return btn;
    //}

    ////-----------------------------------------------------------------------------------------

    LPCWSTR stdStringToLpcwstr(std::string input)
    {
        std::wstring stemp = std::wstring(input.begin(), input.end());
        LPCWSTR sw = stemp.c_str();
        return sw;
    }


    //-----------------------------------------------------------------------------------------

    std::wstring PMDawn::s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }

    void AddItemToListView(gTAPEBLOCK& theBlock, HWND hwndListView)
    {
        // Status / BlockType / Filename / AutostartLine / Address / Length
        // TAPE block types
        /*enum
        {
            ePROGRAM_HEADER = 0,
            eNUMERIC_DATA_HEADER,
            eALPHANUMERIC_DATA_HEADER,
            eBYTE_HEADER,
            eDATA_BLOCK,
            eFRAGMENTED_DATA_BLOCK,
            eUNKNOWN_BLOCK = 99
        };*/

        if (!ShowAlternativeTapeLengths)
        {
            if (theBlock.length < 2)
            {
                theBlock.length = 0;
            }
            else
            {
                theBlock.length -= 2;
            }
        }

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_COLFMT;
        lvi.iItem = 0;
        lvi.iSubItem = 0;

        std::wstring ws;
        ws.assign(theBlock.status.begin(), theBlock.status.end());
        lvi.pszText = &ws[0];
        ListView_InsertItem(hwndListView, &lvi);

        std::wstring bt;
        bt.assign(theBlock.blocktype.begin(), theBlock.blocktype.end());
        ListView_SetItemText(hwndListView, 0, 1, &bt[0]);

        std::wstring fn;
        fn.assign(theBlock.filename.begin(), theBlock.filename.end());
        ListView_SetItemText(hwndListView, 0, 2, &fn[0]);

        std::wstring asl;
        if (theBlock.autostartline != 0)
        {
            asl = std::to_wstring(theBlock.autostartline);
        }
        else 
        {
            asl = L"";
        }
        ListView_SetItemText(hwndListView, 0, 3, &asl[0]);

        std::wstring addy;
        if (theBlock.address != 0)
        {
            addy = std::to_wstring(theBlock.address);
        }
        else
        {
            addy = L"";
        }
        ListView_SetItemText(hwndListView, 0, 4, &addy[0]);

        std::wstring length;
        length = std::to_wstring(theBlock.length);
        ListView_SetItemText(hwndListView, 0, 5, &length[0]);
    }

}