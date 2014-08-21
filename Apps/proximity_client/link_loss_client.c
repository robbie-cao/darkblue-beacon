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
* Link Loss Client for Proximity Client Application
*
* Link Loss Client module is responsible to perform GATT database
* search of the Link Loss portion of the GATT database.  The
* required characteristic is Immediate Alert.  The client finds
* the handle of the Immediate Alert characteristics and writes
* value of the Alert Level that server should use when connection
* is lost.
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "proximity_client.h"

/******************************************************
 *                      Constants
 ******************************************************/
#define LINK_LOSS_CLIENT_STATE_IDLE							0
#define LINK_LOSS_CLIENT_STATE_READ_GATT_CHARACTERISTIC 	1
#define LINK_LOSS_CLIENT_STATE_WRITE_ALERT_LEVEL 			2

/******************************************************
 *                     Structures
 ******************************************************/
#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    UINT16  alert_level_handle;
}  FINDME_CLIENT_HOSTINFO;
#pragma pack()


/******************************************************
 *               Function Prototypes
 ******************************************************/

static void     link_loss_client_process_rsp(int len, int attr_len, UINT8 *data);

/******************************************************
 *               Variables Definitions
 ******************************************************/

UINT8   linik_loss_client_state 					= LINK_LOSS_CLIENT_STATE_IDLE;

// handle of the end of the service
UINT16  link_loss_client_e_handle                   = 0;

// NVRAM save area
FINDME_CLIENT_HOSTINFO link_loss_client_hostinfo;

static UINT8					  link_loss_client_alert_level                  = NO_ALERT;
static BLEPROFILE_SINGLE_PARAM_CB link_loss_client_initialize_complete_callback = NULL;

//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void link_loss_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("link_loss_client, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda = (UINT8 *)emconninfo_getPeerAddr();
        UINT8 writtenbyte;

        memcpy(link_loss_client_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

        // every link_loss after pairing we need to perform discovery to
        // find out handles of the service.
        link_loss_client_hostinfo.alert_level_handle = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_FINDME_CLIENT_HOST_INFO, sizeof(link_loss_client_hostinfo), (UINT8 *)&link_loss_client_hostinfo);
        ble_trace1("NVRAM write:%d", writtenbyte);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good link_loss to
// send it out
//
void link_loss_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8             status = *((UINT8 *)(evt + 1));
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace1("\rFINDME client encryption changed: %02x", status);

    if (status == 0)
    {
        // Connection has been encrypted meaning that we have correct/paired device
        // read the handles from the NVRAM
        bleprofile_ReadNVRAM(VS_FINDME_CLIENT_HOST_INFO, sizeof(link_loss_client_hostinfo), (UINT8 *)&link_loss_client_hostinfo);
    }
}

// command from the main app to start search for characteristics
int link_loss_client_initialize(UINT16 s_handle, UINT16 e_handle, UINT8 alert_level, BLEPROFILE_SINGLE_PARAM_CB initialize_complete_callback)
{
    if ((s_handle == 0) || ((link_loss_client_e_handle = e_handle) == 0))
    {
        return FALSE;
    }

    // Functions of this module willl need to receive FINDME client callbacks
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) link_loss_client_process_rsp);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) link_loss_client_process_rsp);

    link_loss_client_alert_level				  	= alert_level;
    link_loss_client_initialize_complete_callback 	= initialize_complete_callback;

    linik_loss_client_state 						= LINK_LOSS_CLIENT_STATE_READ_GATT_CHARACTERISTIC;
    bleprofile_sendReadByTypeReq(s_handle, e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    return TRUE;
}

// process responses from the server during the FINDME discovery
void link_loss_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int     i;
    UINT16  uuid;
    UINT16  handle, value_handle;
    int     done = FALSE;

    ble_trace2("\rLink Loss Client Rsp len:%d, attr_len:%d", len, attr_len);

    if (linik_loss_client_state == LINK_LOSS_CLIENT_STATE_READ_GATT_CHARACTERISTIC)
    {
		if (len)
		{
			// Char handle (2) Properties(1) Value Handle (2) UUID(2-16)
			// search for uuid
			for (i = 0; i < len; i += attr_len)
			{
				handle       = data[i    ] + (data[i + 1] << 8);
				value_handle = data[i + 3] + (data[i + 4] << 8);
				uuid         = data[i + 5] + (data[i + 6] << 8);

				if (uuid == UUID_CHARACTERISTIC_ALERT_LEVEL)
				{
					// found alert level characteristic of link loss service
					link_loss_client_hostinfo.alert_level_handle = value_handle;
					ble_trace1("link_loss alert_level hdl:%04x", link_loss_client_hostinfo.alert_level_handle);

					// send alert level to be used on the server when link goes down
				    linik_loss_client_state = LINK_LOSS_CLIENT_STATE_WRITE_ALERT_LEVEL;
					bleprofile_sendWriteReq(value_handle, (UINT8 *)&link_loss_client_alert_level, 1);
				}
			}
			// if we are here we did not find immediate alert characteristic, but it is not the end of the service
			bleprofile_sendReadByTypeReq(value_handle + 1, link_loss_client_e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
		}
		else
		{
			ble_trace0("could not find immediate alert characteristic");
			link_loss_client_initialize_complete_callback(1);
		}
    }
    else if (linik_loss_client_state == LINK_LOSS_CLIENT_STATE_WRITE_ALERT_LEVEL)
    {
		link_loss_client_initialize_complete_callback(0);
    }
}

