#include "main.h"

void onCancel(HWND hDlg)
{
    SendMessage(hDlg, WM_CLOSE, 0, 0);
}

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
    case WM_INITDIALOG:
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
void onAbout(HWND hDlg)
{
    DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutDialog);
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