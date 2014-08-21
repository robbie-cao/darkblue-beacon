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
* BLE Heart Rate profile, service, application 
*
* These are updates to the ROM code for BLE Heart Rate Monitor device.
*
* Refer to Bluetooth SIG Heart Rate Profile 1.0 and Heart Rate Service
* 1.0 specifications for details.
*
* The ROM code is published in Wiced-Smart\bleapp\app\blehrm.c file.  This
* code replaces GATT database, configuration and Write Handler function.
*
* Features demonstrated
*  - Heart rate monitor implementation
*  - Replacing some of the ROM functionality
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "blehrm.h"
#include "spar_utils.h"

#define BLE_P2

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
static void   heart_rate_monitor_create(void);
static int    heart_rate_monitor_write_handler(LEGATTDB_ENTRY_HDR *p);
static void   heart_rate_monitor_appFineTimerCb(UINT32 arg);
static void   heart_rate_monitor_FineTimeout(UINT32 count);

extern void   blehrm_DBInit(void);
extern void   blehrm_connUp(void);
extern void   blehrm_connDown(void);
extern void   blehrm_advStop(void);
extern void   blehrm_appTimerCb(UINT32 arg);
extern void   blehrm_appFineTimerCb(UINT32 arg);
extern void   blehrm_smpBondResult(LESMP_PARING_RESULT  result);
extern void   blehrm_encryptionChanged(HCI_EVT_HDR *evt);
extern UINT32 blehrm_HrmButton(UINT32 function);



//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 heart_rate_monitor_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4), 
        0x00, 0x00, 0x00, 0x00,

    // GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),
    
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','H','R',' ','M','o','n','i','t','o','r',0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_HEART_RATE_SENSOR),

    // Heart Rate service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_HEART_RATE),

    CHARACTERISTIC_UUID16 (0x0029, 0x002a, UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT, 
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 4),
        0x08,0x00,0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_HEART_RATE_SENSOR_LOCATION, 
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x01,                       // Body Sensor Location Chest

    CHARACTERISTIC_UUID16_WRITABLE (0x0030, 0x0031, UUID_CHARACTERISTIC_HEART_RATE_CONTROL_POINT,
                                    LEGATTDB_CHAR_PROP_WRITE,  LEGATTDB_PERM_WRITE_REQ,  1),
        0x00, 

    // Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7,
};

const UINT16 heart_rate_monitor_db_size = sizeof(heart_rate_monitor_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG heart_rate_monitor_cfg =
{
    /*.fine_timer_interval            =*/ 100,        //ms
    /*.default_adv                    =*/ 4,           // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,           // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,          // slots
    /*.low_undirect_adv_interval      =*/ 2048,        // slots
    /*.high_undirect_adv_duration     =*/ 300,          // seconds
    /*.low_undirect_adv_duration      =*/ 300,         // seconds
    /*.high_direct_adv_interval       =*/ 0,           // seconds
    /*.low_direct_adv_interval        =*/ 0,           // seconds
    /*.high_direct_adv_duration       =*/ 0,           // seconds
    /*.low_direct_adv_duration        =*/ 0,           // seconds
    /*.local_name                     =*/ "BLE HR Monitor",        // [LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_HEART_RATE_SENSOR), 0, // [COD_LEN];
    /*.ver                            =*/ "1.00",      //char ver[VERSION_LEN];
    /*.encr_required                  =*/ 0,           // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,           // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,           // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04,        // dbm
    /*.con_idle_timeout               =*/ 30,          // second   0-> no timeout
    /*.powersave_timeout              =*/ 5,           // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002f, 0x0031, 0x00, 0x00},                        // [HANDLE_NUM_MAX] GATT HANDLE number
    /*.serv                           =*/ {UUID_SERVICE_HEART_RATE, UUID_SERVICE_HEART_RATE, UUID_SERVICE_HEART_RATE, 0x00, 0x00}, //GATT service UUID
    /*.cha                            =*/ {UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT, UUID_CHARACTERISTIC_HEART_RATE_SENSOR_LOCATION,
                                            UUID_CHARACTERISTIC_HEART_RATE_CONTROL_POINT, 0x00, 0x00},  // GATT characteristic UUID
    /*.findme_locator_enable          =*/ 0,           // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,           // alert level of find me
    /*.client_grouptype_enable        =*/ 0,           // if 1 grouptype read can be used
    /*.linkloss_button_enable         =*/ 0,           // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 0,           // second
    /*.alert_interval                 =*/ 0,           // interval of alert
    /*.high_alert_num                 =*/ 0,           // number of alert for each interval
    /*.mild_alert_num                 =*/ 0,           // number of alert for each interval
    /*.status_led_enable              =*/ 0,           // if 1 status LED is enable
    /*.status_led_interval            =*/ 0,           // second
    /*.status_led_con_blink           =*/ 0,           // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 0,           // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 0,           // blink num of undir adv
    /*.led_on_ms                      =*/ 0,           // led blink on duration in ms
    /*.led_off_ms                     =*/ 0,           // led blink off duration in ms
    /*.buz_on_ms                      =*/ 0,           // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,           // seconds
    /*.button_client_timeout          =*/ 0,           // seconds
    /*.button_discover_timeout        =*/ 1,           // seconds
    /*.button_filter_timeout          =*/ 10,          // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,          // seconds
#endif
};

typedef struct
{
    //NVRAM save area
    BLEPROFILE_HOSTINFO blehrm_hostinfo;

    BLEHRM_HRM_DATA     blehrm_hrm_data;

    UINT32              blehrm_apptimer_count;
    UINT32              blehrm_appfinetimer_count;
    UINT16              blehrm_con_handle;
    BD_ADDR             blehrm_remote_addr;

    INT32               blehrm_ee_offset;
    UINT16              blehrm_hrm_hdl;
    UINT16              blehrm_hrm_client_hdl;
    UINT16              blehrm_hrm_cp_hdl;
    UINT16              blehrm_hrm_bsl_hdl;
    UINT8               blehrm_bat_enable;
    UINT8               blehrm_notification_enable;
    UINT8               blehrm_measurement_done;
} tHrmAppState;

extern tHrmAppState *hrmAppState;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////


// Application initialization.  UART and GPIO configuration are used from the ROM
extern const BLE_PROFILE_PUART_CFG blehrm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blehrm_gpio_cfg;

APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)heart_rate_monitor_db_data,
                   sizeof(heart_rate_monitor_db_data),
                   (void *)&heart_rate_monitor_cfg,
                   (void *)&blehrm_puart_cfg,
                   (void *)&blehrm_gpio_cfg,
                   heart_rate_monitor_create);
}

// RAM Create function.  We do not call ROM blehrm_Create function because
// it will register its own write handler, which we need to avoid.
void heart_rate_monitor_create(void)
{
    ble_trace0("heart_rate_monitor_create");

    hrmAppState = (tHrmAppState *)cfa_mm_Sbrk(sizeof(tHrmAppState));
    memset(hrmAppState, 0x00, sizeof(tHrmAppState));

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blehrm_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blehrm_connUp);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blehrm_connDown);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blehrm_advStop);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blehrm_encryptionChanged);

    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blehrm_smpBondResult);

    // write handler
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)heart_rate_monitor_write_handler);

    bleprofile_regButtonFunctionCb(blehrm_HrmButton);

    //data init
    memset(&(hrmAppState->blehrm_hrm_data), 0x00, sizeof(BLEHRM_HRM_DATA));
    hrmAppState->blehrm_hrm_data.flag = HRM_ENERGY_EXPENDED_STATUS;

    bleprofile_regTimerCb(heart_rate_monitor_appFineTimerCb, blehrm_appTimerCb);
    bleprofile_StartTimer();

    blehrm_connDown();
}


// this callback is issued after peer device wrote something into the GATT database
int heart_rate_monitor_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT16  handle      = legattdb_getHandle(p);
    int     len         = legattdb_getAttrValueLen(p);
    UINT8   *attrPtr    = legattdb_getAttrValue(p);

    ble_trace3("WriteCb: handle %04x client_hdl:%04x cp_hdl:%04x", handle, hrmAppState->blehrm_hrm_client_hdl, hrmAppState->blehrm_hrm_cp_hdl);

    if (hrmAppState->blehrm_hrm_client_hdl && handle == hrmAppState->blehrm_hrm_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(hrmAppState->blehrm_hrm_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(hrmAppState->blehrm_remote_addr, hrmAppState->blehrm_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            hrmAppState->blehrm_hostinfo.serv         = UUID_SERVICE_HEART_RATE;
            hrmAppState->blehrm_hostinfo.cha          = UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT;
            hrmAppState->blehrm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

            ble_trace1("NVRAMWr:%04x", writtenbyte);
        }
    }
    else if (hrmAppState->blehrm_hrm_cp_hdl && handle == hrmAppState->blehrm_hrm_cp_hdl)
    {
        BLEHRM_CP_HDR       *cpHdr = (BLEHRM_CP_HDR *) attrPtr ;

        if (cpHdr->opcode == HRM_CP_RESET_ENERGY_EXPENDED)
        {
            // reset variable
            // just keeping the offset value here
            // It should send any message to real sensor and reset it
            hrmAppState->blehrm_ee_offset = hrmAppState->blehrm_hrm_data.ee;
            ble_trace1("blehrm_ee_offset=%d", hrmAppState->blehrm_ee_offset);
        }
        else
        {
            return 0x80;
        }
    }

    return 0;
}

void heart_rate_monitor_appFineTimerCb(UINT32 arg)
{
    (hrmAppState->blehrm_appfinetimer_count)++;

    heart_rate_monitor_FineTimeout(hrmAppState->blehrm_appfinetimer_count);
}

void heart_rate_monitor_FineTimeout(UINT32 count)
{
    char hrm_char[READ_UART_LEN + 1];
    memset(hrm_char, 0x0, READ_UART_LEN+1);
    //ble_trace1("Finetimer:%d", finecount);

    //Reading
    bleprofile_ReadUART(hrm_char);

    //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
    //      hrm_char[0], hrm_char[1], hrm_char[2], hrm_char[3], hrm_char[4], hrm_char[5]);

#if 1
    if (bleprofile_p_cfg->test_enable)
    {
        //This is making faking data
        //For test only
        blehrm_FakeUART(hrm_char, count);
    }
#endif

    if (hrm_char[0] == 'D' && hrm_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        ble_trace0("CSA Filter Dis");
    }
    else if (hrm_char[0] == 'A' && hrm_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        ble_trace0("CSA Filter En");

        blehrm_connDown();
    }
    else  //hrm main reading
    {
        blehrm_handleUART(hrm_char);
    }

    // button control
    bleprofile_ReadButton();
}
