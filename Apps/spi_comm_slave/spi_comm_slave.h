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
* This file provides definitions and function prototypes for SPI Comm 
* Service
*
*/
#ifndef SPI_COMM_SLAVE_H
#define SPI_COMM_SLAVE_H

#define NUM_TO_SEND_UNACKED     200

// following definitions are shared between client and sensor
// to avoid unnecessary GATT Discovery
//
#define HANDLE_SPI_COMM_SERVICE_UUID                            0x28
#define HANDLE_SPI_COMM_CHARACTERISTIC_DATA                     0x2a
#define HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE		        0x2b
#define HANDLE_SPI_COMM_DATA_CLIENT_CONFIGURATION_DESCRIPTOR    0x2c
#define HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL                  0x2d
#define HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE            0x2e
#define HANDLE_SPI_COMM_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR 0x2f

// Please note that all UUIDs need to be reversed when publishing in the database

// {2E6F1D15-F1C5-4BF6-BE38-6E03817CBA10}
#ifdef WIN32
static const GUID GUID_SPI_COMM_SERVICE = { 0x2e6f1d15, 0xf1c5, 0x4bf6, { 0xbe, 0x38, 0x6e, 0x3, 0x81, 0x7c, 0xba, 0x10 } };
#endif
#define UUID_SPI_COMM_SERVICE               0x10, 0xba, 0x7c, 0x81, 0x03, 0x6e, 0x38, 0xbe, 0xf6, 0x4b, 0xc5, 0xf1, 0x15, 0x1d, 0x6f, 0x2e

// {19E97635-4207-4C41-A78F-57A7FBD342D0}
#ifdef WIN32
static const GUID GUID_SPI_COMM_CHARACTERISTIC_DATA = { 0x19e97635, 0x4207, 0x4c41, { 0xa7, 0x8f, 0x57, 0xa7, 0xfb, 0xd3, 0x42, 0xd0 } };
#endif
#define UUID_SPI_COMM_CHARACTERISTIC_DATA  0xd0, 0x42, 0xd3, 0xfb, 0xa7, 0x57, 0x8f, 0xa7, 0x41, 0x4c, 0x07, 0x42, 0x35, 0x76, 0xe9, 0x19

// {E16F6ACB-CA31-4177-B068-CD477AB4EB51}
#ifdef WIN32
static const GUID GUID_SPI_COMM_CHARACTERISTIC_CONTROL_POINT = { 0xe16f6acb, 0xca31, 0x4177, { 0xb0, 0x68, 0xcd, 0x47, 0x7a, 0xb4, 0xeb, 0x51 } };
#endif
#define UUID_SPI_COMM_CHARACTERISTIC_CONTROL_POINT  0x51, 0xeb, 0xb4, 0x7a, 0x47, 0xcd, 0x68, 0xb0, 0x77, 0x41, 0x31, 0xca, 0xcb, 0x6a, 0x6f, 0xe1

#endif
