//
//  PMDawn.hpp
//    
//
//  Created by John Young on 11-01-2020
//  
//

#include <windows.h>
#include <winuser.h>
#include <windef.h>
#include "../../resource.h"
#include "PMDawn.hpp"
#include "TapeViewerWindow.hpp"
#include "..\Emulation Core\Tape\Tape.hpp"
#include <CommCtrl.h>
#include <vector>


#define PM_TAPEDATA_FULL            77
#define PM_TAPE_COMMAND             79
#define PM_TAPE_EJECTED             80
#define PM_TAPE_ACTIVEBLOCK         81
#define PM_TAPE_PLAY                82
#define PM_TAPE_PAUSE               83
#define PM_TAPE_REWIND              84
#define PM_TAPE_INSERT              85
#define PM_TAPE_EJECT               86
#define PM_TAPE_SAVE                87

// Status / BlockType / Filename / AutostartLine / Address / Length
//-----------------------------------------------------------------------------------------

HWND TapeViewer::tapeViewerWindowInternal = nullptr;
HWND mHandle;
static std::vector<PMDawn::gTAPEBLOCK>* myP;

TapeViewer::~TapeViewer()
{
    //PostMessage(mHandle, WM_USER + 2, PM_TAPE_VIEWER_CLOSING, (LPARAM)0);
}

LRESULT CALLBACK TapeViewer::WndProcTV(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndOKButton;
    static HWND hwndCancelButton;
    static HWND hwndPlayButton;
    static HWND hwndPauseButton;
    static HWND hwndRewindButton;
    static HWND hwndInsertButton;
    static HWND hwndEjectButton;
    static HWND hwndSaveButton;
    static HWND hwndListView;
    static TCHAR szTempListData[] = TEXT("Temporary listbox data");

    switch (msg)
    {
    case WM_CREATE:
        RECT rect;

        GetClientRect(hwnd, &rect);

        // Create buttons for tape control (Play/Pause/Rewind/Eject/Save)
        hwndPlayButton = CreateWindow(TEXT("button"),
            TEXT("Play"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer,
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_PLAY,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);
        hwndPauseButton = CreateWindow(TEXT("button"),
            TEXT("Pause"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer + (2 * buttonYBuffer),
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_PAUSE,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);
        hwndRewindButton = CreateWindow(TEXT("button"),
            TEXT("Rewind"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer + (4 * buttonYBuffer),
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_REWIND,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);
        hwndInsertButton = CreateWindow(TEXT("button"),
            TEXT("Insert"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer + (7 * buttonYBuffer),
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_INSERT,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);
        hwndEjectButton = CreateWindow(TEXT("button"),
            TEXT("Eject"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer + (9 * buttonYBuffer),
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_EJECT,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);
        hwndSaveButton = CreateWindow(TEXT("button"),
            TEXT("Save"),
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            rect.right - 100 - windowFrameBuffer, rect.top + windowFrameBuffer + (12 * buttonYBuffer),
            100, 20,
            hwnd,
            (HMENU)IDC_BUTTON_SAVE,
            ((LPCREATESTRUCT)lParam)->hInstance,
            NULL);


        // Create a listbox to hold the tape data - Status/BlockType/Filename/AutostartLine/Address/Length
        hwndListView = CreateListView(hwnd, lParam, rect);
        if (hwndListView == NULL)
        {
            MessageBox(hwnd, TEXT("Couldn't create listview!"), TEXT("LISTVIEW"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            return 0;
        }
        else
        {
            bool addItems = InitListViewColumns(hwndListView, ((LPCREATESTRUCT)lParam)->hInstance);
            if (addItems == false)
            {
                MessageBox(hwnd, TEXT("Couldn't add items!"), TEXT("LISTVIEW"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
                return 0;
            }
            else
            {
                LVITEM lvi;

                lvi.mask = LVIF_TEXT | LVIF_COLFMT;
                //lvi.iItem = 0;
                //lvi.iSubItem = 0;
                //lvi.pszText = TEXT("Kiss");
                //ListView_InsertItem(hwndListView, &lvi);
                //ListView_SetItemText(hwndListView, 0, 1, TEXT("My"));
                //ListView_SetItemText(hwndListView, 0, 2, TEXT("Ass"));
                //ListView_SetItemText(hwndListView, 0, 3, TEXT("!!!"));
                ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
            }
        }
        return 0;
        break;


    case WM_USER + 2:
    {
        if (wParam == PM_TAPEDATA_FULL)
        {
            if (hwndListView != nullptr) {
                ListView_DeleteAllItems(hwndListView);
                size_t numBlocks = lParam;

                myP = (std::vector<PMDawn::gTAPEBLOCK>*)lParam;

                size_t nn = myP->size();
                if (nn == 0)
                {
                    MessageBox(hwnd, TEXT("PMDawn::pData.size() is 0 !!"), TEXT("Tape Viewer"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
                    return 0;
                }

                for (int i = nn - 1; i >= 0; i--)
                {
                    PMDawn::gTAPEBLOCK theBlock = myP->at(i);
                    PMDawn::AddItemToListView(theBlock, hwndListView);
                }

                ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT);
                return 0;
            }
        }
        if (wParam == PM_TAPE_EJECTED)
        {
            if (hwndListView != nullptr) {
                ListView_DeleteAllItems(hwndListView);
                return 0;
            }
        }
        if (wParam == PM_TAPE_ACTIVEBLOCK)
        {
            if (hwndListView != nullptr) {
                uint16_t blockNumber = (uint16_t)lParam;

                //ListView_DeleteAllItems(hwndListView);
                return 0;
            }
        }


        return 0;
        break;
    }


    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_PLAY:
            PostMessage(mHandle, WM_USER + 2, PM_TAPE_COMMAND, (LPARAM)PM_TAPE_PLAY);
            break;
        case IDC_BUTTON_PAUSE:
            PostMessage(mHandle, WM_USER + 2, PM_TAPE_COMMAND, (LPARAM)PM_TAPE_PAUSE);
            break;
        case IDC_BUTTON_REWIND:
            PostMessage(mHandle, WM_USER + 2, PM_TAPE_COMMAND, (LPARAM)PM_TAPE_REWIND);
            break;
        case IDC_BUTTON_INSERT:
            PostMessage(mHandle, WM_USER + 2, PM_TAPE_COMMAND, (LPARAM)PM_TAPE_INSERT);
            break;
        case IDC_BUTTON_EJECT:
            PostMessage(mHandle, WM_USER + 2, PM_TAPE_COMMAND, (LPARAM)PM_TAPE_EJECT);
            break;
        case IDC_BUTTON_SAVE:
            MessageBox(hwnd, TEXT("SAVE"), TEXT("BUTTON"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
            break;
        }

        return 0;
        break;

    case WM_NOTIFY:
    {
        if ((((LPNMHDR)lParam)->hwndFrom) == hwndListView)
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case NM_DBLCLK:
            {
                // code here <--
                LPNMITEMACTIVATE lpNMItem = (LPNMITEMACTIVATE)lParam;
                return 0;
            }
            break;
            }
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//-----------------------------------------------------------------------------------------

// InitListViewColumns: Adds columns to a list-view control.
// hWndListView:        Handle to the list-view control. 
// Returns TRUE if successful, and FALSE otherwise. 
BOOL TapeViewer::InitListViewColumns(HWND hWndListView, HINSTANCE hInst)
{
    WCHAR szText[256];     // Temporary buffer.
    LVCOLUMN lvc;
    int iCol;

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text,
    // and subitem members of the structure are valid.
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // Add the columns.
    for (iCol = 0; iCol < IDS_NUMBEROFCOLUMNS; iCol++)
    {

        lvc.iSubItem = iCol;
        lvc.pszText = szText;
        lvc.cx = 85;               // Width of column in pixels.

        //if (iCol < 2)
        //    lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.
        //else
        //    lvc.fmt = LVCFMT_RIGHT; // Right-aligned column.

        // Load the names of the column headings from the string resources.
        LoadString(GetModuleHandle(NULL),
            IDS_FIRSTCOLUMN + iCol,
            szText,
            sizeof(szText) / sizeof(szText[0]));

        // Insert the columns into the list view.
        auto lvCol = ListView_InsertColumn(hWndListView, iCol, &lvc);
        if (lvCol == -1)
        {
            return FALSE;
        }
    }

    return TRUE;
}

//-----------------------------------------------------------------------------------------

HWND TapeViewer::CreateListView(HWND hwnd, LPARAM lParam, RECT rect)
{
    // Initialize the common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    HWND hlv = CreateWindow(WC_LISTVIEW,
        TEXT("ViewList"),
        WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | CS_DBLCLKS,
        rect.left + windowFrameBuffer, rect.top + windowFrameBuffer,
        rect.right - (3 * windowFrameBuffer) - 100, rect.bottom - windowFrameBuffer,
        hwnd,
        (HMENU)IDC_LISTVIEW_TAPEDATA,
        GetModuleHandle(NULL),
        NULL);
    //ShowWindow(hlv, SW_SHOWNORMAL);
    return hlv;
}

//-----------------------------------------------------------------------------------------

TapeViewer::TapeViewer(HINSTANCE mainWindowInst, HWND mainHandle, DWORD dwTlsIndex)
{
    bool exit_tapeviewer = false;
    MSG	msg;
    WNDCLASSEX wcextv;

    mHandle = mainHandle;

    // thread data
    //PMDawn::pData = (PMDawn::PTHREADDATA)TlsGetValue(dwTlsIndex);

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
    RECT wr = { 0, 0, 640, 240 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);

    // Now create the window
    tapeViewerWindowInternal = CreateWindowEx(WS_EX_APPWINDOW, TEXT("SpectREM_TapeViewer"), TEXT("SpectREM - Tape Viewer"),
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, 0, 0, mainWindowInst, 0);
    if (tapeViewerWindowInternal == NULL)
    {
        MessageBoxA(NULL, "Tape window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    // let the main window know that we are cool for receiving updates
    PostMessage(mainHandle, WM_USER + 1, 1, 1);

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






