#include "main.h"

BOOL onConnect(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom, HWND hStatus)
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
        //      TCHAR msgSTR[64] = { 0 };
        wcscpy_s(msgSTR, L"Device is connected as ");
        wcscat_s(msgSTR, pcCommPort);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)msgSTR);
        return TRUE;
    }
}