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
 * BLE Proximity Client Device definitions and function prototypes
 *
 */

// NVRAM IDs 1-15 are booked by the system
#define VS_GATT_CLIENT_HOST_INFO	0x10
#define VS_FINDME_CLIENT_HOST_INFO	0x11

// define alert levels as per BT SIG
#define NO_ALERT					0
#define MEDIUM_ALERT				1
#define HIGH_ALERT					2

#pragma pack(1)
// host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;
}  HOSTINFO;

#pragma pack()


