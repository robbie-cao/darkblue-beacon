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
* BLE Blood Pressure profile, service, application 
*
* This is a ROM code for BLE Blood Pressure Monitor device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  See blebpmspar.c for example of 
* customisation.
*
* Refer to Bluetooth SIG Blood Pressure Profile 1.0 and Blood Pressure Service
* 1.0 specifications for details.
*
* During initialization the app registers with LE stack to receive various
* notifications including bonding complete, connection status change and
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
*  - NVRAM read/write operation
*  - Sending data to the client
*  - Processing write requests from the client
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "blebpm.h"
#include "dbfw_app.h"

#define BLE_P2
#define BLE_MOD

#define FID   FID_BLEAPP_APP__BLEBPM_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blebpm_Timeout(UINT32 count);
void blebpm_FineTimeout(UINT32 finecount);
void blebpm_handleUART(char *bpm_char);
void blebpm_DBInit(void);
void blebpm_connUp(void);
void blebpm_connDown(void);
void blebpm_advStop(void);
void blebpm_appTimerCb(UINT32 arg);
void blebpm_appFineTimerCb(UINT32 arg);
void blebpm_smpBondResult(LESMP_PARING_RESULT result);
void blebpm_encryptionChanged(HCI_EVT_HDR *evt);
void blebpm_IndicationConf(void);
void blebpm_transactionTimeout(void);
void blebpm_disconnect(int reason);

// This function is for BPM test
void blebpm_FakeUART(char *bpm_char, UINT32 count);
int  blebpm_writeCb(LEGATTDB_ENTRY_HDR *p);

void blebpm_FakeIBP(UINT32 count);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blebpm_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service Changed, handle 0x03 characteristic value
    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 4), 
        0x00, 0x00, 0x00, 0x00,

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),
    
    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','B','P',' ','M','o','n','i','t','o','r',0,0,    // "BLE BP Monitor"

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_BLOOD_PRESSURE),

    // Handle 0x28: Blood Pressure Service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_BLOOD_PRESSURE),

    // Handle 0x29: characteristic Blood Pressure Measurement, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16 (0x0029, 0x002a, UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT, 
                           LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 9),
        0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE, 
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 9),
        0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002e, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x2f: characteristic Blood Pressure Feature, handle 0x30 characteristic value
    CHARACTERISTIC_UUID16 (0x002f, 0x0030, UUID_CHARACTERISTIC_BLOOD_PRESSURE_FEATURE, 
        LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00,0x00,

    // Handle 0x3d: Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x3e: characteristic Manufacturer Name, handle 0x3f characteristic value
    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x40: characteristic Model Number, handle 0x41 characteristic value
    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    // Handle 0x42: characteristic System ID, handle 0x43 characteristic value
    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blebpm_db_size = sizeof(blebpm_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blebpm_cfg =
{
    /*.fine_timer_interval            =*/ 1000, //ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "BLE BP Monitor", // [LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_BLOOD_PRESSURE), 0,   // [COD_LEN] first 2 bytes are appearance
    /*.ver                            =*/ "1.00",           // [VERSION_LEN];
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
#ifdef BLE_MOD
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
#else
    /*.test_enable                    =*/ 0,    // TEST MODE is enabled when 1
#endif
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 20,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x0030, 0x00, 0x00}, // [HANDLE_NUM_MAX];   //GATT HANDLE number
    /*.serv                           =*/ {UUID_SERVICE_BLOOD_PRESSURE, UUID_SERVICE_BLOOD_PRESSURE, UUID_SERVICE_BLOOD_PRESSURE, 0x00, 0x00},
    /*.cha                            =*/ {UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT, UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE,
                                            UUID_CHARACTERISTIC_BLOOD_PRESSURE_FEATURE, 0x00, 0x00},
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 0,    // if 1 grouptype read can be used
    /*.linkloss_button_enable         =*/ 0,    // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 0,    // second
    /*.alert_interval                 =*/ 0,    // interval of alert
    /*.high_alert_num                 =*/ 0,    // number of alert for each interval
    /*.mild_alert_num                 =*/ 0,    // number of alert for each interval
    /*.status_led_enable              =*/ 0,    // if 1 status LED is enable
    /*.status_led_interval            =*/ 0,    // second
    /*.status_led_con_blink           =*/ 0,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 0,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 0,    // blink num of undir adv
    /*.led_on_ms                      =*/ 0,    // led blink on duration in ms
    /*.led_off_ms                     =*/ 0,    // led blink off duration in ms
    /*.buz_on_ms                      =*/ 0,    // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,    // seconds
    /*.button_client_timeout          =*/ 0,    // seconds
    /*.button_discover_timeout        =*/ 0,    // seconds
    /*.button_filter_timeout          =*/ 0,    // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blebpm_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ 32,       // GPIO pin number 20730A0 module need to use 32 instead
    /*.rxpin      =*/ 33,       // GPIO pin number
};

#ifdef BLE_P2
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blebpm_gpio_cfg =
{
    /*.gpio_pin =*/ 
    {
        1,                                                          // pin number of gpio
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    },
    /*.gpio_flag =*/ 
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, 
};
#else
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blebpm_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        31, 
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }, 
    /*.gpio_flag =*/
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, 
};
#endif

typedef struct
{
    //NVRAM save area
    BLEBPM_HOSTINFO blebpm_hostinfo;

    BLEBPM_BPM_DATA blebpm_bpm_data;

    UINT32          blebpm_apptimer_count;
    UINT32          blebpm_appfinetimer_count;
    UINT16          blebpm_con_handle;
    BD_ADDR         blebpm_remote_addr;

    UINT16          blebpm_bpm_hdl;
    UINT16          blebpm_bpm_client_hdl;
    UINT16          blebpm_ibp_hdl;
    UINT16          blebpm_ibp_client_hdl;
    UINT16          blebpm_bpf_hdl;

    UINT8           blebpm_bat_enable;
    UINT8           blebpm_indication_enable;
    UINT8           blebpm_indication_defer;
    UINT8           blebpm_indication_sent;
    UINT8           blebpm_measurement_done;
} tBpmAppState;

tBpmAppState *bpmAppState = NULL;

void blebpm_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0], bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[2], bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    bpmAppState = (tBpmAppState *)cfa_mm_Sbrk(sizeof(tBpmAppState));
    memset(bpmAppState, 0x00, sizeof(tBpmAppState));

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blebpm_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blebpm_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blebpm_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blebpm_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blebpm_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blebpm_smpBondResult);
#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blebpm_IndicationConf);
#endif
    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blebpm_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blebpm_writeCb);

    //data init
    memset(&(bpmAppState->blebpm_bpm_data), 0x00, sizeof(BLEBPM_BPM_DATA));
    bpmAppState->blebpm_bpm_data.flag = BPM_PULSE_RATE;

    bleprofile_regTimerCb(blebpm_appFineTimerCb, blebpm_appTimerCb);
    bleprofile_StartTimer();

    blebpm_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void blebpm_FakeUART(char *bpm_char, UINT32 count)
{
    //This is for test only
    if ((count % 30 == 0) && bpmAppState->blebpm_indication_enable && (bpm_char[0] == 0))
    {
        //faking data measurement
        bpm_char[0] = 'U';
        bpm_char[1] = '8';
        bpm_char[2] = '0';
        bpm_char[3] = '2';
        bpm_char[4] = '8';
        bpm_char[5] = '5';
        bpm_char[6] = '0';
        bpm_char[7] = (count & 0xF0) >> 4;
        if (bpm_char[7] < 0x0A)
        {
            bpm_char[7] += '0';
        }
        else
        {
            bpm_char[7] += ('A' - 10);
        }
        bpm_char[8] = count & 0x0F;
        if (bpm_char[8] < 0x0A)
        {
            bpm_char[8] += '0';
        }
        else
        {
            bpm_char[8] += ('A' - 10);
        }
        bpm_char[9]  = '6';
        bpm_char[10] = '4';
    }
    else if (bpmAppState->blebpm_indication_enable && bpm_char[0] == 0) //this one is used only intermediate measurement
    {
        blebpm_FakeIBP(count);
    }
}

void blebpm_FakeIBP(UINT32 count)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    UINT16            bpm_temp;
    int               i = 0;

    //bpm_sys_dia, bpm_dia, bpm_pulse, bpm_mean;
    bpmAppState->blebpm_bpm_data.flag = BPM_PULSE_RATE;
    bpm_temp = (UINT16)(count % 30) * 120 / 30;
    bpmAppState->blebpm_bpm_data.systolic = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30) * 80 / 30;
    bpmAppState->blebpm_bpm_data.diastolic = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30);
    bpmAppState->blebpm_bpm_data.pulserate = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30) * 120 / 30 / 3 + (count % 30) * 80 / 30 * 2 / 3;
    bpmAppState->blebpm_bpm_data.mat = bleprofile_UINT16toSFLOAT(bpm_temp);

    //write partial based on flag
    // FLAG(1), MEASUREMENT(6) - 7 bytes are mandatory
    memcpy(db_pdu.pdu, &(bpmAppState->blebpm_bpm_data), 7);
    i = 7;

    //Optional item
    if (bpmAppState->blebpm_bpm_data.flag & BPM_TIME_STAMP)
    {
        memcpy(&(db_pdu.pdu[i]), bpmAppState->blebpm_bpm_data.timestamp, 7);
        i += 7;
    }
    if (bpmAppState->blebpm_bpm_data.flag & BPM_PULSE_RATE)
    {
        memcpy(&(db_pdu.pdu[i]), (void*)&(bpmAppState->blebpm_bpm_data.pulserate), 2);
        i += 2;
    }
    if (bpmAppState->blebpm_bpm_data.flag & BPM_USER_ID)
    {
        memcpy(&(db_pdu.pdu[i]), &(bpmAppState->blebpm_bpm_data.userid), 1);
        i += 1;
    }
    if (bpmAppState->blebpm_bpm_data.flag & BPM_MEASUREMENT_STATUS)
    {
        memcpy(&(db_pdu.pdu[i]), (void*)&(bpmAppState->blebpm_bpm_data.measurementstatus), 2);
        i += 2;
    }

    db_pdu.len = i;
    bleprofile_WriteHandle(bpmAppState->blebpm_ibp_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    //check client char cfg
    bleprofile_ReadHandle(bpmAppState->blebpm_ibp_client_hdl, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

    // this is the first indication
    if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
    {
        bleprofile_sendNotification( bpmAppState->blebpm_ibp_hdl, (UINT8 *)db_pdu.pdu, db_pdu.len);
    }
}

void blebpm_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);

    if (bpmAppState->blebpm_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}

void blebpm_FineTimeout(UINT32 finecount)
{
    char bpm_char[READ_UART_LEN + 1];
    //ble_trace1("Finetimer:%d", finecount);

    //Reading
    bleprofile_ReadUART(bpm_char);
    //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
    //      bpm_char[0], bpm_char[1], bpm_char[2], bpm_char[3], bpm_char[4], bpm_char[5]);

#if 1
    if (bleprofile_p_cfg->test_enable)
    {
        //This is making faking data
        //For test only
        blebpm_FakeUART(bpm_char, finecount);
    }
#endif

    if (bpm_char[0] == 'D' && bpm_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Disable", TVF_D(0));
    }
    else if (bpm_char[0] == 'A' && bpm_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Enable", TVF_D(0));

        blebpm_connDown();
    }
    else  //bpm main reading
    {
        blebpm_handleUART(bpm_char);
    }

    // button control
    bleprofile_ReadButton();
}

void blebpm_handleUART(char *bpm_char)
{
    BLEPROFILE_DB_PDU   db_pdu, db_cl_pdu;
    UINT16              bpm_temp;
    int                 i;
    UINT8               writtenbyte;
    
    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        bpmAppState->blebpm_measurement_done = bleprofile_handleUARTCb((UINT8 *)bpm_char, (UINT8 *)&(bpmAppState->blebpm_bpm_data));
    }
    else
    {
        if (bpm_char[0] == 'U' && bpm_char[1] == '8' && bpm_char[2] == '0')
        {
            //change ascii to hex
            for (i = 3; i <= 10; i++)
            {
                if ((bpm_char[i] >= '0') && (bpm_char[i] <= '9'))
                {
                    bpm_char[i] -= '0';
                }
                else if ((bpm_char[i] >= 'A') && (bpm_char[i] <= 'F'))
                {
                    bpm_char[i] = (bpm_char[i] -'A' + 10);
                }
                else if ((bpm_char[i] >= 'a') && (bpm_char[i] <= 'f'))
                {
                    bpm_char[i] = (bpm_char[i] -'a' + 10);
                }
                else
                {
                    bpm_char[i] = 0; //error case
                }
            }

            //bpm_sys_dia, bpm_dia, bpm_pulse, bpm_mean;
            bpmAppState->blebpm_bpm_data.flag = BPM_PULSE_RATE;
            bpm_temp = (UINT16)((bpm_char[3] << 4) + bpm_char[4] + (bpm_char[5] << 4) + bpm_char[6]);
            bpmAppState->blebpm_bpm_data.systolic = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[5] << 4) + bpm_char[6]);
            bpmAppState->blebpm_bpm_data.diastolic = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[7] << 4) + bpm_char[8]);
            bpmAppState->blebpm_bpm_data.pulserate = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[9] << 4) + bpm_char[10]);
            bpmAppState->blebpm_bpm_data.mat = bleprofile_UINT16toSFLOAT(bpm_temp);

            bpmAppState->blebpm_measurement_done = 1; //New measurement is done
        }
    }





    // change EEPROM and GATT DB
    if (bpmAppState->blebpm_measurement_done == 1) //if connected and encrpted, old data is sent
    {
        //write whole in EEPROM
        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BPM_DATA, sizeof(BLEBPM_BPM_DATA), (UINT8 *)&(bpmAppState->blebpm_bpm_data));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM BPM write:%04x", TVF_D(writtenbyte));

        bpmAppState->blebpm_measurement_done = 2; //this will prevent multiple writing
        //bpmAppState->blebpm_measurement_done = 0; //this will permit multiple measurement
        bpmAppState->blebpm_indication_defer = 1;
    }


    //handling deferred indication
    if (bpmAppState->blebpm_indication_enable && bpmAppState->blebpm_indication_defer)
    {
        //load BPM data from NVRAM and send
        if (bleprofile_ReadNVRAM(VS_BLE_BPM_DATA, sizeof(BLEBPM_BPM_DATA), (UINT8 *)&(bpmAppState->blebpm_bpm_data)))
        {
            int i = 0;

            //write partial based on flag
            // FLAG(1), MEASUREMENT(6) - 7 bytes are mandatory
            memcpy(db_pdu.pdu, &(bpmAppState->blebpm_bpm_data), 7);
            i = 7;

            //Optional item
            if (bpmAppState->blebpm_bpm_data.flag & BPM_TIME_STAMP)
            {
                memcpy(&(db_pdu.pdu[i]), bpmAppState->blebpm_bpm_data.timestamp, 7);
                i += 7;
            }
            if (bpmAppState->blebpm_bpm_data.flag & BPM_PULSE_RATE)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(bpmAppState->blebpm_bpm_data.pulserate), 2);
                i += 2;
            }
            if (bpmAppState->blebpm_bpm_data.flag & BPM_USER_ID)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(bpmAppState->blebpm_bpm_data.userid), 1);
                i += 1;
            }
            if (bpmAppState->blebpm_bpm_data.flag & BPM_MEASUREMENT_STATUS)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(bpmAppState->blebpm_bpm_data.measurementstatus), 2);
                i += 2;
            }

            db_pdu.len = i;
            bleprofile_WriteHandle(bpmAppState->blebpm_bpm_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            //check client char cfg
            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            // this is the first indication
            if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
            {
                if (bpmAppState->blebpm_indication_sent == 0)
                {
                    bleprofile_sendIndication( bpmAppState->blebpm_bpm_hdl,
                            (UINT8 *)db_pdu.pdu, db_pdu.len, blebpm_IndicationConf);
                    bpmAppState->blebpm_indication_sent = 1;
                }
                else
                {
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NoConfBlkInd", TVF_D(0));
                }
            }
        }

        bpmAppState->blebpm_indication_defer = 0;
    }
}

void blebpm_DBInit(void)
{
    BLEPROFILE_DB_PDU   db_pdu;
    int                 i;

    //load handle number
    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BLOOD_PRESSURE &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT)
        {
            bpmAppState->blebpm_bpm_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_bpm_hdl:%04x", TVF_D(bpmAppState->blebpm_bpm_hdl));
            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bpmAppState->blebpm_bpm_client_hdl = legattdb_findCharacteristicDescriptor(
                bpmAppState->blebpm_bpm_hdl, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_bpm_client_hdl:%04x", TVF_D(bpmAppState->blebpm_bpm_client_hdl));
            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_client_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }

        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BLOOD_PRESSURE &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE)
        {
            bpmAppState->blebpm_ibp_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_ibp_hdl:%04x", TVF_D(bpmAppState->blebpm_ibp_hdl));
            bleprofile_ReadHandle(bpmAppState->blebpm_ibp_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bpmAppState->blebpm_ibp_client_hdl = legattdb_findCharacteristicDescriptor(
                bpmAppState->blebpm_ibp_hdl, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_ibp_client_hdl:%04x", TVF_D(bpmAppState->blebpm_ibp_client_hdl));
            bleprofile_ReadHandle(bpmAppState->blebpm_ibp_client_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BLOOD_PRESSURE &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BLOOD_PRESSURE_FEATURE)
        {
            bpmAppState->blebpm_bpf_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_bpf_hdl:%04x", TVF_D(bpmAppState->blebpm_bpf_hdl));
            bleprofile_ReadHandle(bpmAppState->blebpm_bpf_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            bpmAppState->blebpm_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(bpmAppState->blebpm_bpm_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif


}

void blebpm_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    bpmAppState->blebpm_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(bpmAppState->blebpm_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(bpmAppState->blebpm_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_connUp[5:2]: %02x %02x %02x %02x", TVF_BBBB(bpmAppState->blebpm_remote_addr[5], bpmAppState->blebpm_remote_addr[4], bpmAppState->blebpm_remote_addr[3], bpmAppState->blebpm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_connUp[1:0]: %02x %02x, handle: %04x", TVF_BBW(bpmAppState->blebpm_remote_addr[1], bpmAppState->blebpm_remote_addr[0], bpmAppState->blebpm_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

    // Client can set values for Client Configuration descriptor once during bonding.  On 
    // every successful connection we need to read value from the NVRAM and set descriptors
    // approprietely
    if (bpmAppState->blebpm_bpm_client_hdl != 0) 
    {
        // if we are connected to not bonded device descriptor is 0
        db_cl_pdu.len    = 2;
        db_cl_pdu.pdu[0] = 0x00;
        db_cl_pdu.pdu[1] = 0x00;

        if ((memcmp(bpmAppState->blebpm_remote_addr, bpmAppState->blebpm_hostinfo.bdAddr, 6) == 0) &&
            (bpmAppState->blebpm_hostinfo.serv[0] == UUID_SERVICE_BLOOD_PRESSURE) &&
            (bpmAppState->blebpm_hostinfo.cha[0]  == UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = bpmAppState->blebpm_hostinfo.cli_cha_desc[0] & 0xFF;
            db_cl_pdu.pdu[1] = bpmAppState->blebpm_hostinfo.cli_cha_desc[0] >> 8;
        }

        bleprofile_WriteHandle(bpmAppState->blebpm_bpm_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    // Do the same for the Intermediate Measurements descriptor
    if (bpmAppState->blebpm_ibp_client_hdl)
    {
        // if we are connected to not bonded device descriptor is 0
        db_cl_pdu.len    = 2;
        db_cl_pdu.pdu[0] = 0x00;
        db_cl_pdu.pdu[1] = 0x00;

        if ((memcmp(bpmAppState->blebpm_remote_addr, bpmAppState->blebpm_hostinfo.bdAddr, 6) == 0) &&
            (bpmAppState->blebpm_hostinfo.serv[1] == UUID_SERVICE_BLOOD_PRESSURE) &&
            (bpmAppState->blebpm_hostinfo.cha[1]  == UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE))
        {
            db_cl_pdu.pdu[0] = bpmAppState->blebpm_hostinfo.cli_cha_desc[1] & 0xFF;
            db_cl_pdu.pdu[1] = bpmAppState->blebpm_hostinfo.cli_cha_desc[1] >> 8;
        }

        bleprofile_WriteHandle(bpmAppState->blebpm_ibp_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    // Start Connection idle timer to disconnect if there is no activity
    bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);

    // If encryption is not required, we are ready to send indications
    if (bleprofile_p_cfg->encr_required == 0)
    {
        bpmAppState->blebpm_indication_enable = 1; //indication enable
        bpmAppState->blebpm_indication_defer  = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "indOn", TVF_D(0));
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blebpm_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_connDown[5:2]: %02x %02x %02x %02x", TVF_BBBB(bpmAppState->blebpm_remote_addr[5], bpmAppState->blebpm_remote_addr[4], bpmAppState->blebpm_remote_addr[3], bpmAppState->blebpm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_connDown[1:0]: %02x %02x, handle: %04x", TVF_BBW(bpmAppState->blebpm_remote_addr[1], bpmAppState->blebpm_remote_addr[0], bpmAppState->blebpm_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

    // Save client characteristic descriptor to NVRAM
    if (memcmp(bpmAppState->blebpm_remote_addr, bpmAppState->blebpm_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
        UINT8             writtenbyte;

        if (bpmAppState->blebpm_bpm_client_hdl)
        {
            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            bpmAppState->blebpm_hostinfo.serv[0]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[0]          = UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);
        }

        if (bpmAppState->blebpm_ibp_client_hdl)
        {
            bleprofile_ReadHandle(bpmAppState->blebpm_ibp_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            bpmAppState->blebpm_hostinfo.serv[1]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[1]          = UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }

    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, bpmAppState->blebpm_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2]: %02x %02x %02x %02x", TVF_BBBB(bpmAppState->blebpm_hostinfo.bdAddr[5], bpmAppState->blebpm_hostinfo.bdAddr[4], bpmAppState->blebpm_hostinfo.bdAddr[3], bpmAppState->blebpm_hostinfo.bdAddr[2] ));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x, handle: %04x", TVF_BBW(bpmAppState->blebpm_hostinfo.bdAddr[1], bpmAppState->blebpm_hostinfo.bdAddr[0], bpmAppState->blebpm_con_handle));
    }

    bpmAppState->blebpm_con_handle          = 0; // reset connection handle
    bpmAppState->blebpm_indication_enable   = 0; // notification enable
    bpmAppState->blebpm_indication_defer    = 0;
    bpmAppState->blebpm_indication_sent     = 0;
    bpmAppState->blebpm_measurement_done    = 0;
}


void blebpm_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADVStp", TVF_D(0));
}

void blebpm_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (bpmAppState->blebpm_apptimer_count)++;

                blebpm_Timeout(bpmAppState->blebpm_apptimer_count);
            }
            break;

    }
}

void blebpm_appFineTimerCb(UINT32 arg)
{
    (bpmAppState->blebpm_appfinetimer_count)++;

    blebpm_FineTimeout(bpmAppState->blebpm_appfinetimer_count);
}


void blebpm_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8             writtenbyte;
        BLEPROFILE_DB_PDU db_cl_pdu;

        memcpy(bpmAppState->blebpm_remote_addr, emconninfo_getPeerPubAddr(), sizeof(bpmAppState->blebpm_remote_addr));
        memcpy(bpmAppState->blebpm_hostinfo.bdAddr, bpmAppState->blebpm_remote_addr, sizeof(BD_ADDR));

        if (bpmAppState->blebpm_bpm_client_hdl)
        {
            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            bpmAppState->blebpm_hostinfo.serv[0]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[0]          = UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);
        }

        if (bpmAppState->blebpm_ibp_client_hdl)
        {
            bleprofile_ReadHandle(bpmAppState->blebpm_ibp_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            bpmAppState->blebpm_hostinfo.serv[1]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[1]          = UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blebpm_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "encChngd", TVF_D(0));

    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo)))
    {
        if (memcmp(bpmAppState->blebpm_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6) == 0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x", TVF_BBBB(bpmAppState->blebpm_hostinfo.bdAddr[5], bpmAppState->blebpm_hostinfo.bdAddr[4], bpmAppState->blebpm_hostinfo.bdAddr[3], bpmAppState->blebpm_hostinfo.bdAddr[2] ));
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x", TVF_BB(bpmAppState->blebpm_hostinfo.bdAddr[1], bpmAppState->blebpm_hostinfo.bdAddr[0] ));
        }
    }
    if (bleprofile_p_cfg->encr_required)
    {
        bpmAppState->blebpm_indication_enable = 1; //indication enable
        bpmAppState->blebpm_indication_defer  = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "indOn", TVF_D(0));
    }
}

void blebpm_disconnect(int reason)
{
    blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

    bpmAppState->blebpm_con_handle        = 0;
    bpmAppState->blebpm_indication_enable = 0;
    bpmAppState->blebpm_indication_defer  = 0;
    bpmAppState->blebpm_measurement_done  = 0;
}

void blebpm_IndicationConf(void)
{
    if (bpmAppState->blebpm_indication_sent)
    {
        bpmAppState->blebpm_indication_sent = 0;
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WrongConf", TVF_D(0));
        return;
    }

    if (bleprofile_p_cfg->disc_required & DISC_AFTER_CONFIRM)
    {
        if (bpmAppState->blebpm_con_handle && bpmAppState->blebpm_indication_enable && bpmAppState->blebpm_measurement_done)
        {
            // Encryption and data measurement is done
            blebpm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_IndicationConf disconnecting", TVF_D(0));
        }
    }
}

void blebpm_transactionTimeout(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ATT TO", TVF_D(0));

    if (bleprofile_p_cfg->disc_required & DISC_ATT_TIMEOUT)
    {
        blebpm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebpm_transactionTimeout disconnecting", TVF_D(0));
    }
}

int blebpm_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x", TVF_D(handle));

    if (bpmAppState->blebpm_bpm_client_hdl && handle == bpmAppState->blebpm_bpm_client_hdl)
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(bpmAppState->blebpm_remote_addr, bpmAppState->blebpm_hostinfo.bdAddr, 6) == 0)
        {
            BLEPROFILE_DB_PDU   db_cl_pdu;
            UINT8               writtenbyte;

            bleprofile_ReadHandle(bpmAppState->blebpm_bpm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            bpmAppState->blebpm_hostinfo.serv[0]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[0]          = UUID_CHARACTERISTIC_BLOOD_PRESSURE_MEASUREMENT;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (bpmAppState->blebpm_ibp_client_hdl && handle == bpmAppState->blebpm_ibp_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(bpmAppState->blebpm_ibp_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(bpmAppState->blebpm_remote_addr, bpmAppState->blebpm_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            bpmAppState->blebpm_hostinfo.serv[1]         = UUID_SERVICE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cha[1]          = UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE;
            bpmAppState->blebpm_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEBPM_HOSTINFO), (UINT8 *)&(bpmAppState->blebpm_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }

    return 0;
}
