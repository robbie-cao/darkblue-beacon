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
* Long Characteristic Sample
*
 * This sample demonstrates how to read and write values to a characteristic value
 * larger than 512 octets in length.
*
* Features demonstrated
 *  - reading long characteristic from the GATT database
 *  - writing long characteristic to the GATT database
 *  - reading value of the long characteristic over the air
 *  - processing of the value of the long characteristic received over the air
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
 * 3. Using some client application discover vendor specific service and
 *    a single vendor specific characteristic of that service
 * 4. Read value of the characteristic.  It should be 160 zeroes.
 * 5. Write a new value with a length of more than 23 bytes.  (GATT procedures for
 *    prepare write and execute write should be used
 * 6. Read value again to make sure that it has been written correctly
 * 7. Push the application button on the board.  During the interrupt processing
 *    all bytes of the characteristic value are increased by 1 and written back
 *    to the GATT database
 * 8. Read value again to make sure that it has been written correctly
*
*/

#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "long_characteristic_support.h"
#include "long_characteristic.h"
#include "thread_and_mem_mgmt.h"

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void long_characteristic_create(void);
static void long_characteristic_connection_up( void );
static void long_characteristic_connection_down( void );
static void long_characteristic_advertisement_stopped( void );
static int  long_characteristic_write_handler( LEGATTDB_ENTRY_HDR *p );
static void long_characteristic_interrupt_handler( UINT8 value );

/******************************************************
 *               Variables Definitions
 ******************************************************/
const UINT8 gatt_database[]= // Define GATT database
{
// ***** Primary service 'Generic Access'
    //<Name>Generic Access</Name>
    //<Uuid>1800</Uuid>
    //Service handle: HDLS_GENERIC_ACCESS
    //Service UUID: UUID_SERVICE_GAP
    PRIMARY_SERVICE_UUID16 (HDLS_GENERIC_ACCESS, UUID_SERVICE_GAP),

    //<Name>Device Name</Name>
    //<Uuid>2A00</Uuid>
    CHARACTERISTIC_UUID16 (HDLC_GENERIC_ACCESS_DEVICE_NAME,
                           HDLC_GENERIC_ACCESS_DEVICE_NAME_VALUE,
                           UUID_CHARACTERISTIC_DEVICE_NAME,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE,
                           16),
    'l','o','n','g','_','c','h','a','r',0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

    //<Name>Appearance</Name>
    //<Uuid>2A01</Uuid>
    CHARACTERISTIC_UUID16 (HDLC_GENERIC_ACCESS_APPEARANCE,
                           HDLC_GENERIC_ACCESS_APPEARANCE_VALUE,
                           UUID_CHARACTERISTIC_APPEARANCE,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE,
                           2),
    BIT16_TO_8(APPEARANCE_GENERIC_TAG),

// ***** Primary service 'Generic Attribute'
    //<Name>Generic Attribute</Name>
    //<Uuid>1801</Uuid>
    //Service handle: HDLS_GENERIC_ATTRIBUTE
    //Service UUID: UUID_SERVICE_GATT
    PRIMARY_SERVICE_UUID16 (HDLS_GENERIC_ATTRIBUTE, UUID_SERVICE_GATT),

// ***** Primary service 'long_characteristic'
    //<Name>long_characteristic</Name>
    //<Uuid>bf 90 21 8e c0 68 4c 00 84 01 2d ba 08 ab 32 88</Uuid>
    //Service handle: HDLS_LONG_CHARACTERISTIC
    //Service UUID: UUID_LONG_CHAR_SERVICE
    PRIMARY_SERVICE_UUID128 (HDLS_LONG_CHARACTERISTIC, UUID_LONG_CHAR_SERVICE),

    //<Name>long_characteristic</Name>
    //<Uuid>34 5b e2 12 5e b1 45 03 b6 29 24 55 8a 11 1e 36</Uuid>
    CHARACTERISTIC_UUID128_WRITABLE (HDLC_LONG_CHARACTERISTIC,
                           HDLC_LONG_CHARACTERISTIC_VALUE,
                           UUID_LONG_CHAR_CHAR,
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_RELIABLE_WRITE | LEGATTDB_PERM_VARIABLE_LENGTH,
                           SIZE_LONG_CHARACTERISTIC_VALUE),
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    //<ClientConfiguration>
    //<Notification>false</Notification>
    //<Indication>false</Indication>
    CHAR_DESCRIPTOR_UUID16_WRITABLE (HDLD_LONG_CHARACTERISTIC_LONG_CHARACTERISTIC_CLIENT_CONFIGURATION,
                                     UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_RELIABLE_WRITE | LEGATTDB_PERM_AUTH_WRITABLE,
                                     2),
    BIT16_TO_8 (CCC_NONE),

};

// Indication sent, waiting for ack
UINT8 long_characteristic_indication_sent = 0;

// current value of client configuration descriptor
UINT16 long_characteristic_client_configuration = 0;

// Length of the GATT database
const UINT16 gatt_database_len = sizeof(gatt_database);

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG long_characteristic_gpio_cfg =
{
    {
        GPIO_PIN_WP,                               // This need to be used to enable/disable NVRAM write protect
        GPIO_PIN_BUTTON, -1, -1, -1, 
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // Other GPIOs are not used
    },
    /*.gpio_flag =*/
    {
        GPIO_SETTINGS_WP,
        GPIO_SETTINGS_BUTTON, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

// Main vendor specific service
UINT8 long_characteristic_uuid_main_vsc_service[16] = {UUID_LONG_CHAR_SERVICE};


const BLE_PROFILE_CFG long_characteristic_cfg =
{
    /*.fine_timer_interval            =*/ 0,    // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "long_char", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_TAG),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0,    //(SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00}, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.cha                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 0,    // if 1 grouptype read can be used
    /*.linkloss_button_enable         =*/ 0,    // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 0,    // second
    /*.alert_interval                 =*/ 0,    // interval of alert
    /*.high_alert_num                 =*/ 0,    // number of alert for each interval
    /*.mild_alert_num                 =*/ 0,    // number of alert for each interval
    /*.status_led_enable              =*/ 1,    // if 1 status LED is enable
    /*.status_led_interval            =*/ 0,    // second
    /*.status_led_con_blink           =*/ 0,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 0,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 0,    // blink num of undir adv
    /*.led_on_ms                      =*/ 0,    // led blink on duration in ms
    /*.led_off_ms                     =*/ 0,    // led blink off duration in ms
    /*.buz_on_ms                      =*/ 100,  // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,    // seconds
    /*.button_client_timeout          =*/ 0,    // seconds
    /*.button_discover_timeout        =*/ 0,    // seconds
    /*.button_filter_timeout          =*/ 0,    // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG long_characteristic_puart_cfg =
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
	// default number of prepare writes is 5, which is not enough for very long characteristics.
	// Setting it to 15 will cover characteristics of up to 256 octets in length.
	bleprofile_SetMaxQueuedWriteRequests(15);

    bleapp_set_cfg((UINT8 *)gatt_database,
                   gatt_database_len,
                   (void *)&long_characteristic_cfg,
                   (void *)&long_characteristic_puart_cfg,
                   (void *)&long_characteristic_gpio_cfg,
                   long_characteristic_create);
}

// Create device
void long_characteristic_create(void)
{
    extern UINT32 blecm_configFlag ;
    blecm_configFlag |= BLECM_DBGUART_LOG | BLECM_DBGUART_LOG_L2CAP | BLECM_DBGUART_LOG_SMP;

    ble_trace0("create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // Initialized ROM code which will monitor the battery
    blebat_Init();

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, long_characteristic_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, long_characteristic_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, long_characteristic_advertisement_stopped);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)long_characteristic_write_handler);

    // register interrupt handler
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB) long_characteristic_interrupt_handler);

    // total length should be less than 31 bytes
    BLE_ADV_FIELD adv[3];
    BLE_ADV_FIELD scr[1];

    // flags
    adv[0].len     = 1 + 1;
    adv[0].val     = ADV_FLAGS;
    adv[0].data[0] = LE_LIMITED_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

    adv[1].len     = 16 + 1;
    adv[1].val     = ADV_SERVICE_UUID128_COMP;
    memcpy(adv[1].data, &long_characteristic_uuid_main_vsc_service[0], 16);

    // Tx power level
    adv[2].len     = TX_POWER_LEN+1;
    adv[2].val     = ADV_TX_POWER_LEVEL;
    adv[2].data[0] = bleprofile_p_cfg->tx_power_level;

    // name
    scr[0].len      = strlen(bleprofile_p_cfg->local_name) + 1;
    scr[0].val      = ADV_LOCAL_NAME_COMP;
    memcpy(scr[0].data, bleprofile_p_cfg->local_name, scr[0].len - 1);

    bleprofile_GenerateADVData(adv, 3);
    bleprofile_GenerateScanRspData(scr, 1);

    blecm_setTxPowerInADV(0);
    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
}

// Connection up callback function is called on every connection establishment
void long_characteristic_connection_up(void)
{
    // Device supports a single connection, stop advertising
    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

// Connection down callback
void long_characteristic_connection_down(void)
{
    // If disconnection was caused by the peer, start low advertisements
    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
}

// Callback function indicates to the application that advertising has stopped.
// restart advertisement if needed
void long_characteristic_advertisement_stopped(void)
{
	// Start low advertisements to keep device connectable
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
}


// Process write request or command from peer device
int long_characteristic_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT8  writtenbyte;
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    BOOL changed;

    ble_trace2("write_handler: handle %04x len:%d", handle, len);

    if ((handle == HDLC_LONG_CHARACTERISTIC_VALUE) && (len <= 160))
    {
    	// do not need to do anything.  If we return 0, the value will be stored the GATT database
    }
    if ((len == 2) && (handle == HDLD_LONG_CHARACTERISTIC_LONG_CHARACTERISTIC_CLIENT_CONFIGURATION))
    {
    	long_characteristic_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
    }
    return 0;
}

// Three Interrupt inputs (Buttons) can be handled here.
// If the following value == 1, Button is pressed. Different than initial value.
// If the following value == 0, Button is depressed. Same as initial value.
// Button1 : value&0x01
// Button2 : (value&0x02)>>1
// Button3 : (value&0x04)>>2
void long_characteristic_interrupt_handler(UINT8 value)
{
	BLEPROFILE_DB_PDU *db_pdu;
	static UINT8 seqnum = 0;

    ble_trace3("(INT)But1:%d But2:%d But3:%d\n", value&0x01, (value& 0x02) >> 1, (value & 0x04) >> 2);

    void *p_mem = cfa_mm_Alloc(160 + 2);

    seqnum++;

    if (p_mem != 0)
    {
    	db_pdu = (BLEPROFILE_DB_PDU *)p_mem;
    	db_pdu->len = 160;

    	if (bleprofile_ReadHandleData(HDLC_LONG_CHARACTERISTIC_VALUE, db_pdu, SIZE_LONG_CHARACTERISTIC_VALUE + 2) == 0)
    	{
    		ble_trace0("ReadHandleData success");
    		ble_tracen(db_pdu->pdu, db_pdu->len);
    	}
    	else
    	{
    		ble_trace0("ReadHandleData failed :-(");
        	cfa_mm_Free(p_mem);
        	return;
    	}
    	memset (db_pdu->pdu, seqnum, 160);
    	if (bleprofile_WriteHandleData(HDLC_LONG_CHARACTERISTIC_VALUE, db_pdu, SIZE_LONG_CHARACTERISTIC_VALUE + 2) == 0)
    	{
    		ble_trace0("WriteHandleData success");
    	}
        else
        {
            ble_trace0("WriteHandleData failed");
        }
    	cfa_mm_Free(p_mem);
    }
    else
    {
        ble_trace0("no mem");
    }
}

// Process indication confirmation.  if client service indication, each indication
// should be acknowledged before the next one can be sent.
void long_characteristic_indication_cfm(void)
{
}
