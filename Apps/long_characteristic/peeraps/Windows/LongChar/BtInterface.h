
// LongCharDlg.h : header file
//

#pragma once

class CBtInterface
{
public:
    CBtInterface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) {m_bth = *bth; m_hLib = hLib; m_NotificationContext = NotificationContext;};

    virtual BOOL Init(GUID uuidServ) = NULL;
    virtual BOOL GetDescriptorValue(GUID  uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue) = NULL;
    virtual BOOL SetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue) = NULL;
    virtual BOOL ReadCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue) = NULL;
    virtual BOOL WriteCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue) = NULL;
    BOOL CBtInterface::GetDescriptorClientConfigValue(GUID uuidChar, USHORT inx, USHORT *Value);
    BOOL CBtInterface::SetDescriptorClientConfigValue(GUID uuidChar, USHORT inx, USHORT Value);

    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    LPVOID m_NotificationContext;
    GUID m_guid;
}; 

