#include "main.h"

void SendCommand(unsigned __int8 CmdToDevice, HANDLE hCom, LPOVERLAPPED oW)
{
    WriteFile(hCom,  &CmdToDevice, 1, NULL, oW);
    while (oW->Internal == STATUS_PENDING);
}

void SendWave(unsigned __int16* aCalculatedWave, HANDLE hCom, HWND hStatus, LPOVERLAPPED oW)
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
        WriteFile(hCom, aUSBChunkBuffer, 61, NULL, oW);
        while (oW->Internal == STATUS_PENDING); // Just wait for operation completion
    }
 
}

void onStartStop(HWND hDlg, TCHAR* pcCommPort, HANDLE hCom,
    HWND hStatus, BOOL isStarted, unsigned __int16* aCalculatedWave, LPOVERLAPPED oW)
{
    if (isStarted == FALSE) //start device
    {
        SendWave(aCalculatedWave, hCom, hStatus, oW);
        SendCommand(USB_DEVICE_START, hCom, oW);
        SetDlgItemTextW(hDlg, IDC_STARTSTOP, L"STOP");
//        SendMessage(hStatus, SB_SETTEXT, 0,
//            (LPARAM)L"Device is RUNNING!");
//        return !isStarted;
    }
    else //stop device
    {
        SendCommand(USB_DEVICE_STOP, hCom, oW);
        SetDlgItemTextW(hDlg, IDC_STARTSTOP,L"START");
//        SendMessage(hStatus, SB_SETTEXT, 0,
 //           (LPARAM)L"Device is STOPPED!");
 //       return !isStarted;
    }
}

BOOL onConnect(HWND hDlg, TCHAR* pcCommPort, HANDLE &hCom, HWND hStatus,
    DCB dcb, unsigned __int16* aCalculatedWave, LPOVERLAPPED oR, LPOVERLAPPED oW,
        DWORD dwEventMask, HANDLE hThread, COMMTIMEOUTS comtimes)
{
    HDEVINFO DeviceInfoSet;
    DWORD DeviceIndex = 0;
    SP_DEVINFO_DATA DeviceInfoData;
    TCHAR stm32ddsId[] = { L"VID_1209&PID_DD83" };
    BYTE szBuffer[1024] = { 0 };
    DEVPROPTYPE ulPropertyType;
    DWORD dwSize = 0;
    TCHAR msgSTR[64] = { 0 };
    TCHAR pcCommPortTemp[10] = { 0 };
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
                    wchar_t pszPortName[10];
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
                                _tcscpy_s(pcCommPortTemp, 10, pszPortName);
                                lstrcatW(pcCommPort, pcCommPortTemp);
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
        FILE_FLAG_OVERLAPPED,      //Overlapped I/O
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
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        //  Build on the current configuration by first retrieving all current
        //  settings.
        if (GetCommState(hCom, &dcb) == FALSE)
            MessageBox(NULL, L"Can't create DCB!",
                L"DCB actions", MB_ICONERROR);
        //  Fill in some DCB values and set the com state: 
        //  57,600 bps, 8 data bits, no parity, and 1 stop bit.
        dcb.BaudRate = CBR_9600;     //  baud rate
        dcb.ByteSize = 8;             //  data size, xmit and rcv
        dcb.Parity = NOPARITY;      //  parity bit
        dcb.StopBits = ONESTOPBIT;    //  stop bit
        if (SetCommState(hCom, &dcb) == FALSE)
            MessageBox(NULL, L"Can't write to DCB!",
                L"DCB actions", MB_ICONERROR);
        //  Get the comm config again.
        if (GetCommState(hCom, &dcb) == FALSE)
            MessageBox(NULL, L"Can't read from DCB!",
                L"DCB actions", MB_ICONERROR);
        //Setup COM Port time outs - not implementd - by default timeouts=0
        //!Create implementation if communication errors occurs
        //for example with if(GetCommTimeouts(.....bla-bla
        comtimes.ReadIntervalTimeout = MAXDWORD;
        comtimes.ReadTotalTimeoutMultiplier = 0;
        comtimes.ReadTotalTimeoutConstant = 0;
        comtimes.WriteTotalTimeoutMultiplier = 0;
        comtimes.WriteTotalTimeoutConstant = 0;

        if (SetCommTimeouts(hCom, &comtimes) == FALSE)
            MessageBox(NULL, L"Can't write to COMMTIMEOUTS!",
                L"COMMTIMEOTS actions", MB_ICONERROR);
        //Register receiving COM event
        if (SetCommMask(hCom, EV_RXCHAR)==FALSE)
            MessageBox(NULL, L"Can't register receiving COM event",
                L"COM Event", MB_ICONERROR);

        // Create an event object for use by WaitCommEvent when READING from COM port. 
        oR->hEvent = CreateEvent(
            NULL,   // default security attributes 
            TRUE,   // manual-reset event 
            FALSE,  // not signaled 
            NULL    // no name
        );

        // Initialize the rest of the READING OVERLAPPED structure to zero.
        oR->Internal = 0;
        oR->InternalHigh = 0;
        oR->Offset = 0;
        oR->OffsetHigh = 0;
     
        //Initialize WRITING OVERLAPPED STRUCTURE
        oW->hEvent = NULL; // Not used Event for data sent
        oW->Internal = 0; //This is pooling to chck that request is complete
        oW->InternalHigh = 0;
        oW->Offset = 0;
        oW->OffsetHigh = 0;
        
        //Display on Status bar connected port name
        wcscpy_s(msgSTR, L"Device is connected as ");
        wcscat_s(msgSTR, pcCommPortTemp);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)msgSTR);
        return TRUE;
    }
}

