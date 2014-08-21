
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
    BOOL SetDescriptors(USHORT Control, USHORT Data);
    BOOL SendSpeedTestCommand(BYTE Command);
    BOOL SendSpeedTestCommand(BYTE Command, BYTE bParam);
    BOOL SendSpeedTestCommand(BYTE Command, USHORT sParam);
    BOOL SendSpeedTestCommand(BYTE Command, ULONG lParam);
    BOOL SendSpeedTestData(BYTE *Data, DWORD len);

    void RegisterNotification();
    void HandleNotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter);

    BOOL m_bConnected;
private:
    HANDLE m_hDevice;
    HANDLE m_hService;
    BTH_LE_GATT_SERVICE         m_service;
    BLUETOOTH_GATT_EVENT_HANDLE m_pEventHandle;
    BTH_LE_GATT_CHARACTERISTIC  m_charSpeedTestControlPoint;
    BTH_LE_GATT_DESCRIPTOR      m_descrSpeedTestControlClientConfig;
    BTH_LE_GATT_CHARACTERISTIC  m_charSpeedTestData;
    BTH_LE_GATT_DESCRIPTOR      m_descrSpeedTestDataClientConfig;
};

