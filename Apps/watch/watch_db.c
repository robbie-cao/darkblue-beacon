/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "watch_db.h"

const UINT8 gatt_database[]= // Define GATT database
{
// ***** GAP service
    //Handle HDLS_GAP: GAP service
    PRIMARY_SERVICE_UUID16 (1, UUID_SERVICE_GAP),

    //<Name>watch</Name>
    //Characteristic 'Device Name'
    CHARACTERISTIC_UUID16(2, 3,
				UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ,
				LEGATTDB_PERM_READABLE, 16),
    //UTF-8 'Device Name' watch
    'M','y',' ','W','a','t','c','h',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    //<Appearance>Unknown</Appearance>
    //Characteristic 'Appearance'
    CHARACTERISTIC_UUID16(4, 5,
				UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ,
				LEGATTDB_PERM_READABLE, 2),
	BIT16_TO_8(APPEARANCE_GENERIC_WATCH),

	CHARACTERISTIC_UUID16(6, 7,
			    UUID_CHARACTERISTIC_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS, LEGATTDB_CHAR_PROP_READ,
				LEGATTDB_PERM_READABLE, 8),
	0x06, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x32, 0x00,

	// ***** GATT service
	    //Handle HDLS_GATT: GATT service
	PRIMARY_SERVICE_UUID16 (8, UUID_SERVICE_GATT),

// ***** Primary service 'Human Interface Device'
    //<Name>Human Interface Device</Name>
    //<Uuid>1812</Uuid>
    //Service handle: HDLS_HID
    //Service UUID: UUID_SERVICE_HID
    PRIMARY_SERVICE_UUID16 (HDLS_HID, UUID_SERVICE_HID),

    //<Name>HID Information</Name>
    //<Uuid>2A4A</Uuid>
    CHARACTERISTIC_UUID16 (HDLC_HID_HID_INFORMATION, HDLC_HID_HID_INFORMATION_VALUE,
            	UUID_CHARACTERISTIC_HID_INFORMATION, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 4),
    0x11,0x01,0x00,0x00, // Verison 1.11, Not localized, Cannot remote wake, not normally connectable

    CHARACTERISTIC_UUID16_WRITABLE (HDLC_HID_PROTOCOL_MODE, HDLC_HID_PROTOCOL_MODE_VALUE,
    			UUID_CHARACTERISTIC_HID_PROTOCOL_MODE, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
    			LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD, 1),
    0x01,

    //<Name>HID Control Point</Name>
    //<Uuid>2A4C</Uuid>
    CHARACTERISTIC_UUID16_WRITABLE (HDLC_HID_HID_CONTROL_POINT, HDLC_HID_HID_CONTROL_POINT_VALUE,
    			UUID_CHARACTERISTIC_HID_CONTROL_POINT, LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE, LEGATTDB_PERM_WRITE_CMD, 1),
    0xff,

    CHARACTERISTIC_UUID16 (HDLC_HID_REPORT_MAP, HDLC_HID_REPORT_MAP_VALUE,
#if 0 // this might need to be set to 1 for Apple 6.0 (fixed in 7.0)
			UUID_CHARACTERISTIC_HID_REPORT_MAP, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 25),
#else
    			UUID_CHARACTERISTIC_HID_REPORT_MAP, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 91),
	// STD_KB_REPORT_ID
	// Input Report, 8 bytes
	// 1st byte:Keyboard LeftControl/Keyboard Right GUI
	// 2nd byte:Constant, 3rd ~ 6th: keycode
	// Output Report, 1 byte: LED control
	0x05 , 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09 , 0x06,                    // USAGE (Keyboard)
	0xA1 , 0x01,                    // COLLECTION (Application)
	0x85 , 0x01,        				//    REPORT_ID (1)
	0x75 , 0x01,                    //    REPORT_SIZE (1)
	0x95 , 0x08,                    //    REPORT_COUNT (8)
	0x05 , 0x07,                    //    USAGE_PAGE (Keyboard)
	0x19 , 0xE0,                    //    USAGE_MINIMUM (Keyboard LeftControl)
	0x29 , 0xE7,                    //    USAGE_MAXIMUM (Keyboard Right GUI)
	0x15 , 0x00,                    //    LOGICAL_MINIMUM (0)
	0x25 , 0x01,                    //    LOGICAL_MAXIMUM (1)
	0x81 , 0x02,                    //    INPUT (Data,Var,Abs)
	0x95 , 0x01,                    //    REPORT_COUNT (1)
	0x75 , 0x08,                    //    REPORT_SIZE (8)
	0x81 , 0x03,                    //    INPUT (Cnst,Var,Abs)
	0x95 , 0x05,                    //    REPORT_COUNT (5)
	0x75 , 0x01,                    //    REPORT_SIZE (1)
	0x05 , 0x08,                    //    USAGE_PAGE (LEDs)
	0x19 , 0x01,                    //    USAGE_MINIMUM (Num Lock)
	0x29 , 0x05,                    //    USAGE_MAXIMUM (Kana)
	0x91 , 0x02,                    //    OUTPUT (Data,Var,Abs)
	0x95 , 0x01,                    //    REPORT_COUNT (1)
	0x75 , 0x03,                    //    REPORT_SIZE (3)
	0x91 , 0x03,                    //    OUTPUT (Cnst,Var,Abs)
	0x95 , 0x06,                    //    REPORT_COUNT (6)
	0x75 , 0x08,                    //    REPORT_SIZE (8)
	0x15 , 0x00,                    //    LOGICAL_MINIMUM (0)
	0x26 , 0xFF , 0x00,             //    LOGICAL_MAXIMUM (255)
	0x05 , 0x07,                    //    USAGE_PAGE (Keyboard)
	0x19 , 0x00,                    //    USAGE_MINIMUM (Reserved (no event indicated))
	0x29 , 0xFF,                    //    USAGE_MAXIMUM (Reserved (no event indicated))
	0x81 , 0x00,                    //    INPUT (Data,Ary,Abs)
	0xC0,                           // END_COLLECTION
#endif
	0x05, 0x0C,       // USAGE_PAGE (Consumer Devices)
	0x09, 0x01,       // USAGE (Consumer Control)
	0xA1, 0x01,       // COLLECTION (Application)
	0x85, HID_REPORT_ID,       // Report ID (2)
	0x75, 0x10,       // REPORT_SIZE (10)
	0x95, 0x02,       // REPORT_COUNT (2)
	0x15, 0x01,       // LOGICAL_MINIMUM (1)
	0x26, 0x8c, 0x02, // LOGICAL_MAXIMUM (28c)
	0x19, 0x01,       // USAGE_MINIMUM (Button 1)
	0x2a, 0x8c, 0x02, // USAGE_MAXIMUM (Button 28c)
	0x81, 0x60,       // INPUT (data, array, abs)
	0xc0,             // END COLLECTION

    //<Name>Report</Name>
    //<Uuid>2A4D</Uuid>
    CHARACTERISTIC_UUID16 (HDLC_HID_REPORT, HDLC_HID_REPORT_VALUE,
    			UUID_CHARACTERISTIC_HID_REPORT, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY,
    			LEGATTDB_PERM_READABLE, 8),
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_HID_REPORT_CLIENT_CONFIGURATION,
				UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_AUTH_WRITABLE, 2),
    BIT16_TO_8 (CCC_NONE),

    CHAR_DESCRIPTOR_UUID16 (HDLD_HID_REPORT_REFERENCE,
    			UUID_DESCRIPTOR_REPORT_REFERENCE, LEGATTDB_PERM_READABLE, 2),
    0x01, 0x01,	// report ID = 1, Report type Input = 1

    CHARACTERISTIC_UUID16_WRITABLE (HDLC_HID_REPORT1, HDLC_HID_REPORT1_VALUE,
    			UUID_CHARACTERISTIC_HID_REPORT, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
    			LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 1),
    0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_HID_REPORT1_CLIENT_CONFIGURATION,
				UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_AUTH_WRITABLE, 2),
    BIT16_TO_8 (CCC_NONE),

    CHAR_DESCRIPTOR_UUID16 (HDLD_HID_REPORT1_REFERENCE,
    			UUID_DESCRIPTOR_REPORT_REFERENCE, LEGATTDB_PERM_READABLE, 2),
    0x01, 0x02,	// report ID = 1, Report type Input = 1

    CHARACTERISTIC_UUID16 (HDLC_HID_REPORT2, HDLC_HID_REPORT2_VALUE,
    			UUID_CHARACTERISTIC_HID_REPORT, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY,
    			LEGATTDB_PERM_READABLE, 4),
    0x00, 0x00, 0x00, 0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_HID_REPORT2_CLIENT_CONFIGURATION,
				UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_AUTH_WRITABLE, 2),
    BIT16_TO_8 (CCC_NONE),

    CHAR_DESCRIPTOR_UUID16 (HDLD_HID_REPORT2_REFERENCE,
    			UUID_DESCRIPTOR_REPORT_REFERENCE, LEGATTDB_PERM_READABLE, 2),

    0x02, 0x01,	// report ID = 1, Report type Input = 1

#if MSFT
    CHARACTERISTIC_UUID16 (30, 31,
                UUID_CHARACTERISTIC_HID_BOOT_KEYBOARD_INPUT,
    			LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 6),
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif
#if MSFT
    PRIMARY_SERVICE_UUID16 (HDLS_SCAN_PARAMETERS, UUID_SERVICE_SCAN_PARAMETERS),

    CHARACTERISTIC_UUID16  (HDLC_SCAN_REFRESH, HDLC_SCAN_REFRESH_VALUE,
    			UUID_CHARACTERISTIC_SCAN_REFRESH, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 1),
    0x00, 
    
    CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_SCAN_REFRESH_CLIENT_CONFIGURATION,
				UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
    BIT16_TO_8 (CCC_NONE),

    CHARACTERISTIC_UUID16_WRITABLE  (HDLC_SCAN_INTERVAL_WINDOW, HDLC_SCAN_INTERVAL_WINDOW_VALUE,
    			UUID_CHARACTERISTIC_SCAN_INTERVAL_WINDOW, LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_WRITE_CMD, 4),
    0x00, 0x00, 0x00, 0x00,
#endif

    // ***** Primary service 'Link Loss'
        //<Name>Link Loss</Name>
        //<Uuid>1803</Uuid>
        //Service handle: HDLS_LINK_LOSS
        //Service UUID: UUID_SERVICE_LINK_LOSS
        PRIMARY_SERVICE_UUID16 (HDLS_LINK_LOSS, UUID_SERVICE_LINK_LOSS),

        //<Name>Alert Level</Name>
        //<Uuid>2A06</Uuid>
        CHARACTERISTIC_UUID16_WRITABLE (HDLC_LINK_LOSS_ALERT_LEVEL, HDLC_LINK_LOSS_ALERT_LEVEL_VALUE,
    				UUID_CHARACTERISTIC_ALERT_LEVEL,
    				LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE,
    				LEGATTDB_PERM_READABLE  | LEGATTDB_PERM_WRITE_REQ, 1),
        0x01,

    // ***** Primary service 'Immediate Alert'
        //<Name>Immediate Alert</Name>
        //<Uuid>1802</Uuid>
        //Service handle: HDLS_IMMEDIATE_ALERT
        //Service UUID: UUID_SERVICE_IMMEDIATE_ALERT
        PRIMARY_SERVICE_UUID16 (HDLS_IMMEDIATE_ALERT, UUID_SERVICE_IMMEDIATE_ALERT),

        //<Name>Alert Level</Name>
        //<Uuid>2A06</Uuid>
        CHARACTERISTIC_UUID16_WRITABLE (HDLC_IMMEDIATE_ALERT_ALERT_LEVEL, HDLC_IMMEDIATE_ALERT_ALERT_LEVEL_VALUE,
    				UUID_CHARACTERISTIC_ALERT_LEVEL, LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE, LEGATTDB_PERM_WRITE_CMD, 0x1),
        0x00,

    // ***** Primary service 'Tx Power'
        //<Name>Tx Power</Name>
        //<Uuid>1804</Uuid>
        //Service handle: HDLS_TX_POWER
        //Service UUID: UUID_SERVICE_TX_POWER
        PRIMARY_SERVICE_UUID16 (HDLS_TX_POWER, UUID_SERVICE_TX_POWER),

        //<Name>Tx Power Level</Name>
        //<Uuid>2A07</Uuid>
        CHARACTERISTIC_UUID16 (HDLC_TX_POWER_TX_POWER_LEVEL, HDLC_TX_POWER_TX_POWER_LEVEL_VALUE,
    				UUID_CHARACTERISTIC_TX_POWER_LEVEL, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x04,

    // ***** Primary service 'Battery Service'
	//<Name>Battery Service</Name>
	//<Uuid>180F</Uuid>
	//Service handle: HDLS_BATTERY_SERVICE
	//Service UUID: UUID_SERVICE_BATTERY
	PRIMARY_SERVICE_UUID16 (HDLS_BATTERY_SERVICE, UUID_SERVICE_BATTERY),

	//<Name>Battery Level</Name>
	//<Uuid>2A19</Uuid>
	CHARACTERISTIC_UUID16 (HDLC_BATTERY_SERVICE_BATTERY_LEVEL, HDLC_BATTERY_SERVICE_BATTERY_LEVEL_VALUE,
				UUID_CHARACTERISTIC_BATTERY_LEVEL, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
	0x64,


// ***** Primary service 'Device Information'
    //<Name>Device Information</Name>
    //<Uuid>180A</Uuid>
    //Service handle: HDLS_DEVICE_INFORMATION
    //Service UUID: UUID_SERVICE_DEVICE_INFORMATION
    PRIMARY_SERVICE_UUID16 (HDLS_DEVICE_INFORMATION, UUID_SERVICE_DEVICE_INFORMATION),

    //<Name>PnP ID</Name>
    //<Uuid>2A50</Uuid>
    CHARACTERISTIC_UUID16 (HDLC_DEVICE_INFORMATION_PNP_ID, HDLC_DEVICE_INFORMATION_PNP_ID_VALUE,
    		    UUID_CHARACTERISTIC_PNP_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE | LEGATTDB_PERM_AUTH_READABLE, 7),
//    0x01,0x0f,0x00,0x03,0x00,0x01,0x00,	// BT SIG, BRCM, 0x0003, 0x0001
    0x02,0x5C,0x0A,0x20,0x20,0x01,0x00,

    //<Name>Manufacturer Name String</Name>
    //<Uuid>2A29</Uuid>
//    CHARACTERISTIC_UUID16 (HDLC_DEVICE_INFORMATION_MANUFACTURER_NAME_STRING, HDLC_DEVICE_INFORMATION_MANUFACTURER_NAME_STRING_VALUE,
//    			UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
//    'B','r','o','a','d','c','o','m',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

};
const UINT16 gatt_database_len = sizeof(gatt_database);	// Define GATT database

// Flag meaning value 'Report' is changed. Notification or Indication has to be sent to the client
UINT8 b_changed_human_interface_device_report = 0;

// timer callback function for gatt_db_reg_timer
void gatt_db_timeout(UINT32 arg)
{
    if(arg == BLEPROFILE_GENERIC_APP_TIMER)
        gatt_db_timer_1s();
}
// timer callback function for gatt_db_reg_timer
void gatt_db_fine_timeout(UINT32 arg)
{
    gatt_db_timer_fine();
}

// registers timer. Should be called from gatt_db_create()
void gatt_db_reg_timer()
{
    bleprofile_regTimerCb(gatt_db_fine_timeout, gatt_db_timeout);
    bleprofile_StartTimer();
}

// initializes common generated persistent values
void gatt_db_init_common_generated_values()
{
}

// Writes characteristic descriptors from HOSTINFO_GENERATED to gatt db
void write_hostinfo_generated_to_gatt_db()
{
}


// it should be called when 'Alert Level' is changed.
void changed_link_loss_alert_level(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_LINK_LOSS_ALERT_LEVEL_VALUE, &db_pdu);
}

// it should be called when 'Tx Power Level' is changed.
void changed_tx_power_tx_power_level(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_TX_POWER_TX_POWER_LEVEL_VALUE, &db_pdu);
}

// it should be called when 'Battery Level' is changed.
void changed_battery_service_battery_level(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_BATTERY_SERVICE_BATTERY_LEVEL_VALUE, &db_pdu);
}

// it should be called when 'Report Map' is changed.
void changed_human_interface_device_report_map(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_HID_REPORT_MAP_VALUE, &db_pdu);
}

// it should be called when 'HID Information' is changed.
void changed_human_interface_device_hid_information(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_HID_HID_INFORMATION_VALUE, &db_pdu);
}

// it should be called when 'Report' is changed.
void changed_hid_report(UINT8* p_value, UINT8 value_len, BOOL store, BOOL notify)
{
}

// it should be called when 'PnP ID' is changed.
void changed_device_information_pnp_id(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_DEVICE_INFORMATION_PNP_ID_VALUE, &db_pdu);
}

// it should be called when 'Manufacturer Name String' is changed.
void changed_device_information_manufacturer_name_string(UINT8* p_value, UINT8 value_len)
{
    // write value to the GATT DB
    BLEPROFILE_DB_PDU db_pdu;
    memcpy(&db_pdu.pdu[0], p_value, value_len);
    db_pdu.len = value_len;
    bleprofile_WriteHandle(HDLC_DEVICE_INFORMATION_MANUFACTURER_NAME_STRING_VALUE, &db_pdu);
}

