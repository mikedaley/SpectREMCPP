//
//  TapeViewerWindow.hpp
//    This window holds the tape viewer which allows the user to interact with the tape blocks etc.
//
//  Created by John Young on 05-01-2020
//  
//

#include <windows.h>
#include <winuser.h>
#include <windef.h>

#define IDC_BUTTON_PLAY         100
#define IDC_BUTTON_PAUSE        101
#define IDC_BUTTON_REWIND       102
#define IDC_BUTTON_INSERT       103
#define IDC_BUTTON_EJECT        104
#define IDC_BUTTON_SAVE         105
#define IDC_LISTVIEW_TAPEDATA   106
#define IDS_NUMBEROFCOLUMNS     006


class TapeViewer
{
public:
    TapeViewer(HINSTANCE mainWindowInst, HWND mainHandle, DWORD dwTlsIndex);// , std::vector<PMDawn::gTAPEBLOCK>myPTAPE);
    ~TapeViewer();
    static LRESULT CALLBACK WndProcTV(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static HWND TapeViewer::CreateListView(HWND hwnd, LPARAM lParam, RECT rect);
    static BOOL TapeViewer::InitListViewColumns(HWND hWndListView, HINSTANCE hInst);
    static const uint8_t windowFrameBuffer = 8;
    static const uint8_t buttonYBuffer = 12;
    static HWND tapeViewerWindowInternal;
    static HINSTANCE g_hInst;


};