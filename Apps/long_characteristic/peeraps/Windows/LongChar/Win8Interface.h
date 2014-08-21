
// LongCharDlg.h : header file
//

#pragma once
#include "BtInterface.h"

class CBtWin8Interface : public CBtInterface
{
public:
    CBtWin8Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext);
    virtual ~CBtWin8Interface();

    BOOL Init(GUID uuidServ);
    BOOL GetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue);
    BOOL SetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue);
    BOOL ReadCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue);
    BOOL WriteCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue);
    BOOL CBtWin8Interface::FindChar(GUID uuid, USHORT inx, PBTH_LE_GATT_CHARACTERISTIC pOutChar);
    VOID FindChar(GUID uuid, PBTH_LE_GATT_CHARACTERISTIC pOutChar, PBTH_LE_GATT_DESCRIPTOR pOutDescr, USHORT num, PBTH_LE_GATT_CHARACTERISTIC pChars);
    PBTH_LE_GATT_DESCRIPTOR FindDescr(GUID uuidChar, USHORT inx, USHORT uuidDescr);

    void RegisterNotification();
    void PostNotification(BTW_GATT_VALUE *pValue, BLUETOOTH_GATT_VALUE_CHANGED_EVENT *pEvent, PVOID EventOutParameter, PVOID Context);

    BOOL m_bConnected;
private:
    HANDLE m_hDevice;

    BTH_LE_GATT_SERVICE m_service;

    USHORT num_chars;
    BTH_LE_GATT_CHARACTERISTIC *m_pchar;

    USHORT *pnum_descr;
    BTH_LE_GATT_DESCRIPTOR **m_ppdescr;

    BLUETOOTH_GATT_EVENT_HANDLE m_pEventHandle;
};

