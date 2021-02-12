#include "main.h"

#pragma comment (lib, "Setupapi.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

//Status bar variable
HWND hStatus;

//USB Device variables
DCB dcb;
HANDLE hCom;
BOOL isConnected = FALSE; // Is Device connected or NOT?
BOOL isStarted = 0; //is device started or NOT?
TCHAR pcCommPort[20] = { 0 };

//Wave variables
unsigned __int16 aCalculatedWave[360];// Wave that will be sent to device


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) /* more compact, each "case" through a single line, easier on the eyes */
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL: onCancel(hDlg); return TRUE; /* call subroutine */
        case IDC_ABOUT: onAbout(hDlg); return TRUE;
        case IDC_CONNECT: if(isConnected==FALSE)
            isConnected = onConnect(hDlg, pcCommPort, hCom, hStatus, &dcb, aCalculatedWave); return TRUE;
        case IDC_STARTSTOP: if (isConnected == TRUE)
            isStarted = onStartStop(hDlg, pcCommPort, hCom, hStatus,
                isStarted, aCalculatedWave); return TRUE;
        }
        break;
    case WM_CLOSE:   onClose(hDlg, hCom); return TRUE; /* call subroutine */
    case WM_DESTROY: PostQuitMessage(0); return TRUE; /*called by onClose*/
    }

    return FALSE;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
    // Main window variables
    HWND hDlg;
    MSG msg;
    BOOL ret;

    InitCommonControls();

// Create the window.
// Dialog type windows are not registrated as separate window class
    hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), 0, DialogProc, 0);
//Create simple statusbar on main dialog window
    hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
      WS_CHILD | WS_VISIBLE , 0, 0, 0, 0,hDlg,
        (HMENU)IDC_DIALOG_STATUS, GetModuleHandle(NULL), NULL);
 // write first init message on status bar
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"Hi there :)");


    ShowWindow(hDlg, nCmdShow);

    while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
        if (ret == -1)
            return -1;

        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}