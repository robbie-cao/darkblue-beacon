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
* BLE Proximity profile, service, application
*
* These are updates to the ROM code for BLE Proximity device.
*
* Refer to Bluetooth SIG Proximity Profile 1.0 specifications for details.
*
* The ROM code is published in Wiced-Smart\bleapp\app\bleprox.c file. This
* code replaces GATT database.
*
* Features demonstrated
*  - Proximity implementation
*  - Replacing some of the ROM functionality
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "bleprox.h"

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 proximity_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
        0x00, 0x00, 0x00, 0x00,

    // GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','P','r','o','x',' ','k','e','y',' ','f','o','b',

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00,0x00,

    // Link Loss service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_LINK_LOSS),

    CHARACTERISTIC_UUID16_WRITABLE (0x0029, 0x002a, UUID_CHARACTERISTIC_ALERT_LEVEL,
                                     LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ,  1),
        0x01,

    // Immediate alert service
    PRIMARY_SERVICE_UUID16 (0x002B, UUID_SERVICE_IMMEDIATE_ALERT),

    CHARACTERISTIC_UUID16_WRITABLE (0x002c, 0x002d, UUID_CHARACTERISTIC_ALERT_LEVEL,
                                     LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE, LEGATTDB_PERM_WRITE_CMD,  1),
        0x00,

    // Tx Power service
    PRIMARY_SERVICE_UUID16 (0x002e, UUID_SERVICE_TX_POWER),

    CHARACTERISTIC_UUID16 (0x002f, 0x0030, UUID_CHARACTERISTIC_TX_POWER_LEVEL,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE,  1),
        0x04,                       // this should be matched to ADV data

    // Battery service
    PRIMARY_SERVICE_UUID16 (0x0031, UUID_SERVICE_BATTERY),

    CHARACTERISTIC_UUID16 (0x0032, 0x0033, UUID_CHARACTERISTIC_BATTERY_LEVEL,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE,  1),
        0x64,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0034, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x041, 0x0042, UUID_CHARACTERISTIC_BATTERY_POWER_STATE,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 1),
        BLEBAT_POWERSTATE_PRESENT_PRESENT|
        BLEBAT_POWERSTATE_DISCHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_CHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_LEVEL_GOODLEVEL,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0043, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0044, 0x0045, UUID_CHARACTERISTIC_SERVICE_REQUIRED,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 1),
        BLEBAT_SERVICEREQUIRED_NOSERVICEREQUIRED,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0046, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0047, 0x0048, UUID_CHARACTERISTIC_REMOVABLE,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE,  1),
        BLEBAT_REMOVABLE_UNKNOWN,

    CHARACTERISTIC_UUID16 (0x004a, 0x004b, UUID_CHARACTERISTIC_BATTERY_LEVEL_STATE,
                           LEGATTDB_CHAR_PROP_BROADCAST | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 5),
        0x64,           // Level
        BLEBAT_POWERSTATE_PRESENT_PRESENT|
        BLEBAT_POWERSTATE_DISCHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_CHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_LEVEL_GOODLEVEL,
        0x00,           // Namespace
        0x00, 0x00,     // Description

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x004C, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x004D, UUID_DESCRIPTOR_SERVER_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)proximity_db_data, sizeof(proximity_db_data), (void *)&bleprox_cfg,
       (void *)&bleprox_puart_cfg, (void *)&bleprox_gpio_cfg, bleprox_Create);
}

