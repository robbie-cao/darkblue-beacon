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
* This file provides definitions and function prototypes for Speed Test
* Service
*
*/
#ifndef SPEED_TEST_H
#define SPEED_TEST_H

// following definitions are shared between client and sensor
// to avoid unnecessary GATT Discovery
//
#define HANDLE_SPEED_TEST_SERVICE_UUID                              0x28
#define HANDLE_SPEED_TEST_CHARACTERISTIC_DATA                       0x2a
#define HANDLE_SPEED_TEST_CHARACTERISTIC_DATA_VALUE                 0x2b
#define HANDLE_SPEED_TEST_DATA_CLIENT_CONFIGURATION_DESCRIPTOR      0x2c
#define HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL                    0x2d
#define HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL_VALUE              0x2e
#define HANDLE_SPEED_TEST_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR   0x2f

#define SPEED_TEST_START											1
#define SPEED_TEST_STOP                                             2

// {DFDD3E89-D394-4023-9D8F-C453B2CFFC22}
#ifdef WIN32
static const GUID GUID_SPEED_TEST_SERVICE = { 0xdfdd3e89, 0xd394, 0x4023, { 0x9d, 0x8f, 0xc4, 0x53, 0xb2, 0xcf, 0xfc, 0x22 } };
#endif
#define UUID_SPEED_TEST_SERVICE               0x22, 0xfc, 0xcf, 0xb2, 0x53, 0xc4, 0x8f, 0x9d, 0x23, 0x40, 0x94, 0xd3, 0x89, 0x3e, 0xdd, 0xdf

// {8A848ACE-9082-474E-8990-4F1755C695AF}
#ifdef WIN32
static const GUID GUID_SPEED_TEST_CHARACTERISTIC_DATA = { 0x8a848ace, 0x9082, 0x474e, { 0x00, 0x90, 0x4f, 0x17, 0x55, 0xc6, 0x95, 0xaf } };
#endif
#define UUID_SPEED_TEST_CHARACTERISTIC_DATA  0xaf, 0x95, 0xc6, 0x55, 0x17, 0x4f, 0x90, 0x00, 0x4e, 0x47, 0x82, 0x90, 0xce, 0x8a, 0x84, 0x8a

// {6D4824D7-D102-4BAB-83B1-C3322541721D}
#ifdef WIN32
static const GUID GUID_SPEED_TEST_CHARACTERISTIC_CONTROL_POINT = { 0x6d4824d7, 0xd102, 0x4bab, { 0x83, 0xb1, 0xc3, 0x32, 0x25, 0x41, 0x72, 0x1d } };
#endif
#define UUID_SPEED_TEST_CHARACTERISTIC_CONTROL_POINT  0x1d, 0x72, 0x41, 0x25, 0x32, 0xc3, 0xb1, 0x83, 0xab, 0x4b, 0x02, 0xd1, 0xd7, 0x24, 0x48, 0x6d

#endif
