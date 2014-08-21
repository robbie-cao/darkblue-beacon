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
* BLE Weight Scale profile, service, application 
*
* This is a ROM code for Weight Scale device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  See blethermspar.c for example of 
* customisation.
*
* Refer to Bluetooth SIG Weight Scale Profile 0.7 and Weight Scale
* Service 0.7 specifications for details.
*
* During initialization the app registers with LE stack to receive various
* notifications including bonding complete, connection status change and
* peer write.  When device is successfully bonded, application saves
* peer's Bluetooth Device address to the NVRAM.  Bonded device can also 
* write client configuration descriptor for thermometer measurement and 
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
#include "blews.h"

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blews_Timeout(UINT32 count);
void blews_FineTimeout(UINT32 finecount);
void blews_handleUART(char *ws_char);
void blews_DBInit(void);
void blews_connUp(void);
void blews_connDown(void);
void blews_advStop(void);
void blews_appTimerCb(UINT32 arg);
void blews_appFineTimerCb(UINT32 arg);
void blews_smpBondResult(LESMP_PARING_RESULT  result);
void blews_encryptionChanged(HCI_EVT_HDR *evt);
void blews_IndicationConf(void);
void blews_transactionTimeout(void);


//This function is for WS test
void blews_FakeUART(char *ws_char, UINT32 count);
int blews_writeCb(LEGATTDB_ENTRY_HDR *p);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blews_db_data[]=
{
0x01, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x01,0x18,
0x02, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x10,0x03,0x00,0x05,0x2a,
0x03, 0x00, // handle
        0, // permission
        0x06, // length
        0x05, 0x2a, // uuid
	0x00,0x00,0x00,0x00,
0x14, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x00,0x18,
0x15, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x16,0x00,0x00,0x2a,
0x16, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x12, // length
        0x00, 0x2a, // uuid
	0x42,0x4c,0x45,0x20,0x57,0x65,0x69,0x67,0x68,0x74,0x53,0x63,0x61,0x6C,0x65,0x00,// "BLE WeightScale"
0x17, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x18,0x00,0x01,0x2a,
0x18, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x01, 0x2a, // uuid
	0x00,0x00,
0x28, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x00,0xb0,
0x29, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x20,0x2a,0x00,0x01,0xb0,    //UUID_CHARACTERISTIC_WEIGHT_SCALE_MEASUREMENT
0x2a, 0x00, // handle
        0 , // permission Blood Pressure Measurement, not readable.
        0x07, // length
        0x01, 0xb0, // uuid
	0x00,0x00,0x00,0x00,0x00,
0x2b, 0x00, // handle
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,
0x2d, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x0a,0x18,
0x2e, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x2f,0x00,0x29,0x2a,
0x2f, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x29, 0x2a, // uuid
	0x42,0x72,0x6f,0x61,0x64,0x63,0x6f,0x6d,
0x30, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x31,0x00,0x24,0x2a,
0x31, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x24, 0x2a, // uuid
	0x31,0x32,0x33,0x34,0x00,0x00,0x00,0x00,
0x32, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x33,0x00,0x23,0x2a,
0x33, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x23, 0x2a, // uuid System ID.
	0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blews_db_size = sizeof(blews_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blews_cfg =
{
    1000, // UINT16 fine_timer_interval; //ms
    4, // HIGH_UNDIRECTED_DISCOVERABLE, // UINT8 default_adv; //default adv
    0, // UINT8 button_adv_toggle; //pairing button make adv toggle (if 1) or always on (if 0)
    32, //UINT16 high_undirect_adv_interval; //slots
    2048, //UINT16 low_undirect_adv_interval; //slots
    30, // UINT16 high_undirect_adv_duration; //seconds
    300, // UINT16 low_undirected_adv_duration; //seconds
    0, //UINT16 high_direct_adv_interval; //seconds
    0,  //UINT16 low_direct_adv_interval; //seconds
    0,  // UINT16 high_direct_adv_duration; //seconds
    0, //  UINT16 low_direct_adv_duration; //seconds
    "BLE WeightScale", //char local_name[LOCAL_NAME_LEN_MAX];
    "\x00\x09\x0C", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
    0, // UINT8 disc_required;// if 1, disconnection after confirmation
    1, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x04,  //  UINT8 tx_power_level; //dbm
    120, // UINT8 con_idle_timeout; //second   0-> no timeout
    0, //    UINT8 powersave_timeout; //second  0-> no timeout
    {0x002a,
      0x002b,
      0x00,
      0x00,
      0x00}, // UINT16 hdl[HANDLE_NUM_MAX];   //GATT HANDLE number
    {UUID_SERVICE_WEIGHT_SCALE,
      UUID_CHARACTERISTIC_WEIGHT_SCALE_MEASUREMENT,
      0x00,
      0x00,
      0x00}, // UINT16 serv[HANDLE_NUM_MAX];  //GATT service UUID
    {UUID_CHARACTERISTIC_WEIGHT_SCALE_MEASUREMENT,
      UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
      0x00,
      0x00,
      0x00}, // UINT16 cha[HANDLE_NUM_MAX];   // GATT characteristic UUID
    0, // UINT8 findme_locator_enable; //if 1 Find me locator is enable
    0, // UINT8 findme_alert_level; //alert level of find me
    0, // UINT8 client_grouptype_enable; // if 1 grouptype read can be used
    0, // UINT8 linkloss_button_enable; //if 1 linkloss button is enable
    0, // UINT8 pathloss_check_interval; //second
    0, //UINT8 alert_interval; //interval of alert
    0, //UINT8 high_alert_num;     //number of alert for each interval
    0, //UINT8 mild_alert_num;     //number of alert for each interval
    0, // UINT8 status_led_enable;    //if 1 status LED is enable
    0, //UINT8 status_led_interval; //second
    0, // UINT8 status_led_con_blink; //blink num of connection
    0, // UINT8 status_led_dir_adv_blink; //blink num of dir adv
    0, //UINT8 status_led_un_adv_blink; //blink num of undir adv
    0, // UINT16 led_on_ms;  //led blink on duration in ms
    0, // UINT16 led_off_ms; //led blink off duration in ms
    0, // UINT16 buz_on_ms; //buzzer on duration in ms
    0, // UINT8 button_power_timeout; // seconds
    0, // UINT8 button_client_timeout; // seconds
    0, //UINT8 button_discover_timeout; // seconds
    0, //UINT8 button_filter_timeout; // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_timeout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blews_puart_cfg =
{
    115200, // UINT32 baudrate;
    32, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    33, // UINT8  rxpin; //GPIO pin number
};


PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blews_gpio_cfg =
{
    {31, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
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
      0,
      0,
      0,
      0,
      0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};


UINT32 blews_apptimer_count=0;
UINT32 blews_appfinetimer_count=0;
UINT16 blews_con_handle=0;
BD_ADDR  blews_remote_addr;

UINT16 blews_ws_hdl=0;
UINT16 blews_ws_client_hdl=0;
UINT8 blews_bat_enable=0;
UINT8 blews_indication_enable=0;
UINT8 blews_indication_defer=0;
UINT8 blews_indication_sent=0;
UINT8 blews_measurement_done=0;

//NVRAM save area
BLEPROFILE_HOSTINFO blews_hostinfo;

BLEWS_WS_DATA blews_ws_data;


void blews_Create(void)
{
    ble_trace0("blews_Create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blews_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blews_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blews_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blews_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blews_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blews_smpBondResult);
#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blews_IndicationConf);
#endif
    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blews_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blews_writeCb);

    //data init
    memset(&blews_ws_data, 0x00, sizeof(BLEWS_WS_DATA));

    bleprofile_regTimerCb(blews_appFineTimerCb, blews_appTimerCb);
    bleprofile_StartTimer();

    blews_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void blews_FakeUART(char *ws_char, UINT32 count)
{
    //This is for test only
    if(count % 30 == 0 && blews_indication_enable && ws_char[0] == 0)
    {
        //"ST,+123.52kg"
        //faking data measurement
        ws_char[0] = 'S';
        ws_char[1] = 'T';
        ws_char[2] = ',';
        ws_char[3] = '+';
        ws_char[4] = ((count%1000)/100)+'0';
        ws_char[5] = ((count%100)/10)+'0';
        ws_char[6] = (count%10)+'0';
        ws_char[7] = '.';
        ws_char[8] = ((count%100)/10)+'0';
        ws_char[9] = (count%10)+'0';
	 if(count % 60 == 0)
	 {
            ws_char[10] = 'k';
            ws_char[11] = 'g';
	 }
	 else
	 {
            ws_char[10] = 'l';
            ws_char[11] = 'b';
	 }
    }
}

void blews_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);

    if(blews_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}

void blews_FineTimeout(UINT32 finecount)
{
    //ble_trace1("Finetimer:%d", finecount);

    {
        char ws_char[READ_UART_LEN+1];

        //Reading
        bleprofile_ReadUART(ws_char);
        //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
	 //      ws_char[0], ws_char[1], ws_char[2], ws_char[3], ws_char[4], ws_char[5]);

#if 1
        if(bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            blews_FakeUART(ws_char, finecount);
        }
#endif

	 if(ws_char[0] == 'D' && ws_char[1] == 'D') //download start
	 {
            blecm_setFilterEnable(0);
            ble_trace0("CSA_filter disabled");
	 }
	 else if(ws_char[0] == 'A' && ws_char[1] == 'A') //download start
	 {
            blecm_setFilterEnable(1);
            ble_trace0("CSA_filter enabled");

            blews_connDown();
	 }
        else  //ws main reading
        {
            blews_handleUART(ws_char);
        }
    }

    // button control
    bleprofile_ReadButton();
}

void blews_handleUART(char *ws_char)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    int i;
    UINT8 writtenbyte;

    UINT8 minus = 0;
    UINT8 lbs = 0;

    //this part can be replaced by callback function
    if(bleprofile_handleUARTCb)
    {
        blews_measurement_done = bleprofile_handleUARTCb((UINT8 *)ws_char, (UINT8 *)&blews_ws_data);
    }
    else
    {
        // Data example (14 bytes)
        //{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        //{0x53, 0x54, 0x2c, 0x2b, 0x31, 0x32, 0x33, 0x2e, 0x34, 0x35, 0x6b, 0x67, 0x0d, 0x0a};
        //{"S" , "T" , "," , "+" , "0" , "0" , "0" , "." , "0" , "0" , "k" , "g" , 0x0d, 0x0a};

        //Writing DB with new value
        if((ws_char[0] == 'S' && ws_char[1] == 'T' && ws_char[2] == ',')||
            (ws_char[0] == 'U' && ws_char[1] == 'S' && ws_char[2] == ',')||
            (ws_char[0] == 'O' && ws_char[1] == 'L' && ws_char[2] == ','))
        {
            //set measurement data
            if(ws_char[3] == '-')
            {
                minus = 1;
            }
            else
            {
                minus = 0;
            }

            for(i=4; i<=6; i++)
            {
                if((ws_char[i] >= '0') && (ws_char[i] <= '9'))
                {
                    ws_char[i] -= '0';
                }
                else
                {
                    ws_char[i] = 0; //error case
                }
            }
            for(i=8; i<=9; i++)
            {
                if((ws_char[i] >= '0') && (ws_char[i] <= '9'))
                {
                    ws_char[i] -= '0';
                }
                else
                {
                    ws_char[i] = 0; //error case
                }
            }

            if((ws_char[10] == 'l') && (ws_char[11] == 'b'))
            {
                lbs = 1;
            }
            else
            {
                lbs = 0;
            }

            blews_ws_data.weight = bleprofile_UINT16_UINT16toFLOAT32(minus,
        			    ws_char[4]*100+ws_char[5]*10+ws_char[6], ws_char[8]*10+ws_char[9]);
            blews_ws_data.flag = lbs;


            blews_measurement_done = 1; //New measurement is done
        }
    }





    // change EEPROM and GATT DB
    if(blews_measurement_done == 1) //if connected and encrpted, old data is sent
    {
        //write whole in EEPROM
	 writtenbyte = bleprofile_WriteNVRAM(VS_BLE_WS_DATA, sizeof(BLEWS_WS_DATA), (UINT8 *)&blews_ws_data);
        ble_trace1("NVRAM WS write:%04x", writtenbyte);

        blews_measurement_done = 2; //this will prevent multiple writing
        //blews_measurement_done = 0; //this will permit multiple measurement
        blews_indication_defer =1;
    }


    //handling deferred indication
    if(blews_indication_enable && blews_indication_defer)
    {
        //load WS data from NVRAM and send
        if(bleprofile_ReadNVRAM(VS_BLE_WS_DATA, sizeof(BLEWS_WS_DATA), (UINT8 *)&blews_ws_data))
        {
            int i=0;

            //write partial based on flag
            // FLAG(1(?)2), WEIGHT(4) - 6 bytes are mandatory
            memcpy(db_pdu.pdu, &blews_ws_data, 5/*6*/);
            i=5/*6*/;

            //Optional item
            if(blews_ws_data.flag & WS_TIME_AND_DATE)
            {
                memcpy(&(db_pdu.pdu[i]), blews_ws_data.timeanddate, 7);
                i+=7;
            }
            if(blews_ws_data.flag & WS_TIME_STAMP)
            {
                memcpy(&(db_pdu.pdu[i]), blews_ws_data.timestamp, 7);
                i+=7;
            }
            if(blews_ws_data.flag & WS_BMI)
            {
                if(i+4 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.bmi), 4);
                    i+=4;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_BMI;
                }
            }
            if(blews_ws_data.flag & WS_HEIGHT)
            {
                if(i+4 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.height), 4);
                    i+=4;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_HEIGHT;
                }
            }
            if(blews_ws_data.flag & WS_PERSON_ID)
            {
                if(i+1 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.personid), 1);
                    i+=1;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_PERSON_ID;
                }
            }
            if(blews_ws_data.flag & WS_DATE_OF_BIRTH)
            {
                if(i+7 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), blews_ws_data.dateofbirth, 7);
                    i+=7;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_DATE_OF_BIRTH;
                }
            }
#if 0
// temporarily eliminated for supporting UINT8 flag;
            if(blews_ws_data.flag & WS_GENDER)
            {
                if(i+1 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.gender), 1);
                    i+=1;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_GENDER;
                }
            }
            if(blews_ws_data.flag & WS_BODY_FAT_PERCENTAGE)
            {
                if(i+2 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.bodyfatpercentage), 2);
                    i+=2;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_BODY_FAT_PERCENTAGE;
                }
            }
            if(blews_ws_data.flag & WS_BASAL_METABOLISM)
            {
                if(i+4 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.basalmetabolism), 4);
                    i+=4;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_BASAL_METABOLISM;
                }
            }
            if(blews_ws_data.flag & WS_SKELETAL_MUSCLE_PERCENTAGE)
            {
                if(i+2 <= LEATT_ATT_MTU-1)
                {
                    memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blews_ws_data.skeletalmusclepercentage), 2);
                    i+=2;
                }
                else
                {
                    blews_ws_data.flag &= ~WS_SKELETAL_MUSCLE_PERCENTAGE;
                }
            }
#endif

            db_pdu.len = i;
            bleprofile_WriteHandle(blews_ws_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            //check client char cfg
            bleprofile_ReadHandle(blews_ws_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            // this is the first indication
            if(db_cl_pdu.len==2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
            {
                if(blews_indication_sent==0)
                {
                    bleprofile_sendIndication( blews_ws_hdl,
                            (UINT8 *)db_pdu.pdu, db_pdu.len, blews_IndicationConf);
                    blews_indication_sent = 1;
                }
                else
                {
                    ble_trace0("No Confirmation blocks Indication");
                }
            }
        }

        blews_indication_defer = 0;
    }
}

void blews_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;
    //load handle number

    for(i=0; i<HANDLE_NUM_MAX; i++)
    {
        if(bleprofile_p_cfg->serv[i] == uuid_service_weight_scale &&
            bleprofile_p_cfg->cha[i] == uuid_characteristic_weight_scale_measurements)
        {
            blews_ws_hdl=bleprofile_p_cfg->hdl[i];
            ble_trace1("\blews_ws_hdl:%04x", blews_ws_hdl);
            bleprofile_ReadHandle(blews_ws_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if(bleprofile_p_cfg->serv[i] == uuid_characteristic_weight_scale_measurements &&
            bleprofile_p_cfg->cha[i] == UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION)
        {
            blews_ws_client_hdl=bleprofile_p_cfg->hdl[i];
            ble_trace1("\blews_ws_client_hdl:%04x", blews_ws_client_hdl);
            bleprofile_ReadHandle(blews_ws_client_hdl, &db_pdu);
            //ble_trace5("%02x %02x %02x %02x(%02x)",
            //    db_pdu.pdu[0], db_pdu.pdu[1], db_pdu.pdu[2], db_pdu.pdu[3], db_pdu.len);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if(bleprofile_p_cfg->serv[i]==UUID_SERVICE_BATTERY &&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            blews_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(blews_ws_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif


}

void blews_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    blews_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(blews_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(blews_remote_addr));

    ble_trace3("\rblews_connUp: %08x%04x %d",
                      (blews_remote_addr[5]<<24)+(blews_remote_addr[4]<<16)+
                      (blews_remote_addr[3]<<8)+blews_remote_addr[2],
                      (blews_remote_addr[1]<<8)+blews_remote_addr[0],
                      blews_con_handle);

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if(memcmp(blews_remote_addr, blews_hostinfo.bdAddr, 6) == 0)
    {
        if((blews_hostinfo.serv == uuid_service_weight_scale) &&
            (blews_hostinfo.cha == uuid_characteristic_weight_scale_measurements))
        {
            db_cl_pdu.pdu[0] = blews_hostinfo.cli_cha_desc & 0xFF;
            db_cl_pdu.pdu[1] = blews_hostinfo.cli_cha_desc >>8;
        }
    }

    //reset client char cfg
    if(blews_ws_client_hdl)
    {
        bleprofile_WriteHandle(blews_ws_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    if(bleprofile_p_cfg->encr_required==0)
    {
        blews_indication_enable = 1; //indication enable
        blews_indication_defer = 1;

        ble_trace0("\rblews, indication on");
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blews_connDown(void)
{
    ble_trace3("\rblews_connDown: %08x%04x %d",
                      (blews_remote_addr[5]<<24)+(blews_remote_addr[4]<<16)+
                      (blews_remote_addr[3]<<8)+blews_remote_addr[2],
                      (blews_remote_addr[1]<<8)+blews_remote_addr[0],
                      blews_con_handle);

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);

    // Save client characteristic descriptor to NVRAM
    if(memcmp(blews_remote_addr, blews_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
	 UINT8 writtenbyte;

        if(blews_ws_client_hdl)
        {
            bleprofile_ReadHandle(blews_ws_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blews_hostinfo.serv = uuid_service_weight_scale;
            blews_hostinfo.cha = uuid_characteristic_weight_scale_measurements;
            blews_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);
            //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blews_hostinfo);

            ble_trace1("NVRAM write:%04x", writtenbyte);
        }
    }

    // go back to Adv.
    // should be select after read NVRAM data
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);	//for test only


    // Mandatory discovery mode
    if(bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, blews_hostinfo.bdAddr);

        ble_trace3("\rADV start: %08x%04x %d",
                      (blews_hostinfo.bdAddr[5]<<24)+(blews_hostinfo.bdAddr[4]<<16)+
                      (blews_hostinfo.bdAddr[3]<<8)+blews_hostinfo.bdAddr[2],
                      (blews_hostinfo.bdAddr[1]<<8)+blews_hostinfo.bdAddr[0],
                      blews_con_handle);
    }

    blews_con_handle = 0; //reset connection handle
    blews_indication_enable = 0; //notification enable
    blews_indication_defer = 0;
    blews_indication_sent = 0;
    blews_measurement_done = 0;
}

void blews_advStop(void)
{
    ble_trace0("ADV stop!!!!");
}

void blews_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                blews_apptimer_count++;

                blews_Timeout(blews_apptimer_count);
            }
            break;

    }
}

void blews_appFineTimerCb(UINT32 arg)
{
    blews_appfinetimer_count++;

    blews_FineTimeout(blews_appfinetimer_count);
}


void blews_smpBondResult(LESMP_PARING_RESULT  result)
{
    ble_trace1("\rblews, bond result %02x", result);

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
	 // saving bd_addr in nvram
	 UINT8 writtenbyte;

        memcpy(blews_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(blews_remote_addr));
        memcpy(blews_hostinfo.bdAddr, blews_remote_addr, sizeof(BD_ADDR));

        if(blews_ws_client_hdl)
        {
            BLEPROFILE_DB_PDU db_cl_pdu;

            bleprofile_ReadHandle(blews_ws_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blews_hostinfo.serv = uuid_service_weight_scale;
            blews_hostinfo.cha = uuid_characteristic_weight_scale_measurements;
            blews_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);
        //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blews_hostinfo);

        ble_trace1("NVRAM write:%04x", writtenbyte);
    }
}

void blews_encryptionChanged(HCI_EVT_HDR *evt)
{
    ble_trace0("\rblews, encryption changed");

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo))
    {
        if(memcmp(blews_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6)==0)
        {
            ble_trace2("\rEncOn for Last paired device: %08x%04x",
                      (blews_hostinfo.bdAddr[5]<<24)+(blews_hostinfo.bdAddr[4]<<16)+
                      (blews_hostinfo.bdAddr[3]<<8)+blews_hostinfo.bdAddr[2],
                      (blews_hostinfo.bdAddr[1]<<8)+blews_hostinfo.bdAddr[0]);
        }
    }

    if(bleprofile_p_cfg->encr_required)
    {
        blews_indication_enable = 1; //indication enable
        blews_indication_defer = 1;

        ble_trace0("\rblews, indication on");
    }
}



void blews_IndicationConf(void)
{
    if(blews_indication_sent)
    {
        blews_indication_sent = 0;
    }
    else
    {
        ble_trace0("WS Wrong Confirmation!!!");
        return;
    }

    if((bleprofile_p_cfg->disc_required) & DISC_AFTER_CONFIRM)
    {
        if(blews_con_handle && blews_indication_enable && blews_measurement_done)
        //Encryption and data measurement is done
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            blews_con_handle= 0;
            blews_indication_enable = 0;
            blews_indication_defer = 0;
            blews_measurement_done = 0;
            ble_trace0("WS Disconnect after confirm");
        }
    }
}

void blews_transactionTimeout(void)
{
    ble_trace0("WS ATT timeout");

    if((bleprofile_p_cfg->disc_required) & DISC_ATT_TIMEOUT)
    {
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            blews_con_handle= 0;
            blews_indication_enable = 0;
            blews_indication_defer = 0;
            blews_measurement_done = 0;
            ble_trace0("WS Disconnect after ATT timeout");
        }
    }
}

int blews_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);

    ble_trace1("\rWriteCb: handle %04x", handle);

    if(blews_ws_client_hdl && handle == blews_ws_client_hdl)
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);

        // Save client characteristic descriptor to NVRAM
        if(memcmp(blews_remote_addr, blews_hostinfo.bdAddr, 6) == 0)
        {
            BLEPROFILE_DB_PDU db_cl_pdu;
	     UINT8 writtenbyte;

            bleprofile_ReadHandle(blews_ws_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blews_hostinfo.serv         = uuid_service_weight_scale;
            blews_hostinfo.cha          = uuid_characteristic_weight_scale_measurements;
            blews_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blews_hostinfo);
            //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blews_hostinfo);

            ble_trace1("NVRAM write:%04x", writtenbyte);

        }
    }

    return 0;
}
