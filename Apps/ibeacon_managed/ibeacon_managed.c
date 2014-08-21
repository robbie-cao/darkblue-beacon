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
 * iBeacon Managed device
 *
 * The iBeacon Managed application provides a sample of the device which
 * can perform as an iBeacon, but in addition to that sports the Vendor
 * specific service which allows peer device, for example a smart phone
 * to connect and configure iBeacon parameters.  The configured information
 * is saved in the NVRAM, so that iBeacon would start up correctly after
 * the power cycle and does not need to be configured again.  For the
 * security purposes iBeacon Managed device advertises and allowed peer
 * to be connected only when the button is pushed.
 *
 * To build the application optional iBeacon library need to be included
 * in the make file.  Please note that the library can be distributed only
 * to the companies members of the Apple's iBeacon program.  Contact
 * your Broadcom support representative if you are a member of iBeacon
 * program to receive the library.
 *
 * Features demonstrated
 *  - Using of the advertisement_control library to create an iBeacon
 *  - GATT database and Device configuration initialization
 *  - Registration with LE stack for various events
 *  - NVRAM read/write operation
 *  - Processing write requests from the client
 *
 * To demonstrate the app use over the air sniffer to monitor advertisements,
 * work through the following steps.
 * 1. Plug the WICED eval board into your computer
 * 2. Build and download the application (to the WICED board)
 * 3. Push the app button on the eval board to set device connectable
 * 4. Connect from the client device and configure iBeacon params.  The app's
 *    database has 4 characteristics.  16 byte UUID, 2 byte major, 2 byte
 *    minor and 1 byte measured RSSI.  Use any of the shelf client software
 *    to change any of the characteristic values.
 * 5. Release the button
 * 6. Monitor advertisement using over the air sniffer
 * 7. Repeat from step 4, modify iBeacon parameters and watch changes on the sniffer
 *
 */

#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"

#include "ibeacon_managed_db.h"

/******************************************************
 *                     Constants
 ******************************************************/

#define IBEACON_MANAGED_FINE_TIMER           0
#define IBEACON_MANAGED_DEVICE_NAME          "ibeacon_managed"
#define IBEACON_MANAGED_DEVICE_APPEARENCE    APPEARANCE_GENERIC_TAG
#define IBEACON_MANAGED_MAIN_SERVICE_UUID    0
#define IBEACON_MANAGED_MAIN_CHAR_UUID       0
#define IBEACON_MANAGED_MAIN_CHAR_HANDLE     0

/******************************************************
 *                     Structures
 ******************************************************/

#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    UINT8  ibeacon_company_uuid[16];
    UINT16 ibeacon_major;
    UINT16 ibeacon_minor;
    INT8   ibeacon_measured_power;
}  HOSTINFO;
#pragma pack()

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void ibeacon_managed_create(void);
static int  ibeacon_managed_write_handler( LEGATTDB_ENTRY_HDR *p );
static void ibeacon_managed_interrupt_handler( UINT8 value );

/******************************************************
 *               Variables Definitions
 ******************************************************/

const BLE_PROFILE_CFG ibeacon_managed_cfg =
{
    /*.fine_timer_interval            =*/ IBEACON_MANAGED_FINE_TIMER, // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 10,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ IBEACON_MANAGED_DEVICE_NAME, // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(IBEACON_MANAGED_DEVICE_APPEARENCE),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0,    //(SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {IBEACON_MANAGED_MAIN_CHAR_HANDLE, 0x00, 0x00, 0x00, 0x00}, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ {IBEACON_MANAGED_MAIN_SERVICE_UUID, 0x00, 0x00, 0x00, 0x00},
    /*.cha                            =*/ {IBEACON_MANAGED_MAIN_CHAR_UUID, 0x00, 0x00, 0x00, 0x00},
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
const BLE_PROFILE_PUART_CFG ibeacon_managed_puart_cfg =
{
    /*.baudrate   =*/ 115200,
#ifdef GATT_DB_ENABLE_UART
    /*.txpin      =*/ GPIO_PIN_UART_TX,
    /*.rxpin      =*/ GPIO_PIN_UART_RX,
#else
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
#endif
};

// NVRAM save area
HOSTINFO ibeacon_managed_hostinfo;
//pointer to the generated part of hostinfo assuming it is the beginning of the hostinfo


/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)gatt_database,
                   gatt_database_len,
                   (void *)&ibeacon_managed_cfg,
                   (void *)&ibeacon_managed_puart_cfg,
                   (void *)&ibeacon_managed_gpio_cfg,
                   ibeacon_managed_create);
}

// Create device
void ibeacon_managed_create(void)
{
	int i;

    extern UINT32 blecm_configFlag;
    blecm_configFlag |= BLECM_DBGUART_LOG | BLECM_DBGUART_LOG_L2CAP | BLECM_DBGUART_LOG_SMP;
    lesmpkeys_removeAllBondInfo();

    ble_trace0("create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // Initialized ROM code which will monitor the battery
    blebat_Init();

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)ibeacon_managed_write_handler);

    // register interrupt handler
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB) ibeacon_managed_interrupt_handler);

    blecm_setTxPowerInADV(4);

    // Read NVRAM
    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(ibeacon_managed_hostinfo), (UINT8 *)&ibeacon_managed_hostinfo) == 0)
    {
    	// nothing in the NVRAM, use hardcoded default values from the GATT database
        BLEPROFILE_DB_PDU db_pdu;
        bleprofile_ReadHandle(HDLC_IBEACON_MANAGED_COMPANY_UUID_VALUE, &db_pdu);
        memcpy (ibeacon_managed_hostinfo.ibeacon_company_uuid, db_pdu.pdu, 16);

        bleprofile_ReadHandle(HDLC_IBEACON_MANAGED_MAJOR_ID_VALUE, &db_pdu);
        ibeacon_managed_hostinfo.ibeacon_major = db_pdu.pdu[0] + (db_pdu.pdu[1] << 8);

        bleprofile_ReadHandle(HDLC_IBEACON_MANAGED_MINOR_ID_VALUE, &db_pdu);
        ibeacon_managed_hostinfo.ibeacon_minor = db_pdu.pdu[0] + (db_pdu.pdu[1] << 8);

        bleprofile_ReadHandle(HDLC_IBEACON_MANAGED_MINOR_ID_VALUE, &db_pdu);
        ibeacon_managed_hostinfo.ibeacon_measured_power = db_pdu.pdu[0];

		bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(ibeacon_managed_hostinfo), (UINT8 *)&ibeacon_managed_hostinfo);
    }

    // if this device has been configured (UUID is not 0) start beaconing
    for (i = 0; i < 16; i++)
    {
    	if (ibeacon_managed_hostinfo.ibeacon_company_uuid[i] != 0)
    	{
    		break;
    	}
    }
#if 0
    if (i == 16)
    {
    	return;
    }
#endif
    ibeacon_start(ibeacon_managed_hostinfo.ibeacon_company_uuid, ibeacon_managed_hostinfo.ibeacon_major,
    		ibeacon_managed_hostinfo.ibeacon_minor, ibeacon_managed_hostinfo.ibeacon_measured_power);
}

// Process write request or command from peer device
int ibeacon_managed_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT8  writtenbyte;
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    BOOL changed;

    ble_trace1("write_handler: handle %04x", handle);

    changed = __write_handler(handle, len, attrPtr);

    // Save update to NVRAM if it has been changed.
    if (changed)
    {
        ibeacon_start(ibeacon_managed_hostinfo.ibeacon_company_uuid, ibeacon_managed_hostinfo.ibeacon_major,
        		ibeacon_managed_hostinfo.ibeacon_minor, ibeacon_managed_hostinfo.ibeacon_measured_power);

		writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(ibeacon_managed_hostinfo), (UINT8 *)&ibeacon_managed_hostinfo);
		ble_trace1("NVRAM write:%04x", writtenbyte);
    }
    return 0;
}

// Three Interrupt inputs (Buttons) can be handled here.
// If the following value == 1, Button is pressed. Different than initial value.
// If the following value == 0, Button is depressed. Same as initial value.
// Button1 : value&0x01
// Button2 : (value&0x02)>>1
// Button3 : (value&0x04)>>2
void ibeacon_managed_interrupt_handler(UINT8 value)
{
    UINT8  	u8 = 1;
    BOOL 	button_pushed = value & 0x01;

	ble_trace2("ibeacon_managed pushed:%d discoverable:%d", button_pushed, bleprofile_GetDiscoverable());

    // this application sets device discoverable and allows connection only
    // while the button is pushed
    if (button_pushed)
    {
    	ble_trace1("ibeacon_managed discoverable:%d", bleprofile_GetDiscoverable());
		if (bleprofile_GetDiscoverable() == NO_DISCOVERABLE)
		{
			bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
		}
    }
    else
    {
		bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
    }
}

//------ generated code

// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_ibeacon_managed_company_uuid(int len, UINT8 *attrPtr)
{
	if (memcmp(ibeacon_managed_hostinfo.ibeacon_company_uuid, attrPtr, len) != 0)
	{
		memcpy(ibeacon_managed_hostinfo.ibeacon_company_uuid, attrPtr, len);
		return TRUE;
	}
    return FALSE;
}

// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_ibeacon_managed_major_id(int len, UINT8 *attrPtr)
{
	UINT16 major_id = attrPtr[0] + (attrPtr[1] << 8);
	if (ibeacon_managed_hostinfo.ibeacon_major != major_id)
	{
		ibeacon_managed_hostinfo.ibeacon_major = major_id;
		return TRUE;
	}
    return FALSE;
}

// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_ibeacon_managed_minor_id(int len, UINT8 *attrPtr)
{
	UINT16 minor_id = attrPtr[0] + (attrPtr[1] << 8);
	if (ibeacon_managed_hostinfo.ibeacon_minor != minor_id)
	{
		ibeacon_managed_hostinfo.ibeacon_minor = minor_id;
		return TRUE;
	}
    return FALSE;
}

// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_ibeacon_managed_measured_power(int len, UINT8 *attrPtr)
{
	if (ibeacon_managed_hostinfo.ibeacon_measured_power != attrPtr[0])
	{
		ibeacon_managed_hostinfo.ibeacon_measured_power = attrPtr[0];
		return TRUE;
	}
    return FALSE;
}

