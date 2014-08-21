
// BtInterface.h : header file
//

#pragma once

class CBtInterface
{
public:
    CBtInterface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) {m_bth = *bth; m_hLib = hLib; m_NotificationContext = NotificationContext;};

    virtual BOOL Init() = NULL;
    virtual BOOL GetDescriptorValue(USHORT *DescriptorValue) = NULL;
    virtual BOOL SetDescriptorValue(USHORT Value) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command, USHORT sParam) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command, ULONG lParam) = NULL;
    virtual BOOL SendWsUpgradeData(BYTE *Data, DWORD len) = NULL;

    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    LPVOID m_NotificationContext;
}; 

