#include <wtypes.h>
#include <string.h>
#include <Windows.h>
#include <winuser.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <strsafe.h>
#include <initguid.h>
#include <SetupAPI.h>
#include <math.h>       /* sin */
#include "resource.h"

#define PI 3.14159265

extern void onCancel(HWND hDlg);
extern void onClose(HWND hDlg, HANDLE hCom);
extern void onAbout(HWND hDlg);
extern BOOL onConnect(HWND hDlg, TCHAR *pcCommPort, HANDLE &hCom, HWND hStatus,DCB* dcb, unsigned __int16* aCalculatedWave);
extern BOOL onStartStop(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom,
	HWND hStatus, BOOL isStarted, unsigned __int16* aCalculatedWave);
extern void CreateWave(unsigned __int16* aCalculatedWave);
extern void SendWave(unsigned __int16* aCalculatedWave, HANDLE hCom, HWND hStatus);