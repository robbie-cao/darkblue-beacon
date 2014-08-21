#ifndef _BLEBPM_H_
#define _BLEBPM_H_
/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
*/

/** @file
*
* BLE Blood Pressure profile, service, application 
*
* This file contains definitions and function ptototypes for BLE Blood
* Pressure Monitor implementation.
*
*/
#include "bleprofile.h"

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

//flag enum
enum blebpm_flag
{
    BPM_UNIT_KPA            = 0x01,
    BPM_TIME_STAMP          = 0x02,
    BPM_PULSE_RATE          = 0x04,
    BPM_USER_ID             = 0x08,
    BPM_MEASUREMENT_STATUS  = 0x10,
};


typedef PACKED struct
{
    UINT8       flag;
    SFLOAT      systolic;
    SFLOAT      diastolic;
    SFLOAT      mat;
    TIMESTAMP   timestamp; //7bytes
    SFLOAT      pulserate;
    UINT8       userid;
    UINT16      measurementstatus;
}  BLEBPM_BPM_DATA;

//host information for NVRAM
typedef PACKED struct
//typedef struct
{
    // BD address of the bonded host
    BD_ADDR     bdAddr;
    UINT16      serv[2];
    UINT16      cha[2];
    UINT16      cli_cha_desc[2];	
}  BLEBPM_HOSTINFO;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blebpm_Create(void);

extern const UINT8                  blebpm_db_data[];
extern const UINT16                 blebpm_db_size;
extern const BLE_PROFILE_CFG        blebpm_cfg;
extern const BLE_PROFILE_PUART_CFG  blebpm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG   blebpm_gpio_cfg;

#endif // end of #ifndef _BLEBPM_H_

