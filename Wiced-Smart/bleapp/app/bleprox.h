#ifndef _BLEPROX_H_
#define _BLEPROX_H_
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
* BLE Proximity client profile, service, application
*
* Refer to Bluetooth SIG Proximity Profile1.0 specifications for details.
*
* This file contains definitions and function declarations for the proximity
* profile.
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


//Alert mode
enum ble_prox_alert_level
{
    NO_ALERT                        = 0,
    MILD_ALERT                      = 1,
    HIGH_ALERT                      = 2,
    UNDIRECTED_DISCOVERABLE_ALERT   = 3,
}; 

//host information for NVRAM
typedef PACKED struct
{
    BD_ADDR     bdAddr;             // BD address of the bonded host
    UINT16      findme_hdl;         // Handle of immediate alert characteristic used by findme logic
}  BLEPROX_HOSTINFO;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()


//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void   bleprox_Create(void);
UINT32 bleprox_ProxButton(UINT32 function);

extern const UINT8 bleprox_db_data[];
extern const UINT16 bleprox_db_size;
extern const BLE_PROFILE_CFG bleprox_cfg;
extern const BLE_PROFILE_PUART_CFG bleprox_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG bleprox_gpio_cfg;

#endif // end of #ifndef _BLEPROX_H_

