// WsUartFu.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Windows.h>
#include <conio.h>

COMMTIMEOUTS  OrigTimeOuts;

// downloader will use 16 byte chunks
#define WS_UPGRADE_READ_CHUNK                   15

HANDLE Connect (char *port_name)
{
	HANDLE hDevice = CreateFile(port_name,
								 GENERIC_READ|GENERIC_WRITE,
								 0,								// dwShareMode
								 NULL,							// lpSecurityAttributes
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
								 NULL);							// hTemplateFile
	if (hDevice == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: failed to open COM port:%s Err:%d\n", port_name, err);
        return NULL;
    }

    // Read current Timeouts
    if (!GetCommTimeouts (hDevice, &OrigTimeOuts))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to read original port timeout settings, err:%d!", err);
        CloseHandle(hDevice);
        return NULL;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);

    // Read current DCB values
    if (!GetCommState(hDevice, &dcb))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to determine original DCB settings, err:%d!", err);
        CloseHandle(hDevice);
        return NULL;
    }

    dcb.BaudRate            = 115200;
    dcb.ByteSize            = 8;
    dcb.Parity              = NOPARITY; 
    dcb.StopBits            = ONESTOPBIT; 

	dcb.EvtChar             = '\0';

    dcb.fDtrControl         = 1;
    dcb.fRtsControl         = 1;
    dcb.fOutxCtsFlow        = 0;
    dcb.fOutxDsrFlow        = 0;
    dcb.fDsrSensitivity     = 0;
    dcb.fOutX               = 0;
    dcb.fInX                = 0;
    dcb.fTXContinueOnXoff   = 0;
    dcb.XonChar             = 17;
    dcb.XoffChar            = 19;
    dcb.XonLim              = 0;
    dcb.XoffLim             = 0;

    // set new state
    if (!SetCommState(hDevice, &dcb))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to set new comm settings, err:%d!", err);
        CloseHandle(hDevice);
        return NULL;
    }

    COMMTIMEOUTS  TimeOuts;

    TimeOuts.ReadIntervalTimeout            = 1;
    TimeOuts.ReadTotalTimeoutConstant       = 0;
    TimeOuts.ReadTotalTimeoutMultiplier     = 0;
    TimeOuts.WriteTotalTimeoutConstant      = 0;
    TimeOuts.WriteTotalTimeoutMultiplier    = 0;

    if (!SetCommTimeouts (hDevice, &TimeOuts))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to change timeout settings, err:%d!", err);
        CloseHandle(hDevice);
        return NULL;
    }

    SetupComm (hDevice, 4096, 4096);

    if (!EscapeCommFunction(hDevice, SETDTR))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to set DTR, err:%d!", err);
        CloseHandle(hDevice);
        return NULL;
    }

    COMSTAT     ComStat;
    DWORD       dwErrors;
    ClearCommError(hDevice, &dwErrors, &ComStat);

    return hDevice;
}

BOOL ReleaseCommPort(HANDLE hDevice)
{
    // lower DTR
    if (!EscapeCommFunction(hDevice, CLRDTR))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to clear DTR, err:%d!", err);
    }

    // restore original comm timeouts
    if (!SetCommTimeouts(hDevice, &OrigTimeOuts ))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to restore original timeouts, err:%d!", err);
    }

    // Purge reads/writes, input buffer and output buffer
    if (!PurgeComm(hDevice, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR))
    {
        DWORD err = GetLastError();
        printf ("WsUartFu: Failed to Purge COM, err:%d!", err);
    }

    CloseHandle (hDevice);
    return TRUE;
}

BOOL WriteBytes(HANDLE hDevice, BYTE *lpBuf, DWORD dwToWrite)
{
    OVERLAPPED  osWrite = {0};
    DWORD       dwWritten = 0;
    DWORD       dwRes;

    // create this writes overlapped structure hEvent
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    // issue write
    if (!WriteFile(hDevice, lpBuf, dwToWrite, &dwWritten, &osWrite)) 
    {
        if (GetLastError() == ERROR_IO_PENDING) 
        { 
            // write is delayed
            dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
            if (dwRes == WAIT_OBJECT_0)
            {
                GetOverlappedResult(hDevice, &osWrite, &dwWritten, FALSE);
            }
        }
    }
    CloseHandle(osWrite.hEvent);
    return (dwWritten == dwToWrite);
}

BOOL ReadByte(HANDLE hDevice, BYTE *lpBuf)
{
    OVERLAPPED osRead = {0};    // overlapped structure for read operations
    DWORD 	   dwRead = 0;          // bytes actually read
    DWORD      dwRes;           // result from WaitForSingleObject

    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!ReadFile(hDevice, lpBuf, 1, &dwRead, &osRead)) 
    {
        if (GetLastError() != ERROR_IO_PENDING)	  // read not delayed?
            return 0;

        dwRes = WaitForSingleObject(osRead.hEvent, 5000);
        if (dwRes == WAIT_OBJECT_0)
        {
            if (!GetOverlappedResult(hDevice, &osRead, &dwRead, FALSE)) 
            {
                printf("WsUartFu: Read failed\n");
            }
        }
        else
        {
            printf("WsUartFu: Read timeout\n");
        }
    }
    CloseHandle (osRead.hEvent);
    return (dwRead == 1);
}

#define POLYNOMIAL			    0x04C11DB7
#define WIDTH                   (8 * sizeof(unsigned long))
#define TOPBIT                  (1 << (WIDTH - 1))
#define INITIAL_REMAINDER	    0xFFFFFFFF
#define FINAL_XOR_VALUE		    0xFFFFFFFF
#define REFLECT_DATA(X)	        ((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)	((unsigned long) reflect((X), WIDTH))
#define CHECK_VALUE			    0xCBF43926

/*********************************************************************
 *
 * Function:    reflect()
 * 
 * Description: Reorder the bits of a binary sequence, by reflecting
 *				them about the middle position.
 *
 * Notes:		No checking is done that nBits <= 32.
 *
 * Returns:		The reflection of the original data.
 *
 *********************************************************************/
static unsigned long reflect(unsigned long data, unsigned char nBits)
{
	unsigned long  reflection = 0x00000000;
	unsigned char  bit;

    // Reflect the data about the center bit.
	for (bit = 0; bit < nBits; ++bit)
	{
        // If the LSB bit is set, set the reflection of it.
		if (data & 0x01)
		{
			reflection |= (1 << ((nBits - 1) - bit));
		}
		data = (data >> 1);
	}
	return (reflection);

}	/* reflect() */



unsigned long crcSlow(unsigned long  crc32, unsigned char const message[], int nBytes)
{
	int            byte;
	unsigned char  bit;

    // Perform modulo-2 division, a byte at a time.
    for (byte = 0; byte < nBytes; ++byte)
    {
        // Bring the next byte into the crc32.
        crc32 ^= (REFLECT_DATA(message[byte]) << (WIDTH - 8));

        // Perform modulo-2 division, a bit at a time.
        for (bit = 8; bit > 0; --bit)
        {
            // Try to divide the current data bit.
            if (crc32 & TOPBIT)
            {
                crc32 = (crc32 << 1) ^ POLYNOMIAL;
            }
            else
            {
                crc32 = (crc32 << 1);
            }
        }
    }
    return crc32;

}   /* crcSlow() */

unsigned long crcComplete(unsigned long crc32)
{
    // The final crc32 is the CRC result.
    return (REFLECT_REMAINDER(crc32) ^ FINAL_XOR_VALUE);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf ("WsUartFu: Usage: WsUartFu <image.bin> <COMx>\n");
        return 0;
    }

    printf ("\nWARNING:  If EEPROM or Serial Flash installed on the device is less then 64\n");
    printf ("KBytes, the memory after the upgrade might be corrupted.  Use the recovery\n");
    printf ("procedure described in the Quick Start Guide to continue using the device.\n\n");
    printf ("Click y to continue or any other key to exit\n");

    int c = _getch();
    if (c != 'y')
        return 0;

	char port_name[16];
	strcpy_s(port_name, 16, "\\\\.\\");
	strcat_s(port_name, 12, argv[2]);

    HANDLE hDevice;
    if ((hDevice = Connect(port_name)) == NULL)
    {
        return NULL;
    }

    FILE *fPatch;
    if (fopen_s(&fPatch, argv[1], "rb"))
    {
        printf ("WsUartFu: failed to open image file:%s\n", argv[1]);
        ReleaseCommPort(hDevice);
        return 0;
    }
    // read private key
    fseek(fPatch, 0, SEEK_END);
    DWORD fileSize = ftell(fPatch);
    rewind(fPatch);
    LPBYTE pPatch = (LPBYTE)new BYTE[fileSize];

    DWORD readBytes = fread(pPatch, 1, fileSize, fPatch);
    fclose(fPatch);

#define WS_UPGRADE_COMMAND_START    0x33
#define WS_UPGRADE_COMMAND_FINISH   0x55
#define WS_UPGRADE_EVENT_OK         0xF0

    BYTE startCommand[] = {WS_UPGRADE_COMMAND_START, 0, 0};
    startCommand[1] = fileSize & 0xff;
    startCommand[2] = (fileSize >> 8) & 0xff;
    if (!WriteBytes (hDevice, startCommand, sizeof (startCommand)))
    {
        printf ("WsUartFu: failed to write start command\n");
        ReleaseCommPort(hDevice);
        return 0;
    }
    BYTE ReadBuf[2];
    if (!ReadByte (hDevice, ReadBuf) || (ReadBuf[0] != WS_UPGRADE_EVENT_OK))
    {
        printf ("WsUartFu: failed to start\n");
        ReleaseCommPort(hDevice);
        return 0;
    }
    unsigned int i;
    unsigned long  crc32 = INITIAL_REMAINDER;
    for (i = 0; i < readBytes; i += WS_UPGRADE_READ_CHUNK)
    {
        int bytesToWrite = i + WS_UPGRADE_READ_CHUNK < readBytes ? WS_UPGRADE_READ_CHUNK : readBytes - i;
        crc32 = crcSlow(crc32, &pPatch[i], bytesToWrite);
        if (!WriteBytes (hDevice,  &pPatch[i], bytesToWrite))
        {
            printf ("WsUartFu: failed to write at %d\n", i);
            ReleaseCommPort(hDevice);
            return 0;
        }
        if (!ReadByte (hDevice, ReadBuf) || (ReadBuf[0] != WS_UPGRADE_EVENT_OK))
        {
            printf ("WsUartFu: failed to read at:%d\n", i);
            ReleaseCommPort(hDevice);
            return 0;
        }
    }
    crc32 = crcComplete(crc32);

    BYTE finishCommand[] = {WS_UPGRADE_COMMAND_FINISH, 0, 0, 0, 0};
    finishCommand[1] =  crc32        & 0xff;
    finishCommand[2] = (crc32 >> 8)  & 0xff;
    finishCommand[3] = (crc32 >> 16) & 0xff;
    finishCommand[4] = (crc32 >> 24) & 0xff;
        
    if (!WriteBytes (hDevice, finishCommand, sizeof (finishCommand)))
    {
        printf ("WsUartFu: failed to write finish\n");
        ReleaseCommPort(hDevice);
        return 0;
    }
    if (!ReadByte (hDevice, ReadBuf))
    {
        printf ("WsUartFu: failed to read finish ack:%d\n");
        ReleaseCommPort(hDevice);
        return 0;
    }
    else
    {
        printf ("WsUartFu: upgrade status %s\n", (ReadBuf[0] == WS_UPGRADE_EVENT_OK) ? "Success" : "Failed");
        ReleaseCommPort(hDevice);
        return 1;
    }
}

