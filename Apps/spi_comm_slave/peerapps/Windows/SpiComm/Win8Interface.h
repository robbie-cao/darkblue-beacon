
// Win8Interface.h : header file
//

#pragma once
#include "BtInterface.h"

class CBtWin8Interface : public CBtInterface
{
public:
    CBtWin8Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext);
    virtual ~CBtWin8Interface();

    BOOL Init();
    BOOL SetDescriptors();
    BOOL SendSpiCommCommand(BYTE Command);
    BOOL SendSpiCommCommand(BYTE Command, USHORT sParam);
    BOOL SendSpiCommCommand(BYTE Command, ULONG lParam);
    BOOL SendSpiCommData(BYTE *Data, DWORD len);

    void RegisterNotification();
    void HandleNotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter);

    BOOL m_bConnected;
private:
    HANDLE m_hDevice;
    BTH_LE_GATT_SERVICE         m_service;
    BLUETOOTH_GATT_EVENT_HANDLE m_pEventHandle;
    BTH_LE_GATT_CHARACTERISTIC  m_charSpiCommControlPoint;
    BTH_LE_GATT_DESCRIPTOR      m_descrSpiControlClientConfig;
    BTH_LE_GATT_CHARACTERISTIC  m_charSpiCommData;
    BTH_LE_GATT_DESCRIPTOR      m_descrSpiDataClientConfig;
};

