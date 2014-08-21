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
* FINDME Client for Watch application 
*
* If FINDME server contains Update DB characteristic record register for notification
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "watch_db.h"

/******************************************************
 *                     Structures
 ******************************************************/
#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    UINT16  immediate_alert_handle;
}  FINDME_CLIENT_HOSTINFO;
#pragma pack()


/******************************************************
 *               Function Prototypes
 ******************************************************/

static void     findme_client_process_rsp(int len, int attr_len, UINT8 *data);

/******************************************************
 *               Variables Definitions
 ******************************************************/

// handle of the end of the service
UINT16  findme_client_e_handle                    = 0;

// NVRAM save area
FINDME_CLIENT_HOSTINFO findme_client_hostinfo;

static BLEPROFILE_SINGLE_PARAM_CB findme_client_initialize_complete_callback = NULL;

//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void findme_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("findme_client, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda = (UINT8 *)emconninfo_getPeerPubAddr();
        UINT8 writtenbyte;

        memcpy(findme_client_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

        // every findme after pairing we need to perform discovery to
        // find out handles of the service.
        findme_client_hostinfo.immediate_alert_handle = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_FINDME_CLIENT_HOST_INFO, sizeof(findme_client_hostinfo), (UINT8 *)&findme_client_hostinfo);
        ble_trace1("NVRAM write:%d", writtenbyte);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good findme to
// send it out
//
void findme_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8             status = *((UINT8 *)(evt + 1));
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace1("\rFINDME client encryption changed: %02x", status);

    if (status == 0)
    {
        // Connection has been encrypted meaning that we have correct/paired device
        // read the handles from the NVRAM
        bleprofile_ReadNVRAM(VS_FINDME_CLIENT_HOST_INFO, sizeof(findme_client_hostinfo), (UINT8 *)&findme_client_hostinfo);
    }
}

// command from the main app to start search for characteristics
int findme_client_initialize(UINT16 s_handle, UINT16 e_handle, BLEPROFILE_SINGLE_PARAM_CB initialize_complete_callback)
{
    if ((s_handle == 0) || ((findme_client_e_handle = e_handle) == 0))
    {
        return FALSE;
    }

    // Functions of this module willl need to receive FINDME client callbacks
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) findme_client_process_rsp);

    findme_client_initialize_complete_callback = initialize_complete_callback;

    bleprofile_sendReadByTypeReq(s_handle, e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    return TRUE;
}

// process responses from the server during the FINDME discovery
void findme_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int     i;
    UINT16  uuid;
    UINT16  handle, value_handle;
    int     done = FALSE;

    ble_trace2("\rFINDME Client Rsp len:%d, attr_len:%d", len, attr_len);

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
                findme_client_hostinfo.immediate_alert_handle = value_handle;
                ble_trace1("findme alert level hdl:%04x", findme_client_hostinfo.immediate_alert_handle);
                findme_client_initialize_complete_callback(0);
            }
        }
        // if we are here we did not find immdediate alert characteristic, but it is not the end of the service
        bleprofile_sendReadByTypeReq(value_handle + 1, findme_client_e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    }
    else
    {
        findme_client_initialize_complete_callback(1);
    }
}

