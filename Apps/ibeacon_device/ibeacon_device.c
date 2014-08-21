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
* BLE iBeacon sample
*
* During initialization the app configures stack to send advertisement
* packets.  Non-connectable undirected advertisements are used.
*
* To build the application optional iBeacon library need to be included
* in the make file.  Please note that the library can be distributed only
* to the companies members of the Apple's iBeacon program.  Contact
* your Broadcom support representative if you are a member of iBeacon
* program to receive the library.
*
* Features demonstrated
*  - iBeacon advertisements
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Monitor advertisement packets on over the air sniffer
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "platform.h"
#include "spar_utils.h"

/******************************************************
 *               Function Prototypes
 ******************************************************/
static void ibeacon_create(void);

/******************************************************
 *               Variables Definitions
 ******************************************************/
const UINT8  ibeacon_company_uuid[16] = {0x87, 0xf8, 0xac, 0xb5, 0x9a, 0xd0, 0x29, 0xa7, 0x20, 0x4c, 0xf0, 0x80, 0xe2, 0xb5, 0x5c, 0x0e};
const UINT16 ibeacon_major = 0x11;
const UINT16 ibeacon_minor = 0x0001;
const INT8   ibeacon_measured_power = -50;

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG ibeacon_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

/******************************************************
 *               Function Definitions
 ******************************************************/
// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg(NULL, 0, NULL, NULL, (void *)&ibeacon_puart_cfg, ibeacon_create);
}

// Create iBeacon device
void ibeacon_create(void)
{
    ble_trace0("\rmybeacon_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    ibeacon_start(ibeacon_company_uuid, ibeacon_major, ibeacon_minor, ibeacon_measured_power);
}

