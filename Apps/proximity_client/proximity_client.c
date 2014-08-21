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
* BLE Proximity Client Device
*
* The Proximity Client device includes Clients for Link Loss, Immediate
* Alert and GATT.  During initialization application registers to
* receive various notifications from the LE stack.  including bonding complete,
* connection status change and peer notification.  When device is successfully
* bonded, application saves peer's Bluetooth Device address to the NVRAM.  It
* Also performs GATT Service discovery to find services available on the peer
* device.  For each of the services Proximity Client calls corresponding
* Client so that it can perform appropriate initialization.
*
* Features demonstrated
*  - Registration with LE stack for various events
*  - Service discovery
*  - Processing of the interrupts
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 6. Push and hold application button on the tag board for 6 seconds.  That
*    will start connection process.  Application will connect and pair to
*    a device which has link loss service in the advertisements.
* 7. Start advertisements on the proximity device
* 8. After connection is established quickly push and release the application
*    button to send Alert notification to the proximity device.
* 9. Push and release the application button to stop Alert.
* 10. Push and hold button for 6 seconds, to disconnect and repeat connection.
*
*/
#include "bleprofile.h"
#include "../../include/blecen.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "proximity_client.h"
#include "devicelpm.h"
#include "spar_utils.h"

/******************************************************
 *                      Constants
 ******************************************************/
#define CONNECT_ANY                 0x00

/******************************************************
 *                     Structures
 ******************************************************/

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void     prox_client_create(void);
static void     prox_client_timeout(UINT32 count);
static void     prox_client_fine_timeout(UINT32 finecount);
static void     prox_client_app_timer(UINT32 count);
static void     prox_client_app_fine_timer(UINT32 finecount);
static void     prox_client_advertisement_report(HCIULP_ADV_PACKET_REPORT_WDATA *evt);
static void     prox_client_connection_up(void);
static void     prox_client_connection_down(void);
static void     prox_client_smp_bond_result(LESMP_PARING_RESULT result);
static void     prox_client_encryption_changed(HCI_EVT_HDR *evt);
static void     prox_client_notification_handler(int len, int attr_len, UINT8 *data);
static void     prox_client_indication_handler(int len, int attr_len, UINT8 *data);
static void     prox_client_process_rsp(int len, int attr_len, UINT8 *data);
static UINT32   prox_client_interrupt_handler(UINT32 value);
static void     prox_client_timer_callback(UINT32 arg);
static BOOL     prox_client_is_paired_device(UINT8 *addr, UINT8 addressType);

static void prox_client_init_next_client(void);


/******************************************************
 *               Variables Definitions
 ******************************************************/
/*
 * This is the GATT database for the Proximity Client application.  It defines
 * services, characteristics and descriptors supported by the sensor.
 * Currently it is GATT, GAP, DIS and BAT
 */
const UINT8 prox_client_gatt_database[]=
{
    // Handle 0x01: GATT service
    // Service change characteristic is optional and is not present
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x14: GAP service
    // Device Name and Appearance are mandatory characteristics.
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value.
    // Any 16 byte string can be used to identify the sensor.  Just need to
    // replace the "Hello" string below.  Keep it short so that it fits in
    // advertisement data along with 16 byte UUID.
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'P','r','o','x',' ','C','l','i','e','n','t',0x00,0x00,0x00,0x00,0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value.
    // List of approved appearances is available at bluetooth.org.  Current
    // value is set to 0x200 - Generic Tag
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_TAG),

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
        '6','A','3','6',0x00,0x00,0x00,0x00,

    // Handle 0x52: characteristic System ID, handle 0x53 characteristic value
    CHARACTERISTIC_UUID16 (0x0052, 0x0053, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x93,0xb8,0x63,0x80,0x5f,0x9f,0x91,0x71,

    // Handle 0x61: Battery service
    // This is an optional service which allows peer to read current battery level.
    PRIMARY_SERVICE_UUID16 (0x0061, UUID_SERVICE_BATTERY),

    // Handle 0x62: characteristic Battery Level, handle 0x63 characteristic value
    CHARACTERISTIC_UUID16 (0x0062, 0x0063, UUID_CHARACTERISTIC_BATTERY_LEVEL,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x64,
};

const BLE_PROFILE_CFG prox_client_cfg =
{
    /*.fine_timer_interval            =*/ 100,  // ms
    /*.default_adv                    =*/ NO_DISCOVERABLE,
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 0,    // seconds
    /*.low_undirect_adv_duration      =*/ 0,    // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "Prox Client", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_TAG),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ (SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 4,    // dbm
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
    /*.status_led_con_blink           =*/ 1,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 1,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 1,    // blink num of undir adv
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
const BLE_PROFILE_PUART_CFG prox_client_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ GPIO_PIN_UART_TX,
    /*.rxpin      =*/ GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG prox_client_gpio_cfg =
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

typedef struct PACKED t_APP_STATE
{
    UINT8   gatt_client_initialized;	// True if GATT module was intitialized
    UINT8   link_loss_initialized;
    UINT8   findme_client_initialized;

#define RECONNECT_PAIRED            0
#define CONNECT_LINK_LOSS_SERVER    1
    UINT8   connect_flag;
    UINT8   alert_sent;

    UINT32 	app_timer_count;
    UINT32 	app_fine_timer_count;

    UINT16  gatt_s_handle;
    UINT16  gatt_e_handle;
    UINT16  link_loss_s_handle;
    UINT16  link_loss_e_handle;
    UINT16  findme_s_handle;
    UINT16  findme_e_handle;

    HOSTINFO hostinfo;					// NVRAM save area

} tAPP_STATE;

tAPP_STATE prox_client;

 BD_ADDR bd_addr_any                           	= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
 BD_ADDR prox_client_target_addr[6]             = {0xBA, 0xF0, 0xA5, 0x00, 0x00, 0x08};
UINT8   prox_client_target_addr_type            = 0;

UINT8   prox_client_alert_level_on_link_loss  = HIGH_ALERT;

// Following variables are in ROM 
extern BLE_CEN_CFG		blecen_cen_cfg;
extern BLEAPP_TIMER_CB	blecen_usertimerCb;
extern UINT16           blecen_con_handle;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)prox_client_gatt_database, sizeof (prox_client_gatt_database),
            (void *)&prox_client_cfg, (void *)&prox_client_puart_cfg,
            (void *)&prox_client_gpio_cfg, prox_client_create);
}


// Create aio client device
void prox_client_create(void)
{
    bleprofile_LEDBlink(250, 250, 1);

    extern UINT32 blecm_configFlag ;
    blecm_configFlag |= BLECM_DBGUART_LOG | BLECM_DBGUART_LOG_L2CAP | BLECM_DBGUART_LOG_SMP;

    memset (&prox_client, 0, sizeof (prox_client));

    ble_trace0("\rprox_client_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // Blecen default parameters.  Change if appropritate
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
    blecen_cen_cfg.low_supervision_timeout  = 400;
    blecen_cen_cfg.high_supervision_timeout = 600;
    blecen_usertimerCb						= prox_client_timer_callback;

    prox_client.connect_flag = CONNECT_LINK_LOSS_SERVER;

    blecen_Create();

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, prox_client_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, prox_client_connection_down);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(prox_client_encryption_changed);

    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) prox_client_smp_bond_result);

    // register to process peripheral advertisements, notifications and indications
    blecm_RegleAdvReportCb((BLECM_FUNC_WITH_PARAM) prox_client_advertisement_report);
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) prox_client_notification_handler);
    leatt_regIndicationCb((LEATT_TRIPLE_PARAM_CB) prox_client_indication_handler);

    // GATT client callbacks
    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) prox_client_process_rsp);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) prox_client_process_rsp);
    leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) prox_client_process_rsp);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) prox_client_process_rsp);

    // process button interrupts
    bleprofile_regIntCb(prox_client_interrupt_handler);


    // blecen starts scanning in the create.  Stop it until user pushes the button.
    blecen_Scan(NO_SCAN);

    // change timer callback function.  because we are running ROM app, need to
    // stop timer first.
    bleprofile_KillTimer();
    bleprofile_regTimerCb(prox_client_app_fine_timer, prox_client_app_timer);
    bleprofile_StartTimer();
}

// This function will be called on every connection establishmen
void prox_client_connection_up(void)
{
    ble_trace0("\rprox_client_connection_up");

    blecen_connUp();

    if (bleprofile_p_cfg->encr_required)
    {
        ble_trace0("\rprox_client_connection_up start pairing");

        // following call will start bonding if devices are not pairing, or will request
        // encryption if bonding has been established before
        lesmp_startPairing(NULL);
    }
    else
    {
        bleprofile_sendReadByGroupTypeReq(1, 0xffff, UUID_ATTRIBUTE_PRIMARY_SERVICE);
    }
}


void prox_client_advertisement_report(HCIULP_ADV_PACKET_REPORT_WDATA *evt)
{
    int connect = FALSE;

    blecen_leAdvReportCb(evt);

#ifdef PROX_CLIENT_MIN_RSSI
    if (evt->rssi < AIO_CLIENT_MIN_RSSI)      //filter out low RSSI
    {
        return;
    }
#endif
#if 0
    ble_trace3("\rADV report:%08x%04x type:%d appconfig:%04x",
                  (evt->wd_addr[5] << 24) + (evt->wd_addr[4] << 16) +
                  (evt->wd_addr[3] << 8) + evt->wd_addr[2],
                  (evt->wd_addr[1] << 8) + evt->wd_addr[0], evt->addressType);
#endif
#if defined CONNECT_ANY
    // connect to a device with specific bdaddr and type

    if ((memcmp (prox_client_target_addr, bd_addr_any, 6) == 0) ||
        ((memcmp(evt->wd_addr, prox_client_target_addr, 6) == 0) &&
         (evt->addressType == prox_client_target_addr_type)))
    {
        connect = TRUE;
    }
    else
#endif

    if ((prox_client.connect_flag == RECONNECT_PAIRED) &&
        prox_client_is_paired_device (evt->wd_addr, evt->addressType))
    {
        connect = TRUE;
    }
#if !CONNECT_ANY
    else if (prox_client.connect_flag == CONNECT_LINK_LOSS_SERVER)
    {
        // connect to any device which advertises Link Loss Service in the database
        BLE_ADV_FIELD *p_field;
        UINT8         *data = (UINT8 *)(evt->data);
        UINT8         *ptr;
        UINT8         dataLen = (UINT8)(evt->dataLen);
        int    		  i;

        ble_tracen((char *)(UINT8 *)(evt->data), dataLen <= HCIULP_MAX_DATA_LENGTH ? dataLen : HCIULP_MAX_DATA_LENGTH);

        // make sure that it is good data
        if (dataLen <= HCIULP_MAX_DATA_LENGTH)
        {
            // go through all the fields in advertisement data and search for Link Loss UUIDs
            for (ptr = data; ptr < data + dataLen; ptr += (p_field->len + 1))
            {
                p_field = (BLE_ADV_FIELD *)ptr;

                if ((p_field->len >= 2 + 1) && (p_field->len < sizeof (p_field->data)) &&
                    ((p_field->val == ADV_SERVICE_UUID16_COMP) || (p_field->val == ADV_SERVICE_UUID16_MORE)))
                {
                    // this field has full or shortened list of 16 bit UUIDs
                    for (i = 0; i < p_field->len - 1; i += 2)
                    {
                        if ((p_field->data[i] + (p_field->data[i + 1] << 8)) == UUID_SERVICE_LINK_LOSS)
                        {
                            ble_trace0("\rFound service");
                            connect = TRUE;
                            break;
                        }
                    }
                    // could not find desired service in the list of UUID16s
                    break;
                }
            }
        }
    }
#endif
    if (connect)
    {
        blecen_Scan(NO_SCAN);
        blecen_Conn(HIGH_CONN, evt->wd_addr, evt->addressType);
    }
}

// This function will be called when connection goes down
void prox_client_connection_down(void)
{
    ble_trace1("\rDisc_Reason: %02x", emconinfo_getDiscReason());

    // blecen_connDown will clear up handle and starts High duty scan
    blecen_connDown();
}

void prox_client_timeout(UINT32 count)
{
}

void prox_client_fine_timeout(UINT32 count)
{
    char input[READ_UART_LEN + 1];

    //ble_trace1("Finetimer:%d", finecount);

    //Reading
    bleprofile_ReadUART(input);
    //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
    //      input[0], input[1], input[2], input[3], input[4], input[5]);

    if (input[0] == 'D' && input[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        ble_trace0("CSA_filter disabled");
    }
    else if (input[0] == 'A' && input[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        ble_trace0("CSA_filter enabled");
    }
}

void prox_client_app_timer(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                prox_client.app_timer_count++;

                prox_client_timeout(prox_client.app_timer_count);
            }
            break;
    }

    blecen_appTimerCb(arg);
}

void prox_client_app_fine_timer(UINT32 arg)
{
    prox_client.app_fine_timer_count++;
    prox_client_fine_timeout(prox_client.app_fine_timer_count);

    // blecen_appFineTimerCb(arg);
}


//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void prox_client_smp_bond_result(LESMP_PARING_RESULT  result)
{
    ble_trace1("\rprox_client_smp_bond_result result:%d", result);
    blecen_smpBondResult(result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        bleprofile_sendReadByGroupTypeReq(1, 0xffff, UUID_ATTRIBUTE_PRIMARY_SERVICE);
    }
    else
    {
        blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void prox_client_encryption_changed(HCI_EVT_HDR *evt)
{
    UINT8 status = *((UINT8 *)(evt + 1));

    ble_trace1("\rProx Client encryption changed: %02x", status);

    blecen_encryptionChanged(evt);
}


void prox_client_timer_callback(UINT32 arg)
{
    switch(arg)
    {
    case BLEAPP_APP_TIMER_SCAN:
        if ((blecen_GetScan() == HIGH_SCAN) || (blecen_GetScan() == LOW_SCAN))
        {
            blecen_Scan(LOW_SCAN);
            ble_trace0("\rLow Scan");
        }
        break;

    case BLEAPP_APP_TIMER_CONN:
        if ((blecen_GetConn() == HIGH_CONN) || (blecen_GetConn() == LOW_CONN))
        {
            blecen_Conn(NO_CONN, NULL, 0);

            prox_client.connect_flag = RECONNECT_PAIRED;
            blecen_Scan(HIGH_SCAN);
            ble_trace0("\rConn Fail, Restart Scan");
        }
        break;
    }
}

void prox_client_readCb(int len, int attr_len, UINT8 *data)
{
    ble_trace2("\rprox_client_readCb:%d, %d", len, attr_len);
    ble_tracen((char *)data, len);
}

void prox_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int    i;
    UINT16 uuid, s_handle, e_handle;

    ble_trace2("\rClient Rsp len:%d, attr_len:%d", len, attr_len);

    if (len)
    {
        // always use group type
        // search UUID
        // S_HANDLE(2) E_HANDLE(2) UUID(2-16)
        for (i = 0; i < len; i += attr_len)
        {
            s_handle = data[i]     + (data[i + 1] << 8);
            e_handle = data[i + 2] + (data[i + 3] << 8);

            // services on the server can be based on 2 or 16 bytes UUIDs
            if (attr_len == 6)
            {
                uuid = data[i + 4] + (data[i + 5] << 8);
                ble_trace3("\rs:%04x e:%04x uuid:%04x", s_handle, e_handle, uuid);
                if (uuid == UUID_SERVICE_GATT)
                {
                    // remember GATT service start and end handles
                    prox_client.gatt_s_handle = s_handle;
                    prox_client.gatt_e_handle = e_handle;
                    ble_trace3("\rGatt s:%04x e:%04x uuid:%04x", s_handle, e_handle, uuid);
                }
                else if (uuid == UUID_SERVICE_LINK_LOSS)
                {
                    prox_client.link_loss_s_handle = s_handle;
                    prox_client.link_loss_e_handle = e_handle;
                    ble_trace3("\rLink Loss s:%04x e:%04x uuid:%04x", s_handle, e_handle, uuid);
                }
                else if (uuid == UUID_SERVICE_IMMEDIATE_ALERT)
                {
                    prox_client.findme_s_handle = s_handle;
                    prox_client.findme_e_handle = e_handle;
                    ble_trace3("\rImmed Alert s:%04x e:%04x uuid:%04x", s_handle, e_handle, uuid);
                }
            }
        }
        // continue reading
        // start reading from the last handle + 1
        bleprofile_sendReadByGroupTypeReq(e_handle + 1, 0xffff, UUID_ATTRIBUTE_PRIMARY_SERVICE);
        ble_trace2("Service Search s_hdl:%04x, e_hdl:%04x", e_handle + 1, 0xffff);
    }
    else
    {
        prox_client_init_next_client();
    }
}

void prox_client_gatt_initialization_callback(int result)
{
    prox_client.gatt_client_initialized = TRUE;
    prox_client_init_next_client();
}

void prox_client_link_loss_initialization_callback(int result)
{
    prox_client.link_loss_initialized = TRUE;
    prox_client_init_next_client();
}

void prox_client_findme_initialization_callback(int result)
{
    prox_client.findme_client_initialized = TRUE;
    prox_client_init_next_client();
}

void prox_client_init_next_client(void)
{
    ble_trace3("client %d %d %d", prox_client.gatt_client_initialized, prox_client.link_loss_initialized, prox_client.findme_client_initialized);

    if (!prox_client.gatt_client_initialized && (prox_client.gatt_s_handle != 0) && (prox_client.gatt_e_handle != 0))
    {
        gatt_client_initialize(prox_client.gatt_s_handle, prox_client.gatt_e_handle, &prox_client_gatt_initialization_callback);
    }
    else if (!prox_client.link_loss_initialized && (prox_client.link_loss_s_handle != 0) && (prox_client.link_loss_e_handle != 0))
    {
        link_loss_client_initialize(prox_client.link_loss_s_handle, prox_client.link_loss_e_handle, prox_client_alert_level_on_link_loss, &prox_client_link_loss_initialization_callback);
    }
    else if (!prox_client.findme_client_initialized && (prox_client.findme_s_handle != 0) && (prox_client.findme_e_handle != 0))
    {
        findme_client_initialize(prox_client.findme_s_handle, prox_client.findme_e_handle, &prox_client_findme_initialization_callback);
    }
    else
    {
        // We are done with initial settings, and need to stay connected.  It is a good
        // time to slow down the pace of master polls to save power.  Following request asks
        // host to setup polling every 100-500 msec, with link supervision timeout 7 seconds.
        blecen_ConnUpdate(SLOW_CONN);
    }
}


void prox_client_process_data(int len, int attr_len, UINT8 *data)
{
}

// process notifications from the client
void prox_client_notification_handler(int len, int handle, UINT8 *data)
{
    ble_trace2("\rprox_client Notification handle:%02x len:%d", (UINT16)handle, len);
    if ((handle >= prox_client.gatt_s_handle) && (handle <= prox_client.gatt_e_handle))
    {
        gatt_client_notification_handler(len, handle, data);
    }
}

void prox_client_indication_handler(int len, int attr_len, UINT8 *data)
{
    ble_trace2("\rProx Client Indication:%02x, %d", (UINT16)attr_len, len);
    ble_tracen((char *)data, len);

    bleprofile_sendHandleValueConf();
}

UINT32 prox_client_interrupt_handler(UINT32 value)
{
    BLEPROFILE_DB_PDU 	db_pdu;
    static  UINT32 		button_pushed_time = 0;
    BOOL 				button_pushed = value & 0x01;

    ble_trace3 ("\r(INT)But1:%d But2:%d But3:%d", value & 0x01, (value & 0x02) >> 1, (value & 0x04) >> 2);

    if (button_pushed)
    {
        button_pushed_time = prox_client.app_timer_count;
    }
    else if (button_pushed_time != 0)
    {
        // if we are connected, the button is to send Alert notification to the server
        if (blecen_con_handle != 0)
        {
            if (prox_client.app_timer_count - button_pushed_time > 5)
            {
                blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

                lesmpkeys_removeAllBondInfo();
                ble_trace0("\rPairing Key removed");
            }
            else
            {
                if (!prox_client.alert_sent)
                {
                    prox_client.alert_sent = TRUE;
                    findme_client_send_alert(HIGH_ALERT);
                }
                else
                {
                    prox_client.alert_sent = FALSE;
                    findme_client_send_alert(NO_ALERT);
                }
            }
        }
        else
        {
            // We are not connected.  Pushing the button for less than 5 seconds means
            // that user wants to reconnected previously bonded device.  Pushing the
            // button for more than 5 seconds will remove all bonding information and
            // set device in the mode when new bonding is accepted
            // pairing is accepted.
            if (prox_client.app_timer_count - button_pushed_time < 5)
            {
                prox_client.connect_flag = RECONNECT_PAIRED;
            }
            else
            {
                blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

                prox_client.connect_flag = CONNECT_LINK_LOSS_SERVER;

                lesmpkeys_removeAllBondInfo();
                ble_trace0("\rPairing Key removed");
            }
            blecen_Scan(HIGH_SCAN);
            ble_trace0("\rDiscover High Scan");
        }
    }
    return 0;
}

BOOL prox_client_is_paired_device(UINT8 *addr, UINT8 addressType)
{
    int index;
    // if this is a private address need to resolve.
    if ((addressType != 0) && ((addr[0] & 0xc0) == 0x40))
    {
        index = blecm_resolveRPA(HCIULP_RANDOM_ADDRESS, (UINT32 *)addr);   // This one calls lookup first
    }
    else
    {
        // this is public or static random address
        index = lesmpkeys_find(addr, addressType);
    }
    return (index != -1);
}

