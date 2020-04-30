//
//  PMDawn.hpp
//    This window holds the tape viewer which allows the user to interact with the tape blocks etc.
//
//  Created by John Young on 05-01-2020
//  
//

#ifndef PMDawn_hpp
#define PMDawn_hpp

#include <string>
#include <vector>
#include <windows.h>
#include <windef.h>
#include <winnt.h>
#include <string>
#include <iosfwd>
#include <fstream>


namespace PMDawn
{
    struct gTAPEBLOCK {
        std::string status;
        std::string blocktype;
        std::string filename;
        uint16_t autostartline;
        uint16_t address;
        uint16_t length;
    };

    static std::vector<gTAPEBLOCK> pData;

    // LogType:
    //    LOG_INFO is for general info
    //    LOG_DEBUG is for debugging info, note that it will also include INFO
    enum LogType
    {
        LOG_NONE, LOG_INFO, LOG_DEBUG, LOG_FULL
    };

    static uint8_t logLevel = LOG_NONE;
    const std::string logFilename = "spectrem_win32.log";
    static std::string logFullFilename = "";
    static std::ofstream logFileStream;
    static bool ShowAlternativeTapeLengths = false;

    bool Log(LogType lType, std::string text);
    bool LogClose(void);
    std::string GetTimeAsString(void);
    std::string GetApplicationBasePath(void);
    std::string GetCurrentDirectoryAsString(void);
    std::vector<std::string> GetFilesInDirectory(std::string folder, std::string filter);
    //static HWND PMDawn::CreateButton(HWND owner, std::string buttonText, int x, int y, int w, int h);
    LPCWSTR stdStringToLpcwstr(std::string input);
    std::wstring s2ws(const std::string& s);
    std::string GetFilenameUsingDialog(std::string initialFolder);
    std::string GetFolderUsingDialog(std::string initialFolder);
    bool fileExists(const std::string& filename);
    bool LogOpenOrCreate(std::string filename);
    void AddItemToListView(gTAPEBLOCK& theBlock, HWND hwndListView);


}
#endif /* PMDawn_hpp */

