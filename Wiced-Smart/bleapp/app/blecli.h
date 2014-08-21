#ifndef _BLECLI_H_
#define _BLECLI_H_
/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP 
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: blecli.h
*
* Abstract: This file implements the BLE Generic client profile, service, application
*
*
* Functions:
*
*******************************************************************************/
#include "bleprofile.h"



//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

#define CLIENT_HANDLE_NUM_MAX 3

// Client handle 
typedef PACKED struct
{
    UINT16 svc_uuid;
    UINT16 cha_uuid;
    UINT16 desc_uuid;
    UINT16 cha_hdl;
    UINT16 desc_hdl;
}  BLE_CLIENT_HANDLE;



#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()


//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecli_Init(void);
UINT8 blecli_ClientButton(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid, 
	UINT8 action, UINT8 *data, UINT8 len, LEATT_TRIPLE_PARAM_CB cb);
void blecli_ClientHandleReset(void);
void blecli_ClientReq(void);
void blecli_ClientHandleTest(void);
INT8 blecli_ClientGetIndex(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid);
UINT16 blecli_ClientGetHandle(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid);
UINT8 blecli_ClientSetHandle(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid, UINT16 hdl);
UINT8 blecli_ClientReplaceHandle(INT8 index, UINT16 desc_uuid, UINT16 new_hdl);

#endif // end of #ifndef _BLECLI_H_
