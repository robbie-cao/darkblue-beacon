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
* WICED Smart Secure OTA Upgrade
*
* This file provides definitions and function prototypes for the
* WICED Smart Secure Over the Air Upgrade
*
*/
#ifndef WS_SEC_UPGRADE_OTA_H
#define WS_SEC_UPGRADE_OTA_H

// Please note that all UUIDs need to be reversed when publishing in the database

//
// {A86ABC2D-D44C-442E-99F7-80059A873E36}
#ifdef _WIN32
static const GUID GUID_WS_SECURE_UPGRADE_SERVICE = { 0xa86abc2d, 0xd44c, 0x442e, { 0x99, 0xf7, 0x80, 0x5, 0x9a, 0x87, 0x3e, 0x36 } };
#endif
#define UUID_WS_SECURE_UPGRADE_SERVICE                      0x36, 0x3e, 0x87, 0x9a, 0x05, 0x80, 0xf7, 0x99, 0x2e, 0x44, 0x4c, 0xd4, 0x2d, 0xbc, 0x6a, 0xa8

// {1BD19C14-B78A-4E0F-AEB5-8E0352BAC382}
#ifdef _WIN32
static const GUID GUID_WS_SECURE_UPGRADE_CHARACTERISTIC_CONTROL_POINT = { 0x1bd19c14, 0xb78a, 0x4e0f, { 0xae, 0xb5, 0x8e, 0x3, 0x52, 0xba, 0xc3, 0x82 } };
#endif
#define UUID_WS_SECURE_UPGRADE_CHARACTERISTIC_CONTROL_POINT 0x82, 0xc3, 0xba, 0x52, 0x03, 0x8e, 0xb5, 0xae, 0x0f, 0x4e, 0x8a, 0xb7, 0x14, 0x9c, 0xd1, 0x1b

// {279F9DAB-79BE-4663-AF1D-24407347AF13}
#ifdef _WIN32
static const GUID GUID_WS_SECURE_UPGRADE_CHARACTERISTIC_DATA = { 0x279f9dab, 0x79be, 0x4663, { 0xaf, 0x1d, 0x24, 0x40, 0x73, 0x47, 0xaf, 0x13 } };
#endif
#define UUID_WS_SECURE_UPGRADE_CHARACTERISTIC_DATA          0x13, 0xaf, 0x47, 0x73, 0x40, 0x24, 0x1d, 0xaf, 0x63, 0x46, 0xbe, 0x79, 0xab, 0x9d, 0x9f, 0x27

// {6AA5711B-0376-44F1-BCA1-8647B48BDB55}
#ifdef _WIN32
static const GUID GUID_WS_SECURE_UPGRADE_CHARACTERISTIC_APP_INFO = { 0x6aa5711b, 0x376, 0x44f1, { 0xbc, 0xa1, 0x86, 0x47, 0xb4, 0x8b, 0xdb, 0x55 } };
#endif
#define UUID_WS_SECURE_UPGRADE_CHARACTERISTIC_APP_INFO      0x55, 0xdb, 0x8b, 0xb4, 0x47, 0x86, 0xa1, 0xbc, 0xf1, 0x44, 0x76, 0x03, 0x1b, 0x71, 0xa5, 0x6a

// command definitions for the OTA FW upgrade
#define WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD                 1
#define WS_UPGRADE_COMMAND_DOWNLOAD                         2
#define WS_UPGRADE_COMMAND_VERIFY                           3
#define WS_UPGRADE_COMMAND_FINISH                           4 // not currently used
#define WS_UPGRADE_COMMAND_GET_STATUS                       5 // not currently used
#define WS_UPGRADE_COMMAND_CLEAR_STATUS                     6 // not currently used
#define WS_UPGRADE_COMMAND_ABORT                            7

// event definitions for the OTA FW upgrade
#define WS_UPGRADE_STATUS_OK                                0
#define WS_UPGRADE_STATUS_UNSUPPORTED_COMMAND               1
#define WS_UPGRADE_STATUS_ILLEGAL_STATE                     2
#define WS_UPGRADE_STATUS_VERIFICATION_FAILED               3
#define WS_UPGRADE_STATUS_INVALID_IMAGE                     4
#define WS_UPGRADE_STATUS_INVALID_IMAGE_SIZE                5
#define WS_UPGRADE_STATUS_MORE_DATA                         6
#define WS_UPGRADE_STATUS_INVALID_APPID                     7
#define WS_UPGRADE_STATUS_INVALID_VERSION                   8

#define WS_UPGRADE_WRITE_STATUS_SUCCESS                     0x00
#define WS_UPGRADE_WRITE_STATUS_BAD_ID                      0x81
#define WS_UPGRADE_WRITE_STATUS_BAD_MAJOR                   0x82
#define WS_UPGRADE_WRITE_STATUS_TOO_MUCH_DATA               0x83
#define WS_UPGRADE_WRITE_STATUS_TOO_SHORT                   0x84
#define WS_UPGRADE_WRITE_STATUS_ABORTED                     0x85

// following definitions can be shared between client and sensor
// to avoid unnecessary GATT Discovery
//
#define HANDLE_WS_UPGRADE_SERVICE                           0xff00
#define HANDLE_WS_UPGRADE_CHARACTERISTIC_CONTROL_POINT      0xff01
#define HANDLE_WS_UPGRADE_CONTROL_POINT                     0xff02
#define HANDLE_WS_UPGRADE_CLIENT_CONFIGURATION_DESCRIPTOR   0xff03
#define HANDLE_WS_UPGRADE_CHARACTERISTIC_DATA               0xff04
#define HANDLE_WS_UPGRADE_DATA                              0xff05
#define HANDLE_WS_UPGRADE_CHARACTERISTIC_APP_INFO           0xff06
#define HANDLE_WS_UPGRADE_APP_INFO                          0xff07

// Maximum data packet length we can process
#define WS_UPGRADE_MAX_DATA_LEN                             23

// Application ID 2 bytes plus 1 bytes major and minor versions
#define WS_UPGRADE_PREFIX_LEN                               4

#define WS_UPGRADE_RSA_SIGNATURE_LEN		128

#pragma pack(1)

// structure to pass application information to upgrade application
typedef struct
{
    UINT16 ID;
    UINT8  Version_Major;
    UINT8  Version_Minor;
} WS_UPGRADE_APP_INFO;
#pragma pack()


// define entry points to upgrade functionality
int ws_upgrade_ota_init(void);
int ws_upgrade_ota_handle_command (UINT8 *data, int len);
int ws_upgrade_ota_handle_configuration (UINT8 *data, int len);
int ws_upgrade_ota_handle_data (UINT8 *data, int len);

#endif
