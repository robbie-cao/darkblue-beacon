
// BtInterface.h : header file
//

#pragma once

class CBtInterface
{
public:
    CBtInterface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) 
    {
        m_bConnected = FALSE;
        m_bth = *bth; 
        m_hLib = hLib; 
        m_NotificationContext = NotificationContext;
    };

    virtual BOOL Init() = NULL;
    virtual BOOL SetDescriptors() = NULL;
    virtual BOOL SendSpiCommCommand(BYTE Command) = NULL;
    virtual BOOL SendSpiCommCommand(BYTE Command, USHORT sParam) = NULL;
    virtual BOOL SendSpiCommCommand(BYTE Command, ULONG lParam) = NULL;
    virtual BOOL SendSpiCommData(BYTE *Data, DWORD len) = NULL;

    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    LPVOID m_NotificationContext;
    BOOL m_bConnected;
}; 

