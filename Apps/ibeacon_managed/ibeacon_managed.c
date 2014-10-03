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

#define IBEACON_MANAGED_FINE_TIMER           								1000
#define IBEACON_MANAGED_DEVICE_NAME          								"darkblue"
#define IBEACON_MANAGED_DEVICE_APPEARENCE    								APPEARANCE_GENERIC_TAG
#define IBEACON_MANAGED_MAIN_SERVICE_UUID    								0
#define IBEACON_MANAGED_MAIN_CHAR_UUID      								0
#define IBEACON_MANAGED_MAIN_CHAR_HANDLE     								0

#define MYBEACON_TYPE														0x01

#define HCIULP_ADV_NONCONNECTABLE_EVENT                                 	0x03
#define HCIULP_ADV_CHANNEL_37                                          		0x01
#define HCIULP_ADV_CHANNEL_38                                           	0x02
#define HCIULP_ADV_CHANNEL_39                                           	0x04
#define HCIULP_ADV_CHANNEL_MAP_MASK                                     	(HCIULP_ADV_CHANNEL_37 | HCIULP_ADV_CHANNEL_38 | HCIULP_ADV_CHANNEL_39)
#define HCIULP_PUBLIC_ADDRESS                                           	0x00
#define HCIULP_ADV_FILTER_POLICY_WHITE_LIST_NOT_USED                    	0x00    // white list not used

#define U8_SECONDS_IN_A_MINUTE												((UINT8)60)

/******************************************************
 *                     Structures
 ******************************************************/

#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;
    UINT8  ibeacon_company_uuid[16];
    UINT8  ibeacon_seed[16];
    UINT16 ibeacon_major;
    UINT16 ibeacon_minor;
    INT8   ibeacon_measured_power;
}  HOSTINFO;
#pragma pack()

#ifdef IL_BIGENDIAN
//Union to hold CRC.
typedef union TUu32_parse
{
   UINT32 u32int;
   struct
   {
	   UINT16 u16WordMSB;
	   UINT16 u16WordLSB;
   } WORD;
}TUu32_parser;
#else
typedef union TUu32_parse
{
   UINT32 u32int;
   struct
   {
	   UINT16 u16WordLSB;
	   UINT16 u16WordMSB;
   } WORD;
}TUu32_parser;
#endif

#define	SECURE_BEACON


/******************************************************
 *               Function Prototypes
 ******************************************************/

static void ibeacon_managed_create(void);
static void ibeacon_managed_timeout(UINT32 arg);
static void ibeacon_managed_fine_timeout(UINT32 arg);
static int  ibeacon_managed_write_handler( LEGATTDB_ENTRY_HDR *p );
static void ibeacon_managed_interrupt_handler( UINT8 value );
static void advertisement_packet_transmission(UINT8 type);
static void darkblue_ibeacon_start( void );
static void darkblue_create_beacon_adv( void );
static void darkblue_create_beacon_manager_adv( void );
static void ibeacon_connection_up( void );
static void ibeacon_connection_down( void );
static void ibeacon_advertisement_stopped( void );
static void ibeacon_encryption_changed(HCI_EVT_HDR *evt);
static void ibeacon_smp_bond_result(LESMP_PARING_RESULT  result);
static UINT32 xorshift128( void )
;/******************************************************
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

const UINT8 apple_ibeacon_prefix[4] = {0x4c, 0x00, 0x02, 0x15};
const UINT8 ibeacon_test_uuid[18] = { 0x4c, 0x00, 0x02, 0x15, 0xe2, 0xc5, 0x6d, 0xb5, 0xdf, 0xfb, 0x48, 0xd2, 0xb0, 0x60, 0xd0, 0xf5, 0xa7, 0x10};
const UINT8 ibeacon_uuid[16] = { 0xe2, 0xc5, 0x6d, 0xb5, 0xdf, 0xfb, 0x48, 0xd2, 0xb0, 0x60, 0xd0, 0xf5, 0xa7, 0x10, 0x96, 0xe0};
const UINT8 ibeacon_major[2] = {0x00, 0x01};
const UINT8 ibeacon_minor[2] = {0x00, 0x01};
const UINT8 ibeacon_power[1] = {0xC5};
BD_ADDR ibeacon_remote_addr        = {0, 0, 0, 0, 0, 0};
UINT32 	ibeacon_timer_count        = 0;
UINT32 	ibeacon_fine_timer_count   = 0;
UINT16 	ibeacon_connection_handle	= 0;	// HCI handle of connection, not zero when connected
UINT8 	ibeacon_indication_sent    = 0;	// indication sent, waiting for ack
UINT8   ibeacon_num_to_write       = 0;  	// Number of messages we need to send
UINT8   ibeacon_stay_connected		= 1;	// Change that to 0 to disconnect when all messages are sent

//JDD - global variables for the seed for our security xor shift
UINT32 x, y, z, w;
TUu32_parser secure_token;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)beacon_gatt_database,
                   beacon_gatt_database_len,
                   (void *)&ibeacon_managed_cfg,
                   (void *)&ibeacon_managed_puart_cfg,
                   (void *)&ibeacon_managed_gpio_cfg,
                   ibeacon_managed_create);
}

// Create device
void ibeacon_managed_create(void)
{
	int i;
	#ifdef SECURE_BEACON
		UINT32 test_seed1 = 0x12345678;
		UINT32 test_seed2 = 0x12345678;
		UINT32 test_seed3 = 0x12345678;
		UINT32 test_seed4 = 0x12345678;
	#endif
    extern UINT32 blecm_configFlag;
    blecm_configFlag |= BLECM_DBGUART_LOG | BLECM_DBGUART_LOG_L2CAP | BLECM_DBGUART_LOG_SMP;
    lesmpkeys_removeAllBondInfo();

	#ifdef SECURE_BEACON
		//JDD - store our seed information received
		x = test_seed1;
		y = test_seed2;
		z = test_seed3;
		w = test_seed4;
	#endif

    ble_trace0("create()\r\n");
    ble_trace0(bleprofile_p_cfg->ver);
    ble_trace0("\r\n");

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // Initialized ROM code which will monitor the battery
    blebat_Init();


    //JDD - adding for now to try to get the management side working
    //***************************************************************

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, ibeacon_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, ibeacon_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, ibeacon_advertisement_stopped);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(ibeacon_encryption_changed);

    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) ibeacon_smp_bond_result);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)ibeacon_managed_write_handler);

    // register interrupt handler
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB) ibeacon_managed_interrupt_handler);

    // Read NVRAM
    //if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(ibeacon_managed_hostinfo), (UINT8 *)&ibeacon_managed_hostinfo) == 0)
    //{
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
    //}


	//JDD - removed this for now, it uses the ibeacon library from Broadcom , which you need to be a licensed Apple developer
	//    ibeacon_start(ibeacon_managed_hostinfo.ibeacon_company_uuid, ibeacon_managed_hostinfo.ibeacon_major,
	//    		ibeacon_managed_hostinfo.ibeacon_minor, ibeacon_managed_hostinfo.ibeacon_measured_power);

	bleprofile_regTimerCb(ibeacon_managed_fine_timeout, ibeacon_managed_timeout);
	bleprofile_StartTimer();

    //JDD - startup our own ibeacon library
    darkblue_ibeacon_start();
}

void ibeacon_managed_timeout(UINT32 arg)
{
    ble_trace1("darkblue_timeout:%d\n", ibeacon_timer_count);

    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
        {
        	ibeacon_timer_count++;
        }
        break;
    }
}

void ibeacon_managed_fine_timeout(UINT32 arg)
{
	ibeacon_fine_timer_count++;
    ble_trace1("darkblue_fine_timeout:%d\r\n", ibeacon_fine_timer_count);
    if(ibeacon_fine_timer_count >= U8_SECONDS_IN_A_MINUTE)
    {
    	ibeacon_fine_timer_count = 0;
    	//JDD - generate a new advertisement with new maj and min values
    	//darkblue_create_beacon_adv();
		ble_trace1("Next x [%u]\r\n", x);
		ble_trace1("Next y [%u]\r\n", y);
		ble_trace1("Next z [%u]\r\n", z);
		ble_trace1("Next w [%u]\r\n", w);
    	secure_token.u32int = xorshift128();
    	ble_trace2("It's been a minute, new major value [%04x] minor value [%04x].\r\n", secure_token.WORD.u16WordMSB, secure_token.WORD.u16WordLSB);
    }
}

// this function is called 2.5 msec before the advertisement event.  In this sample
// just bump the sequence number and modify advertisement data.
void advertisement_packet_transmission(UINT8 type)
{
    if (type == 0)
    {
    	darkblue_create_beacon_adv();
    }
}

// Process write request or command from peer device
int ibeacon_managed_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT8  writtenbyte;
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    BOOL changed;

    ble_trace1("write_handler: handle %04x\r\n", handle);

    changed = __write_handler(handle, len, attrPtr);

    // Save update to NVRAM if it has been changed.
    if (changed)
    {
    	//JDD - removed this for now, it uses the ibeacon library from Broadcom , which you need to be a licensed Apple developer
//        ibeacon_start(ibeacon_managed_hostinfo.ibeacon_company_uuid, ibeacon_managed_hostinfo.ibeacon_major,
//        		ibeacon_managed_hostinfo.ibeacon_minor, ibeacon_managed_hostinfo.ibeacon_measured_power);
    	//JDD - start our own ibeacon library
    	//darkblue_ibeacon_start();

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

	ble_trace2("ibeacon_managed pushed:%d discoverable:%d\r\n", button_pushed, bleprofile_GetDiscoverable());

    // this application sets device discoverable and allows connection only
    // while the button is pushed
    if (button_pushed)
    {
    	ble_trace1("ibeacon_managed discoverable:%d\r\n", bleprofile_GetDiscoverable());
		if (bleprofile_GetDiscoverable() == NO_DISCOVERABLE)
		{
		    //JDD - testing for now
		    darkblue_create_beacon_manager_adv();
		    blecm_setTxPowerInADV(0);
		    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, ibeacon_remote_addr);
		}
    }
    else
    {
    	ble_trace0("ibeacon_managed NOT discoverable:%d\r\n");
		bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
		darkblue_ibeacon_start();
    }
}

//------ generated code

// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_ibeacon_managed_company_uuid(int len, UINT8 *attrPtr)
{
	int counter = 0;
	ble_trace1("Beacon company uuid written handler, len [%d]\r\n", len);

	for(counter = 0; counter < len; counter++)
	{
		ble_trace1("Data written : [%02x]\r\n", *attrPtr);
		attrPtr++;
	}

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
	ble_trace0("Beacon major id written handler.\r\n");
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
	ble_trace0("Beacon minor id written handler.\r\n");
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
	ble_trace0("Beacon measured power written handler.\r\n");
	if (ibeacon_managed_hostinfo.ibeacon_measured_power != attrPtr[0])
	{
		ibeacon_managed_hostinfo.ibeacon_measured_power = attrPtr[0];
		return TRUE;
	}
    return FALSE;
}

/**
 *
 */
void darkblue_ibeacon_start( void )
{
	darkblue_create_beacon_adv();

	// register with LE stack to be called 2.5msec before the advertisement event
	bleprofile_notifyAdvPacketTransmissions(&advertisement_packet_transmission, 2500);

	// Set TX power for advertisement packets at 4dBm
	blecm_setTxPowerInADV(4);

	bleapputils_changeLPOSource(LPO_MIA_LPO, FALSE, 500);

	// Start advertisements
	blecm_startAdv(
		HCIULP_ADV_NONCONNECTABLE_EVENT,                // non-connectable undirected advertisement
		160,                                            // adv interval 100 msec
		HCIULP_ADV_CHANNEL_MAP_MASK,                    // all channels
		HCIULP_PUBLIC_ADDRESS,                          // int advAdrType,
		HCIULP_ADV_FILTER_POLICY_WHITE_LIST_NOT_USED,   // int advFilterPolicy,
		HCIULP_PUBLIC_ADDRESS,                          // int initiatorAdrType,
		NULL);                                          // UINT8* initiatorAdr
	//******************************************************************
}

/**
 *
 */
void darkblue_create_beacon_adv( void )
{
    BLE_ADV_FIELD adv[2];
    static UINT8 test = 0;

	adv[0].len     = 1 + 1;
	adv[0].val     = ADV_FLAGS;
	adv[0].data[0] = LE_GENERAL_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

	adv[1].len     = 0x1A;
	adv[1].val     = ADV_MANUFACTURER_DATA; // (AD_TYPE == 0xff)

	BT_MEMCPY(&adv[1].data[0], apple_ibeacon_prefix, 4);
	BT_MEMCPY(&adv[1].data[4], ibeacon_managed_hostinfo.ibeacon_company_uuid, 16);

	#ifdef SECURE_BEACON
		if(test == 0)
		{
			test++;
			ble_trace1("Initial x [%u]\r\n", x);
			ble_trace1("Initial y [%u]\r\n", y);
			ble_trace1("Initial z [%u]\r\n", z);
			ble_trace1("Initial w [%u]\r\n", w);
			secure_token.u32int = xorshift128();
			ble_trace2("Init major minor, new major value [%04x] minor value [%04x].\r\n", secure_token.WORD.u16WordMSB, secure_token.WORD.u16WordLSB);
		}

		//JDD - we are using our security mechanism for major and minor
		//JDD - by placing the random, sync'ed value with the server
		BT_MEMCPY(&adv[1].data[20], secure_token.WORD.u16WordMSB, 2);
		BT_MEMCPY(&adv[1].data[22], secure_token.WORD.u16WordLSB, 2);
	#else
		BT_MEMCPY(&adv[1].data[20], ibeacon_managed_hostinfo.ibeacon_major, 2);
		BT_MEMCPY(&adv[1].data[22], ibeacon_managed_hostinfo.ibeacon_minor, 2);
	#endif

	BT_MEMCPY(&adv[1].data[24], ibeacon_power, 1);

	bleprofile_GenerateADVData(adv, 2);
}

/**
 *
 */
void darkblue_create_beacon_manager_adv( void )
{
	// total length should be less than 31 bytes
	BLE_ADV_FIELD adv[3];

	// flags
	adv[0].len     = 1 + 1;
	adv[0].val     = ADV_FLAGS;
	adv[0].data[0] = LE_LIMITED_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

	adv[1].len     = 16 + 1;
	adv[1].val     = ADV_SERVICE_UUID128_COMP;
	memcpy(adv[1].data, ibeacon_managed_hostinfo.ibeacon_company_uuid, 16);

	// name
	adv[2].len      = strlen(bleprofile_p_cfg->local_name) + 1;
	adv[2].val      = ADV_LOCAL_NAME_COMP;
	memcpy(adv[2].data, bleprofile_p_cfg->local_name, adv[2].len - 1);

	bleprofile_GenerateADVData(adv, 3);
}



/**
 *
 */
void ibeacon_connection_up( void )
{
    UINT8 writtenbyte;
    UINT8 *bda;

    ble_trace0("connection up\r\n");

    ibeacon_connection_handle = (UINT16)emconinfo_getConnHandle();

    // save address of the connected device and print it out.
    memcpy(ibeacon_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(ibeacon_remote_addr));

    // Stop advertising
    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);

    bleprofile_StopConnIdleTimer();

    // as we require security for every connection, we will not send any indications until
    // encryption is done.
    if (bleprofile_p_cfg->encr_required != 0)
    {
    	if (emconninfo_deviceBonded())
    	{
    		ble_trace0("device bonded\r\n");
    	}
    	else
    	{
    		ble_trace0("device not bonded\r\n");
    	    lesmp_pinfo->pairingParam.AuthReq  |= LESMP_AUTH_FLAG_BONDING;
            lesmp_sendSecurityRequest();
    	}
        return;
    }
    // saving bd_addr in nvram

    bda =(UINT8 *)emconninfo_getPeerPubAddr();

    memcpy(ibeacon_managed_hostinfo.bdaddr, bda, sizeof(BD_ADDR));

    ibeacon_encryption_changed(NULL);
}

/**
 *
 */
void ibeacon_connection_down( void )
{
    ble_trace0("connection down\r\n");
    ble_trace1("handle:%d\r\n", ibeacon_connection_handle);

	memset (ibeacon_remote_addr, 0, 6);
	ibeacon_connection_handle = 0;

    // If we are configured to stay connected, disconnection was caused by the
    // peer, start low advertisements, so that peer can connect when it wakes up.
    if (ibeacon_stay_connected)
    {
        bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, ibeacon_managed_hostinfo.bdaddr);
    }
}

/**
 *
 */
void ibeacon_advertisement_stopped( void )
{
    ble_trace0("ADV stop!!!!\r\n");

    // If we are configured to stay connected, disconnection was caused by the
    // peer, start low advertisements, so that peer can connect when it wakes up.
    if (ibeacon_stay_connected)
    {
        bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, ibeacon_managed_hostinfo.bdaddr);
    }
}

/**
 *
 * @param evt
 */
void ibeacon_encryption_changed(HCI_EVT_HDR *evt)
{
    ble_trace0("encryption changed\r\n");
    BLEPROFILE_DB_PDU db_pdu;

    UINT8 *bda = emconninfo_getPeerPubAddr();

    // Connection has been encrypted meaning that we have correct/paired device
    // restore values in the database
    //bleprofile_ReadNVRAM(NVRAM_ID_HOST_LIST, sizeof(hello_sensor_hostinfo), (UINT8 *)&hello_sensor_hostinfo);


    // We are done with initial settings, and need to stay connected.  It is a good
	// time to slow down the pace of master polls to save power.  Following request asks
	// host to setup polling every 100-500 msec, with link supervision timeout 7 seconds.
    bleprofile_SendConnParamUpdateReq(80, 400, 0, 700);
}

/**
 *
 * @param result
 */
void ibeacon_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace3("hello_sample, bond result %02x smpinfo addr type:%d emconninfo type:%d\r\n",
    		result, lesmp_pinfo->lesmpkeys_bondedInfo.adrType, emconninfo_getPeerAddrType());

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 *bda;
        UINT8 writtenbyte;

        bda = (UINT8 *)emconninfo_getPeerPubAddr();

        memcpy(ibeacon_managed_hostinfo.bdaddr, bda, sizeof(BD_ADDR));


        ble_trace2("Bond successful %08x%04x\r\n", (bda[5] << 24) + (bda[4] << 16) + (bda[3] << 8) + bda[2], (bda[1] << 8) + bda[0]);
        //writtenbyte = bleprofile_WriteNVRAM(NVRAM_ID_HOST_LIST, sizeof(hello_sensor_hostinfo), (UINT8 *)&hello_sensor_hostinfo);
        ble_trace1("NVRAM write:%04x\r\n", writtenbyte);
    }
}


/**
 * A security mechanism we are using to place a random value sync'ed with the server to reject incorrect beacons with
 */
UINT32 xorshift128(void) {

	UINT32 t = x ^ (x << 11);
	ble_trace1("T value [%u].\r\n", t);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

