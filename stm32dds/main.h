#include <wtypes.h>
#include <string.h>
#include <Windows.h>
#include <winuser.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <strsafe.h>
#include <initguid.h>
#include <SetupAPI.h>
#include "resource.h"

extern void onCancel(HWND hDlg);
extern void onClose(HWND hDlg, HANDLE hCom);
extern void onAbout(HWND hDlg);
extern BOOL onConnect(HWND hDlg, TCHAR *pcCommPort, HANDLE hCom, HWND hStatus);