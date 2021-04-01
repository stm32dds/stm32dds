#include "main.h"

void onClose(HWND hDlg, HANDLE hCom, HANDLE hThread)
{
    CloseHandle(hThread); // Close reading thread
    CloseHandle(hCom); //Closes hStatus also
    DestroyWindow(hDlg); //Destroys Main Dialog Window
                         //& issue WM_DESTROY message to DialogProc
    
}

// Message handler for "About" box.
INT_PTR CALLBACK AboutDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
            case NM_CLICK:
                ShellExecute(NULL, L"open",
                    L"http://www.stm32dds.tk",NULL, NULL, SW_SHOW);
            return (INT_PTR)TRUE;
        }
    case WM_INITDIALOG:
        SetDlgItemTextA(hDlg, IDC_VERSION,
             (LPCSTR)"0.1"
        );
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
} 

// Create and Show the box - "About".
void onAbout(HWND hDlg,HINSTANCE hInst)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutDialog);
}

//Change and display SPW parameter
SamplesPerWave onChngSPW(HWND hDlg)
{
    return
    (SamplesPerWave)SendMessageW(GetDlgItem(hDlg, IDC_COMBO_SPW), CB_GETCURSEL, 0, 0);
}

// Just refresh Frequency edit box based on current parameters
void refreshFreq(HWND hDlg, unsigned __int16 uFrqSP, SamplesPerWave eSPW)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspFrq(uFrqSP, eSPW), 8);
    SetDlgItemTextA(hDlg, IDC_EDIT_FRQ, szChDlgTmp);
}

// Just refresh Offset edit box based on current parameters
void refreshOffs(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspOffs(uOffsSP, eAmpPow),3);
    SetDlgItemTextA(hDlg, IDC_EDIT_OFFS, szChDlgTmp);
}

// Just refresh Amplitude (Vpp) edit box based on current parameters
void refreshVpp(HWND hDlg, unsigned __int16 VppSP, AmpPower eAmpPow)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspVpp(VppSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_VPP, szChDlgTmp);
}

//Increments Frequency SP
unsigned __int16  onChgFrqUp(HWND hDlg, unsigned __int16 uFrqSP, SamplesPerWave eSPW, BOOL byStep)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (byStep == TRUE)
    { 
        if (uFrqSP < (0x000F + 0x0007)) uFrqSP = uFrqSP - 0x0001;
        else if (uFrqSP < (0x00FF + 0x0007)) uFrqSP = uFrqSP - 0x000F;
        else if (uFrqSP < (0x0FFF + 0x0007)) uFrqSP = uFrqSP - 0x00FF;
        else if (uFrqSP <= (0xFFFF)) uFrqSP = uFrqSP - 0x0FFF;
    }
    else uFrqSP = uFrqSP - 1;
    if (uFrqSP < 7) uFrqSP = 7; // minimal value for DDS timer
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspFrq(uFrqSP, eSPW), 8);
    SetDlgItemTextA(hDlg, IDC_EDIT_FRQ, szChDlgTmp);
    return uFrqSP;
}

//Decrements Frequency SP
unsigned __int16  onChgFrqDown(HWND hDlg, unsigned __int16 uFrqSP, SamplesPerWave eSPW, BOOL byStep)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (byStep == TRUE)
    {
        if (uFrqSP < (0x000F + 0x0007)) uFrqSP = uFrqSP + 0x0001;
        else if (uFrqSP < (0x00FF + 0x0007)) uFrqSP = uFrqSP + 0x000F;
        else if (uFrqSP < (0x0FFF + 0x0007)) uFrqSP = uFrqSP + 0x00FF;
        else if (uFrqSP <= (0xFFFF))
        { 
            uFrqSP = uFrqSP + 0x0FFF;
            if (uFrqSP < 0x0FFF) uFrqSP = 0xFFFF;
        }
    }
    else uFrqSP = uFrqSP + 1; 
    if (uFrqSP < 7) uFrqSP = 0xFFFF; // minimal value for DDS timer
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspFrq(uFrqSP, eSPW), 8);
    SetDlgItemTextA(hDlg, IDC_EDIT_FRQ, szChDlgTmp);
    return uFrqSP;
}

//Increments Offset SP
unsigned __int8  onChgOffsUp(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (uOffsSP < 0x3E) uOffsSP = uOffsSP + 1;
    else uOffsSP = 0x3E;
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspOffs(uOffsSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_OFFS, szChDlgTmp);
    return uOffsSP;
}

//Decrements Offset SP
unsigned __int8  onChgOffsDown(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (uOffsSP > 0x00) uOffsSP = uOffsSP - 1;
    else uOffsSP = 0x00;
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspOffs(uOffsSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_OFFS, szChDlgTmp);
    return uOffsSP;
}

//Used to get PWM setpoint "on the fly"
unsigned __int8 fnGetPwm(HWND hDlg)
{
   return (unsigned __int8)SendDlgItemMessageW(hDlg, IDC_SPIN_PWM, UDM_GETPOS, 0, 0);
}

//Increments Vpp SP
unsigned __int16 onChgVppUp(HWND hDlg, unsigned __int16 VppSP, AmpPower eAmpPow, BOOL byStep)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (byStep == TRUE) VppSP = VppSP + 0X03FF;
    else VppSP = VppSP + 0x07F;
    if (VppSP > 0x7FFF) VppSP = 0x7FFF; // maximal value for Vpp amplitude
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspVpp(VppSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_VPP, szChDlgTmp);
    return VppSP;
}

//Decrements Vpp SP
unsigned __int16 onChgVppDown(HWND hDlg, unsigned __int16 VppSP, AmpPower eAmpPow, BOOL byStep)
{
    CHAR szChDlgTmp[_CVTBUFSIZE]; // for converted double numbers
    if (byStep == TRUE) VppSP = VppSP - 0X03FF;
    else VppSP = VppSP - 0x007F;
    if ((VppSP < 0x00FF)||(VppSP > 0x7FFF)) VppSP = 0x00FF; // minimal value for Vpp amplitude
    _gcvt_s(szChDlgTmp, sizeof(szChDlgTmp), CalcWavDspVpp(VppSP, eAmpPow), 3);
    SetDlgItemTextA(hDlg, IDC_EDIT_VPP, szChDlgTmp);
    return VppSP;
}