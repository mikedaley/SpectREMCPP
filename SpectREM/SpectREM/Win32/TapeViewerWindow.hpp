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

class TapeViewer
{
public:
    TapeViewer(HINSTANCE mainWindowInst, HWND mainHandle);
    ~TapeViewer();
    static LRESULT CALLBACK WndProcTV(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


public:
    HWND tapeViewerWindowInternal;


};