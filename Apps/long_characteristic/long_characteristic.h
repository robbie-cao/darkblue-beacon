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
* Long Characteristic Sample device
*
* This file provides definitions and function prototypes for Long Characteristic
* sample device
*
*/

// long_characteristic_db.h
#ifndef LONG_CHARACTERISTIC_H
#define LONG_CHARACTERISTIC_H

// ***** Primary service 'Generic Access'
#define HDLS_GENERIC_ACCESS                     0x0100
#define UUID_SERVICE_GAP                        0x1800

// ----- Characteristic 'Device Name'
#define HDLC_GENERIC_ACCESS_DEVICE_NAME         0x0110
#define HDLC_GENERIC_ACCESS_DEVICE_NAME_VALUE   0x0111
#define UUID_CHARACTERISTIC_DEVICE_NAME         0x2A00

// ----- Characteristic 'Appearance'
#define HDLC_GENERIC_ACCESS_APPEARANCE          0x0120
#define HDLC_GENERIC_ACCESS_APPEARANCE_VALUE    0x0121
#define UUID_CHARACTERISTIC_APPEARANCE          0x2A01

// ***** Primary service 'Generic Attribute'
#define HDLS_GENERIC_ATTRIBUTE                  0x0200
#define UUID_SERVICE_GATT                       0x1801

// ***** Primary service 'long_characteristic'
#define HDLS_LONG_CHARACTERISTIC                0x0300

// ----- Characteristic 'long_characteristic'
#ifdef _WIN32
static const GUID GUID_LONG_CHAR_SERVICE =      { 0x8832ab08, 0xba2d, 0x0184, { 0x00, 0x4c, 0x68, 0xc0, 0x8e, 0x21, 0x90, 0xbf } };
#endif
#define UUID_LONG_CHAR_SERVICE                   0xbf, 0x90, 0x21, 0x8e, 0xc0, 0x68, 0x4c, 0x00, 0x84, 0x01, 0x2d, 0xba, 0x08, 0xab, 0x32, 0x88

#define HDLC_LONG_CHARACTERISTIC                 0x0310
#define HDLC_LONG_CHARACTERISTIC_VALUE           0x0311
#define SIZE_LONG_CHARACTERISTIC_VALUE           160

#ifdef _WIN32
static const GUID  GUID_LONG_CHAR_CHAR =         { 0x361e118a, 0x5524, 0x29b6, { 0x03, 0x45, 0xb1, 0x5e, 0x12, 0xe2, 0x5b, 0x34 } };
#endif
#define UUID_LONG_CHAR_CHAR                      0x34, 0x5b, 0xe2, 0x12, 0x5e, 0xb1, 0x45, 0x03, 0xb6, 0x29, 0x24, 0x55, 0x8a, 0x11, 0x1e, 0x36

// Client Configuration
#define HDLD_LONG_CHARACTERISTIC_LONG_CHARACTERISTIC_CLIENT_CONFIGURATION 0x0320

// External definitions
extern const UINT8  gatt_database[];
extern const UINT16 gatt_database_len;
extern UINT8        long_characteristic_indication_sent;           // 1 means waiting for ack
extern UINT16       long_characteristic_connection_handle;         // HCI handle of connection, not zero when connected

extern UINT8        long_characteristic_uuid_main_vsc_service[16]; // Main vendor specific service

#endif
