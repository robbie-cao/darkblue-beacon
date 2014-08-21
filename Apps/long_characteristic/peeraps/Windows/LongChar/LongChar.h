
// LongChar.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "..\..\..\long_characteristic.h"

// CLongCharApp:
// See LongChar.cpp for the implementation of this class
//

class CLongCharApp : public CWinApp
{
public:
	CLongCharApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

#define BTW_GATT_UUID_SERVCLASS_BATTERY                     0x180F    /* Battery Service  */
#define BTW_GATT_UUID_SERVCLASS_DEVICE_INFO                 0x180A    /* Device Information Service  */

#define BTW_GATT_UUID_CHAR_DIS_SYSTEM_ID                    0x2A23
#define BTW_GATT_UUID_CHAR_DIS_MODEL_NUMBER                 0x2A24
#define BTW_GATT_UUID_CHAR_DIS_SERIAL_NUMBER                0x2A25
#define BTW_GATT_UUID_CHAR_DIS_FIRMWARE_REVISION            0x2A26
#define BTW_GATT_UUID_CHAR_DIS_HARDWARE_REVISION            0x2A27
#define BTW_GATT_UUID_CHAR_DIS_SOFTWARE_REVISION            0x2A28
#define BTW_GATT_UUID_CHAR_DIS_MANUFACTURER_NAME            0x2A29

#define BTW_GATT_UUID_CHAR_BATTERY_LEVEL                    0x2A19
#define BTW_GATT_UUID_CHAR_BATTERY_STATE                    0x2A1A
#define BTW_GATT_UUID_CHAR_BATTERY_STATELEVEL               0x2A1B

#define BTW_GATT_UUID_DESCRIPTOR_USER_DESCRIPTION           0x2901
#define BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG              0x2902      /*  Client Characteristic Configuration */
#define BTW_GATT_UUID_DESCRIPTOR_NUMBER_OF_DIGITALS         0x2909
#define BTW_GATT_UUID_DESCRIPTOR_TRIGGER_SETTING            0x290A

       
#define WM_CONNECTED            (WM_USER + 0xff)
#define WM_LONG_CHAR            (WM_USER + 0xfe)


const GUID guidBT                          = {0,                                         0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
extern GUID guidSvcLongChar; 
extern GUID guidCharLongChar;

extern void ods(char * fmt_str, ...);
extern void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha);
extern void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid);

extern CLongCharApp theApp;
