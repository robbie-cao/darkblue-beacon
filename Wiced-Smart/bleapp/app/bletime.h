#ifndef _BLETIME_H_
#define _BLETIME_H_
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
* File Name: bletime.h
*
* Abstract: This file implements the BLE Time profile, service, application
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


#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void bletime_Create(void);
void bletime_Init(LEATT_TRIPLE_PARAM_CB cb);
UINT32 bletime_TimeButton(UINT32 function);
void bletime_TimeHandleReset(void);
void bletime_TimeReq(void);

extern const BLE_PROFILE_CFG bletime_cfg;
extern const BLE_PROFILE_PUART_CFG bletime_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG bletime_gpio_cfg;

typedef struct
{
    //NVRAM save area
    BLEPROFILE_HOSTINFO bletime_hostinfo;

    UINT32 bletime_apptimer_count;
    UINT32 bletime_appfinetimer_count;
    UINT16 bletime_con_handle;
    BD_ADDR  bletime_remote_addr;

    UINT8 bletime_button_count;

    UINT8 bletime_bat_enable;

    UINT8 bletime_client_state;
    UINT16 bletime_s_handle;
    UINT16 bletime_e_handle;
    UINT16 bletime_ct_hdl;
    UINT16 bletime_ct_client_hdl;
    UINT16 bletime_ct_client_val;

    LEATT_TRIPLE_PARAM_CB bletime_cb;
} tTimeAppState;


#endif // end of #ifndef _BLETIME_H_

