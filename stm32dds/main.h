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

#define PI					3.14159265
#define USB_DEVICE_START	0x55
#define USB_DEVICE_STOP		0xAA
#define USB_DEVICE_TYPE		0x00


extern void onCancel(HWND hDlg);
extern void onClose(HWND hDlg, HANDLE hCom, HANDLE hThread);
extern void onAbout(HWND hDlg);
extern BOOL onConnect(HWND hDlg, TCHAR *pcCommPort, HANDLE &hCom, HWND hStatus,DCB dcb,
	unsigned __int16* aCalculatedWave, LPOVERLAPPED oR, LPOVERLAPPED oW,
		DWORD dwEventMask, HANDLE hThread, COMMTIMEOUTS comtimes);
extern void onStartStop(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom,
	HWND hStatus, BOOL isStarted, unsigned __int16* aCalculatedWave, LPOVERLAPPED oW);
extern void CreateWave(unsigned __int16* aCalculatedWave);
extern void SendWave(unsigned __int16* aCalculatedWave, HANDLE hCom, HWND hStatus, LPOVERLAPPED oW);
//void SendCommand(unsigned __int8 CmdToDevice, HANDLE hCom, LPOVERLAPPED oW);