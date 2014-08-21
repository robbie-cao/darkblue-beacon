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
* BLE Vendor Specific Device
*
* This file provides definitions and function prototypes for A4WP PRU device
*
*/
#ifndef _BLEPRU_H_
#define _BLEPRU_H_

#include "a4wp_hal_api.h"

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

// following flag enables the PMU interfaces to configure and read the dynamic
// parameters from PMU chip when PRU is powered from PTU.
// Disabling this flag can be used to verify the BLE protocol functionality of PRU
// without the PMU chip.
#define BLEPRU_PMU_INF_ENABLE

// Define to use 128 bit UUID for characteristic UUIDs.
// Disabling this flag enables the sample 16 bit UUIDs for PRU characteristics.
#define PRU_128BIT_UUID

// following definitions shows the GPIO mapping used in the application
#define BLEPRU_GPIO_PIN_WP                          31
#define BLEPRU_GPIO_PIN_UART_TX                     24
#define BLEPRU_GPIO_PIN_UART_RX                     25

// GPIO line configured for interrupt from PMU chip.
#define BLEPRU_GPIO_PIN_PMU_INT                     3

// GPIO to configure HIGH on boot up to enable the PMU lines
// This can serve as switch to connect the PMU hardware after boot up
#define BLEPRU_GPIO_PIN_BOOT_COMPLETE               4


// A4WP 16 bit Service UUID (SIG adopted)
#define UUID_SERVCLASS_A4WP                         0xFFFE

// Sample 16 bit characteristic UUIDs, used when configured to use 16 bit characteristic UUIDs.
#define GATT_UUID_A4WP_CHARGING_PRU_CONTROL         0xF002
#define GATT_UUID_A4WP_CHARGING_PTU_STATIC          0xF003
#define GATT_UUID_A4WP_CHARGING_PRU_ALERT           0xF004
#define GATT_UUID_A4WP_CHARGING_PRU_STATIC          0xF005
#define GATT_UUID_A4WP_CHARGING_PRU_DYNAMIC         0xF006

// 128 bit base UUID value for PRU characteristics
// The characteristic UUIDs will take the offsetted value from base in the order of
// characteristic defined in BSS 1.2
#define UUID_SERVCLASS_A4WP_128 \
0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64

#ifdef PRU_128BIT_UUID
#define GATT_UUID_A4WP_CHARGING_PRU_CONTROL_128 \
    0x67, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64
#define GATT_UUID_A4WP_CHARGING_PTU_STATIC_128 \
    0x68, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64
#define GATT_UUID_A4WP_CHARGING_PRU_ALERT_128 \
    0x69, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64
#define GATT_UUID_A4WP_CHARGING_PRU_STATIC_128 \
    0x6a, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64
#define GATT_UUID_A4WP_CHARGING_PRU_DYNAMIC_128 \
    0x6b, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x96, 0x9e, 0xe2, 0x11, 0x46, 0xa1, 0x70, 0xe6, 0x55, 0x64
#endif

// A4WP ADV flag/mask definitions as per specification
#define BLEPRU_RFU_a                                0x01
#define BLEPRU_RFU_b                                0x02
#define BLEPRU_TIMESET_SUPPORT                      0x04
#define BLEPRU_OVP_STATUS_SUPPORT                   0x08
#define BLEPRU_REBOOT_INDICATOR                     0x10
// Impedance shift bits
#define BLEPRU_NEVER_CREATE_IMPEDANCE               0x00
#define BLEPRU_CAT1                                 0x20
#define BLEPRU_CAT2                                 0x40
#define BLEPRU_CAT3                                 0x60
#define BLEPRU_CAT4                                 0x80
#define BLEPRU_CAT5                                 0xA0
#define BLEPRU_RESERVED_a                           0xC0
#define BLEPRU_RESERVED_b                           0xE0

// A4WP PRU ADV service data payload format
PACKED struct t_BLEPRU_ADV_PAYLOAD
{
    UINT16  wpt_service_uuid;
    UINT16  gatt_primary_service_handle;
    UINT8   pru_rssi_parameters;
    UINT8   adv_flag;
};
typedef struct t_BLEPRU_ADV_PAYLOAD BLEPRU_ADV_PAYLOAD;

// A4WP PRU Static parameter format
PACKED struct t_BLEPRU_PRU_STATIC
{
    UINT8  optional_fields;
    UINT8  rev;
    UINT8  rfu1;
    UINT8  pru_category;
    UINT8  pru_information;
    UINT8  hw_rev;
    UINT8  fw_rev;
    UINT8  maximum_power_desired;
    UINT16 v_rect_min_static;
    UINT16 v_rect_high_static;
    UINT16 v_rect_set;
    UINT16 delta_r1;
    UINT8  rfu[4];
};
typedef struct t_BLEPRU_PRU_STATIC BLEPRU_PRU_STATIC;

// A4WP PRU Dynamic parameter format
PACKED struct t_BLEPRU_PRU_DYNAMIC
{
    UINT8  optional_fields;
    UINT16 v_rect;
    UINT16 i_rect;
    UINT16 v_out;
    UINT16 i_out;
    UINT8  temperature;
    UINT16 v_rect_min_dyn;
    UINT16 v_rect_set_dyn;
    UINT16 v_rect_high_dyn;
    UINT8  pru_alert;
    UINT8  rfu[3];
};
typedef struct t_BLEPRU_PRU_DYNAMIC BLEPRU_PRU_DYNAMIC;

// PRU Control characteristic value format
PACKED struct t_BLEPRU_PRU_CTRL_CHAR_VAL
{
    PACKED union
    {
        PACKED struct 
        {
            UINT8  rfu:4;
            UINT8  pwrcmd:2;
            UINT8  charging:1;
            UINT8  output:1;
        } s;
        UINT8  byte;
    } enables;
    UINT8  permission;
    UINT8  time_set;
    UINT8  rfu[2];
};
typedef struct t_BLEPRU_PRU_CTRL_CHAR_VAL BLEPRU_PRU_CTRL_CHAR_VAL;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

#endif // _BLEPRU_H_
