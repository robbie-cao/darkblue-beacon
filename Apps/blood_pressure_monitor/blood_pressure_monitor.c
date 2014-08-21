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
* BLE Blood Pressure profile, service, application 
*
* These are updates to the ROM code for BLE Blood Pressure Monitor device.
*
* Refer to Bluetooth SIG Blood Pressure Profile 1.0 and Blood Pressure Service
* 1.0 specifications for details.
*
* The ROM code is published in Wiced-Smart\bleapp\app\blebpm.c file.  This
* code replaces GATT database, Fine-timer Callback function, and Fake UART function.
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
#include "blebpm.h"
#include "spar_utils.h"

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
static void   blood_pressure_monitor_create(void);
static void   blood_pressure_monitor_FakeUART(char *bpm_char, UINT32 count);
static void   blood_pressure_monitor_FakeIBP(UINT32 count);
static void   blood_pressure_monitor_FineTimeout(UINT32 finecount);
static void   blood_pressure_monitor_handleUART(char *bpm_char);
static void   blood_pressure_monitor_appFineTimerCb(UINT32 arg);

extern void   blebpm_Create(void);
extern void   blebpm_connDown(void);
extern void   blebpm_appTimerCb(UINT32 arg);
extern void   blebpm_IndicationConf(void);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////
const UINT8 blood_pressure_monitor_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service Changed, handle 0x03 characteristic value
    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
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
                           LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 11),
        0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_INTERMEDIATE_BLOOD_PRESSURE,
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 11),
        0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

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

const BLE_PROFILE_CFG blood_pressure_monitor_cfg =
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
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 5,    // second  0-> no timeout
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

extern tBpmAppState *bpmAppState;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)blood_pressure_monitor_db_data,
                   sizeof(blood_pressure_monitor_db_data),
                   (void *)&blood_pressure_monitor_cfg,
                   (void *)&blebpm_puart_cfg,
                   (void *)&blebpm_gpio_cfg,
                   blood_pressure_monitor_create);
}

void blood_pressure_monitor_create(void)
{
    blebpm_Create();
    bleprofile_KillTimer();

    bleprofile_regTimerCb(blood_pressure_monitor_appFineTimerCb, blebpm_appTimerCb);
    bleprofile_StartTimer();
}

void blood_pressure_monitor_FakeUART(char *bpm_char, UINT32 count)
{
    //This is for test only
    if ((count % 4 == 0) && bpmAppState->blebpm_indication_enable && (bpm_char[0] == 0))
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
        blood_pressure_monitor_FakeIBP(count);
    }
}

void blood_pressure_monitor_FakeIBP(UINT32 count)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    UINT16            bpm_temp;
    int               i = 0;

    //bpm_sys_dia, bpm_dia, bpm_pulse, bpm_mean;
    bpmAppState->blebpm_bpm_data.flag = BPM_PULSE_RATE | BPM_MEASUREMENT_STATUS;
    bpm_temp = (UINT16)(count % 30) * 120 / 30;
    bpmAppState->blebpm_bpm_data.systolic = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30) * 80 / 30;
    bpmAppState->blebpm_bpm_data.diastolic = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30);
    bpmAppState->blebpm_bpm_data.pulserate = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpm_temp = (UINT16)(count % 30) * 120 / 30 / 3 + (count % 30) * 80 / 30 * 2 / 3;
    bpmAppState->blebpm_bpm_data.mat = bleprofile_UINT16toSFLOAT(bpm_temp);
    bpmAppState->blebpm_bpm_data.measurementstatus = 0x0000;

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

void blood_pressure_monitor_FineTimeout(UINT32 finecount)
{
    char bpm_char[READ_UART_LEN + 1];
    memset(bpm_char, 0x0, READ_UART_LEN+1);
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
    	blood_pressure_monitor_FakeUART(bpm_char, finecount);
    }
#endif

    if (bpm_char[0] == 'D' && bpm_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        ble_trace0("CSA Filter Disable");
    }
    else if (bpm_char[0] == 'A' && bpm_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        ble_trace0("CSA Filter Enable");

        blebpm_connDown();
    }
    else  //bpm main reading
    {
        blood_pressure_monitor_handleUART(bpm_char);
    }

    // button control
    bleprofile_ReadButton();
}

void blood_pressure_monitor_handleUART(char *bpm_char)
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
            bpmAppState->blebpm_bpm_data.flag = BPM_PULSE_RATE | BPM_MEASUREMENT_STATUS;
            bpm_temp = (UINT16)((bpm_char[3] << 4) + bpm_char[4] + (bpm_char[5] << 4) + bpm_char[6]);
            bpmAppState->blebpm_bpm_data.systolic = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[5] << 4) + bpm_char[6]);
            bpmAppState->blebpm_bpm_data.diastolic = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[7] << 4) + bpm_char[8]);
            bpmAppState->blebpm_bpm_data.pulserate = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpm_temp = (UINT16)((bpm_char[9] << 4) + bpm_char[10]);
            bpmAppState->blebpm_bpm_data.mat = bleprofile_UINT16toSFLOAT(bpm_temp);
            bpmAppState->blebpm_bpm_data.measurementstatus = 0x0000;

            bpmAppState->blebpm_measurement_done = 1; //New measurement is done
        }
    }





    // change EEPROM and GATT DB
    if (bpmAppState->blebpm_measurement_done == 1) //if connected and encrpted, old data is sent
    {
        //write whole in EEPROM
        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BPM_DATA, sizeof(BLEBPM_BPM_DATA), (UINT8 *)&(bpmAppState->blebpm_bpm_data));
        ble_trace1("NVRAM BPM write:%04x", writtenbyte);

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
                    ble_trace0("NoConfBlkInd");
                }
            }
        }

        bpmAppState->blebpm_indication_defer = 0;
    }
}

void blood_pressure_monitor_appFineTimerCb(UINT32 arg)
{
    (bpmAppState->blebpm_appfinetimer_count)++;

    blood_pressure_monitor_FineTimeout(bpmAppState->blebpm_appfinetimer_count);
}
