#include "main.h"

void onCancel(HWND hDlg)
{
    SendMessage(hDlg, WM_CLOSE, 0, 0);
}

void onClose(HWND hDlg, HANDLE hCom)
{
    CloseHandle(hCom);
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
