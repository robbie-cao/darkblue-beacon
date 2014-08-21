/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#define HID_REPORT_ID 2

// NVRAM IDs 1-15 are booked by the system
#define VS_WATCH_HOST_INFO              0x10
#define VS_GATT_CLIENT_HOST_INFO		0x11
#define VS_ANCS_CLIENT_HOST_INFO		0x12
#define VS_TIME_CLIENT_HOST_INFO		0x13
#define VS_FINDME_CLIENT_HOST_INFO		0x14

// ***** GATT service
#define HDLS_GATT 												0x0100

#define HDLC_GATT_SERVICE_CHANGED								0x0110
#define HDLC_GATT_SERVICE_CHANGED_VALUE							0x0111

// ***** GAP service
#define HDLS_GAP 												0x0200

// ----- Characteristic 'Device Name'
#define HDLC_GAP_DEVICE_NAME 									0x0210
#define HDLC_GAP_DEVICE_NAME_VALUE 								0x0211

// ----- Characteristic 'Appearance'
#define HDLC_GAP_APPEARANCE 									0x0220
#define HDLC_GAP_APPEARANCE_VALUE 								0x0221

// ***** Primary service 'Link Loss'
#define HDLS_LINK_LOSS 											0x0300

// ----- Characteristic 'Alert Level'
#define HDLC_LINK_LOSS_ALERT_LEVEL 								0x0310
#define HDLC_LINK_LOSS_ALERT_LEVEL_VALUE 						0x0311

// ***** Primary service 'Immediate Alert'
#define HDLS_IMMEDIATE_ALERT									0x0400

// ----- Characteristic 'Alert Level'
#define HDLC_IMMEDIATE_ALERT_ALERT_LEVEL 						0x0410
#define HDLC_IMMEDIATE_ALERT_ALERT_LEVEL_VALUE 					0x0411

// ***** Primary service 'Tx Power'
#define HDLS_TX_POWER 											0x0500

// ----- Characteristic 'Tx Power Level'
#define HDLC_TX_POWER_TX_POWER_LEVEL 							0x0510
#define HDLC_TX_POWER_TX_POWER_LEVEL_VALUE 						0x0511

// ***** Primary service 'Human Interface Device'
#define HDLS_HID 												0x0009

// ----- Characteristic 'HID Information'
#define HDLC_HID_HID_INFORMATION 								0x000a
#define HDLC_HID_HID_INFORMATION_VALUE 							0x000b

// ----- Characteristic 'Report Mode'
#define HDLC_HID_PROTOCOL_MODE 									0x000c
#define HDLC_HID_PROTOCOL_MODE_VALUE 							0x000d

// ----- Characteristic 'HID Control Point'
#define HDLC_HID_HID_CONTROL_POINT 								0x000e
#define HDLC_HID_HID_CONTROL_POINT_VALUE						0x000f

// ----- Characteristic 'Report Map'
#define HDLC_HID_REPORT_MAP 									0x0010
#define HDLC_HID_REPORT_MAP_VALUE 								0x0011

#define HDLC_HID_REPORT 										0x0012
#define HDLC_HID_REPORT_VALUE 									0x0013

// Client Configuration Descriptor
#define HDLD_HID_REPORT_CLIENT_CONFIGURATION 					0x0014

// Report Reference Descriptor
#define HDLD_HID_REPORT_REFERENCE								0x0015

#define HDLC_HID_REPORT1 										0x0016
#define HDLC_HID_REPORT1_VALUE 									0x0017
#define HDLD_HID_REPORT1_CLIENT_CONFIGURATION 					0x0018
#define HDLD_HID_REPORT1_REFERENCE								0x0019

#define HDLC_HID_REPORT2 										0x001a
#define HDLC_HID_REPORT2_VALUE 									0x001b
#define HDLD_HID_REPORT2_CLIENT_CONFIGURATION 					0x001c
#define HDLD_HID_REPORT2_REFERENCE								0x001d

// ***** Primary service 'Device Information'
#define HDLS_SCAN_PARAMETERS 								    0x07A0
#define HDLC_SCAN_REFRESH                                       0x07A2
#define HDLC_SCAN_REFRESH_VALUE                                 0x07A3
#define HDLD_SCAN_REFRESH_CLIENT_CONFIGURATION                  0x07A4
#define HDLC_SCAN_INTERVAL_WINDOW								0x07B0
#define HDLC_SCAN_INTERVAL_WINDOW_VALUE							0x07B1

// ***** Primary service 'Battery Service'
#define HDLS_BATTERY_SERVICE 									0x07F0

// ----- Characteristic 'Battery Level'
#define HDLC_BATTERY_SERVICE_BATTERY_LEVEL 						0x07F1
#define HDLC_BATTERY_SERVICE_BATTERY_LEVEL_VALUE 				0x07F2


// ***** Primary service 'Device Information'
#define HDLS_DEVICE_INFORMATION 								0x0800

// ----- Characteristic 'PnP ID'
#define HDLC_DEVICE_INFORMATION_PNP_ID 							0x0810
#define HDLC_DEVICE_INFORMATION_PNP_ID_VALUE 					0x0811

// ----- Characteristic 'Manufacturer Name String'
#define HDLC_DEVICE_INFORMATION_MANUFACTURER_NAME_STRING 		0x0820
#define HDLC_DEVICE_INFORMATION_MANUFACTURER_NAME_STRING_VALUE 	0x0821

#pragma pack(1)
// host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    // Current value of the client configuration descriptor for characteristic 'Report'
    UINT16 	human_interface_device_report_client_configuration;
    UINT16 	gatt_s_handle;
    UINT16 	gatt_e_handle;
    UINT16  current_time_s_handle;
    UINT16  current_time_e_handle;
    UINT16  findme_s_handle;
    UINT16  findme_e_handle;
    UINT16  ancs_s_handle;
    UINT16  ancs_e_handle;
}  HOSTINFO;

#pragma pack()

//extern definitions
extern const UINT8  gatt_database[];
extern const UINT16 gatt_database_len;
extern UINT8        gatt_db_stay_connected;     //= 1;  // Change that to 0 to disconnect when all messages are sent
extern UINT8        gatt_db_indication_sent;    //= 0;	// indication sent, waiting for ack
extern UINT16       gatt_db_connection_handle;  // HCI handle of connection, not zero when connected
extern HOSTINFO     *p_hostinfo_generated;      //pointer to the generated part of hostinfo

// functions definitions
extern void gatt_db_indication_cfm();
// registers timer. Should be called from gatt_db_create()
extern void gatt_db_reg_timer();
// find bonded peer in the hostinfo red from NVRAM. On exit gatt_db_bonded_idx points on result or 0xff if not found
extern void gatt_db_find_bonded_peer(UINT8 *bda);
// initializes common generated persistent values
extern void gatt_db_init_common_generated_values();
// initializes persistent values in the hostinfo to add bonded peer
extern void gatt_db_add_bond(UINT8 *bda);
// Writes characteristic descriptors from HOSTINFO_GENERATED to gdb
extern void write_hostinfo_generated_to_gatt_db();
// Sends notifications or/and indications for every client configuration descriptor
extern void send_message_generated(LEATT_NO_PARAM_CB cb);

// Updates HOSTINFO_GENERATED by the value written by peer.
// Returns true if any persistent value is changed
extern BOOL write_handler(UINT16 handle, int len, UINT8 *attrPtr);


// ------------------- You have to implement these functions ----------------

// custom function to be called every fine timer tick
extern void gatt_db_timer_fine();

// custom function to be called every 1 s
extern void gatt_db_timer_1s();

// It will be called at the write handler and should return TRUE if any persistent value is changed
extern BOOL on_write_hid_control_point(int len, UINT8 *attrPtr);

// ------------------- You can call these functions ----------------

// it should be called when 'Alert Level' is changed
extern void changed_link_loss_alert_level(UINT8* p_value, UINT8 value_len);

// it should be called when 'Tx Power Level' is changed
extern void changed_tx_power_tx_power_level(UINT8* p_value, UINT8 value_len);

// it should be called when 'Battery Level' is changed
extern void changed_battery_service_battery_level(UINT8* p_value, UINT8 value_len);

// it should be called when 'Report Map' is changed
extern void changed_human_interface_device_report_map(UINT8* p_value, UINT8 value_len);

// it should be called when 'HID Information' is changed
extern void changed_human_interface_device_hid_information(UINT8* p_value, UINT8 value_len);

// it should be called when 'Protocol Mode' is changed
extern void changed_human_interface_device_protocol_mode(UINT8* p_value, UINT8 value_len);

// it should be called when 'Report' is changed
extern void changed_hid_report(UINT8* p_value, UINT8 value_len, BOOL store, BOOL notify);

// it should be called when 'PnP ID' is changed
extern void changed_device_information_pnp_id(UINT8* p_value, UINT8 value_len);

// it should be called when 'Manufacturer Name String' is changed
extern void changed_device_information_manufacturer_name_string(UINT8* p_value, UINT8 value_len);


