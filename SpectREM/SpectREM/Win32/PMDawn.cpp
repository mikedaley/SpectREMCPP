//
//  PMDawn.cpp
//    Useful routines
//
//  Created by John Young on 05-01-2020
//  
//

#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <time.h>
#include <Shlwapi.h>
#include <shobjidl.h> 

namespace PMDawn
{
    // LogType:
    //    LOG_INFO is for general info
    //    LOG_DEBUG is for debugging info, note that it will also include INFO
    enum LogType
    {
        LOG_NONE, LOG_INFO, LOG_DEBUG, LOG_FULL
    };

    //-----------------------------------------------------------------------------------------

    static bool LogOpenOrCreate(std::string filename);
    static bool Log(LogType lType, std::string text);
    static bool LogClose();
    static std::string GetTimeAsString();
    static std::string GetApplicationBasePath();
    static std::string GetCurrentDirectoryAsString();
    static std::vector<std::string> GetFilesInDirectory(std::string folder, std::string filter);

    //-----------------------------------------------------------------------------------------

    static uint8_t logLevel = LOG_NONE;
    static const std::string logFilename = "spectrem_win32.log";
    static std::string logFullFilename = "";
    static std::ofstream logFileStream;

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

    static std::vector<std::string> GetFilesInDirectory(std::string folder, std::string filter)
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

    static std::string GetFolderUsingDialog(std::string initialFolder = "")
    {
        wchar_t *fold;
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
        } else 
        {
            return "";
        }
    }

    //-----------------------------------------------------------------------------------------



    //-----------------------------------------------------------------------------------------



    //-----------------------------------------------------------------------------------------



    //-----------------------------------------------------------------------------------------







}