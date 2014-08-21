/*
* Copyright 2013, Broadcom Corporation
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom Corporation.
*/

/** @file
*
* BLE Proximity profile, service, application 
*
* This is a ROM code for complete implementation of BLE proximity device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  
*
* Refer to Bluetooth SIG Proximity Profile 1.0, Immediate Alert Service
* 1.0, Link Loss Service 1.0, Tx Power Service, and Battery Service 1.0 
* specifications for details.
*
* During initialization the app registers with LE stack to receive various
* notification including bonding complete, connection status change and
* peer write.  When device is successfully bonded, application saves
* peer's Bluetooth Device address to the NVRAM.  Bonded device can also 
* write client configuration descriptor for BPM measurement and BPM 
* intermediate measurement and that is also save in the NVRAM.  When new 
* measurement is received (for example over the fake UART), it is sent to 
* the bonded and registered host.
*
* Features demonstrated
*  - GATT database and Device configuration initialization
*  - Registration with LE stack for various events
*  - NVRAM read/write operations
*  - Timer and Fine timer operations
*    Processing Writes from the client
*  - Sending data to the client
*  - Processing of a Button
*  - Control LED 
*  - Control Buzzer
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "bleprox.h"
#include "blefind.h"
#include "blebat.h"
#include "dbfw_app.h"

#define BLE_P2

#define FID   FID_BLEAPP_APP__BLEPROX_C
//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void bleprox_Timeout(UINT32 count);
void bleprox_ReadButton(void);
void bleprox_SetAlert(UINT8 level);
void bleprox_SetLED(UINT8 interval, UINT8 duration, UINT8 blinknum, UINT8 timeout);
void bleprox_SetBuzzer(UINT8 interval, UINT8 duration, UINT8 timeout);
void bleprox_ControlAlert(void);
void bleprox_ControlLED(void);
void bleprox_ControlBuzzer(void);
void bleprox_DBInit(void);
void bleprox_CheckPathLoss(void);
void bleprox_Reconnect(void);
void bleprox_connUp(void);
void bleprox_connDown(void);
void bleprox_advStop(void);
void bleprox_appTimerCb(UINT32 arg);
void bleprox_appFineTimerCb(UINT32 arg);

void bleprox_smpBondResult(LESMP_PARING_RESULT  result);
void bleprox_encryptionChanged(HCI_EVT_HDR *evt);

int bleprox_notification_writeCb(LEGATTDB_ENTRY_HDR *p);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 bleprox_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 4), 
        0x00, 0x00, 0x00, 0x00,

    // GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),
    
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','P','r','o','x',' ','k','e','y',' ','f','o','b',  

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00,0x00,

    // Link Loss service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_LINK_LOSS),

    CHARACTERISTIC_UUID16_WRITABLE (0x0029, 0x002a, UUID_CHARACTERISTIC_ALERT_LEVEL, 
                                     LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, 
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ,  1),
        0x01,                       

    // Immediate alert service
    PRIMARY_SERVICE_UUID16 (0x002B, UUID_SERVICE_IMMEDIATE_ALERT),

    CHARACTERISTIC_UUID16_WRITABLE (0x002c, 0x002d, UUID_CHARACTERISTIC_ALERT_LEVEL, 
                                     LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE, LEGATTDB_PERM_WRITE_CMD,  1),
        0x00,

    // Tx Power service
    PRIMARY_SERVICE_UUID16 (0x002e, UUID_SERVICE_TX_POWER),

    CHARACTERISTIC_UUID16 (0x002f, 0x0030, UUID_CHARACTERISTIC_TX_POWER_LEVEL, 
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE,  1),
        0x04,                       // this should be matched to ADV data

    // Battery service
    PRIMARY_SERVICE_UUID16 (0x0031, UUID_SERVICE_BATTERY),

    CHARACTERISTIC_UUID16 (0x0032, 0x0033, UUID_CHARACTERISTIC_BATTERY_LEVEL, 
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE,  1),
        0x64,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0034, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x041, 0x0042, UUID_CHARACTERISTIC_BATTERY_POWER_STATE, 
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 1),
        BLEBAT_POWERSTATE_PRESENT_PRESENT|
        BLEBAT_POWERSTATE_DISCHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_CHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_LEVEL_GOODLEVEL,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0043, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0044, 0x0045, UUID_CHARACTERISTIC_SERVICE_REQUIRED, 
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 1),
        BLEBAT_SERVICEREQUIRED_NOSERVICEREQUIRED,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0046, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0047, 0x0048, UUID_CHARACTERISTIC_REMOVABLE, 
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE,  1),
        BLEBAT_REMOVABLE_UNKNOWN,

    CHARACTERISTIC_UUID16 (0x004a, 0x004b, UUID_CHARACTERISTIC_BATTERY_LEVEL_STATE, 
                           LEGATTDB_CHAR_PROP_BROADCAST | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 5),
        0x64,           // Level
        BLEBAT_POWERSTATE_PRESENT_PRESENT|
        BLEBAT_POWERSTATE_DISCHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_CHARGING_NOTSUPPORTED|
        BLEBAT_POWERSTATE_LEVEL_GOODLEVEL,
        0x00,           // Namespace
        0x00, 0x00,     // Description

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x004C, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x004D, UUID_DESCRIPTOR_SERVER_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,
};

const UINT16 bleprox_db_size = sizeof(bleprox_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG bleprox_cfg =
{
    /*.fine_timer_interval            =*/ 500,  // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 5,    // seconds
    /*.low_direct_adv_interval        =*/ 10,   // seconds
    /*.high_direct_adv_duration       =*/ 30,   // seconds
    /*.low_direct_adv_duration        =*/ 300,  // seconds
    /*.local_name                     =*/ "BLE Prox key fob", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_KEYRING), 0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 0,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 5,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x0030, 0x0033, 0x0034}, // [HANDLE_NUM_MAX] GATT HANDLE number
    /*.serv                           =*/ {UUID_SERVICE_LINK_LOSS, UUID_SERVICE_IMMEDIATE_ALERT, 
                                            UUID_SERVICE_TX_POWER, UUID_SERVICE_BATTERY, 
                                            UUID_CHARACTERISTIC_BATTERY_LEVEL}, // GATT service UUID
    /*.cha                            =*/ {UUID_CHARACTERISTIC_ALERT_LEVEL, UUID_CHARACTERISTIC_ALERT_LEVEL, 
                                            UUID_CHARACTERISTIC_TX_POWER_LEVEL, UUID_CHARACTERISTIC_BATTERY_LEVEL,
                                            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION}, // GATT characteristic UUID
    /*.findme_locator_enable          =*/ 3,    // if 1 Find me locator is enable if 3 Find me alert will be toggled (Alert/No Alert)
    /*.findme_alert_level             =*/ HIGH_ALERT, // alert level of find me
    /*.client_grouptype_enable        =*/ 1,    // if 1 groupbytype read can be used
    /*.linkloss_button_enable         =*/ 1,    // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 1,    // second
    /*.alert_interval                 =*/ 1,    // interval of alert
    /*.high_alert_num                 =*/ 5,    // number of alert for each interval
    /*.mild_alert_num                 =*/ 2,    // number of alert for each interval
    /*.status_led_enable              =*/ 1,    // if 1 status LED is enable
    /*.status_led_interval            =*/ 5,    // second
    /*.status_led_con_blink           =*/ 1,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 2,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 3,    // blink num of undir adv
    /*.led_on_ms                      =*/ 5,    // led blink on duration in ms
    /*.led_off_ms                     =*/ 250,  // led blink off duration in ms
    /*.buz_on_ms                      =*/ 100,  // buzzer on duration in ms
    /*.button_power_timeout           =*/ 1,    // seconds
    /*.button_client_timeout          =*/ 3,    // seconds
    /*.button_discover_timeout        =*/ 5,    // seconds
    /*.button_filter_timeout          =*/ 10,   // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};


#ifdef BLE_POWERSAVETEST
PLACE_IN_DROM const BLE_PROFILE_PUART_CFG bleprox_puart_cfg =
{
    /*.baudrate   =*/ 115200, 
    /*.txpin      =*/ PUARTDISABLE | 32, // GPIO pin number 20730A0 module need to use 32 instead, normally it is 31
    /*.rxpin      =*/ PUARTDISABLE | 33, // GPIO pin number
};
#else
PLACE_IN_DROM const BLE_PROFILE_PUART_CFG bleprox_puart_cfg =
{
    /*.baudrate   =*/ 115200, 
    /*.txpin      =*/ 32,               // GPIO pin number 20730A0 module need to use 32 instead, normally it is 31
    /*.rxpin      =*/ 33,               // GPIO pin number
};
#endif

#ifdef BLE_P2
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG bleprox_gpio_cfg =
{
    {1, 0, 15, 14, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BAT,
      GPIO_OUTPUT|GPIO_INIT_HIGH|GPIO_LED,
      GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_BUZ,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};
#else
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG bleprox_gpio_cfg =
{
    {6, 14, 15, 26, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BAT,
      GPIO_OUTPUT|GPIO_INIT_HIGH|GPIO_LED,
      GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_BUZ,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};
#endif

typedef struct
{
    BLEPROX_HOSTINFO            bleprox_hostinfo;           // NVRAM save area
    BD_ADDR                     bleprox_remote_addr;        // bdaddr of connected device

    UINT32                      bleprox_apptimer_count;
    UINT32                      bleprox_appfinetimer_count;

    UINT16                      bleprox_con_handle;
    UINT16                      bleprox_isReconnect;
    UINT8                       bleprox_linkloss_alert;
    UINT8                       bleprox_pathloss_alert;
    INT8                        bleprox_pathloss_alert_count;
    UINT8                       bleprox_powersave;
    UINT8                       bleprox_button_count;
    UINT8                       bleprox_button_led;
    BLE_PROFILE_TIMER_CONTROL   bleprox_led;
    BLE_PROFILE_TIMER_CONTROL   bleprox_buzzer;

    //GATT related variables
    UINT16                      bleprox_ll_alt_hdl;
    UINT16                      bleprox_ia_alt_hdl;
    UINT16                      bleprox_tx_pw_hdl;

    //Battery service related variables
    UINT8                       bleprox_bat_enable;
} tProxAppState;

tProxAppState *proxAppState = NULL;


void bleprox_Create(void)
{
    BLEPROFILE_DB_PDU db_pdu;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox__Create(), Ver[0:3] %02x %02x %02x %02x:", TVF_BBBB(bleprofile_p_cfg->ver[0], bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[2], bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]: %02x %02x", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    proxAppState = (tProxAppState *)cfa_mm_Sbrk(sizeof(tProxAppState));
    memset(proxAppState, 0x00, sizeof(tProxAppState));

    //initialize the default value of proxAppState
    proxAppState->bleprox_linkloss_alert        = NO_ALERT;
    proxAppState->bleprox_pathloss_alert        = NO_ALERT;
    proxAppState->bleprox_pathloss_alert_count  = -1;

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg); //This will change Tx power based on config
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    bleprox_DBInit(); //load handle number

    //change Tx power GATT characteristic
    db_pdu.len    = 1;
    db_pdu.pdu[0] = bleprofile_p_cfg->tx_power_level;
    bleprofile_WriteHandle(proxAppState->bleprox_tx_pw_hdl, &db_pdu);

    blebat_Init();

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, bleprox_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, bleprox_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, bleprox_advStop );

    // this is for checking notification
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)bleprox_notification_writeCb);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(bleprox_encryptionChanged);
    // hander for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) bleprox_smpBondResult);
    //handler for Find me result
    if (bleprofile_p_cfg->findme_locator_enable)
    {
        blefind_Init();
    }

    bleprofile_regButtonFunctionCb(bleprox_ProxButton);

    // show set up is complete
    bleprofile_LEDBlink(bleprofile_p_cfg->led_off_ms, bleprofile_p_cfg->led_off_ms, 1);

    bleprofile_regTimerCb(bleprox_appFineTimerCb, bleprox_appTimerCb);
    bleprofile_StartTimer();

    bleprox_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);
}


void bleprox_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);

    if (!proxAppState->bleprox_powersave)
    {
        bleprox_Reconnect();

        if (count%(bleprofile_p_cfg->pathloss_check_interval) == 0)
        {
            bleprox_CheckPathLoss();
        }

        bleprox_ControlAlert();

        if (proxAppState->bleprox_bat_enable)
        {
            blebat_pollMonitor();
        }
    }

    if (bleprofile_pollPowersave())
    {
        proxAppState->bleprox_powersave = 1;
    }
}

void bleprox_FineTimeout(UINT32 finecount)
{
    char prox_char[READ_UART_LEN + 1];

    //ble_trace1("Finetimer:%d", finecount);

    //Reading Proxy
    bleprofile_ReadUART(prox_char);

    if (prox_char[0] == 'D' && prox_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Disable", TVF_D(0));
    }
    else if (prox_char[0] == 'A' && prox_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Enable", TVF_D(0));
    }

    bleprofile_ReadButton();

    if (!proxAppState->bleprox_powersave && bleprofile_p_cfg->findme_locator_enable) // not powersaving mode
    {
        blefind_FindmeReq();
    }

#if 0
    if (finecount % bleprox_call_per_sec)
    {
        bleprox_Timeout(finecount/bleprox_call_per_sec);
    }
#endif
}


UINT32 bleprox_ProxButton(UINT32 function)
{
    if (function == BUTTON_PRESS)
    {
        proxAppState->bleprox_button_led = 1;
    }
    else if (function == BUTTON_RELEASE)
    {
        proxAppState->bleprox_button_led = 0;
    }
    else if (function == BUTTON_POWER)
    {
        if (proxAppState->bleprox_powersave) //button powersave is on
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Power Save Off", TVF_D(0));
            proxAppState->bleprox_powersave = 0; //disable powersave
        }
        else
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Power Save On", TVF_D(0));
            proxAppState->bleprox_powersave = 1; //enable powersave

            proxAppState->bleprox_con_handle = 0;
        }
    }
    else if (function == BUTTON_CLIENT)
    {
        if (proxAppState->bleprox_con_handle && bleprofile_p_cfg->findme_locator_enable)
        {
            blefind_FindmeButton();
        }
        else if (proxAppState->bleprox_powersave && bleprofile_p_cfg->linkloss_button_enable)
        {
            BLEPROFILE_DB_PDU db_pdu;
            UINT8 alertlevel;
            int result;

            //read link loss alert
            result = bleprofile_ReadHandle(proxAppState->bleprox_ll_alt_hdl, &db_pdu);

            if (!result)
            {
                //toggle link loss alert
                alertlevel = db_pdu.pdu[0] + 1;
                if (alertlevel > HIGH_ALERT)
                {
                    alertlevel = NO_ALERT;
                }

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "LL_Alert_Level:%x->%x", TVF_BB(db_pdu.pdu[0], alertlevel));

                db_pdu.len    = 1;
                db_pdu.pdu[0] = alertlevel;
                bleprofile_WriteHandle(proxAppState->bleprox_ll_alt_hdl, &db_pdu);
                ble_tracen((char *)db_pdu.pdu, db_pdu.len);

                bleprofile_LEDBlink(bleprofile_p_cfg->led_off_ms, bleprofile_p_cfg->led_off_ms, alertlevel);
            }
        }
    }
    else if (function == BUTTON_DISCOVER)
    {
        proxAppState->bleprox_powersave = 0; //disable powersave

        if ((bleprofile_GetDiscoverable() != HIGH_UNDIRECTED_DISCOVERABLE) &&
            (bleprofile_GetDiscoverable() != LOW_UNDIRECTED_DISCOVERABLE))
        {
            if (proxAppState->bleprox_con_handle)
            {
                proxAppState->bleprox_con_handle = 0;
            }

            proxAppState->bleprox_linkloss_alert = UNDIRECTED_DISCOVERABLE_ALERT;

            bleprox_SetAlert(UNDIRECTED_DISCOVERABLE_ALERT);

        }
        else if (bleprofile_p_cfg->button_adv_toggle)
        {
            proxAppState->bleprox_linkloss_alert = NO_ALERT;

            bleprox_SetAlert(NO_ALERT);
        }
    }
    else if (function == TIMEOUT_POWER)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Power Save On timeout", TVF_D(0));
        proxAppState->bleprox_powersave = 1; //enable powersave

        proxAppState->bleprox_con_handle = 0;
    }

    return 0;
}


void bleprox_SetAlert(UINT8 level)
{
    if (level == NO_ALERT) // no alert
    {
        // need to differentiate Connected No Alert state and non-connected No Alert state (no pairing state)
        if (proxAppState->bleprox_con_handle && bleprofile_p_cfg->status_led_enable) //Connected No Alert
        {
            bleprox_SetLED(bleprofile_p_cfg->status_led_interval, 0, bleprofile_p_cfg->status_led_con_blink, 0xFF); //no timeout
            bleprox_SetBuzzer(0, 0, 0);
        }
        else //Disconnected No alert
        {
            bleprox_SetLED(0, 0, 0, 0);
            bleprox_SetBuzzer(0, 0, 0);
        }
    }
    else if (level == MILD_ALERT) //mild alert
    {
        bleprox_SetLED(bleprofile_p_cfg->alert_interval, 0, 1, bleprofile_p_cfg->mild_alert_num);//brcmPROXSetLED(5, 0, 2, 0xFF); //no timeout   //moved to brcmPROXReconnect() function

        bleprox_SetBuzzer(bleprofile_p_cfg->alert_interval, 0, bleprofile_p_cfg->mild_alert_num);
    }
    else if (level == HIGH_ALERT) //high alert
    {
        bleprox_SetLED(bleprofile_p_cfg->alert_interval, 0, 1, bleprofile_p_cfg->high_alert_num);//brcmPROXSetLED(5, 0, 2, 0xFF); //no timeout   //moved to brcmPROXReconnect() function

        bleprox_SetBuzzer(bleprofile_p_cfg->alert_interval, 0, bleprofile_p_cfg->high_alert_num);
    }
    else if (level == UNDIRECTED_DISCOVERABLE_ALERT)
    {
        if (bleprofile_p_cfg->status_led_enable)
        {
            bleprox_SetLED(bleprofile_p_cfg->status_led_interval, 0, bleprofile_p_cfg->status_led_un_adv_blink, 0xFF); //no timeout
        }
    }
}

void bleprox_SetLED(UINT8 interval, UINT8 duration, UINT8 blinknum, UINT8 timeout)
{
    proxAppState->bleprox_led.count = 0;

    if ((interval != 0) && (interval >= duration))
    {
        proxAppState->bleprox_led.interval = interval;
        proxAppState->bleprox_led.duration = duration;
        proxAppState->bleprox_led.blinknum = blinknum; //this is just a number

        if (timeout == 0xFF)
            proxAppState->bleprox_led.timeout = 0xFF;
        else
            proxAppState->bleprox_led.timeout = timeout;
    }
    else
    {
        proxAppState->bleprox_led.interval = 0;
        proxAppState->bleprox_led.duration = 0;
        proxAppState->bleprox_led.blinknum = 0;
        proxAppState->bleprox_led.timeout  = 0;

        // GPIO26 turn on (LED will off) - needed for handling continuouse on case
        bleprofile_LEDOff();
    }
}

void bleprox_SetBuzzer(UINT8 interval, UINT8 duration, UINT8 timeout)
{
    proxAppState->bleprox_buzzer.count = 0;

    if ((interval != 0) && (interval >= duration))
    {
        proxAppState->bleprox_buzzer.interval = interval;
        proxAppState->bleprox_buzzer.duration = duration;
        if (timeout == 0xFF)
            proxAppState->bleprox_buzzer.timeout = 0xFF;
        else
            proxAppState->bleprox_buzzer.timeout = timeout;
    }
    else
    {
        proxAppState->bleprox_buzzer.interval = 0;
        proxAppState->bleprox_buzzer.duration = 0;
        proxAppState->bleprox_buzzer.timeout  = 0;
    }
}

void bleprox_ControlAlert(void)
{
    bleprox_ControlLED();

    bleprox_ControlBuzzer();
}

void bleprox_ControlLED(void)
{
    if (proxAppState->bleprox_button_led) //button is pressed. Button LED has the highest priority, return without change LED.
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "LED Blocked", TVF_D(0));
        return;
    }

    if (proxAppState->bleprox_led.interval == 0) //LED is not working
    {
        return;
    }

    if (proxAppState->bleprox_led.timeout == 0xFF)
    {
        //do nothing
        //no stopping
    }
    else if (proxAppState->bleprox_led.count >= proxAppState->bleprox_led.timeout) //timeout, turn off timer
    {
        proxAppState->bleprox_led.count    = 0;
        proxAppState->bleprox_led.interval = 0;
        proxAppState->bleprox_led.duration = 0;
        proxAppState->bleprox_led.blinknum = 0;
        proxAppState->bleprox_led.timeout  = 0;

        return;
    }

    if (proxAppState->bleprox_led.count % proxAppState->bleprox_led.interval == 0) // turn on
    {
        if (proxAppState->bleprox_led.duration == 0) // blink
        {
            bleprofile_LEDBlink(bleprofile_p_cfg->led_on_ms, bleprofile_p_cfg->led_off_ms, proxAppState->bleprox_led.blinknum);

            //ble_trace1("LED Off(Blink:%02x)", proxAppState->bleprox_led.blinknum);
        }
        else // This is the normal on case that has duration
        {
            bleprofile_LEDOn();

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "LED On", TVF_D(0));
        }
    }
    else if (proxAppState->bleprox_led.count % proxAppState->bleprox_led.interval == proxAppState->bleprox_led.duration) // turn off
    {
        // when interval and duration is set as same number, this loop will not enter because of else if
        // It will turn on LED only
        bleprofile_LEDOff();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "LED Off", TVF_D(0));
    }

    (proxAppState->bleprox_led.count)++;
}

void bleprox_ControlBuzzer(void)
{
    if (proxAppState->bleprox_buzzer.interval == 0) //Buzzer is not working
    {
        return;
    }

    if (proxAppState->bleprox_buzzer.timeout == 0xFF)
    {
        //do nothing
        //no stopping
    }
    else if (proxAppState->bleprox_buzzer.count >= proxAppState->bleprox_buzzer.timeout) //timeout, turn off timer
    {
        proxAppState->bleprox_buzzer.count    = 0;
        proxAppState->bleprox_buzzer.interval = 0;
        proxAppState->bleprox_buzzer.duration = 0;
        proxAppState->bleprox_buzzer.timeout  = 0;

        return;
    }

    if (proxAppState->bleprox_buzzer.count % proxAppState->bleprox_buzzer.interval == 0) // turn on
    {
        bleprofile_BUZBeep(bleprofile_p_cfg->buz_on_ms);

        //ble_trace0("Buzzer(Short Beep)");
    }
    else if (proxAppState->bleprox_buzzer.count % proxAppState->bleprox_buzzer.interval == proxAppState->bleprox_buzzer.duration) // turn off
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Buzzer Off", TVF_D(0));
    }

    proxAppState->bleprox_buzzer.count++;
}


void bleprox_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;
    //load handle number

    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_LINK_LOSS &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_ALERT_LEVEL)
        {
            proxAppState->bleprox_ll_alt_hdl=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox_ll_alt_hdl:%04x", TVF_D(proxAppState->bleprox_ll_alt_hdl));
            bleprofile_ReadHandle(proxAppState->bleprox_ll_alt_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_IMMEDIATE_ALERT &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_ALERT_LEVEL)
        {
            proxAppState->bleprox_ia_alt_hdl=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox_ia_alt_hdl:%04x", TVF_D(proxAppState->bleprox_ia_alt_hdl));
            bleprofile_ReadHandle(proxAppState->bleprox_ia_alt_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_TX_POWER &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_TX_POWER_LEVEL)
        {
            proxAppState->bleprox_tx_pw_hdl=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox_tx_pw_hdl:%04x", TVF_D(proxAppState->bleprox_tx_pw_hdl));
            bleprofile_ReadHandle(proxAppState->bleprox_tx_pw_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            proxAppState->bleprox_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    db_pdu.len = 1;
    db_pdu.pdu[0] = 2;
    bleprofile_WriteHandle(proxAppState->bleprox_ll_alt_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif
}

void bleprox_CheckPathLoss(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int result;

    result = bleprofile_ReadHandle(proxAppState->bleprox_ia_alt_hdl, &db_pdu);

    if (!result)
    {
        if (db_pdu.pdu[0] == HIGH_ALERT) //high
        {
            if (proxAppState->bleprox_pathloss_alert != HIGH_ALERT)
            {
                proxAppState->bleprox_pathloss_alert = HIGH_ALERT;

                 if (proxAppState->bleprox_linkloss_alert == NO_ALERT) //linkloss alert has higher priority than pathloss alert
                 {
                    proxAppState->bleprox_pathloss_alert_count = 0;
                    bleprox_SetAlert(HIGH_ALERT);
                 }
            }
            else
            {
                proxAppState->bleprox_pathloss_alert_count += (bleprofile_p_cfg->pathloss_check_interval/bleprofile_p_cfg->alert_interval);

                if (proxAppState->bleprox_pathloss_alert_count >= bleprofile_p_cfg->high_alert_num)
                {
                    proxAppState->bleprox_pathloss_alert_count = -1;
                    //immediate alert DB reset
                    db_pdu.len = 1;
                    db_pdu.pdu[0] = NO_ALERT;
                    bleprofile_WriteHandle(proxAppState->bleprox_ia_alt_hdl, &db_pdu);
                    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
                }
            }
        }
        else if (db_pdu.pdu[0] == MILD_ALERT) //low
        {
            if (proxAppState->bleprox_pathloss_alert != MILD_ALERT)
            {
                proxAppState->bleprox_pathloss_alert = MILD_ALERT;

                if (proxAppState->bleprox_linkloss_alert == NO_ALERT) //linkloss alert has higher priority than pathloss alert
                {
                    proxAppState->bleprox_pathloss_alert_count = 0;
                    bleprox_SetAlert(MILD_ALERT);
                }
            }
            else
            {
                proxAppState->bleprox_pathloss_alert_count += (bleprofile_p_cfg->pathloss_check_interval/bleprofile_p_cfg->alert_interval);

                if (proxAppState->bleprox_pathloss_alert_count >= bleprofile_p_cfg->mild_alert_num)
                {
                    proxAppState->bleprox_pathloss_alert_count = -1;
                    //immediate alert DB reset
                    db_pdu.len = 1;
                    db_pdu.pdu[0] = NO_ALERT;
                    bleprofile_WriteHandle(proxAppState->bleprox_ia_alt_hdl, &db_pdu);
                    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
                }
            }
        }
        else if (db_pdu.pdu[0] == NO_ALERT) //low
        {
            if (proxAppState->bleprox_pathloss_alert != NO_ALERT)
            {
                proxAppState->bleprox_pathloss_alert = NO_ALERT;

                if (proxAppState->bleprox_linkloss_alert == NO_ALERT) //linkloss alert has higher priority than pathloss alert
                {
                    bleprox_SetAlert(NO_ALERT);
                }
            }
        }
    }
}

void bleprox_Reconnect(void)
{
    if (proxAppState->bleprox_isReconnect>0)
    {
        if (bleprofile_p_cfg->status_led_enable)
        {
            if (((proxAppState->bleprox_linkloss_alert == HIGH_ALERT) && (proxAppState->bleprox_isReconnect == (1 + bleprofile_p_cfg->high_alert_num + 1))) ||
                ((proxAppState->bleprox_linkloss_alert == MILD_ALERT) && (proxAppState->bleprox_isReconnect == (1 + bleprofile_p_cfg->mild_alert_num + 1))) ||
                ((proxAppState->bleprox_linkloss_alert == NO_ALERT)   && (proxAppState->bleprox_isReconnect == 1)))
            {
                UINT8 discoverable = bleprofile_GetDiscoverable();

                if (discoverable == HIGH_DIRECTED_DISCOVERABLE ||
                    discoverable == LOW_DIRECTED_DISCOVERABLE)
                {
                    bleprox_SetLED(bleprofile_p_cfg->status_led_interval, 0, bleprofile_p_cfg->status_led_dir_adv_blink, 0xFF); //no timeout
                }
                else if (discoverable == HIGH_UNDIRECTED_DISCOVERABLE ||
                         discoverable == LOW_UNDIRECTED_DISCOVERABLE)
                {
                    bleprox_SetLED(bleprofile_p_cfg->status_led_interval, 0, bleprofile_p_cfg->status_led_un_adv_blink, 0xFF); //no timeout
                }
                else
                {
                    bleprox_SetLED(0, 0, 0, 0);
                }

                proxAppState->bleprox_isReconnect = 0;

                return;
            }
        }
        (proxAppState->bleprox_isReconnect)++;
        if (proxAppState->bleprox_isReconnect==0)
        {
            proxAppState->bleprox_isReconnect = 1;
        }
    }
}


void bleprox_connUp(void)
{
    UINT8 *bda, *puba;

    proxAppState->bleprox_isReconnect = 0;
    proxAppState->bleprox_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    bda  = (UINT8 *)emconninfo_getPeerAddr();
    puba = (UINT8 *)emconninfo_getPeerPubAddr();

    memcpy(proxAppState->bleprox_remote_addr, bda, sizeof(proxAppState->bleprox_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox_connUp: bda[5..2] = %02X %02x %02x %02x", TVF_BBBB(bda[5], bda[4], bda[3], bda[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox_connUp: bda[1..0], handle = %02X %02x, %04X", TVF_BBW(bda[1], bda[0], proxAppState->bleprox_con_handle));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "puba[5..2] = %02X %02x %02x %02x", TVF_BBBB(puba[5], puba[4], puba[3], puba[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "puba[1..0] = %02X %02x", TVF_BB(puba[1], puba[0]));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROX_HOSTINFO), (UINT8 *)&(proxAppState->bleprox_hostinfo));

    if (bleprofile_p_cfg->findme_locator_enable)
    {
        // if we are paired with the device we likely to know handle that should be used by Findme button
        UINT16 findme_hdl = 0;
        if (memcmp(bda, proxAppState->bleprox_hostinfo.bdAddr, 6) == 0)
            findme_hdl = proxAppState->bleprox_hostinfo.findme_hdl;

        blefind_FindmeSetHandle(findme_hdl);
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
    proxAppState->bleprox_linkloss_alert = NO_ALERT;
    bleprox_SetAlert(NO_ALERT);

    // This need to be called if battery notification is used.
    blebat_connUp();

    bleprofile_setidletimer_withNotification();

#if 0
    lesmp_sendSecurityRequest();
#endif
}

void bleprox_connDown(void)
{
    BLEPROFILE_DB_PDU   db_pdu;
    int                 host_info_present = FALSE;
    int                 result;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "connDown", TVF_D(0));

    proxAppState->bleprox_con_handle  = 0; //reset connection handle
    proxAppState->bleprox_isReconnect = 1;

    // go back to Adv.
    // should be select after read NVRAM data
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);	//for test only

#if 0
            //proxAppState->bleprox_linkloss_alert = MILD_ALERT;
            //bleprox_SetAlert(MILD_ALERT);

            proxAppState->bleprox_linkloss_alert = HIGH_ALERT;
            bleprox_SetAlert(HIGH_ALERT);
#endif

#if 0
    //When Button is pressed and set to Undirected ADV, this function will call only once.
    //Should not be called this function from brcmPROXReconnect for button pressure.
    brcmPROXselectDiscoverbyNVRAM(HIGH_DIRECTED_DISCOVERABLE);
#endif

    host_info_present = bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROX_HOSTINFO), (UINT8 *)&(proxAppState->bleprox_hostinfo));

    // if we disconnected from peer device get findme handle to save in the NVRAM
    if (host_info_present && bleprofile_p_cfg->findme_locator_enable)
    {
        UINT16 findme_hdl = blefind_FindmeGetHandle();
        if ((memcmp(proxAppState->bleprox_remote_addr, proxAppState->bleprox_hostinfo.bdAddr, 6) == 0) &&
            (findme_hdl != proxAppState->bleprox_hostinfo.findme_hdl))
        {
            UINT8 writtenbyte;

            proxAppState->bleprox_hostinfo.findme_hdl = findme_hdl;

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(proxAppState->bleprox_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(result));
        }
    }

    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else if (host_info_present)
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, proxAppState->bleprox_hostinfo.bdAddr);
        //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, proxAppState->bleprox_hostinfo.bdAddr);

        ble_tracen((char *)(proxAppState->bleprox_hostinfo.bdAddr), sizeof(BD_ADDR));
    }
    else
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }

    result = bleprofile_ReadHandle(proxAppState->bleprox_ll_alt_hdl, &db_pdu);

    if (!result)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "LL_Alert_Level:%x", TVF_D(db_pdu.pdu[0]));

        if (db_pdu.pdu[0] == HIGH_ALERT) //high
        {
            proxAppState->bleprox_linkloss_alert = HIGH_ALERT;
            bleprox_SetAlert(HIGH_ALERT);
        }
        else if (db_pdu.pdu[0] == MILD_ALERT) //low
        {
            proxAppState->bleprox_linkloss_alert = MILD_ALERT;
            bleprox_SetAlert(MILD_ALERT);
        }
    }

    // This need to be called if battery notification is used.
    blebat_connDown();
}

void bleprox_advStop(void)
{
    bleprox_SetAlert(NO_ALERT);
}

void bleprox_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (proxAppState->bleprox_apptimer_count)++;

                bleprox_Timeout(proxAppState->bleprox_apptimer_count);
#if 0
                 //this is for test only
                bleprofile_LEDBlink(bleprofile_p_cfg->led_on_ms, bleprofile_p_cfg->led_off_ms, proxAppState->bleprox_apptimer_count&0x03);
                bleprofile_BUZBeep((proxAppState->bleprox_apptimer_count&0x03)*bleprofile_p_cfg->buz_on_ms);
                bleprofile_WPEnable(proxAppState->bleprox_apptimer_count&0x01);
#endif
            }
            break;

    }
}

void bleprox_appFineTimerCb(UINT32 arg)
{
    (proxAppState->bleprox_appfinetimer_count)++;

    bleprox_FineTimeout(proxAppState->bleprox_appfinetimer_count);
}

void bleprox_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bleprox, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
	 // saving bd_addr in nvram
        UINT8 *bda;
        UINT8 writtenbyte;

        bda = (UINT8 *)emconninfo_getPeerPubAddr();

        memcpy(proxAppState->bleprox_hostinfo.bdAddr, bda, sizeof(BD_ADDR));

        // Let FindMe know that handle is not valid anymore
        proxAppState->bleprox_hostinfo.findme_hdl = 0;
        blefind_FindmeSetHandle(0);

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(proxAppState->bleprox_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(result));
    }

    // This need to be called if battery notification is used.
    blebat_smpBondResult(result);
}

void bleprox_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Encryption Changed", TVF_D(0));
}

int bleprox_notification_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    bleprofile_setidletimer_withNotification();
    return 0;
}

