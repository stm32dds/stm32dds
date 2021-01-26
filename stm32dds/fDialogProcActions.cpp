#include <wtypes.h>
#include "resource.h"
#include <CommCtrl.h>

void onCancel(HWND hDlg)
{
    SendMessage(hDlg, WM_CLOSE, 0, 0);
}

void onClose(HWND hDlg)
{
    if (MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
        MB_ICONQUESTION | MB_YESNO) == IDYES)
    {
        DestroyWindow(hDlg);
    }
}

void onOK(HWND hStatus)
{
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) L"Mama ti!");
}

// Message handler for "About" box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
    DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
}
