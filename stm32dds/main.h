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

enum class SamplesPerWave { SPW360, SPW180, SPW90, SPW45, SPW24 };
enum class AmpPower { x2_0, x1_5, x1_0, x0_5 };
enum class WaveType { Sine, Square, Triangle, SawTooth, RewSawTooth, Random, Zero };

extern void onCancel(HWND hDlg);
extern void onClose(HWND hDlg, HANDLE hCom, HANDLE hThread);
extern void onAbout(HWND hDlg);
extern BOOL onConnect(HWND hDlg, TCHAR *pcCommPort, HANDLE &hCom, HWND hStatus,DCB dcb,
	unsigned __int16* aCalculatedWave, LPOVERLAPPED oR, LPOVERLAPPED oW,
		DWORD dwEventMask, HANDLE hThread, COMMTIMEOUTS comtimes);
extern void onStartStop(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom, HWND hStatus,
	BOOL isStarted, unsigned __int16* aCalculatedWave,
	LPOVERLAPPED oW, WaveType eWaveType, unsigned __int16 VppSP, unsigned __int8 uPwmSP);
extern void CreateWave(unsigned __int16* aCalculatedWave,
	WaveType eWaveType, unsigned __int16 VppSP, unsigned __int8 uPwmSP);
extern void SendWave(unsigned __int16* aCalculatedWave, HANDLE hCom, HWND hStatus,
	LPOVERLAPPED oW, WaveType eWaveType, unsigned __int16 VppSP, unsigned __int8 uPwmSP);
extern double CalcWavDspFrq(unsigned __int16 uFrqSP, SamplesPerWave  eSPW);
extern SamplesPerWave onChngSPW(HWND hDlg);
extern unsigned __int16  onChgFrqUp(HWND hDlg, unsigned __int16 uFrqSP,SamplesPerWave eSPW, BOOL byStep);
extern unsigned __int16  onChgFrqDown(HWND hDlg, unsigned __int16 uFrqSP, SamplesPerWave eSPW, BOOL byStep);
extern void refreshFreq(HWND hDlg, unsigned __int16 uFrqSP, SamplesPerWave eSPW);
extern double CalcWavDspOffs(unsigned __int8 uOffsSP, AmpPower  AmpPow);
extern unsigned __int8 fnGetPwm(HWND hDlg);
extern void refreshOffs(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow);
extern unsigned __int8  onChgOffsUp(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow);
extern unsigned __int8  onChgOffsDown(HWND hDlg, unsigned __int8 uOffsSP, AmpPower eAmpPow);
extern unsigned __int16 onChgVppUp(HWND hDlg, unsigned __int16 VppSP,AmpPower eAmpPow,BOOL byStep);
extern unsigned __int16 onChgVppDown(HWND hDlg, unsigned __int16 VppSP, AmpPower eAmpPow, BOOL byStep);
extern double CalcWavDspVpp(unsigned __int16 VppSP, AmpPower eAmpPow);
extern void refreshVpp(HWND hDlg, unsigned __int16 VppSP, AmpPower eAmpPow);