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
* BLE Automation IO profile, service, application
*
* These are updates to the ROM code for BLE Automation IO device.
*
* Refer to Bluetooth SIG Automation IO Profile 0.9 and Automation IO Service
* 0.9 specifications for details.
*
* The ROM code is published in Wiced-Smart\bleapp\app\bleaio.c file. This
* code replaces GATT database.
*
* Features demonstrated
*  - Automation IO implementation
*  - Replacing some of the ROM functionality
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "bleaio.h"

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

const UINT8 automation_io_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service Changed, handle 0x03 characteristic value
    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
        0x00, 0x00, 0x00, 0x00,

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP ),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','A','u','t','o','m','a','t','i','o','n','I','O',

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00, 0x00,

    // Handle 0x21:  Automation IO
    PRIMARY_SERVICE_UUID16 (0x0021, UUID_SERVICE_AUTOMATION_IO),

    // Handle 0x22: characteristic Digital Input, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16 (0x0022, 0x0023, UUID_CHARACTERISTIC_DIGITAL_INPUT,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE, 1),
        0x00,

    // Handle 0x24: Client Characteristic Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0024, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x25: Number of Digitials Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0025, UUID_DESCRIPTOR_NUMBER_OF_DIGITALS, LEGATTDB_PERM_READABLE, 1),
        0x02,                       // 2 digital input

    // Handle 0x26: Trigger Settings Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0026, UUID_DESCRIPTOR_TRIGGER_SETTING,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,

    // Handle 0x27: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0027, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT,
                            LEGATTDB_PERM_READABLE, 7),
        0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, // 2bit

    // Handle 0x28: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0028, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'D','I','0','0',

    // Handle 0x32: characteristic Digital Output, handle 0x33 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0032, 0x0033, UUID_CHARACTERISTIC_DIGITAL_OUTPUT,
                           LEGATTDB_CHAR_PROP_WRITE| LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                           LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ,  1),
        0x00,

    // Handle 0x34: Number of Digitials Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0034, UUID_DESCRIPTOR_NUMBER_OF_DIGITALS, LEGATTDB_PERM_READABLE, 1),
        0x02,                       // 2 digital output

    // Handle 0x37: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0037, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT, LEGATTDB_PERM_READABLE, 7),
        0x03,0x00,0x00,0x00,0x01,0x00,0x00, // 4bit

    // Handle 0x38: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16( 0x0038, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'D','O','0','0',

    // Handle 0x42: characteristic Analog Input, handle 0x43 characteristic value
    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_ANALOG_INPUT,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE, 2),
        0x00, 0x00,

    // Handle 0x44: Client Characteristic Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0044, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,                 // write 1 will trigger notification, 2 - indication

    // Handle 0x46: Trigger Settings Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0046, UUID_DESCRIPTOR_TRIGGER_SETTING,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,     //

    // Handle 0x47: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16( 0x0047, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT, LEGATTDB_PERM_READABLE, 7),
        0x06,0x00,0x00,0x00,0x01,0x00,0x00, //uint16

    // Handle 0x48: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16( 0x0048, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'A','I','0','0',

    // Handle 0x52: characteristic Digital Input, handle 0x53 characteristic value
    CHARACTERISTIC_UUID16 (0x0052, 0x0053, UUID_CHARACTERISTIC_ANALOG_INPUT,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE, 2),
        0x00, 0x00,

    // Handle 0x54: Client Characteristic Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0054, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,                 // write 1 will trigger notification, 2 - indication

    // Handle 0x56: Trigger Settings Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0056, UUID_DESCRIPTOR_TRIGGER_SETTING,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,     //

    // Handle 0x57: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0057, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT, LEGATTDB_PERM_READABLE, 7),
        0x06,0x00,0x00,0x00,0x01,0x00,0x00, //uint16

    // Handle 0x58: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0058, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'A','I','0','1',

    // Handle 0x62: characteristic Analog Output, handle 0x63 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0062, 0x0063, UUID_CHARACTERISTIC_ANALOG_OUTPUT,
                                    LEGATTDB_CHAR_PROP_WRITE| LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                                    LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x67: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0067, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT,LEGATTDB_PERM_READABLE, 7),
        0x06,0x00,0x00,0x00,0x01,0x00,0x00, // uint16

    // Handle 0x68: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0068, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'A','O','0','0',

    // Handle 0x72: characteristic Analog Output, handle 0x73 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0072, 0x0073, UUID_CHARACTERISTIC_ANALOG_OUTPUT,
                                     LEGATTDB_CHAR_PROP_WRITE| LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                                     LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x77: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0077, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT, LEGATTDB_PERM_READABLE, 7),
        0x06,0x00,0x00,0x00,0x01,0x00,0x00, // uint16

    // Handle 0x78: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0078, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'A','O','0','1',

    // Handle 0x82: characteristic Aggregate Input, handle 0x83 characteristic value
    CHARACTERISTIC_UUID16 (0x0082, 0x0083, UUID_CHARACTERISTIC_AGGREGATE_INPUT,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE, 5),
        0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x84: Client Characteristic Descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0084, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),

        0x00, 0x00,                 // write 1 will trigger notification, 2 - indication

    // Handle 0x87: Characteristic Presentation Format Descriptor
    CHAR_DESCRIPTOR_UUID16( 0x0087, UUID_DESCRIPTOR_CHARACTERISTIC_PRESENTATION_FORMAT, LEGATTDB_PERM_READABLE, 7),
        0x19, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, //utf8s

    // Handle 0x88: User Description Descriptor
    CHAR_DESCRIPTOR_UUID16( 0x0088, UUID_DESCRIPTOR_CHARACTERISTIC_USER_DESCRIPTION, LEGATTDB_PERM_READABLE, 4),
        'A','g','g','I',

#ifdef DUMMY_TRIGGER
    // Dummy trigger
    PRIMARY_SERVICE_UUID16 (0x0091, UUID_SERVICE_AUTOMATION_IO_TRIGGER),

    // Handle 0x92: characteristic Dummy Digital Input Trigger, handle 0x93 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0092, 0x0093, UUID_CHARACTERISTIC_DIGITAL_INPUT_TRIGGER,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,     //

    // Handle 0x94: characteristic Dummy Analog Input Trigger, handle 0x95 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0094, 0x0095, UUID_CHARACTERISTIC_ANALOG_INPUT_TRIGGER,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,     //

    // Handle 0x96: characteristic Dummy Digital Input Trigger, handle 0x97 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0096, 0x0097, UUID_CHARACTERISTIC_ANALOG_INPUT_TRIGGER,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 4),
        0x00, 0x00, 0x00, 0x00,     //
#endif
    // Handle 0xa1: Battery service
    PRIMARY_SERVICE_UUID16 (0x00a1, UUID_SERVICE_BATTERY),

    // Handle 0xa2: characteristic Battery Level, handle 0xa3 characteristic value
    CHARACTERISTIC_UUID16 (0x00a2, 0x00a3, UUID_CHARACTERISTIC_BATTERY_LEVEL,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x64
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)automation_io_db_data, sizeof(automation_io_db_data), (void *)&bleaio_cfg,
        (void *)&bleaio_puart_cfg, (void *)&bleaio_gpio_cfg, bleaio_Create);
}

