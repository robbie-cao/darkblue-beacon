#ifndef _BLEHRM_H_
#define _BLEHRM_H_
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
* BLE Heart Rate profile, service, application 
*
* This file contains definitions and function ptototypes for BLE Heart
* Rate Monitor implementation.
*
* Refer to Bluetooth SIG Heart Rate Profile 1.0 abd Heart Rate Service 1.0
* specifications for details.
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

#define RR_MAX_NUM 9

//flag enum
enum blehrm_flag
{
    HRM_HEARTRATE_VALUE = 0x01, //if set, 16 bit. not set, 8 bit
    HRM_SENSOR_CONTACT = 0x02, // if set, sensor is detected
    HRM_SENSOR_CONTACT_SUPPORT = 0x04, //if set, sensor contact is supported 
    HRM_ENERGY_EXPENDED_STATUS = 0x08,
    HRM_RR_INTERVAL_SUPPORT = 0x10,
};

// sensor location
enum blehrm_sensorlocation
{
    HRM_LOC_START = 0x00,
    HRM_LOC_CHEST = 0x01,
    HRM_LOC_WRIST = 0x02,
    HRM_LOC_FINGER = 0x03,
    HRM_LOC_HAND = 0x04,
    HRM_LOC_EARLOBE = 0x05,
    HRM_LOC_FOOT = 0x06,    
    HRM_LOC_END = 0x06,
};

// CP Op Code
enum blehrm_opcode
{
    HRM_CP_RESET_ENERGY_EXPENDED = 0x01,
};

typedef PACKED struct
{
    UINT8 flag;
    UINT16 hrm;
    UINT16 ee;
    UINT16 rr[RR_MAX_NUM];
    UINT8 rr_len;
}  BLEHRM_HRM_DATA;

typedef PACKED struct
{
    UINT8     opcode;
} BLEHRM_CP_HDR;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blehrm_Create(void);

extern const UINT8 blehrm_db_data[];
extern const UINT16 blehrm_db_size;
extern const BLE_PROFILE_CFG blehrm_cfg;
extern const BLE_PROFILE_PUART_CFG blehrm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blehrm_gpio_cfg;

#endif // end of #ifndef _BLEHRM_H_

