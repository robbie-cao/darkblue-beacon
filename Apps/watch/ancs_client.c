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
* During initialization the app registers with LE stack to receive various
* notifications including bonding complete, connection status change and
* peer notification.  When device is successfully bonded, application saves
* peer's Bluetooth Device address to the NVRAM and starts GATT service
* discovery.  The ANCS UUIDs are published at
* https://developer.apple.com/library/IOS/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html
* If service discovery is successful application writes into
* appropriate Characteristic Client Configuration descriptor registering
* for notifications from the iOS device.  Received messages are printed
* out to the device output.
*
* Features demonstrated
*  - Registration with LE stack for various events
*  - performing GATT service discovery
*  - working with ANCS service on iOS device
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Start tracing to monitor the activity (see quick start guide for details)
* 4. Pair with an iOS device (please note that iOS does not like to pair with a device, use some app instead)
* 5. Send an SMS or generate incoming call to you iOS device
*
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "watch_db.h"
#include "ancs_client.h"
#include "spar_utils.h"

// define 1 if ANCS is a full application, i.e. needs to do advertisements, pairing...
#define ANCS_FULL_APP	0

/******************************************************
 *                      Constants
 ******************************************************/

#define ANCS_CHAR_TYPE_NONE                 	0
#define ANCS_CHAR_TYPE_NOTIFICATION_SOURCE  	1
#define ANCS_CHAR_TYPE_CONTROL_POINT        	2
#define ANCS_CHAR_TYPE_DATA_SOURCE          	3

#define ANCS_EVENT_ID_NOTIFICATION_ADDED    	0
#define ANCS_EVENT_ID_NOTIFICATION_MODIFIED 	1
#define ANCS_EVENT_ID_NOTIFICATION_REMOVED  	2
#define ANCS_EVENT_ID_MAX                   	3

#define ANCS_NOTIFICATION_ATTR_ID_APP_ID        0
#define ANCS_NOTIFICATION_ATTR_ID_TITLE         1
#define ANCS_NOTIFICATION_ATTR_ID_SUBTITLE      2
#define ANCS_NOTIFICATION_ATTR_ID_MESSAGE       3
#define ANCS_NOTIFICATION_ATTR_ID_MESSAGE_SIZE  4
#define ANCS_NOTIFICATION_ATTR_ID_DATE          5
#define ANCS_NOTIFICATION_ATTR_ID_MAX           6

#define ANCS_EVENT_FLAG_SILENT     (1 << 0)
#define ANCS_EVENT_FLAG_IMPORTANT  (1 << 1)

#define ANCS_SERVICE_MAX_CHARS                  5

#if 0
static char *EventId[] =
{
    "Added   ",
    "Modified",
    "Removed ",
    "Unknown "
};

static char *EventFlag[] =
{
    "Silent ",
    "!      ",
    "!Silent",
};

#define ANCS_CATEGORY_ID_MAX    12
static char *CategoryId[] =
{
    "Other             ",
    "IncomingCall      ",
    "MissedCall        ",
    "Voicemail         ",
    "Social            ",
    "Schedule          ",
    "Email             ",
    "News              ",
    "HealthAndFitness  ",
    "BusinessAndFinance",
    "Location          ",
    "Entertainment     ",
    "Unknown           "
};

static char *NotificationAttributeID[] =
{
        "AppIdentifier",
        "Title        ",
        "Subtitle     ",
        "Message      ",
        "MessageSize  ",
        "Date         ",
        "Unknown      "
};

#endif

#define ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES     0

// following is the list of notification attributes that we are going
// to request.
UINT8 ancs_client_notification_attribute[] =
{
    ANCS_NOTIFICATION_ATTR_ID_APP_ID,
    ANCS_NOTIFICATION_ATTR_ID_TITLE,
    ANCS_NOTIFICATION_ATTR_ID_SUBTITLE,
    ANCS_NOTIFICATION_ATTR_ID_MESSAGE_SIZE,
    ANCS_NOTIFICATION_ATTR_ID_MESSAGE,
    ANCS_NOTIFICATION_ATTR_ID_DATE,
    0
};
UINT8 ancs_client_notification_attribute_length[] =
{
    0,
    20,
    20,
    0,
    255,
    0,
    0
};
UINT8   ancs_client_notification_attribute_inx = 0;

// service discovery states
enum
{
    ANCS_CLIENT_STATE_IDLE                                           = 0x00,
    ANCS_CLIENT_STATE_SMP_DONE                                       = 0x01,
    ANCS_CLIENT_STATE_DISCOVER_DONE                                  = 0x02,

    ANCS_CLIENT_STATE_WAIT_READ_PRIMARY_SERVICE                      = 0x11,
    ANCS_CLIENT_STATE_WAIT_READ_GATT_CHARACTERISTIC                  = 0x13,
    ANCS_CLIENT_STATE_WAIT_READ_ANCS_CHARACTERISTIC                  = 0x15,
    ANCS_CLIENT_STATE_WAIT_READ_DESCRIPTOR                           = 0x17,
    ANCS_CLIENT_STATE_WRITE_CCC                                      = 0x27,
    ANCS_CLIENT_STATE_WRITE_CCC_RESTARTING                           = 0x28,

    ANCS_CLIENT_STATE_FAIL                                           = 0xFF,
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
}  ANCS_HOSTINFO;
#pragma pack()


/******************************************************
 *               Function Prototypes
 ******************************************************/
void     		ancs_client_connection_up(void);
void     		ancs_client_connection_down(void);
void     		ancs_client_smp_bond_result(LESMP_PARING_RESULT result);
void     		ancs_client_encryption_changed(HCI_EVT_HDR *evt);
void     		ancs_client_notification_handler(int len, int attr_len, UINT8 *data);
static void     ancs_client_process_rsp(int len, int attr_len, UINT8 *data);
static void     ancs_client_write_rsp(void);
static UINT16   ancs_client_find_e_handle(UINT16 hdl);


/******************************************************
 *               Variables Definitions
 ******************************************************/

static  BLEPROFILE_SINGLE_PARAM_CB ancs_client_initialize_complete_callback = NULL;

// NVRAM save area
ANCS_HOSTINFO ancs_client_hostinfo;

typedef struct t_ANCS_CLIENT
{
	UINT8   state;
	UINT16  connection_handle;
	BD_ADDR remote_addr;
	UINT16  ancs_e_handle;
	UINT8   ancs_char_type[ANCS_SERVICE_MAX_CHARS];
	UINT16  ancs_char_hdl[ANCS_SERVICE_MAX_CHARS];
	UINT16  current_characteristic_index;
	UINT16  current_descriptor_index;
    UINT16  service_changed_handle;
    UINT16  notification_source_handle;
    UINT16  data_source_handle;
    UINT16  control_point_handle;
    UINT16  client_ancs_descr_hdl[ANCS_SERVICE_MAX_CHARS];
} ANCS_CLIENT;

ANCS_CLIENT ancs_client;

/******************************************************
 *               Function Definitions
 ******************************************************/

// This function will be called on every connection establishmen
void ancs_client_connection_up(void)
{
    UINT8 *bda;

    ancs_client.connection_handle = (UINT16)emconinfo_getConnHandle();

    // save address of the connected device and print it out.
    memcpy(ancs_client.remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(ancs_client.remote_addr));

    ble_trace3("ancs_client.connection_up: %08x%04x %d\n",
                (ancs_client.remote_addr[5] << 24) + (ancs_client.remote_addr[4] << 16) +
                (ancs_client.remote_addr[3] << 8) + ancs_client.remote_addr[2],
                (ancs_client.remote_addr[1] << 8) + ancs_client.remote_addr[0],
                ancs_client.connection_handle);

    bleprofile_ReadNVRAM(VS_ANCS_CLIENT_HOST_INFO, sizeof(ancs_client_hostinfo), (UINT8 *)&ancs_client_hostinfo);
}


// This function will be called when connection goes down
void ancs_client_connection_down(void)
{
    ble_trace3("ancs_client_connection_down:%08x%04x handle:%d\n",
                (ancs_client.remote_addr[5] << 24) + (ancs_client.remote_addr[4] << 16) +
                (ancs_client.remote_addr[3] << 8) + ancs_client.remote_addr[2],
                (ancs_client.remote_addr[1] << 8) + ancs_client.remote_addr[0],
                ancs_client.connection_handle);

	memset (&ancs_client, 0, sizeof (ancs_client));
}

//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void ancs_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("ancs_client, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda = (UINT8 *)emconninfo_getPeerAddr();
        UINT8 writtenbyte;

        // every time after pairing we need to perform discovery to
        // find out handles of the service.
        memset (&ancs_client_hostinfo, 0, sizeof (HOSTINFO));
        memcpy(ancs_client_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

        writtenbyte = bleprofile_WriteNVRAM(VS_ANCS_CLIENT_HOST_INFO, sizeof(ancs_client_hostinfo), (UINT8 *)&ancs_client_hostinfo);
        ble_trace1("NVRAM write:%d", writtenbyte);
    }
}

// command from the main app to start search for characteristics
int ancs_client_initialize(UINT16 s_handle, UINT16 e_handle, BLEPROFILE_SINGLE_PARAM_CB initialize_complete_callback)
{
    if ((s_handle == 0) || ((ancs_client.ancs_e_handle = e_handle) == 0))
    {
        return FALSE;
    }

    // Functions of this module willl need to receive GATT client callbacks
    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) ancs_client_process_rsp);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) ancs_client_process_rsp);
    leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) ancs_client_process_rsp);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) ancs_client_write_rsp);

    ancs_client_initialize_complete_callback = initialize_complete_callback;

    memset (&ancs_client, 0, sizeof (ancs_client));
    ancs_client.state = ANCS_CLIENT_STATE_WAIT_READ_ANCS_CHARACTERISTIC;
    ble_trace1("state = %02x", ancs_client.state);
    bleprofile_sendReadByTypeReq(s_handle, e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
    return TRUE;
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void ancs_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8             status = *((UINT8 *)(evt + 1));
    ble_trace2("ANCS encryption changed:%02x bonded:%d", status, emconninfo_deviceBonded());
}

// process responses from the server during the GATT discovery
void ancs_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int    i;
    UINT16 uuid, s_handle, e_handle;

    ble_trace3("\rANCS Client Rsp len:%d, attr_len:%d state = %02x", len, attr_len, ancs_client.state);

    switch (ancs_client.state)
    {
    case ANCS_CLIENT_STATE_WAIT_READ_ANCS_CHARACTERISTIC:
        if (len)
        {
            // HANDLE(2) PERM(1) HANDLE(2) UUID(2-16)
            if (attr_len == 21)
            {
                UINT16 handle = data[i + 3] + (data[i + 4] << 8);
                ancs_client.ancs_char_hdl[ancs_client.current_characteristic_index] = handle;
                if (BT_MEMCMP(&data[i + 5], ANCS_NOTIFICATION_SOURCE, 16) == 0)
                {
                    ancs_client.ancs_char_type[ancs_client.current_characteristic_index] = ANCS_CHAR_TYPE_NOTIFICATION_SOURCE;
                    ancs_client.notification_source_handle = handle;
                    ble_trace1("notification source hdl:%04x", ancs_client.notification_source_handle);
                }
                else if (BT_MEMCMP(&data[i + 5], ANCS_CONTROL_POINT, 16) == 0)
                {
                    ancs_client.ancs_char_type[ancs_client.current_characteristic_index] = ANCS_CHAR_TYPE_CONTROL_POINT;
                    ancs_client.control_point_handle = handle;
                    ble_trace1("control point hdl:%04x", ancs_client.control_point_handle);
                }
                else if (BT_MEMCMP(&data[i + 5], ANCS_DATA_SOURCE, 16) == 0)
                {
                    ancs_client.ancs_char_type[ancs_client.current_characteristic_index] = ANCS_CHAR_TYPE_DATA_SOURCE;
                    ancs_client.data_source_handle = handle;
                    ble_trace1("data source hdl:%04x", ancs_client.data_source_handle);
                }
                else
                {
                    ancs_client.ancs_char_type[ancs_client.current_characteristic_index] = ANCS_CHAR_TYPE_NONE;
                    ble_trace1("unknown hdl:%04x", handle);
                }
                ancs_client.current_characteristic_index++;
                bleprofile_sendReadByTypeReq(handle + 1, ancs_client.ancs_e_handle, UUID_ATTRIBUTE_CHARACTERISTIC);
            }
        }
        else
        {
            // done with ANCS characteristics, start reading descriptor handles
            for (ancs_client.current_descriptor_index = 0;
                 ancs_client.current_descriptor_index < ancs_client.current_characteristic_index;
                 ancs_client.current_descriptor_index++)
            {
                if ((ancs_client.ancs_char_type[ancs_client.current_descriptor_index] == ANCS_CHAR_TYPE_NOTIFICATION_SOURCE) ||
                    (ancs_client.ancs_char_type[ancs_client.current_descriptor_index] == ANCS_CHAR_TYPE_DATA_SOURCE))
                {
                    ancs_client.state = ANCS_CLIENT_STATE_WAIT_READ_DESCRIPTOR;
                    bleprofile_sendReadByTypeReq(ancs_client.ancs_char_hdl[ancs_client.current_descriptor_index],
                                                 ancs_client_find_e_handle (ancs_client.ancs_char_hdl[ancs_client.current_descriptor_index]),
                                                 UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
                    break;
                }
            }
            if (ancs_client.current_descriptor_index == ancs_client.current_characteristic_index)
            {
                ancs_client.state = ANCS_CLIENT_STATE_IDLE;
                (*ancs_client_initialize_complete_callback)(1);
            }
        }
        ble_trace1("state = %02x", ancs_client.state);
        break;

    case ANCS_CLIENT_STATE_WAIT_READ_DESCRIPTOR:
        if (len)
        {
            UINT16 handle = data[0] + (data[1] << 8);
            ble_trace3("descriptor type:%d char hdl:%04x descr hdl:%04x",
                    ancs_client.ancs_char_type[ancs_client.current_descriptor_index],
                    ancs_client.ancs_char_hdl[ancs_client.current_descriptor_index], handle);
            ancs_client.client_ancs_descr_hdl[ancs_client.current_descriptor_index++] = handle;
        }
        for (; ancs_client.current_descriptor_index < ancs_client.current_characteristic_index;
               ancs_client.current_descriptor_index++)
        {
            if ((ancs_client.ancs_char_type[ancs_client.current_descriptor_index] == ANCS_CHAR_TYPE_NOTIFICATION_SOURCE) ||
                (ancs_client.ancs_char_type[ancs_client.current_descriptor_index] == ANCS_CHAR_TYPE_DATA_SOURCE))
            {
                bleprofile_sendReadByTypeReq(ancs_client.ancs_char_hdl[ancs_client.current_descriptor_index],
                                             ancs_client_find_e_handle (ancs_client.ancs_char_hdl[ancs_client.current_descriptor_index]),
                                             UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
                break;
            }
        }
        if (ancs_client.current_descriptor_index == ancs_client.current_characteristic_index)
        {
            // done with discovering ANCS descriptors, registering for notifications
            for (ancs_client.current_descriptor_index = 0;
                 ancs_client.current_descriptor_index < ancs_client.current_characteristic_index;
                 ancs_client.current_descriptor_index++)
            {
                if (ancs_client.client_ancs_descr_hdl[ancs_client.current_descriptor_index] != 0)
                {
                    UINT16 u16 = CCC_NOTIFICATION;
                    ancs_client.state = ANCS_CLIENT_STATE_WRITE_CCC;
                    bleprofile_sendWriteReq(ancs_client.client_ancs_descr_hdl[ancs_client.current_descriptor_index], (UINT8 *)&u16, 2);
                    break;
                }
            }
            if (ancs_client.current_descriptor_index == ancs_client.current_characteristic_index)
            {
                ancs_client.state = ANCS_CLIENT_STATE_IDLE;
                (*ancs_client_initialize_complete_callback)(1);
            }
        }
        ble_trace2("ancs_client_state = %02x current_descr:%d", ancs_client.state, ancs_client.current_descriptor_index);
        break;
    }
    ble_trace1("\rANCS Client Rsp state = %02x", ancs_client.state);
}

void     ancs_client_write_rsp(void)
{
    if ((ancs_client.state == ANCS_CLIENT_STATE_WRITE_CCC) || (ancs_client.state == ANCS_CLIENT_STATE_WRITE_CCC_RESTARTING))
    {
        for (++ancs_client.current_descriptor_index;
             ancs_client.current_descriptor_index < ancs_client.current_characteristic_index;
             ancs_client.current_descriptor_index++)
        {
            if (ancs_client.client_ancs_descr_hdl[ancs_client.current_descriptor_index] != 0)
            {
                UINT16 u16 = CCC_NOTIFICATION;
                bleprofile_sendWriteReq(ancs_client.client_ancs_descr_hdl[ancs_client.current_descriptor_index], (UINT8 *)&u16, 2);
                break;
            }
        }
        if (ancs_client.current_descriptor_index == ancs_client.current_characteristic_index)
        {
        	if (ancs_client.state == ANCS_CLIENT_STATE_WRITE_CCC)
        	{
            UINT8 writtenbyte;

            ancs_client.state = ANCS_CLIENT_STATE_IDLE;

            // this is the end of the discovery and registration.
            // save handles in the NVRAM
            writtenbyte = bleprofile_WriteNVRAM(VS_ANCS_CLIENT_HOST_INFO, sizeof(ancs_client_hostinfo), (UINT8 *)&ancs_client_hostinfo);
            ble_trace1("NVRAM write:%d\n", writtenbyte);

            (*ancs_client_initialize_complete_callback)(0);
        }
        	else
        	{
				ancs_client.state = ANCS_CLIENT_STATE_IDLE;
        	}
        }
        ble_trace1("\rANCS Client Rsp state = %02x", ancs_client.state);
    }
}

void ancs_client_send_next_get_notification_attributes_command(UINT32 uid)
{
    UINT8 command[20];
    UINT8 *p_command = &command[5];

    command[0] = ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES;
    command[1] = uid & 0xff;
    command[2] = (uid >> 8) & 0xff;
    command[3] = (uid >> 16) & 0xff;
    command[4] = (uid >> 24) & 0xff;

    *p_command++ = ancs_client_notification_attribute[ancs_client_notification_attribute_inx];
    if (ancs_client_notification_attribute_length[ancs_client_notification_attribute_inx] != 0)
    {
        *p_command++ = ancs_client_notification_attribute_length[ancs_client_notification_attribute_inx] & 0xff;
        *p_command++ = (ancs_client_notification_attribute_length[ancs_client_notification_attribute_inx] >> 8) & 0xff;
    }
    bleprofile_sendWriteReq(ancs_client.control_point_handle, command, (UINT8)(p_command - command));
}

void ancs_client_process_notification_source(int len, UINT8 *data)
{
//    ble_trace5 ("\rANCS Notification EventID:%s EventFlags:%s CategoryID:%s CategoryCount:%3d NotificationUID:%04x",
//            (UINT32)(data[0] < ANCS_EVENT_ID_MAX) ? EventId[data[0]] : EventId[ANCS_EVENT_ID_MAX],
//            (UINT32)(data[1] == ANCS_EVENT_FLAG_SILENT) ? EventFlag[0] : ((data[1] == ANCS_EVENT_FLAG_IMPORTANT) ? EventFlag[1] : EventFlag[2]),
//            (UINT32)(data[2] < ANCS_CATEGORY_ID_MAX) ? CategoryId[data[2]] : CategoryId[ANCS_CATEGORY_ID_MAX],
//            data[3],
//            data[4] + (data[5] << 8) + (data[5] << 16) + (data[5] << 24));
//    ble_tracen((char *)data, len);
//    ble_trace2("notification type %d, ancs_client_notification_attribute_inx = %d \r\n", data[0], ancs_client_notification_attribute_inx);
    if ((data[0] == ANCS_EVENT_ID_NOTIFICATION_ADDED) || (data[0] == ANCS_EVENT_ID_NOTIFICATION_MODIFIED))
    {
        // if it is a new or modified notification, write to the control point to retrieve details.
        if (ancs_client_notification_attribute_inx == 0)
        {
            ancs_client_send_next_get_notification_attributes_command(data[4] + (data[5] << 8) + (data[6] << 16) + (data[7] << 24));
        }
        else
        {
        	ble_trace0("skip retrieve ntf att\r\n");
        }
    }
    else
    {
    	ble_trace0("unsupported ntf type\r\n");
    }
}

UINT16 current_leftovers = 0;
UINT16 current_data_source_buffer_offset = 0;
UINT8  data_source_buffer[256];

void ancs_client_display_message(UINT8 *data, int len)
{
	ble_trace1("data length %d\r\n", len);
	len = len > 128? 128 : len;
    data[len] = 0;
    ble_trace2("Type :%d %s", data[0], (UINT32)&data[3]);
}

// process data source stream.
void ancs_client_process_data_source(int len, UINT8 *data)
{
    UINT8 attr_id;
    UINT8 attr_len;

//    ble_trace2("Data source leftovers:%d len:%d", current_leftovers, len);
//    ble_tracen (data, len);
    // check if this is a continuation of the previous message
    if (current_leftovers)
    {
        BT_MEMCPY(&data_source_buffer[current_data_source_buffer_offset], data, len);
        current_data_source_buffer_offset += len;
        current_leftovers -= len;
        if (current_leftovers <= 0)
        {
            ancs_client_display_message(&data_source_buffer[5], current_data_source_buffer_offset - 5);

            if (ancs_client_notification_attribute[++ancs_client_notification_attribute_inx] != 0)
            {
                ancs_client_send_next_get_notification_attributes_command(data_source_buffer[1] + (data_source_buffer[2] << 8) + (data_source_buffer[3] << 16) + (data_source_buffer[4] << 24));
            }
            else
            {
                ancs_client_notification_attribute_inx = 0;
            }
        }
    }
    else
    {
        // start of the new message
        attr_id  = data[5];
        attr_len = data[6] + (data[7] << 8);
        ble_trace2("\rANCS Data Notification Attribute:%04x len %d", attr_id, attr_len);
        if (attr_len <= len - 8)
        {
            ancs_client_display_message((char *)&data[5], len - 5);
            current_leftovers = 0;
            if (ancs_client_notification_attribute[++ancs_client_notification_attribute_inx] != 0)
            {
                ancs_client_send_next_get_notification_attributes_command(data[1] + (data[2] << 8) + (data[3] << 16) + (data[4] << 24));
            }
            else
            {
                ancs_client_notification_attribute_inx = 0;
            }
        }
        else
        {
            BT_MEMCPY(&data_source_buffer[0], data, len);
            current_data_source_buffer_offset = len;
            current_leftovers = attr_len - len + 8;
        }
    }
}

// process notifications from the client
void ancs_client_notification_handler(int len, int handle, UINT8 *data)
{
    if (current_leftovers || (handle == ancs_client.data_source_handle))
    {
        ancs_client_process_data_source(len, data);
    }
    else if (handle == ancs_client.notification_source_handle)
    {
        ancs_client_process_notification_source(len, data);
    }
    else
    {
        ble_trace2("\rANCS Notification bad handle:%02x, %d", (UINT16)handle, len);
    }
}

void ancs_client_interrupt_handler(UINT8 value)
{
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace3("\r(INT)But1:%d But2:%d But3:%d", value&0x01, (value& 0x02) >> 1, (value & 0x04) >> 2);
    ble_trace3("\rAdv high", value&0x01, (value& 0x02) >> 1, (value & 0x04) >> 2);
    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, ancs_client.remote_addr);
}

// find end handle for the current characteristic
UINT16 ancs_client_find_e_handle(UINT16 char_hdl)
{
    UINT16 return_hdl = ancs_client.ancs_e_handle;
    UINT16 i;

    for (i = 0; i < ANCS_SERVICE_MAX_CHARS; i++)
    {
        if ((ancs_client.ancs_char_hdl[i] != 0) &&
            (ancs_client.ancs_char_hdl[i] > char_hdl) &&
            (ancs_client.ancs_char_hdl[i] < return_hdl))
        {
            return_hdl = ancs_client.ancs_char_hdl[i];
        }
    }
    return (return_hdl);
}

// Send request to client to update connection parameters
void bleprofile_SendConnParamUpdateReq(UINT16 minInterval, UINT16 maxInterval, UINT16 slaveLatency, UINT16 timeout)
{
    lel2cap_sendConnParamUpdateReq(minInterval, maxInterval, slaveLatency, timeout);
}

