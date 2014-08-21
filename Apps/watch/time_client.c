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
* TIME Client for Watch application 
*
* If TIME server contains Update DB characteristic record register for notification
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "watch_db.h"


// service discovery states
enum
{
    TIME_CLIENT_STATE_IDLE                            = 0x00,

    TIME_CLIENT_STATE_SEARCH_CHARACTERISTIC           = 0x01,
    TIME_CLIENT_STATE_SEARCH_DESCRIPTOR               = 0x02,
    TIME_CLIENT_STATE_WRITE_DESCRIPTOR                = 0x03,
    TIME_CLIENT_STATE_READ_CHARACTERISTIC             = 0x04,
};

/******************************************************
 *                     Structures
 ******************************************************/
#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    UINT16  current_time_handle;
    UINT16  client_config_descriptor_handle;
}  TIME_CLIENT_HOSTINFO;
#pragma pack()


/******************************************************
 *               Function Prototypes
 ******************************************************/

void time_client_process_rsp(int len, int attr_len, UINT8 *data);
void time_client_notification_handler(int len, int attr_len, UINT8 *data);

/******************************************************
 *               Variables Definitions
 ******************************************************/

UINT8   time_client_state						= TIME_CLIENT_STATE_IDLE;

// handle of the end of the service
UINT16  time_client_e_handle                    = 0;
UINT16  current_time_handle						= 0;
UINT16  client_config_descriptor_handle			= 0;

// NVRAM save area
TIME_CLIENT_HOSTINFO time_client_hostinfo;

static BLEPROFILE_SINGLE_PARAM_CB time_client_initialize_complete_callback = NULL;

//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void time_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("time_client, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda = (UINT8 *)emconninfo_getPeerPubAddr();
        UINT8 writtenbyte;

        memcpy(time_client_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

        // every time after pairing we need to perform discovery to
        // find out handles of the service.
        time_client_hostinfo.current_time_handle             = 0;
        time_client_hostinfo.client_config_descriptor_handle = 0;

        current_time_handle             = 0;
        client_config_descriptor_handle = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_TIME_CLIENT_HOST_INFO, sizeof(time_client_hostinfo), (UINT8 *)&time_client_hostinfo);
        ble_trace1("NVRAM write:%d", writtenbyte);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void time_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8             status = *((UINT8 *)(evt + 1));
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace1("\rTIME client encryption changed: %02x", status);

    if (status == 0)
    {
        // Connection has been encrypted meaning that we have correct/paired device
        // read the handles from the NVRAM
        bleprofile_ReadNVRAM(VS_TIME_CLIENT_HOST_INFO, sizeof(time_client_hostinfo), (UINT8 *)&time_client_hostinfo);
    }
}

// command from the main app to start search for characteristics
int time_client_initialize(UINT16 s_handle, UINT16 e_handle, BLEPROFILE_SINGLE_PARAM_CB initialize_complete_callback)
{
    if ((s_handle == 0) || ((time_client_e_handle = e_handle) == 0))
    {
        return FALSE;
    }

    // Functions of this module willl need to receive TIME client callbacks
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) time_client_process_rsp);
    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) time_client_process_rsp);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) time_client_process_rsp);

    time_client_initialize_complete_callback = initialize_complete_callback;

    time_client_state = TIME_CLIENT_STATE_SEARCH_CHARACTERISTIC;
    bleprofile_sendReadByTypeReq(s_handle, e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    return TRUE;
}

// process responses from the server during the TIME discovery
void time_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int     i;
    UINT16  uuid;
    UINT16  handle, value_handle;

    ble_trace3("\rTIME Client Rsp state:%d len:%d attr_len:%d", time_client_state, len, attr_len);

    switch (time_client_state)
    {
    case TIME_CLIENT_STATE_SEARCH_CHARACTERISTIC:
        if (len)
        {
            // Char handle (2) Properties(1) Value Handle (2) UUID(2-16) 
            // search for uuid
            for (i = 0; i < len; i += attr_len)
            {
                handle       = data[i    ] + (data[i + 1] << 8);
                value_handle = data[i + 3] + (data[i + 4] << 8);
                uuid         = data[i + 5] + (data[i + 6] << 8);

                if (current_time_handle != 0)
                {
                    // found Current Time and handle of the next char, search for client configuration descriptor
                    time_client_state = TIME_CLIENT_STATE_SEARCH_DESCRIPTOR;
                    bleprofile_sendReadByTypeReq(current_time_handle, handle,
                                                 UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
                    break;
                }
                if (uuid == UUID_CHARACTERISTIC_CURRENT_TIME)
                {
                    current_time_handle = value_handle;
                    ble_trace1("current_time hdl:%04x", current_time_handle);
                }
            }
            // if we are here we did not find current time or a characteristic after
            bleprofile_sendReadByTypeReq(value_handle + 1, time_client_e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
        }
        else if (current_time_handle != 0)
        {
            // current time was last characteristic, search for descriptor up to the end of the service
            time_client_state = TIME_CLIENT_STATE_SEARCH_DESCRIPTOR;
            bleprofile_sendReadByTypeReq(current_time_handle, time_client_e_handle,
                                         UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
        }
        else
        {
            time_client_initialize_complete_callback(1);
        }
        break;
    
    case TIME_CLIENT_STATE_SEARCH_DESCRIPTOR:
        if (len)
        {
            UINT16 u16 = CCC_NOTIFICATION;

        	client_config_descriptor_handle = data[0] + (data[1] << 8);
            ble_trace1("current_time ccc hdl:%04x", client_config_descriptor_handle);

            time_client_state = TIME_CLIENT_STATE_WRITE_DESCRIPTOR;
            bleprofile_sendWriteReq(client_config_descriptor_handle, (UINT8 *)&u16, 2);
        }
        else
        {
			time_client_state = TIME_CLIENT_STATE_READ_CHARACTERISTIC;
			bleprofile_sendReadReq(current_time_handle);
        }
        break;

    case TIME_CLIENT_STATE_WRITE_DESCRIPTOR:
		time_client_state = TIME_CLIENT_STATE_READ_CHARACTERISTIC;
		bleprofile_sendReadReq(current_time_handle);
		break;

    case TIME_CLIENT_STATE_READ_CHARACTERISTIC:
        if (len != 0)
        {
        	time_client_notification_handler (len, attr_len, data);
        }
        time_client_state = TIME_CLIENT_STATE_IDLE;
		time_client_initialize_complete_callback(1);
        break;
    }
}

void time_client_notification_handler(int len, int attr_len, UINT8 *data)
{	
    ble_trace2("\rTIME Notification: attr_len:%d len:%d", (UINT16)attr_len, len);
	
    if (len)
    {
        // show the received time
        ble_trace3("(Date) %d/%02d/%02d", data[0] + (data[1] << 8), data[2], data[3]);
        ble_trace6("(Time) %02d:%02d:%02d.(%d/256) (Day) %d,  Reason:%x", data[4], data[5], data[6], data[7], data[8], data[9]);
    }	
}


