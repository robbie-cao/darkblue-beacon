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
* BLE Beacon Device
*
* This file provides definitions and function prototypes for My Beacon device
*
*/
#ifndef MY_BEACON_H
#define MY_BEACON_H

// Please note that all UUIDs need to be reversed when publishing in the database

#ifdef WIN32
// {0E5CB5E2-80F0-4C20-A729-D09AB5ACF887}
static const GUID GUID_MY_BEACON = {0x0e5cb5e2, 0x80f0, 0x4c20, { 0xa7, 0x29, 0xd0, 0x9a, 0xb5, 0xac, 0xf8, 0x87 } };
#endif
#define UUID_MY_BEACON              0x87, 0xf8, 0xac, 0xb5, 0x9a, 0xd0, 0x29, 0xa7, 0x20, 0x4c, 0xf0, 0x80, 0xe2, 0xb5, 0x5c, 0x0e

#define MYBEACON_TYPE				0x01

#define     HCIULP_ADV_NONCONNECTABLE_EVENT                                 0x03
#define     HCIULP_ADV_CHANNEL_37                                           0x01
#define     HCIULP_ADV_CHANNEL_38                                           0x02
#define     HCIULP_ADV_CHANNEL_39                                           0x04

#define     HCIULP_ADV_CHANNEL_MAP_MASK                                     (HCIULP_ADV_CHANNEL_37 | HCIULP_ADV_CHANNEL_38 | HCIULP_ADV_CHANNEL_39)

#define     HCIULP_PUBLIC_ADDRESS                                           0x00

#define     HCIULP_ADV_FILTER_POLICY_WHITE_LIST_NOT_USED                    0x00    // white list not used

#endif
