#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include "mbt_com.h"

static char _parityChar[] = "NOEMS";
static char* _stopBits[] = { "1", "1.5", "2" };

//
//Class DebugHelper Implementation
//
char DebugHelper::m_Buffer[1024];
DebugHelper Logger;
void DebugHelper::DebugOut(LPCSTR fmt, LPCSTR v)
{
	sprintf_s(m_Buffer, fmt, v);
	OutputDebugStringA(m_Buffer);
}

void DebugHelper::DebugOut(LPCSTR fmt, DWORD v1, DWORD v2)
{
	sprintf_s(m_Buffer, fmt, v1, v2);
	OutputDebugStringA(m_Buffer);
}

void DebugHelper::DebugOut(LPCSTR fmt, DWORD v1)
{
	sprintf_s(m_Buffer, fmt, v1);
	OutputDebugStringA(m_Buffer);
}

void DebugHelper::PrintCommProp(COMMPROP& commProp)
{
	sprintf_s(m_Buffer, "Comm Properties TxQueue=%ld/%ld, RxQueue=%ld/%ld, packetLength=%d \n",
		commProp.dwCurrentTxQueue, commProp.dwMaxTxQueue,
		commProp.dwCurrentRxQueue, commProp.dwMaxRxQueue,
		commProp.wPacketLength
		);
	OutputDebugStringA(m_Buffer);
}

void DebugHelper::PrintCommState(DCB& serial_config)
{
	sprintf_s(Logger.m_Buffer, "OpenPort Port config Baud=%d, %c,%d,%s, Binary Mode=%s, DTR COntrol=%d, FlowControl=%d/%d\n",
		serial_config.BaudRate,
		_parityChar[serial_config.Parity],
		serial_config.ByteSize,
		_stopBits[serial_config.StopBits],
		serial_config.fBinary ? "True" : "Fale",
		serial_config.fDtrControl,
		serial_config.fOutxCtsFlow, serial_config.fOutxDsrFlow
		);
	OutputDebugStringA(m_Buffer);
}

//
//Class ComHelper Implementation
//
ComHelper::ComHelper() :
    m_handle(NULL)
{
    memset(&m_OverlapRead, 0, sizeof(m_OverlapRead));
    memset(&m_OverlapWrite, 0, sizeof(m_OverlapWrite));
}

ComHelper::~ComHelper()
{
	Logger.DebugOut("Close Serial Bus\n");
    if (m_OverlapRead.hEvent != NULL)
        CloseHandle(m_OverlapRead.hEvent);
    if (m_OverlapWrite.hEvent != NULL)
        CloseHandle(m_OverlapWrite.hEvent);
    if (m_handle != NULL&& m_handle != INVALID_HANDLE_VALUE)
    {
        PurgeComm(m_handle, PURGE_RXABORT | PURGE_RXCLEAR |PURGE_TXABORT | PURGE_TXCLEAR);
        CloseHandle(m_handle);
    }
}

//
//Open Serial Bus driver
//
BOOL ComHelper::OpenPort(char* port)
{
    char lpStr[20];
    sprintf_s(lpStr, 20, "\\\\.\\%s", port);

	// open once only
	if (m_handle != NULL&& m_handle != INVALID_HANDLE_VALUE)
		return FALSE;

	m_handle = CreateFile(lpStr,
		GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (m_handle != NULL&& m_handle != INVALID_HANDLE_VALUE)
	{
		// setup serial bus device
		BOOL bResult;
		DWORD dwError = 0;
		COMMTIMEOUTS commTimeout;
		COMMPROP commProp;
		COMSTAT comStat;
		DCB serial_config;

		// create events for Overlapped IO
		m_OverlapRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		Logger.DebugOut("OpenPort Overlap Event Handle=%x\n", (DWORD)m_OverlapRead.hEvent);
		m_OverlapWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		Logger.DebugOut("OpenPort Overlap Event Handle=%x\n", (DWORD)m_OverlapWrite.hEvent);

		// set comm timeout
		memset(&commTimeout, 0, sizeof(COMMTIMEOUTS));
		commTimeout.ReadIntervalTimeout = 1;
		commTimeout.ReadTotalTimeoutConstant = 1000;
		commTimeout.ReadTotalTimeoutMultiplier = 10;
		commTimeout.WriteTotalTimeoutConstant = 1000;
		commTimeout.WriteTotalTimeoutMultiplier = 1;
		bResult = SetCommTimeouts(m_handle, &commTimeout);

		// set comm configuration
		memset(&serial_config, 0, sizeof(serial_config));
		serial_config.DCBlength = sizeof (DCB);
		bResult = GetCommState(m_handle, &serial_config);

		Logger.PrintCommState(serial_config);

		serial_config.BaudRate = CBR_115200;
		serial_config.ByteSize = 8;
		serial_config.Parity = NOPARITY;
		serial_config.StopBits = ONESTOPBIT;
		serial_config.fBinary = TRUE;
		serial_config.fOutxCtsFlow = FALSE; // TRUE;
		serial_config.fOutxDsrFlow = FALSE; // TRUE;
		serial_config.fDtrControl = FALSE;

		serial_config.fOutX = FALSE;
		serial_config.fInX = FALSE;
		serial_config.fErrorChar = FALSE;
		serial_config.fNull = FALSE;
		serial_config.fParity = FALSE;
		serial_config.XonChar = 0;
		serial_config.XoffChar = 0;
		serial_config.ErrorChar = 0;
		serial_config.EofChar = 0;
		serial_config.EvtChar = 0;
		bResult = SetCommState(m_handle, &serial_config);

		if (!bResult)
			Logger.DebugOut("OpenPort SetCommState failed %d\n", GetLastError());
		else
		{
			// verify CommState
			memset(&serial_config, 0, sizeof(serial_config));
			serial_config.DCBlength = sizeof (DCB);
			bResult = GetCommState(m_handle, &serial_config);
			Logger.PrintCommState(serial_config);
		}

		// set IO buffer size
		memset(&commProp, 0, sizeof(commProp));
		bResult = GetCommProperties(m_handle, &commProp);

		if (!bResult)
			Logger.DebugOut("OpenPort GetCommProperties failed %d\n", GetLastError());
		else
		{
			// log comm Comm Property before changing it
			Logger.PrintCommProp(commProp);

			// use 4096 byte as preferred buffer size, adjust to fit within allowed Max
			commProp.dwCurrentTxQueue = 4096;
			commProp.dwCurrentRxQueue = 4096;
			if (commProp.dwCurrentTxQueue > commProp.dwMaxTxQueue)
				commProp.dwCurrentTxQueue = commProp.dwMaxTxQueue;
			if (commProp.dwCurrentRxQueue > commProp.dwMaxRxQueue)
				commProp.dwCurrentRxQueue = commProp.dwMaxRxQueue;
			bResult = SetupComm(m_handle, commProp.dwCurrentRxQueue, commProp.dwCurrentTxQueue);

			if (!bResult)
				Logger.DebugOut("OpenPort SetupComm failed %d\n", GetLastError());
			else
			{
				memset(&commProp, 0, sizeof(commProp));
				bResult = GetCommProperties(m_handle, &commProp);

				if (!bResult)
					Logger.DebugOut("OpenPort GetCommProperties failed %d\n", GetLastError());
				else
					Logger.PrintCommProp(commProp);
			}
		}

		// clear comm error
		memset(&comStat, 0, sizeof(comStat));
		ClearCommError(m_handle, &dwError, &comStat);
	}
	return m_handle != NULL&& m_handle != INVALID_HANDLE_VALUE;
}

// read a number of bytes from Serial Bus Device
// Parameters:
//	lpBytes - Pointer to the buffer
//	dwLen   - number of bytes to read
// Return:	Number of byte read from the device.  
//
DWORD ComHelper::Read(LPBYTE lpBytes, DWORD dwLen)
{
	LPBYTE p = lpBytes;
    DWORD Length = dwLen;
    DWORD dwRead = 0;
    DWORD dwTotalRead = 0;

	Logger.DebugOut("ComHelper::Read Prepare to read %ld bytes\n", dwLen);

    // Loop here until request is fulfilled
	while (Length)
    {
        DWORD dwRet = WAIT_TIMEOUT;
        dwRead = 0;
        ResetEvent(m_OverlapRead.hEvent);
        m_OverlapRead.Internal = ERROR_SUCCESS;
        m_OverlapRead.InternalHigh = 0;
        if (!ReadFile(m_handle, (LPVOID)p, Length, &dwRead, &m_OverlapRead))
        {
			Logger.DebugOut("ComHelper::ReadFile returned with %ld\n", GetLastError());

			// Overlapped IO returns FALSE with ERROR_IO_PENDING
            if (GetLastError() != ERROR_IO_PENDING)
            {
				Logger.DebugOut("ComHelper::ReadFile failed with %ld\n", GetLastError());
                break;
            }

            //Clear the LastError and wait for the IO to Complete
            SetLastError(ERROR_SUCCESS);
            dwRet = WaitForSingleObject(m_OverlapRead.hEvent, 10000);
			Logger.DebugOut("ComHelper::WaitForSingleObject returned with %ld\n", dwRet);

            // IO completed, retrieve Overlapped result
            GetOverlappedResult(m_handle, &m_OverlapRead, &dwRead, TRUE);

			// if dwRead is not updated, retrieve it from OVERLAPPED structure
            if (dwRead == 0)
                dwRead = (DWORD)m_OverlapRead.InternalHigh;
        }

        if (dwRead > Length)
            break;
        p += dwRead;
        Length -= dwRead;
        dwTotalRead += dwRead;
    }

	Logger.DebugOut("dwLen = %d TotalRead = %d\n", dwLen, dwTotalRead);
    return dwTotalRead;
}

// Write a number of bytes to Serial Bus Device
// Parameters:
//	lpBytes - Pointer to the buffer
//	dwLen   - number of bytes to write
// Return:	Number of byte Written to the device.  
//
DWORD ComHelper::Write(LPBYTE lpBytes, DWORD dwLen)
{
	LPBYTE p = lpBytes;
    DWORD Length = dwLen;
    DWORD dwWritten = 0;
    DWORD dwTotalWritten = 0;

	Logger.DebugOut("ComHelper::Write Prepare to Write %ld bytes\n", dwLen);
    while (Length)
    {
        dwWritten = 0;
        SetLastError(ERROR_SUCCESS);
        ResetEvent(m_OverlapWrite.hEvent);
        if (!WriteFile(m_handle, p, Length, &dwWritten, &m_OverlapWrite))
        {
            if (GetLastError() != ERROR_IO_PENDING)
            {
				Logger.DebugOut("ComHelper::WriteFile failed with %ld\n", GetLastError());
                break;
            }
            DWORD dwRet = WaitForSingleObject(m_OverlapWrite.hEvent, 5000);
            if (dwRet != WAIT_OBJECT_0)
            {
				Logger.DebugOut("ComHelper::Write WaitForSingleObject failed with %ld\n", GetLastError());
                break;
            }
            GetOverlappedResult(m_handle, &m_OverlapWrite, &dwWritten, FALSE);
        }
        if (dwWritten > Length)
            break;
        p += dwWritten;
        Length -= dwWritten;
        dwTotalWritten += dwWritten;
    }

	Logger.DebugOut("dwLen = %d TotalWritten = %d\n", dwLen, dwWritten);
    return dwTotalWritten;
}

