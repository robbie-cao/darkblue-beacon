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
* GATT Client for Proximity Client application
*
* If GATT Client is responsible to perform service discovery of the GATT
* portion of the server database.  If server contains Service Changed characteristic
* GATT Client should register for notification
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "proximity_client.h"

/******************************************************
 *                     Structures
 ******************************************************/
#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    UINT16  service_changed_handle;
}  GATT_CLIENT_HOSTINFO;
#pragma pack()


/******************************************************
 *               Function Prototypes
 ******************************************************/

static void gatt_client_process_rsp(int len, int attr_len, UINT8 *data);

/******************************************************
 *               Variables Definitions
 ******************************************************/

UINT16                  gatt_client_e_handle = 0;

// NVRAM save area
GATT_CLIENT_HOSTINFO    gatt_client_hostinfo;

static BLEPROFILE_SINGLE_PARAM_CB gatt_client_initialize_complete_callback = NULL;

//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void gatt_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("gatt_client, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda = (UINT8 *)emconninfo_getPeerAddr();
        UINT8 writtenbyte;

        memcpy(gatt_client_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

        // every time after pairing we need to perform discovery to
        // find out handles of the service.
        gatt_client_hostinfo.service_changed_handle      = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_GATT_CLIENT_HOST_INFO, sizeof(gatt_client_hostinfo), (UINT8 *)&gatt_client_hostinfo);
        ble_trace1("NVRAM write:%d", writtenbyte);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void ancs_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8             status = *((UINT8 *)(evt + 1));
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace1("\rGATT client encryption changed: %02x", status);

    if (status == 0)
    {
        // Connection has been encrypted meaning that we have correct/paired device
        // read the handles from the NVRAM
        bleprofile_ReadNVRAM(VS_GATT_CLIENT_HOST_INFO, sizeof(gatt_client_hostinfo), (UINT8 *)&gatt_client_hostinfo);
    }
}

// command from the main app to start search for characteristics
int gatt_client_initialize(UINT16 s_handle, UINT16 e_handle, BLEPROFILE_SINGLE_PARAM_CB initialize_complete_callback)
{
    if ((s_handle == 0) || ((gatt_client_e_handle = e_handle) == 0))
    {
        return FALSE;
    }

    // Functions of this module willl need to receive GATT client callbacks
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) gatt_client_process_rsp);

    gatt_client_initialize_complete_callback = initialize_complete_callback;

    bleprofile_sendReadByTypeReq(s_handle, e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    return TRUE;
}

// process responses from the server during the GATT discovery
void gatt_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int     i;
    UINT16  uuid;
    UINT16  handle;
    int     done = FALSE;

    ble_trace2("\rGATT Client Rsp len:%d, attr_len:%d", len, attr_len);

    if (len)
    {
        // Char handle (2) Properties(1) Value Handle (2) UUID(2-16) 
        // search for uuid
        for (i = 0; i < len; i += attr_len)
        {
            handle = data[i + 3] + (data[i + 4] << 8);
            uuid   = data[i + 5] + (data[i + 6] << 8);

            if (uuid == UUID_CHARACTERISTIC_SERVICE_CHANGED)
            {
                gatt_client_hostinfo.service_changed_handle = handle;
                ble_trace1("service_changed hdl:%04x", gatt_client_hostinfo.service_changed_handle);
                done = TRUE;

                // TBD need to find descriptor and register for notifications
            }
        }
    }
    else
    {
        done = TRUE;
    }
    if (done)
    {
        gatt_client_initialize_complete_callback(gatt_client_hostinfo.service_changed_handle == 0 ? 0 : 1);
    }
    else
    {
        bleprofile_sendReadByTypeReq(handle + 1, gatt_client_e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    }
}

void gatt_client_notification_handler(int len, int handle, UINT8 *data)
{
    ble_trace1("gatt_client_notification hdl:%04x", handle);
}
