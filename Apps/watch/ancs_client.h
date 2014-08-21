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
* BLE Client for Apple Notification Center Service (ANCS)
*
* Defines external definitions and function prototype for the ANCS client.
*/
#ifndef ANCS_CLIENT__H
#define ANCS_CLIENT__H

// 7905F431-B5CE-4E99-A40F-4B1E122D00D0
const char ANCS_SERVICE[]             = {0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79};

// Notification Source: UUID 9FBF120D-6301-42D9-8C58-25E699A21DBD (notifiable)
const char ANCS_NOTIFICATION_SOURCE[] = {0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F};

// Control Point: UUID 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9 (writeable with response)
const char ANCS_CONTROL_POINT[]       = {0xD9, 0xD9, 0xAA, 0xFD, 0xBD, 0x9B, 0x21, 0x98, 0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69};

// Data Source: UUID 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB (notifiable)
const char ANCS_DATA_SOURCE[]         = {0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, 0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22};

extern void     ancs_client_create(void);
extern void     ancs_client_app_timer(UINT32 count);
extern void     ancs_client_app_fine_timer(UINT32 finecount);
extern void     ancs_client_connection_up(void);
extern void     ancs_client_connection_down(void);
extern void     ancs_client_smp_bond_result(LESMP_PARING_RESULT result);
extern void     ancs_client_encryption_changed(HCI_EVT_HDR *evt);

#endif
