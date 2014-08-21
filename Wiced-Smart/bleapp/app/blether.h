#ifndef _BLETHER_H_
#define _BLETHER_H_
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
* BLE Health Thermometer profile, service, application 
*
* This is a ROM code for Health Thermometer device.
*
* This file contains definitions and function ptototypes for BLE Health
* Thermometer device implementation.
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
enum blether_flag
{
    THER_MEASUREMENT_VALUE      = 0x01,  //this is set, Fahrenheit
    THER_TIME_STAMP             = 0x02,
    THER_TEMPERATURE_TYPE       = 0x04,
};

//temperature type
enum blether_temperature_type
{
    THER_ARMPIT                  = 1,
    THER_BODY                    = 2,
    THER_EAR                     = 3,
    THER_FINGER                  = 4,
    THER_GASTRO_INTESTINAL_TRACT = 5,
    THER_MOUTH                  = 6,
    THER_RECTUM                 = 7,
    THER_TOE                    = 8,
    THER_TYMPANUM               = 9,
};

typedef PACKED struct
{
    UINT8       flag;
    FLOAT32     tempmeasure;
    TIMESTAMP   timestamp; //7 bytes
    UINT8       temptype;	
}  BLETHER_THER_DATA;

//host information for NVRAM
typedef PACKED struct
//typedef struct
{
    // BD address of the bonded host
    BD_ADDR     bdAddr;
    UINT16      serv[3];
    UINT16      cha[3];
    UINT16      cli_cha_desc[3];	
}  BLETHER_HOSTINFO;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blether_Create(void);

extern const UINT8                  blether_db_data[];
extern const UINT16                 blether_db_size;
extern const BLE_PROFILE_CFG        blether_cfg;
extern const BLE_PROFILE_PUART_CFG  blether_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG   blether_gpio_cfg;

#endif // end of #ifndef _BLETHER_H_

