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
* This file provides definitions and function prototypes for Hello Client
* device
*
*/
#ifndef HELLO_CLIENT_H
#define HELLO_CLIENT_H

// following definitions for handles used in the GATT database
#define HANDLE_HELLO_CLIENT_SERVICE_UUID                    0x28
#define HANDLE_HELLO_CLIENT_DATA_VALUE                      0x2a
#define HANDLE_HELLO_CLIENT_CLIENT_CONFIGURATION_DESCRIPTOR 0x2b


// Please note that all UUIDs need to be reversed when publishing in the database
// {DC03900D-7C54-44FA-BCA6-C61732A248EF}
// static const GUID UUID_HELLO_CLIENT_SERVICE = { 0xdc03900d, 0x7c54, 0x44fa, { 0xbc, 0xa6, 0xc6, 0x17, 0x32, 0xa2, 0x48, 0xef } };
#define UUID_HELLO_CLIENT_SERVICE           0xef, 0x48, 0xa2, 0x32, 0x17, 0xc6, 0xa6, 0xbc, 0xfa, 0x44, 0x54, 0x7c, 0x0d, 0x90, 0x03, 0xdc

// {B77ACFA5-8F26-4AF6-815B-74D03B4542C5}
// static const GUID UUID_HELLO_CLIENT_DATA = { 0xb77acfa5, 0x8f26, 0x4af6, { 0x81, 0x5b, 0x74, 0xd0, 0x3b, 0x45, 0x42, 0xc5 } };
#define UUID_HELLO_CLIENT_DATA    			0xc5, 0x42, 0x45, 0x3b, 0xd0, 0x74, 0x5b, 0x81, 0xf6, 0x4a, 0x26, 0x8f, 0xa5, 0xcf, 0x7a, 0xb7

#endif
