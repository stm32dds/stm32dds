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

//Dialog variables
HWND hStatus; //Status bar window
WCHAR szDialogTemp[256]; //used to load resource strings to dialog
DWORD dwDialogTempLen; // count of loaded chars into string above
CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers

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
unsigned __int16 aReturnedWave[360]; // Returned from device wave
unsigned __int16 uFrqSP = 100; // set point for Frequency
unsigned __int16 VppSP = 0x7FFF; // set point for Amplitude
unsigned __int8 uOffsSP = 0x1F; // set point for Offset
unsigned __int8 uPwmSP = 50; // set point for PWM on SQUARE Wave
SamplesPerWave  eSPW; AmpPower eAmpPow; WaveType eWaveType;

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
                if(dwLen == 720)  // received Returned wave
                {
                    for (int i = 0, j = 0; i < 360; ++i, j += 2)
                    {
                        aReturnedWave[i] =
                      //      (aRxBuffer[j] << 8) | aRxBuffer[j + 1];
                            aRxBuffer[j]  | (aRxBuffer[j + 1] << 8);
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
        case IDOK:CreateWave(aCalculatedWave,eWaveType, VppSP, uPwmSP); return TRUE;
        case IDCANCEL: onCancel(hDlg); return TRUE; /* call subroutine */
        case IDC_ABOUT: onAbout(hDlg); return TRUE;
        case IDC_CONNECT: if (isConnected == FALSE)
            isConnected = onConnect(hDlg, pcCommPort, hCom, hStatus,
                dcb, aCalculatedWave, oR, oW,
                dwEventMask, hThread, comtimes); return TRUE;
        case IDC_STARTSTOP: if (isConnected == TRUE)
            onStartStop(hDlg, pcCommPort, hCom, hStatus,
                isStarted, aCalculatedWave, oW,  eWaveType, VppSP, uPwmSP); return TRUE;
        case IDC_COMBO_WAVE:if (HIWORD(wParam) == CBN_SELCHANGE)
            eWaveType = (WaveType)SendMessageW(GetDlgItem(hDlg, IDC_COMBO_WAVE), CB_GETCURSEL, 0, 0);
            return TRUE;
        case IDC_COMBO_SPW: if (HIWORD(wParam) == CBN_SELCHANGE)
            eSPW = onChngSPW(hDlg);
            refreshFreq(hDlg, uFrqSP, eSPW);
            return TRUE;
        case IDC_BUT_FCHG_UP: uFrqSP=onChgFrqUp(hDlg, uFrqSP, eSPW, FALSE); return TRUE;
        case IDC_BUT_FCHG_UP100: uFrqSP = onChgFrqUp(hDlg, uFrqSP, eSPW, TRUE); return TRUE;
        case IDC_BUT_FCHG_DOWN: uFrqSP = onChgFrqDown(hDlg, uFrqSP, eSPW, FALSE); return TRUE;
        case IDC_BUT_FCHG_DOWN100: uFrqSP = onChgFrqDown(hDlg, uFrqSP, eSPW, TRUE); return TRUE;
        case IDC_RADIO_X20: eAmpPow = AmpPower::x2_0;
            refreshOffs(hDlg, uOffsSP, eAmpPow);
            refreshVpp(hDlg, VppSP, eAmpPow); return TRUE;
        case IDC_RADIO_X15: eAmpPow = AmpPower::x1_5; 
            refreshOffs(hDlg, uOffsSP, eAmpPow);
            refreshVpp(hDlg, VppSP, eAmpPow); return TRUE;
        case IDC_RADIO_X10: eAmpPow = AmpPower::x1_0;
            refreshOffs(hDlg, uOffsSP, eAmpPow);
            refreshVpp(hDlg, VppSP, eAmpPow); return TRUE;
        case IDC_RADIO_X05: eAmpPow = AmpPower::x0_5;
            refreshOffs(hDlg, uOffsSP, eAmpPow);
            refreshVpp(hDlg, VppSP, eAmpPow); return TRUE;
        case IDC_BUT_OCHG_UP: uOffsSP = onChgOffsUp(hDlg, uOffsSP, eAmpPow); return TRUE;
        case IDC_BUT_OCHG_DOWN: uOffsSP = onChgOffsDown(hDlg, uOffsSP, eAmpPow); return TRUE;
        case IDC_BUT_VCHG_UP: VppSP = onChgVppUp(hDlg, VppSP, eAmpPow, FALSE); return TRUE;
        case IDC_BUT_VCHG_UP100: VppSP = onChgVppUp(hDlg, VppSP, eAmpPow, TRUE); return TRUE;
        case IDC_BUT_VCHG_DOWN: VppSP = onChgVppDown(hDlg, VppSP, eAmpPow, FALSE); return TRUE;
        case IDC_BUT_VCHG_DOWN100: VppSP = onChgVppDown(hDlg, VppSP, eAmpPow, TRUE); return TRUE;
        }
         break;
    case WM_NOTIFY:uPwmSP = fnGetPwm(hDlg); return TRUE;
    case WM_CLOSE:   onClose(hDlg, hCom, hThread); return TRUE; /* call subroutine */
    case WM_DESTROY: PostQuitMessage(0); return TRUE; /*called by onClose*/
    case WM_INITDIALOG: return TRUE;
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

 //   hComboBox_Freq = CreateWindowEx(0, WC_COMBOBOX, NULL,
   //     WS_VISIBLE | WS_CHILD | CBS_DROPDOWN, 0, 0, 0, 0, hDlg,
   //     (HMENU)IDC_COMBO_WAVE, GetModuleHandle(NULL), NULL);
  
  //  SendMessage(GetDlgItem(hDlg, IDC_COMBO_WAVE), CB_ADDSTRING, 0, (LPARAM)L"Item 1");

 //   SendMessage(GetDlgItem(hDlg, IDC_COMBO_WAVE), CB_ADDSTRING, 0, (LPARAM)L"Item 2");

 //   SendMessage(GetDlgItem(hDlg, IDC_COMBO_WAVE), CB_ADDSTRING, 0, (LPARAM)L"Item 3");

 //   hComboBox_Freq = GetDlgItem(hDlg, IDC_COMBO_WAVE);

// Create  & fill Wave selectin Combo Box
    DialogBoxW(hInst,                   // application instance
        MAKEINTRESOURCE(IDC_COMBO_WAVE), // dialog box resource
        hDlg,                          // owner window
        DialogProc);                // dialog box window procedure
    dwDialogTempLen = LoadStringW(hInst,IDS_STR_WAV_SIN,szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_SQR, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_TRI, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_SAW, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_RSAW, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_RND, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV_CALIB, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);

    SendDlgItemMessageW(hDlg, IDC_COMBO_WAVE, CB_SETCURSEL, (WPARAM)eWaveType, 0);

// Create  & fill Samples per Wave (SPW) selectin Combo Box
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_COMBO_SPW), hDlg, DialogProc);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV360, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV180, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV90, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV45, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    dwDialogTempLen = LoadStringW(hInst, IDS_STR_WAV24, szDialogTemp, 256);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_ADDSTRING,
        dwDialogTempLen, (LPARAM)(LPCSTR)szDialogTemp);
    SendDlgItemMessageW(hDlg, IDC_COMBO_SPW, CB_SETCURSEL,0, 0); 

// Create Frequency Edit Box
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_EDIT_FRQ), hDlg, DialogProc);
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspFrq(uFrqSP, eSPW), 8);
    SetDlgItemTextA(hDlg,IDC_EDIT_FRQ,szChDlgTmp);

//Select x2 radio button as default
    SendDlgItemMessageW(hDlg,IDC_RADIO_X20, BM_SETCHECK, BST_CHECKED, 0);

// Create Offset Edit Box
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_EDIT_OFFS), hDlg, DialogProc);
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspOffs(uOffsSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_OFFS, szChDlgTmp);

//Initialise PWM dialog
    SendDlgItemMessageW(hDlg, IDC_SPIN_PWM, UDM_SETPOS, 0, uPwmSP);

// Create Amplitude (Vpp) Edit Box
    DialogBoxW(hInst, MAKEINTRESOURCE(IDC_EDIT_VPP), hDlg, DialogProc);
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspVpp(VppSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_VPP, szChDlgTmp);

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