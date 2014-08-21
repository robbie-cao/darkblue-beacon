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
* BLE Heart Rate profile, service, application 
*
* This is a ROM code for BLE Heart Rate Monitor device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  See blebpmspar.c for example of 
* customisation.
*
* Refer to Bluetooth SIG Heart Rate Profile 1.0 and Heart Rate Service
* 1.0 specifications for details.
*
* During initialization the app registers with LE stack to receive various
* notifications including bonding complete, connection status change and
* peer write.  When device is successfully bonded, application saves
* peer's Bluetooth Device address to the NVRAM.  Bonded device can also 
* write client configuration descriptor to register to receive HR 
* measurement and that is also save in the NVRAM.  When new 
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
#include "blehrm.h"
#include "dbfw_app.h"

#define BLE_P2

#define FID FID_BLEAPP_APP__BLEHRM_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void   blehrm_Timeout(UINT32 count);
void   blehrm_FineTimeout(UINT32 finecount);
void   blehrm_handleUART(char *hrm_char);
void   blehrm_DBInit(void);
void   blehrm_connUp(void);
void   blehrm_connDown(void);
void   blehrm_advStop(void);
void   blehrm_appTimerCb(UINT32 arg);
void   blehrm_appFineTimerCb(UINT32 arg);
void   blehrm_smpBondResult(LESMP_PARING_RESULT  result);
void   blehrm_encryptionChanged(HCI_EVT_HDR *evt);
void   blehrm_SendNotification(BLEHRM_HRM_DATA *p_blehrm_hrm_data);
UINT32 blehrm_HrmButton(UINT32 function);


//This function is for HRM test
void blehrm_FakeUART(char *hrm_char, UINT32 count);

UINT8 blehrm_DataParse(char *data, char *parse_data, UINT8 num);
int blehrm_writeCb(LEGATTDB_ENTRY_HDR *p);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blehrm_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 4), 
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
                                    LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,  
                                    LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ,  1),  
        0x00, 

    // Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blehrm_db_size = sizeof(blehrm_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blehrm_cfg =
{
    /*.fine_timer_interval            =*/ 1000,        //ms
    /*.default_adv                    =*/ 4,           // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,           // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,          // slots
    /*.low_undirect_adv_interval      =*/ 2048,        // slots
    /*.high_undirect_adv_duration     =*/ 30,          // seconds
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
    /*.con_idle_timeout               =*/ 20,          // second   0-> no timeout
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

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blehrm_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ 32,       // GPIO pin number 20730A0 module need to use 32 instead
    /*.rxpin      =*/ 33,       // GPIO pin number
};

#ifdef BLE_P2
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blehrm_gpio_cfg =
{
    /*.gpio_pin =*/ 
    {
        1, 
        0, 
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    },
    /*.gpio_flag =*/ 
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        GPIO_INPUT  | GPIO_INIT_LOW | GPIO_INT | GPIO_BUTTON,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
};
#else
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blehrm_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        31, 
        14, 
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    }, 
    /*.gpio_flag =*/
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        GPIO_INPUT  | GPIO_INIT_LOW | GPIO_INT | GPIO_BUTTON,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
};
#endif

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

tHrmAppState *hrmAppState = NULL;

void blehrm_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_Create Ver[5:2] = %02x %02x %02x %02x", TVF_BBBB(bleprofile_p_cfg->ver[5], bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[3], bleprofile_p_cfg->ver[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_Create Ver[1:0] = %02x %02x", TVF_BB(bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[0]));

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
    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blehrm_writeCb);

    bleprofile_regButtonFunctionCb(blehrm_HrmButton);

    //data init
    memset(&(hrmAppState->blehrm_hrm_data), 0x00, sizeof(BLEHRM_HRM_DATA));
    hrmAppState->blehrm_hrm_data.flag = HRM_ENERGY_EXPENDED_STATUS;

    bleprofile_regTimerCb(blehrm_appFineTimerCb, blehrm_appTimerCb);
    bleprofile_StartTimer();

    blehrm_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif
}


void blehrm_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);

    if (hrmAppState->blehrm_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}



void blehrm_FineTimeout(UINT32 count)
{
    char hrm_char[READ_UART_LEN + 1];
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
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Dis", TVF_D(0));
    }
    else if (hrm_char[0] == 'A' && hrm_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter En", TVF_D(0));

        blehrm_connDown();
    }
    else  //hrm main reading
    {
        blehrm_handleUART(hrm_char);
    }

    // button control
    bleprofile_ReadButton();

}

void blehrm_FakeUART(char *hrm_char, UINT32 count)
{
    UINT8 hrm;

    //This is for test only
    if (hrmAppState->blehrm_notification_enable == 1 && hrm_char[0] == 0)
    {
        //faking data measurement
        hrm = count&0xFF;

        if (hrm >= 100)
        {
            hrm_char[0] = '0' + (hrm / 100);
            hrm_char[1] = '0' + ((hrm % 100) / 10);
            hrm_char[2] = '0' + (hrm % 10);
            hrm_char[3] = ',';
            hrm_char[4] = '0';
            hrm_char[5] = ',';
            hrm_char[6] = '0';
            hrm_char[7] = 0x0a;
        }
        else if (hrm >= 10)
        {
            hrm_char[0] = '0' + ((hrm % 100) / 10);
            hrm_char[1] = '0' + (hrm % 10);
            hrm_char[2] = ',';
            hrm_char[3] = '0';
            hrm_char[4] = ',';
            hrm_char[5] = '0';
            hrm_char[6] = 0x0a;
        }
        else
        {
            hrm_char[0] = '0' + (hrm % 10);
            hrm_char[1] = ',';
            hrm_char[2] = '0';
            hrm_char[3] = ',';
            hrm_char[4] = '0';
            hrm_char[5] = 0x0a;
        }
    }

    //ble_trace1("hrm=%d", hrm);
    //ble_tracen(hrm_char, 7);
}


void blehrm_handleUART(char *hrm_char)
{
    int     i;
    UINT8   hrm_len;
    char    parse_char[4];
    UINT32  temp;
    
    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        hrmAppState->blehrm_measurement_done = bleprofile_handleUARTCb((UINT8 *)hrm_char, (UINT8 *)&(hrmAppState->blehrm_hrm_data));
    }
    else
    {
        //Parse HRM data
        hrm_len = blehrm_DataParse(hrm_char, parse_char, 0);

        if (hrm_len)
        {
            hrmAppState->blehrm_hrm_data.hrm = 0;
            //Writing DB with new value
            for (i = 0; i < hrm_len; i++)
            {
                hrmAppState->blehrm_hrm_data.hrm += (parse_char[i] - '0');

                if (i < hrm_len - 1)
                {
                    hrmAppState->blehrm_hrm_data.hrm *= 10;
                }
            }
            hrmAppState->blehrm_hrm_data.flag = HRM_ENERGY_EXPENDED_STATUS;

            //Parse calorie data
            hrm_len = blehrm_DataParse(hrm_char, parse_char, 2);

            if (hrm_len)
            {
                hrmAppState->blehrm_hrm_data.ee = 0;
                //Writing DB with new value
                for (i = 0; i < hrm_len; i++)
                {
                    hrmAppState->blehrm_hrm_data.ee += (parse_char[i] - '0');

                    if (i < hrm_len - 1)
                    {
                        hrmAppState->blehrm_hrm_data.ee *= 10;
                    }
                }
                temp = hrmAppState->blehrm_hrm_data.ee;
                temp *= 41868;
                temp /= 10000;
                hrmAppState->blehrm_hrm_data.ee = (UINT16)temp;
            }

            hrmAppState->blehrm_measurement_done = 1; //New measurement is done
        }
    }

    // send Notification
    if (hrmAppState->blehrm_measurement_done) //if connected and encrpted, old data is sent
    {
        blehrm_SendNotification(&hrmAppState->blehrm_hrm_data);
    }
}

// Application can call this function to send notification with HRM data to the connected client
void blehrm_SendNotification(BLEHRM_HRM_DATA *p_blehrm_hrm_data)
{
    BLEPROFILE_DB_PDU   db_pdu, db_cl_pdu;

    if (hrmAppState->blehrm_notification_enable)
    {
        int i;

        //write partial based on flag
        // FLAG(1) - 1 bytes are mandatory
        db_pdu.pdu[0] = p_blehrm_hrm_data->flag;
        i = 1;

        // Variable item
        if (p_blehrm_hrm_data->flag & HRM_HEARTRATE_VALUE) // 16 byte
        {
            db_pdu.pdu[i++] = p_blehrm_hrm_data->hrm & 0xff;
            db_pdu.pdu[i++] = (p_blehrm_hrm_data->hrm >> 8) & 0xff;
        }
        else
        {
            db_pdu.pdu[i++] = p_blehrm_hrm_data->hrm & 0xff;
        }

        //Optional item
        if (p_blehrm_hrm_data->flag & HRM_ENERGY_EXPENDED_STATUS)
        {
            UINT16 ee = p_blehrm_hrm_data->ee - hrmAppState->blehrm_ee_offset;
            db_pdu.pdu[i++] = ee & 0xff;
            db_pdu.pdu[i++] = (ee >> 8) & 0xff;
        }

        if (p_blehrm_hrm_data->flag & HRM_RR_INTERVAL_SUPPORT)
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(p_blehrm_hrm_data->rr[0]), 2 * p_blehrm_hrm_data->rr_len);
            i += (2 * p_blehrm_hrm_data->rr_len);
        }

        // Do not need to save in the DB because attrib is not readable
         db_pdu.len = i;
        // bleprofile_WriteHandle(hrmAppState->blehrm_hrm_hdl, &db_pdu);
        // ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        //check client char cfg
        bleprofile_ReadHandle(hrmAppState->blehrm_hrm_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_sendNotification(hrmAppState->blehrm_hrm_hdl, (UINT8 *)db_pdu.pdu, db_pdu.len);
        }

        hrmAppState->blehrm_measurement_done = 0; //enable new measurement
    }
}


UINT8 blehrm_DataParse(char *data, char *parse_data, UINT8 num)
{
    char    *hrm_char = parse_data;
    int     i;
    INT8    len;

    len = -1 * num; // 0 : before finding first ',' 1-3: number written in hrm_char
    for (i = 0; i < READ_UART_LEN; i++)
    {
        if (len < 0)
        {
            //skip until first ','
            if (data[i] == ',')
            {
                len++;
            }
        }
        else if (len >= 0 && len <= 3)
        {
            if (data[i] == '-') // '-' number means wrong entry
            {
                return 0; //error
            }
            else if (data[i] == ',') // ',' means end of number
            {
                hrm_char[len] = 0; // null character

                return len; //return number bytes written
            }
            else if (data[i] == 0x0a) // '\n' means end of number
            {
                hrm_char[len] = 0; // null character

                return len; //return number bytes written
            }
            else if (data[i] == 0x0d) // '\r' means end of number
            {
                hrm_char[len] = 0; // null character

                return len; //return number bytes written
            }
            else if (data[i] >= '0' && data[i] <='9') //number character
            {
                hrm_char[len] = data[i];
                len++;
            }
            else // error character case
            {
                return 0; //error
            }
        }
        else //length is longer than 3 means error happens.
        {
            return 0; //error
        }
    }

    return 0; //No ',' found case, error
}


void blehrm_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;
    //load handle number

    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEART_RATE &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT)
        {
            hrmAppState->blehrm_hrm_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_hrm_hdl:%04x", TVF_D(hrmAppState->blehrm_hrm_hdl));
            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            hrmAppState->blehrm_hrm_client_hdl = legattdb_findCharacteristicDescriptor(
                hrmAppState->blehrm_hrm_hdl, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_hrm_client_hdl:%04x", TVF_D(hrmAppState->blehrm_hrm_client_hdl));
            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_client_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEART_RATE &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_HEART_RATE_CONTROL_POINT)
        {
            hrmAppState->blehrm_hrm_cp_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_hrm_cp_hdl:%04x", TVF_D(hrmAppState->blehrm_hrm_cp_hdl));
            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_cp_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEART_RATE &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_HEART_RATE_SENSOR_LOCATION)
        {
            hrmAppState->blehrm_hrm_bsl_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_hrm_bsl_hdl:%04x", TVF_D(hrmAppState->blehrm_hrm_bsl_hdl));
            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_bsl_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            hrmAppState->blehrm_bat_enable = 1;
            blebat_Init();
        }
    }

    //init data with HRM
    db_pdu.len = sizeof(BLEHRM_HRM_DATA);
    bleprofile_ReadHandle(hrmAppState->blehrm_hrm_hdl, &db_pdu);
    memcpy((char *)(&(hrmAppState->blehrm_hrm_data)), db_pdu.pdu, db_pdu.len);
    ble_tracen((char *)(&(hrmAppState->blehrm_hrm_data)), db_pdu.len);

}

void blehrm_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    hrmAppState->blehrm_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(hrmAppState->blehrm_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(hrmAppState->blehrm_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_connUp[5:2]: %02x %02x %02x %02x", TVF_BBBB(hrmAppState->blehrm_remote_addr[5], hrmAppState->blehrm_remote_addr[4], hrmAppState->blehrm_remote_addr[3], hrmAppState->blehrm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_connUp[1:0]: %02x %02x, handle: %04x", TVF_BBW(hrmAppState->blehrm_remote_addr[1], hrmAppState->blehrm_remote_addr[0],  hrmAppState->blehrm_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

    // Client can set values for Client Configuration descriptor once during bonding.  On 
    // every successful connection we need to read value from the NVRAM and set descriptors
    // approprietely
    if (hrmAppState->blehrm_hrm_client_hdl != 0) 
    {
        // if we are connected to not bonded device descriptor is 0
        db_cl_pdu.len    = 2;
        db_cl_pdu.pdu[0] = 0x00;
        db_cl_pdu.pdu[1] = 0x00;

        if ((memcmp(hrmAppState->blehrm_remote_addr, hrmAppState->blehrm_hostinfo.bdAddr, 6) == 0) &&
            (hrmAppState->blehrm_hostinfo.serv == UUID_SERVICE_HEART_RATE) &&
            (hrmAppState->blehrm_hostinfo.cha  == UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = hrmAppState->blehrm_hostinfo.cli_cha_desc & 0xFF;
            db_cl_pdu.pdu[1] = hrmAppState->blehrm_hostinfo.cli_cha_desc >>8;
        }
        bleprofile_WriteHandle(hrmAppState->blehrm_hrm_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    // Start Connection idle timer to disconnect if there is no activity
    bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);

    // If encryption is not required, we are ready to send notifications
    if (bleprofile_p_cfg->encr_required == 0)
    {
        hrmAppState->blehrm_notification_enable = 1; //notification enable

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NotOn", TVF_D(0));
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blehrm_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_connDown[5:2]: %02x %02x %02x %02x", TVF_BBBB(hrmAppState->blehrm_remote_addr[5], hrmAppState->blehrm_remote_addr[4],hrmAppState->blehrm_remote_addr[3], hrmAppState->blehrm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_connDown[1:0]: %02x %02x, handle: %04x", TVF_BBW(hrmAppState->blehrm_remote_addr[1], hrmAppState->blehrm_remote_addr[0], hrmAppState->blehrm_con_handle));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm, disc reason: %02x", TVF_D(emconinfo_getDiscReason()));
    
    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

    // Save client characteristic descriptor to NVRAM
    if (memcmp(hrmAppState->blehrm_remote_addr, hrmAppState->blehrm_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
        UINT8             writtenbyte;

        if (hrmAppState->blehrm_hrm_client_hdl)
        {
            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            hrmAppState->blehrm_hostinfo.serv         = UUID_SERVICE_HEART_RATE;
            hrmAppState->blehrm_hostinfo.cha          = UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT;
            hrmAppState->blehrm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }

    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, hrmAppState->blehrm_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2] %02x %02x %02x %02x", TVF_BBBB(hrmAppState->blehrm_hostinfo.bdAddr[5], hrmAppState->blehrm_hostinfo.bdAddr[4], hrmAppState->blehrm_hostinfo.bdAddr[3], hrmAppState->blehrm_hostinfo.bdAddr[2] ));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2] %02x %02x, handle: %04x", TVF_BBW(hrmAppState->blehrm_hostinfo.bdAddr[1], hrmAppState->blehrm_hostinfo.bdAddr[0],  hrmAppState->blehrm_con_handle));
    }

    hrmAppState->blehrm_con_handle          = 0; //reset connection handle
    hrmAppState->blehrm_notification_enable = 0; //notification enable
    hrmAppState->blehrm_measurement_done    = 0;
    hrmAppState->blehrm_ee_offset           = 0;
}


void blehrm_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV Stop", TVF_D(0));
}

void blehrm_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (hrmAppState->blehrm_apptimer_count)++;

                blehrm_Timeout(hrmAppState->blehrm_apptimer_count);
            }
            break;

    }
}

void blehrm_appFineTimerCb(UINT32 arg)
{
    (hrmAppState->blehrm_appfinetimer_count)++;

    blehrm_FineTimeout(hrmAppState->blehrm_appfinetimer_count);
}


void blehrm_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 writtenbyte;

        memcpy(hrmAppState->blehrm_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(hrmAppState->blehrm_remote_addr));
        memcpy(hrmAppState->blehrm_hostinfo.bdAddr, hrmAppState->blehrm_remote_addr, sizeof(BD_ADDR));

        if (hrmAppState->blehrm_hrm_client_hdl)
        {
            BLEPROFILE_DB_PDU db_cl_pdu;

            bleprofile_ReadHandle(hrmAppState->blehrm_hrm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            hrmAppState->blehrm_hostinfo.serv         = UUID_SERVICE_HEART_RATE;
            hrmAppState->blehrm_hostinfo.cha          = UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT;
            hrmAppState->blehrm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blehrm_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Enc Change", TVF_D(0));

    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(hrmAppState->blehrm_hostinfo)))
    {
        if (memcmp(hrmAppState->blehrm_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6) == 0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2] %02x %02x %02x %02x", TVF_BBBB(hrmAppState->blehrm_hostinfo.bdAddr[5], hrmAppState->blehrm_hostinfo.bdAddr[4], hrmAppState->blehrm_hostinfo.bdAddr[3], hrmAppState->blehrm_hostinfo.bdAddr[2] ));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0] %02x %02x", TVF_BB(hrmAppState->blehrm_hostinfo.bdAddr[1], hrmAppState->blehrm_hostinfo.bdAddr[0] ) );
        }
    }

    if (bleprofile_p_cfg->encr_required != 0)
    {
        hrmAppState->blehrm_notification_enable = 1; //notification enable

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Noti On", TVF_D(0));
    }
}

UINT32 blehrm_HrmButton(UINT32 function)
{
    if (function == BUTTON_DISCOVER)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Disc Buttn Reset", TVF_D(0));
        hrmAppState->blehrm_con_handle          = 0; //reset connection handle
        hrmAppState->blehrm_notification_enable = 0; //notification enable
        hrmAppState->blehrm_measurement_done    = 0;
        hrmAppState->blehrm_ee_offset           = 0;
    }

    return 0;
}

int blehrm_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16  handle      = legattdb_getHandle(p);
    int     len         = legattdb_getAttrValueLen(p);
    UINT8   *attrPtr    = legattdb_getAttrValue(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x", TVF_D(handle));

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

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAMWr:%04x", TVF_D(writtenbyte));
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
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blehrm_ee_offset=%d", TVF_D(hrmAppState->blehrm_ee_offset));
        }
    }

    return 0;
}

