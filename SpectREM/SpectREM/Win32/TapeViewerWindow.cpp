//
//  TapeViewerWindow.cpp
//    This window holds the tape viewer which allows the user to interact with the tape blocks etc.
//
//  Created by John Young on 05-01-2020
//  
//

#include <windows.h>
#include <winuser.h>
#include "../../resource.h"
#include "PMDawn.cpp"
#include "TapeViewerWindow.hpp"

//-----------------------------------------------------------------------------------------

LRESULT CALLBACK TapeViewer::WndProcTV(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//-----------------------------------------------------------------------------------------


TapeViewer::TapeViewer(HINSTANCE mainWindowInst, HWND mainHandle)
{
    bool exit_tapeviewer = false;
    MSG	msg;
    WNDCLASSEX wcextv;

    memset(&wcextv, 0, sizeof(WNDCLASSEX));
    wcextv.cbSize = sizeof(WNDCLASSEX);
    wcextv.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcextv.lpfnWndProc = WndProcTV;
    wcextv.cbClsExtra = 0;
    wcextv.cbWndExtra = 0;
    wcextv.hInstance = mainWindowInst;
    wcextv.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcextv.hbrBackground = NULL;
    wcextv.lpszClassName = TEXT("SpectREM_TapeViewer");
    wcextv.hIcon = LoadIcon(mainWindowInst, MAKEINTRESOURCE(IDI_ICON2));
    wcextv.hIconSm = (HICON)LoadImage(mainWindowInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
    RegisterClassEx(&wcextv);

    // Make sure the client size is correct
    RECT wr = { 0, 0, 640, 480 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    tapeViewerWindowInternal = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM_TapeViewer"), TEXT("SpectREM - Tape Viewer"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 0, 0, wr.right - wr.left, wr.bottom - wr.top, 0, 0, mainWindowInst, 0);
    if (tapeViewerWindowInternal == NULL)
    {
        MessageBoxA(NULL, "Tape window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return;
    }
    ShowWindow(tapeViewerWindowInternal, SW_SHOWNORMAL);
    UpdateWindow(tapeViewerWindowInternal);

    while (!exit_tapeviewer)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit_tapeviewer = true;
                if (PMDawn::logLevel != PMDawn::LOG_NONE)
                {
                    PMDawn::Log(PMDawn::LOG_INFO, "Log closed");
                }
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            //{
                //if (!TranslateAccelerator(msg.hwnd, hAcc, &msg))
                //{
                //    TranslateMessage(&msg);
                //    DispatchMessage(&msg);
                //}
            //}
        }
        else
        {
        }
    }
    return;
}





