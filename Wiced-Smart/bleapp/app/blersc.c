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
* BLE Runners Speed and Cadence profile, service, application
*
* Refer to Bluetooth SIG Runners Speed and Cadence Profile 1.0 Runners Speed
* and Cadence Service 1.0 specifications for details.
*
* This is a ROM code for complete implementation of sample BLE Running Speed 
* and Cadence Sensor.  An application can use any portions of this code to 
* simplify development and reduce download and startup time.  
*
*/
#include "blersc.h"
#include "dbfw_app.h"

#define FID   FID_BLEAPP_APP__BLERSC_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void    blersc_Timeout(UINT32 count);
void    blersc_FineTimeout(UINT32 finecount);
void    blersc_handleUART(char *rsc_char);
void    blersc_DBInit(void);
void    blersc_connUp(void);
void    blersc_connDown(void);
void    blersc_advStop(void);
void    blersc_appTimerCb(UINT32 arg);
void    blersc_appFineTimerCb(UINT32 arg);
void    blersc_smpBondResult(LESMP_PARING_RESULT  result);
void    blersc_encryptionChanged(HCI_EVT_HDR *evt);
UINT32  blersc_RscButton(UINT32 function);
void    bleapp_set_cfg_rsc(void);
void blersc_IndicationConf(void);


//This function is for RSC test
void    blersc_FakeUART(char *rsc_char, UINT32 count);

UINT8   blersc_DataParse(char *data);
int     blersc_writeCb(LEGATTDB_ENTRY_HDR *p);
int     blersc_checkClientConfigBeforeCP(void);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blersc_db_data[]=
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
        'B','L','E',' ','R','u','n','n','e','r','s',0x00,0x00,0x00,0x00,0x00,  

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR),

    // Handle 0x28: Running Speed and Cadence service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_RSC),

    // Handle 0x29: characteristic RSC Measurement, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16  (0x0029, 0x002a, UUID_CHARACTERISTIC_RSC_MEASUREMENT, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 9), 
        0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x2c: characteristic RSC Feature, handle 0x2d characteristic value
    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_RSC_FEATURE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x03, 0x00,                 //instantaneous stride length, total distance

    // Handle 0x2e: characteristic Sensor Location, handle 0x2f characteristic value
    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_SENSOR_LOCATION, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x08,                       //in shoe

    // Because of Apple application problem do not use consecutive handle
    // Handle 0x32: characteristic RSC Control Point, handle 0x33 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0032, 0x0033, UUID_CHARACTERISTIC_RSC_CONTROL_POINT, 
                           LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE, 
                           LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 19),
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,

    // Handle 0x34: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0034, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00, 

    // Handle 0x3d: Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x3e: characteristic Manufacturer Name, handle 0x3f characteristic value
    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x40: characteristic Model Number, handle 0x41 characteristic value
    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blersc_db_size = sizeof(blersc_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blersc_cfg =
{
    /*.fine_timer_interval            =*/ 1000, // ms
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
    /*.local_name                     =*/ "BLE Runners",    // [LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR), 0,  // [COD_LEN];
    /*.ver                            =*/ "1.00",            //[VERSION_LEN]
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 120,  // second  0-> no timeout
    /*.powersave_timeout              =*/ 5,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x002f, 0x0033, 0x00}, // [HANDLE_NUM_MAX]
    /*.serv                           =*/ {UUID_SERVICE_RSC, UUID_SERVICE_RSC, UUID_SERVICE_RSC, UUID_SERVICE_RSC, 0x00},
    /*.cha                            =*/ {UUID_CHARACTERISTIC_RSC_MEASUREMENT, UUID_CHARACTERISTIC_RSC_FEATURE,
                                           UUID_CHARACTERISTIC_SENSOR_LOCATION, UUID_CHARACTERISTIC_RSC_CONTROL_POINT, 0x00},
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
    /*.button_discover_timeout        =*/ 1,    // seconds
    /*.button_filter_timeout          =*/ 10,   // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blersc_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ 32,       // GPIO pin number 20730A0 module need to use 32 instead
    /*.rxpin      =*/ 33,       
};

PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blersc_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        1,                                                     // write protect
        14,                                                    // pin button
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // not used
    },
    /*.gpio_flag =*/ 
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,                 // settings write protect
        GPIO_INPUT  | GPIO_INIT_LOW | GPIO_INT | GPIO_BUTTON,  // settings button
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
};


typedef struct
{
    BLERSC_HOSTINFO     blersc_hostinfo;                        //NVRAM save area

    BLERSC_RSC_DATA     blersc_rsc_data;

    UINT32              blersc_apptimer_count;
    UINT32              blersc_appfinetimer_count;
    UINT16              blersc_con_handle;
    BD_ADDR             blersc_remote_addr;

    UINT16              blersc_rsc_hdl;
    UINT16              blersc_rsc_client_hdl;
    UINT16              blersc_rsc_cp_hdl;
    UINT16              blersc_rsc_cp_client_hdl;
    UINT16              blersc_rsc_bsl_hdl;
    UINT8               blersc_bat_enable;
    UINT8               blersc_notification_enable;
    UINT8               blersc_measurement_done;
    UINT8               blersc_indication_sent;
    UINT8               blersc_sup_senloc[SUPPORTED_SENLOC_MAX];
    UINT32              blersc_distance;
} tRscAppState;

tRscAppState *rscAppState = NULL;


void blersc_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0],
                                                                               bleprofile_p_cfg->ver[1],
                                                                               bleprofile_p_cfg->ver[2],
                                                                               bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    rscAppState = (tRscAppState *)cfa_mm_Sbrk(sizeof(tRscAppState));
    memset(rscAppState, 0x00, sizeof(tRscAppState));

    //initialize the default value of rscAppState
    rscAppState->blersc_sup_senloc[0] = RSC_LOC_TOP_OF_SHOE;
    rscAppState->blersc_sup_senloc[1] = RSC_LOC_IN_SHOE;
    rscAppState->blersc_sup_senloc[2] = RSC_LOC_HIP;
           // {RSC_LOC_TOP_OF_SHOE, 0xFF, 0xFF};
           // 0xFF means not used, do not add 0xFF in the middle

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blersc_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blersc_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blersc_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blersc_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blersc_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blersc_smpBondResult);
    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blersc_writeCb);

    bleprofile_regButtonFunctionCb(blersc_RscButton);

    //data init
    memset(&(rscAppState->blersc_rsc_data), 0x00, sizeof(BLERSC_RSC_DATA));
    rscAppState->blersc_rsc_data.flag = 0;

    bleprofile_regTimerCb(blersc_appFineTimerCb, blersc_appTimerCb);
    bleprofile_StartTimer();

    blersc_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void blersc_Timeout(UINT32 count)
{
    if (rscAppState->blersc_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}

void blersc_FineTimeout(UINT32 count)
{
    char rsc_char[READ_UART_LEN+1];

    //Reading
    bleprofile_ReadUART(rsc_char);

    if (rsc_char[0] == 'D' && rsc_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA_filter disabled", TVF_D(0));
    }
    else if (rsc_char[0] == 'A' && rsc_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA_filter enabled", TVF_D(0));

        blersc_connDown();
    }
    else  //rsc main reading
    {
        blersc_handleUART(rsc_char);
    }

    // button control
    bleprofile_ReadButton();

}

#if 0
void blersc_FakeUART(char *rsc_char, UINT32 count)
{
    UINT8 rsc;

    //This is for test only
    if (rscAppState->blersc_notification_enable == 1 && rsc_char[0] == 0)
    {
        //faking data measurement
        rsc = count&0xFF;

        rsc_char[0] = '0';
        rsc_char[1] = '0';
        rsc_char[2] = ',';

        if (rsc >= 100)
        {
            rsc_char[3] = '0' + (rsc / 100);
            rsc_char[4] = '0' + ((rsc % 100) / 10);
            rsc_char[5] = '0' + (rsc % 10);
            rsc_char[6] = ',';
        }
        else if (rsc >= 10)
        {
            rsc_char[3] = '0' + ((rsc % 100) / 10);
            rsc_char[4] = '0' + (rsc % 10);
            rsc_char[5] = ',';
        }
        else
        {
            rsc_char[3] = '0' + (rsc % 10);
            rsc_char[4] = ',';
        }
    }
}
#endif

void blersc_handleUART(char *rsc_char)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    //int i;
    //UINT8 rsc_len;

    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        rscAppState->blersc_measurement_done = bleprofile_handleUARTCb((UINT8 *)rsc_char, (UINT8 *)&(rscAppState->blersc_rsc_data));
    }
    else
    {
#if 1
        if (bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            rscAppState->blersc_rsc_data.flag                        = RSC_INSTANTANEOUS_STRIDE_LENGTH_PRESENT
                                                                      |RSC_TOTAL_DISTANCE_PRESENT;
                                                                      //|RSC_WALKING_OR_RUNNING_STATUS;
            rscAppState->blersc_rsc_data.instantaneous_speed         = rscAppState->blersc_appfinetimer_count&0xFFFF;
            rscAppState->blersc_rsc_data.instantaneous_cadence       = rscAppState->blersc_appfinetimer_count&0xFF;
            rscAppState->blersc_rsc_data.instantaneous_stride_length = rscAppState->blersc_appfinetimer_count&0xFFFF;

            rscAppState->blersc_distance += (rscAppState->blersc_rsc_data.instantaneous_stride_length / 10);
            memcpy(&(rscAppState->blersc_rsc_data.total_distance), (void *)(&rscAppState->blersc_distance), 3);

            rscAppState->blersc_measurement_done = 1; //New measurement is done
        }
#endif

#if 0
        //Parse RSC data
        rsc_len = blersc_DataParse(rsc_char);

        if (rsc_len)
        {
            rscAppState->blersc_rsc_data.step = 0;
            //Writing DB with new value
            for (i = 0; i < rsc_len; i++)
            {
                rscAppState->blersc_rsc_data.step += (rsc_char[i]-'0');

                if (i < rsc_len-1)
                {
                    rscAppState->blersc_rsc_data.step*=10;
                }
            }
            rscAppState->blersc_rsc_data.flag = 0;

            rscAppState->blersc_measurement_done = 1; //New measurement is done
        }
#endif
    }


    // send Notification
    if (rscAppState->blersc_notification_enable && rscAppState->blersc_measurement_done) //if connected and encrpted, old data is sent
    {
        int i = 0;

        //write partial based on flag
        // FLAG(1) instantaneous speed(2) instantaneous cadence(1) - 4 bytes are mandatory
        memcpy(db_pdu.pdu, (UINT8 *)&(rscAppState->blersc_rsc_data), 4);
        i=4;


        // Optional item
        if (rscAppState->blersc_rsc_data.flag & RSC_INSTANTANEOUS_STRIDE_LENGTH_PRESENT) //16 byte
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(rscAppState->blersc_rsc_data.instantaneous_stride_length), 2);
            i+=2;
        }

        if (rscAppState->blersc_rsc_data.flag & RSC_TOTAL_DISTANCE_PRESENT)
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(rscAppState->blersc_rsc_data.total_distance), 3);
            i+=3;
        }

        db_pdu.len = i;
        bleprofile_WriteHandle(rscAppState->blersc_rsc_hdl, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        //check client char cfg
        bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_sendNotification(rscAppState->blersc_rsc_hdl, (UINT8 *)db_pdu.pdu, db_pdu.len);
        }

        rscAppState->blersc_measurement_done = 0; //enable new measurement
    }
}

#if 0
UINT8 blersc_DataParse(char *data)
{
    char rsc_char[4];
    int i;
    INT8 len;

    len = -1; // 0 : before finding first ',' 1-3: number written in rsc_char
    for (i = 0; i < READ_UART_LEN; i++)
    {
        if (len == -1)
        {
            //skip until first ','
	     if (data[i] == ',')
	     {
                len = 0;
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
                rsc_char[len] = 0; // null character

                if (len>0)
                {
                    memcpy(data, rsc_char, 4); // copy back to original data
                }

                return len; //return number bytes written
            }
            else if (data[i] >= '0' && data[i] <='9') //number character
            {
                rsc_char[len] = data[i];
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
#endif

void blersc_DBInit(void)
{
    BLEPROFILE_DB_PDU   db_pdu;
    int                 i;

    //load handle number
    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_RSC&&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_RSC_MEASUREMENT)
        {
            rscAppState->blersc_rsc_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_rsc_hdl:%04x", TVF_D(rscAppState->blersc_rsc_hdl));
            bleprofile_ReadHandle(rscAppState->blersc_rsc_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            rscAppState->blersc_rsc_client_hdl = bleprofile_p_cfg->hdl[i]+1; //chlient handle is next one
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_rsc_client_hdl:%04x", TVF_D(rscAppState->blersc_rsc_client_hdl));
            bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_RSC &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_RSC_CONTROL_POINT)
        {
            rscAppState->blersc_rsc_cp_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_rsc_cp_hdl:%04x", TVF_D(rscAppState->blersc_rsc_cp_hdl));
            bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            rscAppState->blersc_rsc_cp_client_hdl = bleprofile_p_cfg->hdl[i]+1; //chlient handle is next one
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_rsc_cp_client_hdl:%04x", TVF_D(rscAppState->blersc_rsc_cp_client_hdl));
            bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_RSC &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_SENSOR_LOCATION)
        {
            rscAppState->blersc_rsc_bsl_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_rsc_bsl_hdl:%04x", TVF_D(rscAppState->blersc_rsc_bsl_hdl));
            bleprofile_ReadHandle(rscAppState->blersc_rsc_bsl_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            rscAppState->blersc_bat_enable = 1;
            blebat_Init();
        }
    }

    //init data with RSC
    db_pdu.len = sizeof(BLERSC_RSC_DATA);
    bleprofile_ReadHandle(rscAppState->blersc_rsc_hdl, &db_pdu);
    memcpy((char *)(&(rscAppState->blersc_rsc_data)), db_pdu.pdu, db_pdu.len);
    ble_tracen((char *)(&(rscAppState->blersc_rsc_data)), db_pdu.len);

#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(rscAppState->blersc_rsc_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif


}

void blersc_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    rscAppState->blersc_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(rscAppState->blersc_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(rscAppState->blersc_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_connUp[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(rscAppState->blersc_remote_addr[5], rscAppState->blersc_remote_addr[4],
                   rscAppState->blersc_remote_addr[3], rscAppState->blersc_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_connUp[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(rscAppState->blersc_remote_addr[1], rscAppState->blersc_remote_addr[0],
                  rscAppState->blersc_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

    //using default value first
    db_cl_pdu.len    = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
    {
        if ((rscAppState->blersc_hostinfo.serv[0] == UUID_SERVICE_RSC) &&
            (rscAppState->blersc_hostinfo.cha[0]  == UUID_CHARACTERISTIC_RSC_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = rscAppState->blersc_hostinfo.cli_cha_desc[0] & 0xFF;
            db_cl_pdu.pdu[1] = rscAppState->blersc_hostinfo.cli_cha_desc[0] >>8;
        }
    }

    //reset client char cfg
    if (rscAppState->blersc_rsc_client_hdl)
    {
        bleprofile_WriteHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
    {
        bleprofile_StopConnIdleTimer();
    }
    else
    {
        bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);
    }

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
    {
        if ((rscAppState->blersc_hostinfo.serv[1] == UUID_SERVICE_RSC) &&
            (rscAppState->blersc_hostinfo.cha[1]  == UUID_CHARACTERISTIC_RSC_CONTROL_POINT))
        {
            db_cl_pdu.pdu[0] = rscAppState->blersc_hostinfo.cli_cha_desc[1] & 0xFF;
            db_cl_pdu.pdu[1] = rscAppState->blersc_hostinfo.cli_cha_desc[1] >>8;
        }
    }

    //reset client char cfg
    if (rscAppState->blersc_rsc_cp_client_hdl)
    {
        bleprofile_WriteHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }


    if (bleprofile_p_cfg->encr_required == 0)
    {
        rscAppState->blersc_notification_enable = 1; //notification enable

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc, notification on", TVF_D(0));
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blersc_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_connDown[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(rscAppState->blersc_remote_addr[5], rscAppState->blersc_remote_addr[4],
                   rscAppState->blersc_remote_addr[3], rscAppState->blersc_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc_connDown[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(rscAppState->blersc_remote_addr[1], rscAppState->blersc_remote_addr[0],
                  rscAppState->blersc_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

    // Save client characteristic descriptor to NVRAM
    if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
	 UINT8 writtenbyte;

        if (rscAppState->blersc_rsc_client_hdl)
        {
            bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            rscAppState->blersc_hostinfo.serv[0]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_RSC_MEASUREMENT;
            rscAppState->blersc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (rscAppState->blersc_rsc_cp_client_hdl)
        {
            bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            rscAppState->blersc_hostinfo.serv[1]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_RSC_CONTROL_POINT;
            rscAppState->blersc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }

    // go back to Adv.
    // should be select after read NVRAM data
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);	//for test only


    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, rscAppState->blersc_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2]: %02x %02x %02x %02x",
              TVF_BBBB(rscAppState->blersc_hostinfo.bdAddr[5], rscAppState->blersc_hostinfo.bdAddr[4],
                       rscAppState->blersc_hostinfo.bdAddr[3], rscAppState->blersc_hostinfo.bdAddr[2] ));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x, handle: %04x",
              TVF_BBW(rscAppState->blersc_hostinfo.bdAddr[1], rscAppState->blersc_hostinfo.bdAddr[0],
                      rscAppState->blersc_con_handle));
    }

    rscAppState->blersc_con_handle          = 0; //reset connection handle
    rscAppState->blersc_notification_enable = 0; //notification enable
    rscAppState->blersc_measurement_done    = 0;
}

void blersc_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV stop!!!!", TVF_D(0));
}

void blersc_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (rscAppState->blersc_apptimer_count)++;

                blersc_Timeout(rscAppState->blersc_apptimer_count);
            }
            break;

    }
}

void blersc_appFineTimerCb(UINT32 arg)
{
    (rscAppState->blersc_appfinetimer_count)++;

    blersc_FineTimeout(rscAppState->blersc_appfinetimer_count);
}


void blersc_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8               writtenbyte;
        BLEPROFILE_DB_PDU   db_cl_pdu;

        memcpy(rscAppState->blersc_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(rscAppState->blersc_remote_addr));
        memcpy(rscAppState->blersc_hostinfo.bdAddr, rscAppState->blersc_remote_addr, sizeof(BD_ADDR));

        if (rscAppState->blersc_rsc_client_hdl)
        {
            bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            rscAppState->blersc_hostinfo.serv[0]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_RSC_MEASUREMENT;
            rscAppState->blersc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (rscAppState->blersc_rsc_cp_client_hdl)
        {
            bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            rscAppState->blersc_hostinfo.serv[1]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_RSC_CONTROL_POINT;
            rscAppState->blersc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blersc_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc, encryption changed", TVF_D(0));

    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo)))
    {
        if (memcmp(rscAppState->blersc_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6) == 0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x",
              TVF_BBBB(rscAppState->blersc_hostinfo.bdAddr[5], rscAppState->blersc_hostinfo.bdAddr[4],
                       rscAppState->blersc_hostinfo.bdAddr[3], rscAppState->blersc_hostinfo.bdAddr[2] ));
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0]: %02x %02x, handle: %04x",
                  TVF_BB(rscAppState->blersc_hostinfo.bdAddr[1], rscAppState->blersc_hostinfo.bdAddr[0]));
        }
    }

    if (bleprofile_p_cfg->encr_required)
    {
        rscAppState->blersc_notification_enable = 1; //notification enable

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blersc, notification on", TVF_D(0));
    }
}

UINT32 blersc_RscButton(UINT32 function)
{
    if (function == BUTTON_DISCOVER)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RSC: Discover button reset variable", TVF_D(0));
        rscAppState->blersc_con_handle          = 0; //reset connection handle
        rscAppState->blersc_notification_enable = 0; //notification enable
        rscAppState->blersc_measurement_done    = 0;
    }

    return 0;
}

int blersc_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16  handle   = legattdb_getHandle(p);
    int     len      = legattdb_getAttrValueLen(p);
    UINT8   *attrPtr = legattdb_getAttrValue(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x", TVF_D(handle));

    if (rscAppState->blersc_rsc_client_hdl && handle == rscAppState->blersc_rsc_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_StopConnIdleTimer();
        }
        else
        {
            bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);
        }

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            rscAppState->blersc_hostinfo.serv[0] = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[0] = UUID_CHARACTERISTIC_RSC_MEASUREMENT;
            rscAppState->blersc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (rscAppState->blersc_rsc_cp_client_hdl && handle == rscAppState->blersc_rsc_cp_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            rscAppState->blersc_hostinfo.serv[1] = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[1] = UUID_CHARACTERISTIC_RSC_CONTROL_POINT;
            rscAppState->blersc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (rscAppState->blersc_rsc_cp_hdl && handle == rscAppState->blersc_rsc_cp_hdl)
    {
        BLERSC_CP_HDR       *cpHdr      = (BLERSC_CP_HDR *) attrPtr ;
        BLEPROFILE_DB_PDU   db_pdu;
        BLERSC_CP_RSP_HDR   *cprspHdr   = (BLERSC_CP_RSP_HDR *)(db_pdu.pdu);
        UINT8               response;
        UINT8               length      = 3; //default is 3

        if (blersc_checkClientConfigBeforeCP())
        {
            if (cpHdr->opcode == RSC_CP_RESET_TOTAL_DISTANCE)
            {
                //reset variable
                rscAppState->blersc_distance = 0;
            
                response = RSC_CP_SUCCESS;
            }
            else if (cpHdr->opcode == RSC_CP_START_SENSOR_CALIBRATION)
            {
                // This may time consuming job when the real sensor is used.
                response = RSC_CP_OPCODE_NOT_SUPPORTED; //RSC_CP_SUCCESS;
            }
            else if (cpHdr->opcode == RSC_CP_UPDATE_SENSOR_LOCATION)
            {
                //check location value
                if ((cpHdr-> parameter[0] >= RSC_LOC_START) && (cpHdr-> parameter[0] <= RSC_LOC_END))
                {
                    //write GATT DB with changed value
                    if (rscAppState->blersc_rsc_bsl_hdl)
                    {
                        BLEPROFILE_DB_PDU db_pdu_bsc;
                        db_pdu_bsc.pdu[0] = cpHdr-> parameter[0];
                        db_pdu_bsc.len = 1;
                     
                        bleprofile_WriteHandle(rscAppState->blersc_rsc_bsl_hdl, &db_pdu_bsc);
                        ble_tracen((char *)db_pdu_bsc.pdu, db_pdu_bsc.len);
                    }
                
                    response = RSC_CP_SUCCESS;
                }
                else
                {
                    response = RSC_CP_INVALID_PARAMETER;
                }
            }
            else if (cpHdr->opcode == RSC_CP_REQUEST_SUPPORTED_SENSOR_LOCATIONS)
            {
                // support all cases
                int i;
            
                for (i = 0; i < SUPPORTED_SENLOC_MAX; i++)
                {
                    if (rscAppState->blersc_sup_senloc[i] !=0xFF)
                    {
                        cprspHdr->response_parameter[i] = rscAppState->blersc_sup_senloc[i];
                        length++;
                    }
                    else
                    {
                        break;
                    }
                }           
                response = RSC_CP_SUCCESS;
            }
            else
            {
                // error response
                response = RSC_CP_OPCODE_NOT_SUPPORTED;
            }
        
            //client char cfg is already checked
               
            cprspHdr->opcode = RSC_CP_RESPONSE_CODE;
            cprspHdr->request_opcode = cpHdr->opcode;
            cprspHdr->response_value = response;
        
            if (rscAppState->blersc_indication_sent==0)
            {
                ble_tracen((char *)cprspHdr, length);
                bleprofile_sendIndication(rscAppState->blersc_rsc_cp_hdl, (UINT8 *)cprspHdr, length, blersc_IndicationConf);
                rscAppState->blersc_indication_sent = 1;
            }
            else
            {
                 TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "No Confirmation blocks Indication", TVF_D(0));
            }
        }
        else
        {
            return BLERSC_CP_CLIENT_CHAR_CONF_IMPROPERLY;
        }        
    }
    return 0;
}
        
            
int blersc_checkClientConfigBeforeCP(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;
    bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
 
    if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void blersc_IndicationConf(void)
{
    if (rscAppState->blersc_indication_sent)
    {
        rscAppState->blersc_indication_sent = 0;
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WS Wrong Confirmation!!!", TVF_D(0));
        return;
    }
}
