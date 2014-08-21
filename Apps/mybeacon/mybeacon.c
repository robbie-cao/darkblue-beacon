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
* BLE beacon sample
*
* During initialization the app configures stack to send advertisement
* packets.  Non-connectable undirected advertisements are used.
*
* Features demonstrated
*  - configuring vendor specific advertisements
*  - configuring GPIO P0 as an active HIGH indication to pause advertising
*  		for as long as the GPIO is high. When the beacon app is co-located
*  		with a WiFi device, the WiFi device can assert P0 for the duration
*  		of the WiFi activity so the beacon can be turned off to prevent
*  		interference.
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Monitor advertisement packets on over the air sniffer
* 4. Push and hold the big white button on the tag board and check that advertisements stop
* 5. Release the big white button and check that advertisements resume
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "mybeacon.h"
#include "spar_utils.h"
#include "gpiodriver.h"
#include "additional_advertisement_control.h"

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void mybeacon_create(void);

/******************************************************
 *               Variables Definitions
 ******************************************************/

const UINT8 mybeacon_uuid[16] = {0x87, 0xf8, 0xac, 0xb5, 0x9a, 0xd0, 0x29, 0xa7, 0x20, 0x4c, 0xf0, 0x80, 0xe2, 0xb5, 0x5c, 0x0e};
UINT16 mymeacon_sequence_number = 0;

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG mybeacon_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

void advertisement_packet_transmission(UINT8 type);

/******************************************************
 *               Function Definitions
 ******************************************************/
// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg(NULL, 0, NULL, (void *)&mybeacon_puart_cfg, NULL, mybeacon_create);
}

// Create ANCS client device
void mybeacon_create(void)
{
    BLE_ADV_FIELD adv[2];

    ble_trace0("\rmybeacon_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    ble_tracen((UINT8 *)mybeacon_uuid, 16);

    bleprofile_Init(bleprofile_p_cfg);

    // Format advertisement data.  Data consists of 2 fields.  Standard Advertisement flags
    // and Broadcom Vendor specific data.  The vendor specific data consists of 16 byte
    // UUID and two 2 byte values type and subtype.

    // flags
    adv[0].len     = 1 + 1;
    adv[0].val     = ADV_FLAGS;
    adv[0].data[0] = LE_GENERAL_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

    adv[1].len     = 22 + 1;
    adv[1].val     = ADV_MANUFACTURER_DATA; // (AD_TYPE == 0xff)
    adv[1].data[0] = 0x0f;  // Broadcom  (Company Identifier 2 bytes)
    adv[1].data[1] = 0x00;

    BT_MEMCPY(&adv[1].data[2], mybeacon_uuid, 16);

    adv[1].data[18] = MYBEACON_TYPE & 0xff;
    adv[1].data[19] = (MYBEACON_TYPE >> 8) & 0xff;
    adv[1].data[20] = mymeacon_sequence_number & 0xff;
    adv[1].data[21] = (mymeacon_sequence_number >> 8) & 0xff;

    bleprofile_GenerateADVData(adv, 2);

    // Configure P0 as active high input to pause advertisements.
    bleprofile_configureGpioForSkippingRf(GPIO_PIN_P0, GPIO_PIN_INPUT_HIGH);

    // register with LE stack to be called 2.5msec before the advertisement event
    bleprofile_notifyAdvPacketTransmissions(&advertisement_packet_transmission, 2500);

    // Set TX power for advertisement packets at 4dBm
    blecm_setTxPowerInADV(4);

    bleapputils_changeLPOSource(LPO_MIA_LPO, FALSE, 500);

    // Start advertisements
    blecm_startAdv(
        HCIULP_ADV_NONCONNECTABLE_EVENT,                // non-connectable undirected advertisement
        160,                                            // adv interval 100 msec
        HCIULP_ADV_CHANNEL_MAP_MASK,                    // all channels
        HCIULP_PUBLIC_ADDRESS,                          // int advAdrType,
        HCIULP_ADV_FILTER_POLICY_WHITE_LIST_NOT_USED,   // int advFilterPolicy,
        HCIULP_PUBLIC_ADDRESS,                          // int initiatorAdrType,
        NULL);                                          // UINT8* initiatorAdr
}

// this function is called 2.5 msec before the advertisement event.  In this sample
// just bump the sequence number and modify advertisement data.
void advertisement_packet_transmission(UINT8 type)
{
    if (type == 0)
    {
        BLE_ADV_FIELD adv[2];

        // Format advertisement data.  Data consists of 2 fields.  Standard Advertisement flags
        // and Broadcom Vendor specific data.  The vendor specific data consists of 16 byte        
        // UUID, 2 byte type and 2 byte sequence number.

        // flags
        adv[0].len     = 1 + 1;
        adv[0].val     = ADV_FLAGS;
        adv[0].data[0] = LE_GENERAL_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

        adv[1].len     = 22 + 1;
        adv[1].val     = ADV_MANUFACTURER_DATA; // (AD_TYPE == 0xff)
        adv[1].data[0] = 0x0f;  // Broadcom  (Company Identifier 2 bytes)
        adv[1].data[1] = 0x00;

        BT_MEMCPY(&adv[1].data[2], mybeacon_uuid, 16);

        adv[1].data[18] = MYBEACON_TYPE & 0xff;
        adv[1].data[19] = (MYBEACON_TYPE >> 8) & 0xff;
        adv[1].data[20] = ++mymeacon_sequence_number & 0xff;
        adv[1].data[21] = (mymeacon_sequence_number >> 8) & 0xff;

        bleprofile_GenerateADVData(adv, 2);
    }
}
