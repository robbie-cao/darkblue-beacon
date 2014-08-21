#ifndef _BLEGEN_H_
#define _BLEGEN_H_
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
* BLE Generic profile, service, application
*
* This file provides definitions and function prototypes for the BLE 
* generic device
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

#define GEN_DATA_MAX_LEN LEATT_ATT_MTU

typedef PACKED struct
{
    UINT8 data[GEN_DATA_MAX_LEN];
}  BLEGEN_GEN_DATA;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blegen_Create(void);

extern const UINT8 blegen_db_data[];
extern const UINT16 blegen_db_size;
extern const BLE_PROFILE_CFG blegen_cfg;
extern const BLE_PROFILE_PUART_CFG blegen_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blegen_gpio_cfg;

#endif // end of #ifndef _BLEGEN_H_

