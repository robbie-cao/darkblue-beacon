
// WsOtaUpgrade.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWsOtaUpgradeApp:
// See WsOtaUpgrade.cpp for the implementation of this class
//

class CWsOtaUpgradeApp : public CWinApp
{
public:
	CWsOtaUpgradeApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

#define BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG                    0x2902      /*  Client Characteristic Configuration */
const GUID guidClntConfigDesc                                   = {BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, 0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

extern GUID guidSvcWSUpgrade;
extern GUID guidCharWSUpgradeControlPoint;
extern GUID guidCharWSUpgradeData;

// {9E5D1E47-5C13-43A0-8635-82AD38A1386F}
static const GUID GUID_WS_UPGRADE_SERVICE                       = { 0x9e5d1e47, 0x5c13, 0x43a0, { 0x86, 0x35, 0x82, 0xad, 0x38, 0xa1, 0x38, 0x6f } };

// {E3DD50BF-F7A7-4E99-838E-570A086C666B}
static const GUID GUID_WS_UPGRADE_CHARACTERISTIC_CONTROL_POINT  = { 0xe3dd50bf, 0xf7a7, 0x4e99, { 0x83, 0x8e, 0x57, 0xa, 0x8, 0x6c, 0x66, 0x6b } };

// {92E86C7A-D961-4091-B74F-2409E72EFE36}
static const GUID GUID_WS_UPGRADE_CHARACTERISTIC_DATA           = { 0x92e86c7a, 0xd961, 0x4091, { 0xb7, 0x4f, 0x24, 0x9, 0xe7, 0x2e, 0xfe, 0x36 } };

extern void ods(char * fmt_str, ...);
extern void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha);
extern void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid);

extern CWsOtaUpgradeApp theApp;
