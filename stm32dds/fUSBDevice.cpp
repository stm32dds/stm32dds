#include "main.h"

void SendWave(unsigned __int16* aCalculatedWave, HANDLE hCom, HWND hStatus)
{
    unsigned __int8 aOutputBuffer[720];// Data that will sent to device as BYTE USB stream
    unsigned __int8 aUSBChunkBuffer[61];//Chunk of Data to form BYTE USB packet
    CreateWave(aCalculatedWave);
    // 16 bit aCalculatedWaveto to 8 bit aOutputBuffer
    for (int i = 0, j = 0; i < 720; i += 2, ++j)
    {
        aOutputBuffer[i] = aCalculatedWave[j] >> 8; // Hi byte
        aOutputBuffer[i + 1] = aCalculatedWave[j] & 0xFF; // Lo byte
    }
    //Send 720 bytes as chunks of 60 bytes
    for (int i = 0; i < 12; ++i)
    {
        for (int j = 0; j < 60; ++j)
            aUSBChunkBuffer[j] = aOutputBuffer[i * 60 + j];
        aUSBChunkBuffer[60] = i;
        if (WriteFile(hCom, aUSBChunkBuffer, 61, NULL, 0) == FALSE)
            MessageBox(NULL,
                L"Fail! Wave not sent!",
                L"Send wave action",
                MB_ICONERROR);
    }
}

BOOL onStartStop(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom,
    HWND hStatus, BOOL isStarted, unsigned __int16* aCalculatedWave)
{
    if (isStarted == FALSE) //start device
    {
        SendWave(aCalculatedWave, hCom, hStatus);
        SetDlgItemTextW(hDlg, IDC_STARTSTOP, L"STOP");
        SendMessage(hStatus, SB_SETTEXT, 0,
            (LPARAM)L"Device is RUNNING!");
        return TRUE;
    }
    else //stop device
    {
        SetDlgItemTextW(hDlg, IDC_STARTSTOP,L"START");
        SendMessage(hStatus, SB_SETTEXT, 0,
            (LPARAM)L"Device is STOPPED!");
        return FALSE;
    }
}

BOOL onConnect(HWND hDlg, TCHAR* pcCommPort, HANDLE &hCom, HWND hStatus, DCB* dcb, unsigned __int16* aCalculatedWave)
{
    HDEVINFO DeviceInfoSet;
    DWORD DeviceIndex = 0;
    SP_DEVINFO_DATA DeviceInfoData;
    TCHAR stm32ddsId[] = { L"VID_0483&PID_5740" };
    BYTE szBuffer[1024] = { 0 };
    DEVPROPTYPE ulPropertyType;
    DWORD dwSize = 0;
    TCHAR msgSTR[64] = { 0 };
    //SetupDiGetClassDevs returns a handle to a device information set
    DeviceInfoSet = SetupDiGetClassDevs(NULL, L"USB", NULL,
        DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        SendMessage(hStatus, SB_SETTEXT, 0,
            (LPARAM)L"Device NOT connected->No Device Info Set");
        return FALSE;
    }
    //Fills a block of memory with zeros
    ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    //Receive information about an enumerated device
    while (SetupDiEnumDeviceInfo(
        DeviceInfoSet,
        DeviceIndex,
        &DeviceInfoData))
    {
        DeviceIndex++;
        //Retrieves a specified Plug and Play device property
        if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet, &DeviceInfoData, SPDRP_HARDWAREID,
            &ulPropertyType, (BYTE*)szBuffer,
            sizeof(szBuffer),   // The size, in bytes
            &dwSize))
        {
            TCHAR CurrentDeviceId[1024] = { 0 };
            for (int i = 0, j = 0; i < 1024; i++)
            {
                if (szBuffer[i] != 0)
                {
                    CurrentDeviceId[j] = szBuffer[i];
                    j++;
                }
            }
            //Compare retrived ID with our ID
            if (wcsstr(CurrentDeviceId, stm32ddsId) != 0)
            {
                HKEY hDeviceRegistryKey;
                //Take the key
                hDeviceRegistryKey = SetupDiOpenDevRegKey(DeviceInfoSet, &DeviceInfoData,
                    DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                if (hDeviceRegistryKey == INVALID_HANDLE_VALUE)
                {
                    SendMessage(hStatus, SB_SETTEXT, 0,
                        (LPARAM)L"Device NOT connected->Not able to open the registry");
                    return FALSE; //Not able to open registry
                }
                else
                {
                    // Take the name of the port
                    wchar_t pszPortName[20];
                    DWORD dwSize = sizeof(pszPortName);
                    DWORD dwType = 0;
                    if ((RegQueryValueEx(hDeviceRegistryKey, L"PortName", NULL, &dwType,
                        (LPBYTE)pszPortName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
                    {
                        // Check if it really is a com port
                        if (_tcsnicmp(pszPortName, _T("COM"), 3) == 0)
                        {
                            int nPortNr = _ttoi(pszPortName + 3);
                            if (nPortNr != 0)
                            {
                                _tcscpy_s(pcCommPort, 20, pszPortName);
                            }
                        }
                    }
                    // Close the key now that we are finished with it
                    RegCloseKey(hDeviceRegistryKey);
                }
            }
        }
    }
    if (DeviceInfoSet)
    {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    //  Open a handle to the specified com port.
    hCom = CreateFile(pcCommPort,
        GENERIC_READ | GENERIC_WRITE,
        0,      //  must be opened with exclusive-access
        NULL,   //  default security attributes
        OPEN_EXISTING, //  must use OPEN_EXISTING
        0,      //  not overlapped I/O
        NULL); //  hTemplate must be NULL for comm devices

    if (hCom == INVALID_HANDLE_VALUE)
    {
        //  Handle the error.
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)L"No Device or communication Error!");
        return FALSE;
    }
    else
    {
        //  Initialize the DCB structure.
        SecureZeroMemory(dcb, sizeof(DCB));
        dcb->DCBlength = sizeof(DCB);
        //  Build on the current configuration by first retrieving all current
        //  settings.
        if (GetCommState(hCom, dcb) == FALSE)
            MessageBox(NULL, L"Can't create DCB!",
                L"DCB actions", MB_ICONERROR);
        //  Fill in some DCB values and set the com state: 
        //  57,600 bps, 8 data bits, no parity, and 1 stop bit.
        dcb->BaudRate = CBR_9600;     //  baud rate
        dcb->ByteSize = 8;             //  data size, xmit and rcv
        dcb->Parity = NOPARITY;      //  parity bit
        dcb->StopBits = ONESTOPBIT;    //  stop bit
        if (SetCommState(hCom, dcb) == FALSE)
            MessageBox(NULL, L"Can't write to DCB!",
                L"DCB actions", MB_ICONERROR);
        //  Get the comm config again.
        if (GetCommState(hCom, dcb) == FALSE)
            MessageBox(NULL, L"Can't read from DCB!",
                L"DCB actions", MB_ICONERROR);
        //      TCHAR msgSTR[64] = { 0 };
        wcscpy_s(msgSTR, L"Device is connected as ");
        wcscat_s(msgSTR, pcCommPort);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)msgSTR);
        return TRUE;
    }
}