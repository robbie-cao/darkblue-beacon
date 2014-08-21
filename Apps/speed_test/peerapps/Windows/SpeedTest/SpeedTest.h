
// SpeedTest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "..\..\..\speed_test.h"

// CSpeedTestApp:
// See SpeedTest.cpp for the implementation of this class
//

class CSpeedTestApp : public CWinApp
{
public:
	CSpeedTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

#define BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG                    0x2902      /*  Client Characteristic Configuration */
const GUID guidClntConfigDesc   = {BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, 0, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

extern GUID guidSvcSpeedTest;
extern GUID guidCharSpeedTestControlPoint;
extern GUID guidCharSpeedTestData;

extern void ods(char * fmt_str, ...);
extern void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha);
extern void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid);

extern CSpeedTestApp theApp;
