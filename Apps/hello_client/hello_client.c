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
* BLE Vendor Specific Client Device
*
* The Hello Client application is designed to connect and access services
* of the Hello Sensor device.  Because handles of the all attributes of
* the Hello Sensor are well known, Hello Client does not perform GATT
* discovery, but uses them directly.  In addition to that Hello Client
* allows another master to connect, so the device will behave as a slave
* in one Bluetooth piconet and a master in another.  To accomplish that
* application can do both advertisements and scan.  Hello Client assumes
* that Hello Sensor advertises a special UUID and connects to the device
* which publishes it.
*
* Features demonstrated
*  - Registration with LE stack for various events
*  - Connection to a master and a slave
*  - As a master processing notifications from the server and
*    sending notifications to the client
*  - As a slave processing writes from the client and sending writes
*    to the server
*  - Use OOB and Passkey entry pairing
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Connect from some client application (for example LightBlue on iOS)
* 4. From the client application register for notifications
* 5. Make sure that your slave device (hello_sensor) is up and advertising
* 6. Push a button on the tag board for 6 seconds.  That will start
*    connection process.
* 7. Push a button on the hello_sensor to deliver notification through
*    hello_client device up to the client.
*
* To test OOB/Passkey pairing uncomment corresponding compile flag before step 2 and
* use corresponding compile in the hello_client application.
*/
#include "bleprofile.h"
#include "blecen.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "blecm.h"
#include "hello_client.h"
#include "../hello_sensor/hello_sensor.h"
#include "devicelpm.h"
#include "spar_utils.h"

const UINT8 hello_service[16] = {UUID_HELLO_SERVICE};




/******************************************************
 *                      Constants
 ******************************************************/
// Uncomment the following line to enable OOB pairing. Change oob_tk[] below to the desired key.
// #define OOB_PAIRING
// Uncomment the following line to enable passkey pairing. Change passKey[] below to the desired key.
// #define PASSKEY_PAIRING

#define NVRAM_ID_HOST_LIST				0x10	// ID of the memory block used for NVRAM access

#define CONNECT_ANY                 	0x01
#define CONNECT_HELLO_SENSOR        	0x02
#define SMP_PAIRING                 	0x04
#define SMP_ERASE_KEY               	0x08

#define HELLO_CLIENT_MAX_SLAVES			4

#define RMULP_CONN_HANDLE_START			0x40
#define MASTER							0
#define SLAVE							1

#ifdef OOB_PAIRING
const UINT8 oob_tk[LESMP_MAX_KEY_SIZE] =  {0x41, 0x59, 0x1c, 0xd5, 0xea, 0x5a, 0x49, 0x55, 0xb6, 0x3b, 0x18, 0x7c, 0x46, 0xe, 0x13, 0x52};
#endif
#ifdef PASSKEY_PAIRING
//passkey=123456
const UINT8 passKey[LESMP_MAX_KEY_SIZE] = {0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
/******************************************************
 *                     Structures
 ******************************************************/
#pragma pack(1)
//host information for NVRAM
typedef PACKED struct
{
    // BD address of the bonded host
    BD_ADDR  bdaddr;

    // Current value of the client configuration descriptor
    UINT16  characteristic_client_configuration;

}  HOSTINFO;
#pragma pack()

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void   hello_client_create(void);
static void   hello_client_timeout(UINT32 count);
static void   hello_client_fine_timeout(UINT32 finecount);
static void   hello_client_app_timer(UINT32 count);
static void   hello_client_app_fine_timer(UINT32 finecount);
static void   hello_client_advertisement_report(HCIULP_ADV_PACKET_REPORT_WDATA *evt);
static void   hello_client_connection_up(void);
static void   hello_client_connection_down(void);
static void   hello_client_smp_bond_result(LESMP_PARING_RESULT result);
static void   hello_client_encryption_changed(HCI_EVT_HDR *evt);
static void   hello_client_notification_handler(int len, int attr_len, UINT8 *data);
static void   hello_client_indication_handler(int len, int attr_len, UINT8 *data);
static void   hello_client_process_rsp(int len, int attr_len, UINT8 *data);
static void   hello_client_process_write_rsp();
static int    hello_client_write_handler(LEGATTDB_ENTRY_HDR *p);
static UINT32 hello_client_interrupt_handler(UINT32 value);
static void   hello_client_timer_callback(UINT32 arg);
static UINT32 app_device_lpm_queriable(LowPowerModePollType type, UINT32 context);

/******************************************************
 *               Variables Definitions
 ******************************************************/
/*
 * This is the GATT database for the Hello Client application.  Hello Client
 * can connect to hello sensor, but it also provides service for
 * somebody to access.  The database defines services, characteristics and
 * descriptors supported by the application.  Each attribute in the database
 * has a handle, (characteristic has two, one for characteristic itself,
 * another for the value).  The handles are used by the peer to access
 * attributes, and can be used locally by application, for example to retrieve
 * data written by the peer.  Definition of characteristics and descriptors
 * has GATT Properties (read, write, notify...) but also has permissions which
 * identify if peer application is allowed to read or write into it.
 * Handles do not need to be sequential, but need to be in order.
 */
const UINT8 hello_client_gatt_database[]=
{
    // Handle 0x01: GATT service
	// Service change characteristic is optional and is not present
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x14: GAP service
    // Device Name and Appearance are mandatory characteristics.  Peripheral
    // Privacy Flag only required if privacy feature is supported.  Reconnection
    // Address is optional and only when privacy feature is supported.
    // Peripheral Preferred Connection Parameters characteristic is optional
    // and not present.
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value.
    // Any 16 byte string can be used to identify the sensor.  Just need to
    // replace the "Hello Client" string below.
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
    					   LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'H','e','l','l','o',' ','C','l','i','e','n','t',0x00,0x00,0x00,0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value.
    // List of approved appearances is available at bluetooth.org.  Current
    // value is set to 0x200 - Generic Tag
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
    					   LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_TAG),

    // Handle 0x28: Hello Client Service.
    // This is the main proprietary service of Hello Client application.  It has
    // a single characteristic which allows peer to write to and can be configured
    // to send indications to the peer.  Note that UUID of the vendor specific
    // service is 16 bytes, unlike standard Bluetooth UUIDs which are 2 bytes.
    // _UUID128 version of the macro should be used.
    PRIMARY_SERVICE_UUID128 (HANDLE_HELLO_CLIENT_SERVICE_UUID, UUID_HELLO_CLIENT_SERVICE),

    // Handle 0x29: characteristic Hello Notification, handle 0x2a characteristic value
    // we support both notification and indication.  Peer need to allow notifications
    // or indications by writing in the Characteristic Client Configuration Descriptor
    // (see handle 2b below).  Note that UUID of the vendor specific characteristic is
    // 16 bytes, unlike standard Bluetooth UUIDs which are 2 bytes.  _UUID128 version
    // of the macro should be used.
    CHARACTERISTIC_UUID128_WRITABLE (0x0029, HANDLE_HELLO_CLIENT_DATA_VALUE, UUID_HELLO_CLIENT_DATA,
		    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_INDICATE,
		    LEGATTDB_PERM_READABLE  | LEGATTDB_PERM_WRITE_CMD  | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_VARIABLE_LENGTH, 20),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

	// Handle 0x2b: Characteristic Client Configuration Descriptor.
    // This is standard GATT characteristic descriptor.  2 byte value 0 means that
    // message to the client is disabled.  Peer can write value 1 or 2 to enable
    // notifications or indications respectively.  Not _WRITABLE in the macro.  This
    // means that attribute can be written by the peer.
    CHAR_DESCRIPTOR_UUID16_WRITABLE (HANDLE_HELLO_CLIENT_CLIENT_CONFIGURATION_DESCRIPTOR,
                                     UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x4d: Device Info service
    // Device Information service helps peer to identify manufacture or vendor
    // of the device.  It is required for some types of the devices (for example HID,
    // and medical, and optional for others.  There are a bunch of characteristics
    // available, out of which Hello Sensor implements 3.
    PRIMARY_SERVICE_UUID16 (0x004d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x4e: characteristic Manufacturer Name, handle 0x4f characteristic value
    CHARACTERISTIC_UUID16 (0x004e, 0x004f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x50: characteristic Model Number, handle 0x51 characteristic value
    CHARACTERISTIC_UUID16 (0x0050, 0x0051, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '4','3','2','1',0x00,0x00,0x00,0x00,

    // Handle 0x52: characteristic System ID, handle 0x53 characteristic value
    CHARACTERISTIC_UUID16 (0x0052, 0x0053, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0xef,0x48,0xa2,0x32,0x17,0xc6,0xa6,0xbc,

    // Handle 0x61: Battery service
    // This is an optional service which allows peer to read current battery level.
    PRIMARY_SERVICE_UUID16 (0x0061, UUID_SERVICE_BATTERY),

    // Handle 0x62: characteristic Battery Level, handle 0x63 characteristic value
    CHARACTERISTIC_UUID16 (0x0062, 0x0063, UUID_CHARACTERISTIC_BATTERY_LEVEL,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x64,
};


const BLE_PROFILE_CFG hello_client_cfg =
{
    /*.fine_timer_interval            =*/ 1000, // ms
    /*.default_adv                    =*/ HIGH_UNDIRECTED_DISCOVERABLE,
	/*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "Hello Client", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_TAG),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0, // (SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0,    // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00}, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.cha                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 1,    // if 1 grouptype read can be used
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
    /*.button_client_timeout          =*/ 1,    // seconds
    /*.button_discover_timeout        =*/ 3,    // seconds
    /*.button_filter_timeout          =*/ 10,   // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG hello_client_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG hello_client_gpio_cfg =
{
    /*.gpio_pin =*/
    {
    	GPIO_PIN_WP,      // This need to be used to enable/disable NVRAM write protect
    	GPIO_PIN_BUTTON,  // Button GPIO is configured to trigger either direction of interrupt
    	GPIO_PIN_LED,     // LED GPIO, optional to provide visual effects
    	GPIO_PIN_BATTERY, // Battery monitoring GPIO. When it is lower than particular level, it will give notification to the application
    	GPIO_PIN_BUZZER,  // Buzzer GPIO, optional to provide audio effects
    	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // other GPIOs are not used
    },
    /*.gpio_flag =*/
    {
    	GPIO_SETTINGS_WP,
    	GPIO_SETTINGS_BUTTON | GPIO_BOTHEDGE_INT,
    	GPIO_SETTINGS_LED,
    	GPIO_SETTINGS_BATTERY,
    	GPIO_SETTINGS_BUZZER,
    	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

typedef struct t_APP_STATE
{
	UINT8   app_config;
	UINT32 	app_timer_count;
	UINT32 	app_fine_timer_count;

	UINT8	handle_to_master;		    // handle of the master connection
	UINT8   num_slaves;					// number of active slaves

	UINT16  data_handle;    			// handle of the sensor's measurement characteristic
	UINT16  config_handle;    			// handle of the sensor's configuration characteristic
	UINT16  data_descrtiptor_handle;    // handle of the measurements client configuration descriptor

	// space to save device info and smp_info to handle multiple connections
	EMCONINFO_DEVINFO dev_info[HELLO_CLIENT_MAX_SLAVES];
	LESMP_INFO		  smp_info[HELLO_CLIENT_MAX_SLAVES];

	HOSTINFO hostinfo;					// NVRAM save area
} tAPP_STATE;

tAPP_STATE hello_client;

BD_ADDR bd_addr_any                             = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
BD_ADDR hello_client_target_addr                = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
UINT8   hello_client_target_addr_type           = 0;


// Following variables are in ROM 
extern BLE_CEN_CFG		blecen_cen_cfg;
extern BLEAPP_TIMER_CB	blecen_usertimerCb;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)hello_client_gatt_database,
    		       sizeof(hello_client_gatt_database),
    		       (void *)&hello_client_cfg,
    		       (void *)&hello_client_puart_cfg,
                   (void *)&hello_client_gpio_cfg,
                   hello_client_create);
}

// Create hello sensor
void hello_client_create(void)
{
    ble_trace0("hello_client_create()\n");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

	memset (&hello_client, 0, sizeof (hello_client));

    hello_client.app_config = 0
//                          | AUTO_CON
//    						| CONNECT_ANY
                            | CONNECT_HELLO_SENSOR
//                          | SMP_PAIRING
//                          | SMP_ERASE_KEY
                            ;

    // Blecen default parameters.  Change if appropriate
    //blecen_cen_cfg.scan_type                = HCIULP_ACTIVE_SCAN; 
    //blecen_cen_cfg.scan_adr_type            = HCIULP_PUBLIC_ADDRESS;
    //blecen_cen_cfg.scan_filter_policy       = HCIULP_SCAN_FILTER_POLICY_WHITE_LIST_NOT_USED;
    //blecen_cen_cfg.filter_duplicates        = HCIULP_SCAN_DUPLICATE_FILTER_ON;
    //blecen_cen_cfg.init_filter_policy       = HCIULP_INITIATOR_FILTER_POLICY_WHITE_LIST_NOT_USED;
    //blecen_cen_cfg.init_addr_type           = HCIULP_PUBLIC_ADDRESS;
    //blecen_cen_cfg.high_scan_interval       = 96;       // slots
    //blecen_cen_cfg.low_scan_interval        = 2048;     // slots
    //blecen_cen_cfg.high_scan_window         = 48;       // slots
    //blecen_cen_cfg.low_scan_window          = 18;       // slots
    //blecen_cen_cfg.high_scan_duration       = 30;       // seconds
    //blecen_cen_cfg.low_scan_duration        = 300;      // seconds  
    //blecen_cen_cfg.high_conn_min_interval   = 40;       // frames
    //blecen_cen_cfg.low_conn_min_interval    = 400;      // frames
    //blecen_cen_cfg.high_conn_max_interval   = 56;       // frames
    //blecen_cen_cfg.low_conn_max_interval    = 560;      // frames
    //blecen_cen_cfg.high_conn_latency        = 0;        // number of connection event
    //blecen_cen_cfg.low_conn_latency         = 0;        // number of connection event
    //blecen_cen_cfg.high_supervision_timeout = 10;       // N * 10ms
    //blecen_cen_cfg.low_supervision_timeout  = 100;      // N * 10ms
    //blecen_cen_cfg.conn_min_event_len       = 0;        // slots
    //blecen_cen_cfg.conn_max_event_len       = 0;        // slots
    
    //change parameter
    blecen_cen_cfg.filter_duplicates		= HCIULP_SCAN_DUPLICATE_FILTER_OFF; 
    blecen_usertimerCb						= hello_client_timer_callback;
    blecen_cen_cfg.high_supervision_timeout = 400;      // N * 10ms
    blecen_cen_cfg.low_supervision_timeout  = 700;      // N * 10ms

    //enable multi connection
    blecm_ConMuxInit(HELLO_CLIENT_MAX_SLAVES);
    blecm_enableConMux();
    blecm_enablescatternet();

    blecen_Create();

    // we will not do scan until user pushes the button for 5 seconds
    blecen_Scan(NO_SCAN);
    bleprofile_regButtonFunctionCb(NULL);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, hello_client_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, hello_client_connection_down);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(hello_client_encryption_changed);

    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) hello_client_smp_bond_result);

    // set the role for SMP.
    lesmp_setSMPRole(LESMP_ROLE_RESPONDERS);

    // setup the pairing parameters.
    lesmp_setPairingParam(
             LESMP_IO_CAP_DISP_NO_IO,           // IOCapability,
#ifdef OOB_PAIRING
             LESMP_OOB_AUTH_DATA_FROM_REMOTE_PRESENT,// OOBDataFlag,
#else
             LESMP_OOB_AUTH_DATA_NOT_PRESENT,   // OOBDataFlag,
#endif
#ifdef PASSKEY_PAIRING
             LESMP_AUTH_FLAG_BONDING 			// AuthReq,
             | LESMP_AUTH_REQ_FLAG_MITM,
#else
             LESMP_AUTH_FLAG_BONDING, 			// AuthReq,
#endif
             LESMP_MAX_KEY_SIZE,                // MaxEncKeySize,
             // InitiatorKeyDistrib,
             LESMP_KEY_DISTRIBUTION_ENC_KEY
             | LESMP_KEY_DISTRIBUTION_ID_KEY
             | LESMP_KEY_DISTRIBUTION_SIGN_KEY,
             // ResponderKeyDistrib
             LESMP_KEY_DISTRIBUTION_ENC_KEY
             | LESMP_KEY_DISTRIBUTION_ID_KEY
             | LESMP_KEY_DISTRIBUTION_SIGN_KEY
    );

#ifdef OOB_PAIRING
    lesmp_setSMPOOBdata((UINT8*)oob_tk,LESMP_MAX_KEY_SIZE);
    lesmp_setJustWorksNotPermitted();
    lesmp_setPasskeyEntryNotPermitted();
#endif

#ifdef PASSKEY_PAIRING
    lesmp_setSMPassKey((UINT8*)passKey,LESMP_MAX_KEY_SIZE);
    lesmp_setJustWorksNotPermitted();
#endif

    // register to process peripheral advertisements, notifications and indications
    blecm_RegleAdvReportCb((BLECM_FUNC_WITH_PARAM) hello_client_advertisement_report);
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) hello_client_notification_handler);
    leatt_regIndicationCb((LEATT_TRIPLE_PARAM_CB) hello_client_indication_handler);

    // GATT client callbacks
    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) hello_client_process_rsp);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) hello_client_process_rsp);
    leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) hello_client_process_rsp);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) hello_client_process_write_rsp);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)hello_client_write_handler);

    // process button
    bleprofile_regIntCb(hello_client_interrupt_handler);

	// Register callback with devicelpm for sleep and deep sleep polls.
	devlpm_registerForLowPowerQueries(app_device_lpm_queriable, 0);

    // need to do adverts to enable slave connections
    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);

    // change timer callback function.  because we are running ROM app, need to 
	// stop timer first.
    bleprofile_KillTimer();
    bleprofile_regTimerCb(hello_client_app_fine_timer, hello_client_app_timer);
    bleprofile_StartTimer();
}

// This function will be called on every connection establishmen
void hello_client_connection_up(void)
{
    UINT8 *p_remote_addr 	 = (UINT8 *)emconninfo_getPeerAddr();
    UINT8 *p_remote_pub_addr = (UINT8 *)emconninfo_getPeerPubAddr();
    UINT16 con_handle 		 = emconinfo_getConnHandle();
    UINT32 con_index         = con_handle - RMULP_CONN_HANDLE_START;
    int    cm_index;

    if (con_index >= HELLO_CLIENT_MAX_SLAVES)
    {
    	ble_trace1("---!!!hello_client_connection_up handle:%x", con_handle);
    	return;
    }

	cm_index = blecm_FindConMux(con_handle);

	//delete index first
	if (cm_index >= 0)
	{
		blecm_DelConMux(cm_index);
	}

	//find free index
	cm_index = blecm_FindFreeConMux();

	//set information
	if (cm_index < 0)
	{
		ble_trace0("---!!!hello_client_connection_up failed to get mux");
		blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
		return;
	}

	// copy dev_pinfo
	memcpy((UINT8 *)&hello_client.dev_info[con_index], (UINT8 *)emconinfo_getPtr(), sizeof(EMCONINFO_DEVINFO));

	// copy smp_pinfo
	memcpy((UINT8 *)&hello_client.smp_info[con_index], (UINT8 *)lesmp_getPtr(), sizeof(LESMP_INFO));

	blecm_AddConMux(con_index, con_handle, sizeof (hello_client_gatt_database), (void *)hello_client_gatt_database,
			&hello_client.dev_info[con_index], &hello_client.smp_info[con_index]);

	// if we connected as a master configure slave to enable notifications
	if (hello_client.dev_info[con_index].role == MASTER)
	{
		if (bleprofile_p_cfg->encr_required == 0)
		{
			UINT16 u16 = 1;
			bleprofile_sendWriteReq(HANDLE_HELLO_SENSOR_CLIENT_CONFIGURATION_DESCRIPTOR, (UINT8 *)&u16, 2);
		}

		// count number of slave connections
		hello_client.num_slaves++;
	}
	else
	{
		hello_client.handle_to_master = con_handle;

//	    lesmp_setSMPRole(LESMP_ROLE_RESPONDERS);

		// ask master to set preferred connection parameters
        lel2cap_sendConnParamUpdateReq(100, 116, 0, 500);
	}

	ble_trace4("hello_client_connection_up handle:%x slave:%d num:%d to_master:%d", con_handle,
			hello_client.dev_info[con_index].role, hello_client.num_slaves, hello_client.handle_to_master);

	if (hello_client.app_config & SMP_PAIRING)
	{
		// following call will start bonding if devices are not pairing, or will request
		// encryption if bonding has been established before
		ble_trace0("starting security");

		lesmp_startPairing(NULL);
	}

	// if we are not connected to all slaves restart the scan
    if (hello_client.num_slaves < HELLO_CLIENT_MAX_SLAVES)
    {
    	// if we are not connected to the master enable advertisements
        if (!hello_client.handle_to_master)
        {
        	ble_trace0("Adv during conn enable");
        	blecm_setAdvDuringConnEnable(TRUE);
        }
        else
        {
        	ble_trace0("Adv during conn disable");
//            blecm_setAdvDuringConnEnable(FALSE);
        }
//        blecen_Scan(LOW_SCAN);
    }
}

// This function will be called when connection goes down
void hello_client_connection_down(void)
{
    UINT16 con_handle = emconinfo_getConnHandle();
    UINT32 con_index  = con_handle - RMULP_CONN_HANDLE_START;
    int    cm_index;

    if (con_index >= HELLO_CLIENT_MAX_SLAVES)
    {
    	ble_trace1("---!!!hello_client_connection_down handle:%x", con_handle);
    	return;
    }
    if (hello_client.app_config & SMP_ERASE_KEY)
    {
        lesmpkeys_removeAllBondInfo();
        ble_trace0("Pairing Key removed");
    }

    ble_trace3("Conn Down handle:%x Slave:%d Disc_Reason: %02x", con_handle, hello_client.dev_info[con_index].role, emconinfo_getDiscReason());

	if (hello_client.dev_info[con_index].role == SLAVE)
	{
		hello_client.handle_to_master = 0;

		// restart scan
		blecm_setAdvDuringConnEnable (TRUE);
	}
	else
	{
		blecli_ClientHandleReset();
	    blecen_connDown();
    }

    // delete a connection structure
    memset (&hello_client.dev_info[con_index], 0x00, sizeof(EMCONINFO_DEVINFO));
    memset (&hello_client.smp_info[con_index], 0x00, sizeof(LESMP_INFO));

	// count number of slave connections
	hello_client.num_slaves--;

	cm_index = blecm_FindConMux(con_handle);

	//delete index
	if (cm_index >= 0)
	{
		blecm_DelConMux(cm_index);
	}

	// if we are not connected to all slaves restart the scan
    if (hello_client.num_slaves < HELLO_CLIENT_MAX_SLAVES)
    {
        blecen_Scan(LOW_SCAN);
    }
}

void hello_client_timeout(UINT32 count)
{
    ble_trace1("hello_client_timeout:%d", count);
}

void hello_client_fine_timeout(UINT32 count)
{
}

void hello_client_app_timer(UINT32 arg) 
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                hello_client.app_timer_count++;

                hello_client_timeout(hello_client.app_timer_count);
            }
            break;
    }

    blecen_appTimerCb(arg);
}

void hello_client_app_fine_timer(UINT32 arg)
{
    hello_client.app_fine_timer_count++;
    hello_client_fine_timeout(hello_client.app_fine_timer_count);

    blecen_appFineTimerCb(arg);
}


//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void hello_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    blecen_smpBondResult(result);

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
    	// if bonding is successful register with the server to receive notification
    	UINT16 u16 = 1;
        bleprofile_sendWriteReq(HANDLE_HELLO_SENSOR_CLIENT_CONFIGURATION_DESCRIPTOR, (UINT8 *)&u16, 2);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void hello_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8 status = *((UINT8 *)(evt + 1));

    ble_trace1("encryption changed: %02x\n", status);

    blecen_encryptionChanged(evt);
}


void hello_client_timer_callback(UINT32 arg)
{
    ble_trace1("hello_client_timer_callback %d\n", arg);

    switch(arg)
    {
    case BLEAPP_APP_TIMER_SCAN:
		blecen_Scan(LOW_SCAN);
        break;

    case BLEAPP_APP_TIMER_CONN:
        if ((blecen_GetConn() == HIGH_CONN) || (blecen_GetConn() == LOW_CONN))
        {
            blecen_Conn(NO_CONN, NULL, 0);
            blecen_Scan(LOW_SCAN);
            ble_trace0("Connection Fail, Restart Scan\n");
        }
        break;
    }
}

void hello_client_advertisement_report(HCIULP_ADV_PACKET_REPORT_WDATA *evt)
{
    blecen_leAdvReportCb(evt);

#ifdef HELLO_CLIENT_MIN_RSSI
    if (evt->rssi < HELLO_CLIENT_MIN_RSSI)      // filter out adverts with low RSSI
    {
        return;
    }
#endif

#if 0
    // connect to any or specific BDADDR
    if (hello_client.app_config & CONNECT_ANY)
    {
        if ((memcmp (hello_client_target_addr, bd_addr_any, 6) == 0) ||
            ((memcmp(evt->wd_addr, hello_client_target_addr, 6) == 0) &&
             (evt->addressType == hello_client_target_addr_type)))
        {
            blecen_Conn(HIGH_CONN, evt->wd_addr, evt->addressType);
            blecen_Scan(NO_SCAN);
        }
    }
    else
#endif

    // parse and connection
    if (hello_client.app_config & CONNECT_HELLO_SENSOR)
    {
        BLE_ADV_FIELD *p_field;
        UINT8         *data = (UINT8 *)(evt->data);
        UINT8         *ptr = data;
        UINT8         dataLen = (UINT8)(evt->dataLen);

        UINT8 state = 0;

        while(1)
        {
            UINT16 uuid;
            p_field = (BLE_ADV_FIELD *)ptr;

            if ((p_field->len == 16 + 1) &&
                (p_field->val == ADV_SERVICE_UUID128_COMP) &&
                (memcmp (p_field->data, hello_service, 16) == 0))
            {
                ble_trace0("Found service, no discoverable high conn\n");

	    	    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);

//	    	    lesmp_setSMPRole(LESMP_ROLE_INITIATOR);

                blecen_Conn(HIGH_CONN, evt->wd_addr, evt->addressType);
                blecen_Scan(NO_SCAN);
                break;
            }

            ptr += (p_field->len + 1);

            if (ptr >= data + dataLen)
            {
                break;
            }
        }
    }
}


void hello_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    ble_trace2("Client rsp len:%d attr_len:%d", len, attr_len);
}

void hello_client_process_write_rsp(void)
{
    ble_trace0("Client write rsp");
}

void hello_client_process_data_from_slave(int len, UINT8 *data)
{
	// if master allows notifications, forward received data
	// Because we will be sending on the different connection, change Set Pointer to the master context
	if ((hello_client.hostinfo.characteristic_client_configuration & CCC_NOTIFICATION) != 0)
	{
        blecm_SetPtrConMux(hello_client.handle_to_master);
        bleprofile_sendNotification(HANDLE_HELLO_CLIENT_DATA_VALUE, data, len < 20 ? len : 20);
	}
	else if ((hello_client.hostinfo.characteristic_client_configuration & CCC_INDICATION) != 0)
	{
        blecm_SetPtrConMux(hello_client.handle_to_master);
        bleprofile_sendIndication(HANDLE_HELLO_CLIENT_DATA_VALUE, data, len < 20 ? len : 20, NULL);
	}
}

void hello_client_notification_handler(int len, int attr_len, UINT8 *data)
{
    ble_trace2("Notification:%02x, %d\n", (UINT16)attr_len, len);
    ble_tracen((char *)data, len);

    hello_client_process_data_from_slave(len, data);
}

void hello_client_indication_handler(int len, int attr_len, UINT8 *data)
{
    ble_trace2("Indication:%02x, %d\n", (UINT16)attr_len, len);
    ble_tracen((char *)data, len);

    hello_client_process_data_from_slave(len, data);

    bleprofile_sendHandleValueConf();
}

//
// Process write request or command from peer device
//
int hello_client_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT8  writtenbyte;
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);

    ble_trace1("hello_client_write_handler: handle %04x\n", handle);

    // By writing into Characteristic Client Configuration descriptor
    // peer can enable or disable notification or indication
    if ((len == 2) && (handle == HANDLE_HELLO_CLIENT_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        hello_client.hostinfo.characteristic_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("hello_client_write_handler: client_configuration %04x\n", hello_client.hostinfo.characteristic_client_configuration);

        // Save update to NVRAM.  Client does not need to set it on every connection.
        writtenbyte = bleprofile_WriteNVRAM(NVRAM_ID_HOST_LIST, sizeof(hello_client.hostinfo), (UINT8 *)&hello_client.hostinfo);
        ble_trace1("hello_client_write_handler: NVRAM write:%04x\n", writtenbyte);
    }
    else if (handle == HANDLE_HELLO_CLIENT_DATA_VALUE)
    {
        ble_tracen(attrPtr, len);
    }
    else
    {
        ble_trace2("hello_sensor_write_handler: bad write len:%d handle:0x%x\n", len, handle);
    	return 0x80;
    }

    return 0;
}


UINT32 hello_client_interrupt_handler(UINT32 value)
{
    BLEPROFILE_DB_PDU db_pdu;
    UINT8  	u8 = 1;
    BOOL 	button_pushed = value & 0x01;
    static  UINT32 button_pushed_time = 0;

    ble_trace3("(INT)But1:%d But2:%d But3:%d\n", value&0x01, (value& 0x02) >> 1, (value & 0x04) >> 2);
    if (button_pushed)
    {
    	button_pushed_time = hello_client.app_timer_count;
    }
    else if (button_pushed_time != 0)
    {
    	if (hello_client.app_timer_count - button_pushed_time > 5)
    	{
    		ble_trace0("Stop adverts and start high scan");
    	    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
            blecen_Scan(HIGH_SCAN);
    	}
    	else
    	{
    		static char *buf = "From Client";
            blecm_SetPtrConMux(hello_client.handle_to_master);
            bleprofile_sendNotification(HANDLE_HELLO_CLIENT_DATA_VALUE, buf, 12);
    	}
    }
//    u8 = 1;
//    bleprofile_sendWriteReq(HANDLE_HELLO_SENSOR_CONFIGURATION, (UINT8 *)&u8, 1);
}

// Sleep mode should be disabled for scatternet.
// Callback called by the FW when ready to sleep/deep-sleep. Disable both by returning 0
UINT32 app_device_lpm_queriable(LowPowerModePollType type, UINT32 context)
{
   // Disable sleep.
   return 0;
}

