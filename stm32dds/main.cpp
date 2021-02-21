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
COMMTIMEOUTS comtimes;
DCB dcb;
HANDLE hCom;
BOOL isConnected = FALSE; // Is Device connected or NOT?
BOOL isStarted = FALSE; //is device started or NOT ?
TCHAR pcCommPort[20] = L"\\\\.\\";
DWORD dwEventMask;
OVERLAPPED oRead; LPOVERLAPPED oR=&oRead; //Overlapped for reading
OVERLAPPED oWrite; LPOVERLAPPED oW = &oWrite; //Overlapped for writing
//enum USBCommand {DeviceType=0x00, DeviceStart=0x55, DeviceStop=0xAA};
//enum USBCommand CmdToDevice; enum USBCommand CmdOntoDevice;

//Wave variables
unsigned __int16 aCalculatedWave[360];// Wave that will be sent to device

//Thread variable - used to "Pool" serial port for incoming data
HANDLE hThread;

//const WCHAR* pcThreadName = L"COM3";

DWORD WINAPI WaitForDataToRead()
{
    // The new thread starting here
   // if (isConnected == TRUE) //?!To check if this is passed only once
   //     ReceiveFromDevice();
  //  if (hCom != NULL) ReceiveFromDevice();
 //   return 0;
    DWORD dwLen;
    unsigned __int8 aRxBuffer[1000];
    do { ; } while (isConnected != TRUE);
    do
    {
        WaitCommEvent(hCom, &dwEventMask, oR);
        if (WaitForSingleObject(oR->hEvent, INFINITE) == WAIT_OBJECT_0)
        {
   //         DWORD dwBytesRead;
   //         DWORD dwMaxIncomingBytes = 0;
   //         char szBuf[100];
            BOOL bWFret;
            memset(aRxBuffer, 0, sizeof(aRxBuffer));
            do
            {
                bWFret=ReadFile(hCom, aRxBuffer, sizeof(aRxBuffer), &dwLen, oR);
                if (dwLen == 1)//Command Ask received
                {
                    if (aRxBuffer[0] == USB_DEVICE_START)
                    {
                        SendMessage(hStatus, SB_SETTEXT, 0,
                                        (LPARAM)L"Device is RUNNING!");
                        isStarted = TRUE;
                    }
                    if (aRxBuffer[0] == USB_DEVICE_STOP)
                    {
                        SendMessage(hStatus, SB_SETTEXT, 0,
                                       (LPARAM)L"Device is STOPPED!");
                        isStarted = FALSE;
                    }
                    if (aRxBuffer[0] == USB_DEVICE_TYPE)
                    {
                     ;
                    }
                }
             } while (dwLen > 0);
             ;
        }
    } while (TRUE);
 //   return 0;
}


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
            isConnected = onConnect(hDlg, pcCommPort, hCom, hStatus,
                dcb, aCalculatedWave,oR, oW,
                    dwEventMask, hThread, comtimes); return TRUE;
        case IDC_STARTSTOP: if (isConnected == TRUE)
            onStartStop(hDlg, pcCommPort, hCom, hStatus,
                isStarted, aCalculatedWave, oW); return TRUE;
        }
        break;
    case WM_CLOSE:   onClose(hDlg, hCom, hThread); return TRUE; /* call subroutine */
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
// Create a new thread which will start at the WaitForDataToRead function
    hThread = CreateThread(NULL, // security attributes ( default if NULL )
        0, // stack SIZE default if 0
        (LPTHREAD_START_ROUTINE) WaitForDataToRead, // Start Address
        NULL, // input data
        0, // creational flag ( start if  0 )
        NULL); // thread ID

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