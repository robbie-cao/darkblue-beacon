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
* BLE Health Thermometer profile, service, application 
*
* This is a ROM code for Health Thermometer device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  See blethermspar.c for example of 
* customisation.
*
* Refer to Bluetooth SIG Health Thermometer Profile 1.0 and Health Thermometer
* Service 1.0 specifications for details.
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
#include "blether.h"
#include "dbfw_app.h"

#define FID   FID_BLEAPP_APP__BLETHER_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void    blether_Timeout(UINT32 count);
void    blether_FineTimeout(UINT32 finecount);
void    blether_handleUART(char *ther_char);
void    blether_DBInit(void);
void    blether_connUp(void);
void    blether_connDown(void);
void    blether_advStop(void);
void    blether_appTimerCb(UINT32 arg);
void    blether_appFineTimerCb(UINT32 arg);
void    blether_smpBondResult(LESMP_PARING_RESULT  result);
void    blether_encryptionChanged(HCI_EVT_HDR *evt);
void    blether_IndicationConf(void);
void    blether_transactionTimeout(void);
void    bleapp_set_cfg_ther(void);

//This function is for THER test
void    blether_FakeUART(char *ther_char, UINT32 count);
int     blether_writeCb(LEGATTDB_ENTRY_HDR *p);

void    blether_FakeIT(UINT32 count);
void    blether_mi_IndicationConf(void);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const  UINT8 blether_db_data[]=
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
        'B','L','E',' ','T','h','e','r','m','o','m','e','t','e','r',0x00,  

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_THERMOMETER),

    // Handle 0x28: Health thermometer service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_HEALTH_THERMOMETER),

    // Handle 0x29: characteristic temperature measurement, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16 (0x0029, 0x002a, UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT, 
                           LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 5),  
        0x01,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x2c: characteristic temperature type, handle 0x2d characteristic value
    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_TEMPERATURE_TYPE, 
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        THER_BODY,                  // value body (general)

    // Handle 0x2e: characteristic intermediate measurement, handle 0x2f characteristic value
    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE, 
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 5),  
        0x01,0x00,0x00,0x00,0x00,

    // Handle 0x30: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0030, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x31: characteristic measurement interval, handle 0x32 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0031, 0x0032, UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL, 
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE, 
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x1E,0x00,              //30 seconds

    // Handle 0x33: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0033, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x33: Valid Range descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0034, UUID_DESCRIPTOR_VALID_RANGE, LEGATTDB_PERM_READABLE, 4),
        0x00,0x00,0xFF,0xFF,

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

const UINT16 blether_db_size = sizeof(blether_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blether_cfg =
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
    /*.local_name                     =*/ "BLE Thermometer", //[LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_THERMOMETER), 0,  // [COD_LEN]
    /*.ver                            =*/ "1.00",            //[VERSION_LEN]
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 120,  // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x002f, 0x0032, 0x00}, // GATT HANDLE number
    /*.serv                           =*/ {UUID_SERVICE_HEALTH_THERMOMETER, UUID_SERVICE_HEALTH_THERMOMETER,
                                           UUID_SERVICE_HEALTH_THERMOMETER, UUID_SERVICE_HEALTH_THERMOMETER, 0x00}, // GATT service UUID
    /*.cha                            =*/ {UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT, UUID_CHARACTERISTIC_TEMPERATURE_TYPE,
                                           UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE, UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL, 0x00}, // GATT characteristic UUID
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

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blether_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ 32,       // GPIO pin number 20730A0 module need to use 32 instead
    /*.rxpin      =*/ 33,       
};


PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blether_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        1,                                                         // write protect
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // not used
    },
    /*.gpio_flag =*/ 
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,                 // settings write protect
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
};


typedef struct
{
    BLETHER_HOSTINFO    blether_hostinfo;       //NVRAM save area

    BLETHER_THER_DATA   blether_ther_data;

    UINT32              blether_apptimer_count;
    UINT32              blether_appfinetimer_count;
    UINT16              blether_con_handle;
    BD_ADDR             blether_remote_addr;

    UINT16              blether_ther_hdl;
    UINT16              blether_ther_client_hdl;
    UINT16              blether_tt_hdl;
    UINT16              blether_it_hdl;
    UINT16              blether_it_client_hdl;
    UINT16              blether_mi_hdl;
    UINT16              blether_mi_client_hdl;
    UINT16              blether_mi_range_hdl;

    UINT16              blether_mi;
    UINT8               blether_mi_indication_sent;

    UINT8               blether_bat_enable;
    UINT8               blether_indication_enable;
    UINT8               blether_indication_defer;
    UINT8               blether_indication_sent;
    UINT8               blether_measurement_done;
} tTherAppState;

tTherAppState *therAppState = NULL;

void blether_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0],
                                                                               bleprofile_p_cfg->ver[1],
                                                                               bleprofile_p_cfg->ver[2],
                                                                               bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    therAppState = (tTherAppState *)cfa_mm_Sbrk(sizeof(tTherAppState));
    memset(therAppState, 0x00, sizeof(tTherAppState));

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blether_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blether_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blether_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blether_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blether_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blether_smpBondResult);
#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blether_IndicationConf);
#endif
    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blether_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blether_writeCb);

    //data init
    memset(&(therAppState->blether_ther_data), 0x00, sizeof(BLETHER_THER_DATA));
    therAppState->blether_ther_data.flag = THER_MEASUREMENT_VALUE;

    bleprofile_regTimerCb(blether_appFineTimerCb, blether_appTimerCb);
    bleprofile_StartTimer();

    blether_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

#if 0
    // test sfloat
    bleprofile_SFLOATTest();
    // test float32
    bleprofile_FLOAT32Test();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void blether_Timeout(UINT32 count)
{
    if (therAppState->blether_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}


void blether_FineTimeout(UINT32 finecount)
{
    {
        char ther_char[READ_UART_LEN+1];

        //Reading
        bleprofile_ReadUART(ther_char);
#if 1
        if (bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            blether_FakeUART(ther_char, finecount);
        }
#endif

	 if (ther_char[0] == 'D' && ther_char[1] == 'D') //download start
	 {
            blecm_setFilterEnable(0);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA_filter disabled", TVF_D(0));
	 }
	 else if (ther_char[0] == 'A' && ther_char[1] == 'A') //download start
	 {
            blecm_setFilterEnable(1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA_filter enabled", TVF_D(0));

            blether_connDown();
	 }
        else  //ther main reading
        {
            blether_handleUART(ther_char);
        }
    }

    // button control
    bleprofile_ReadButton();
}

void blether_FakeUART(char *ther_char, UINT32 count)
{
    //This is for test only
    if (therAppState->blether_mi && count % (1000/bleprofile_p_cfg->fine_timer_interval * therAppState->blether_mi) == 0 && therAppState->blether_indication_enable && ther_char[0] == 0)
    {
        //"U80+123.52F"
        //faking data measurement
        ther_char[0] = 'U';
        ther_char[1] = '8';
        ther_char[2] = '0';
        ther_char[3] = '+';
        ther_char[4] = ((count % 1000) / 100) + '0';
        ther_char[5] = ((count % 100) / 10) + '0';
        ther_char[6] =  (count % 10) + '0';
        ther_char[7] = '.';
        ther_char[8] = ((count % 100) / 10) + '0';
        ther_char[9] = (count % 10) + '0';

	 {
            ther_char[10] = 'F';
	 }
    }
    else if (therAppState->blether_indication_enable && ther_char[0] == 0) //this one is used only intermediate measurement
    {
        blether_FakeIT(count);
    }
}

void blether_FakeIT(UINT32 count)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    int i = 0;

    therAppState->blether_ther_data.flag = 1;	//F
    therAppState->blether_ther_data.tempmeasure = bleprofile_UINT16_UINT16toFLOAT32(0,
        			    count%1000, 0);


    //write partial based on flag
    // FLAG(1), MEASUREMENT(4) - 5 bytes are mandatory
    memcpy(db_pdu.pdu, (char *)&(therAppState->blether_ther_data), 5);
    i = 5;

    //Optional item
    if (therAppState->blether_ther_data.flag & THER_TIME_STAMP)
    {
        memcpy(&(db_pdu.pdu[i]), therAppState->blether_ther_data.timestamp, 7);
        i += 7;
    }
    if (therAppState->blether_ther_data.flag & THER_TEMPERATURE_TYPE)
    {
        memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(therAppState->blether_ther_data.temptype), 1);
        i += 1;
    }


    db_pdu.len = i;
    bleprofile_WriteHandle(therAppState->blether_it_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    //check client char cfg
    bleprofile_ReadHandle(therAppState->blether_it_client_hdl, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

    if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
    {
            bleprofile_sendNotification( therAppState->blether_it_hdl,
                    (UINT8 *)db_pdu.pdu, db_pdu.len);
    }
}


void blether_handleUART(char *ther_char)
{
    BLEPROFILE_DB_PDU   db_pdu, db_cl_pdu;
    int                 i;
    UINT8               writtenbyte;
    UINT8               minus = 0;
    UINT8               fah   = 0;


    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        therAppState->blether_measurement_done = bleprofile_handleUARTCb((UINT8 *)ther_char, (UINT8 *)&(therAppState->blether_ther_data));
    }
    else
    {
        if (ther_char[0] == 'U' && ther_char[1] == '8' && ther_char[2] == '0')
        {
            //change ascii to hex
            if (ther_char[3] == '-')
            {
                minus = 1;
            }
            else
            {
                 minus = 0;
            }

            for (i = 4; i <= 6; i++)
            {
                if ((ther_char[i] >= '0') && (ther_char[i] <= '9'))
                {
                    ther_char[i] -= '0';
                }
                else
                {
                    ther_char[i] = 0; //error case
                }
            }
            for (i = 8; i <= 9; i++)
            {
                if ((ther_char[i] >= '0') && (ther_char[i] <= '9'))
                {
                    ther_char[i] -= '0';
                }
                else
                {
                    ther_char[i] = 0; //error case
                }
            }

            if (ther_char[10] == 'F')
            {
                fah = 1;
            }
	     else
	     {
	         fah = 0;
	     }

            therAppState->blether_ther_data.tempmeasure = bleprofile_UINT16_UINT16toFLOAT32(minus,
        			    ther_char[4] * 100 + ther_char[5] * 10 + ther_char[6], ther_char[8] * 10 + ther_char[9]);
            therAppState->blether_ther_data.flag = fah;

            therAppState->blether_measurement_done = 1;
        }
    }



    // change EEPROM and GATT DB
    if (therAppState->blether_measurement_done == 1) //if connected and encrpted, old data is sent
    {
        //write whole in EEPROM

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_THER_DATA, sizeof(BLETHER_THER_DATA), (UINT8 *)&(therAppState->blether_ther_data));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM THER write:%04x", TVF_D(writtenbyte));

        therAppState->blether_measurement_done = 2; //this will prevent multiple writing
        //therAppState->blether_measurement_done = 0; //this will permit multiple measurement
        therAppState->blether_indication_defer = 1;
    }


    //handling deferred indication
    if (therAppState->blether_indication_enable && therAppState->blether_indication_defer)
    {
        //load THER data from NVRAM and send
        if (bleprofile_ReadNVRAM(VS_BLE_THER_DATA, sizeof(BLETHER_THER_DATA), (UINT8 *)&(therAppState->blether_ther_data)))
        {
            int i = 0;

            //write partial based on flag
            // FLAG(1), MEASUREMENT(4) - 5 bytes are mandatory
            memcpy(db_pdu.pdu, (char *)&(therAppState->blether_ther_data), 5);
            i = 5;

            //Optional item
            if (therAppState->blether_ther_data.flag & THER_TIME_STAMP)
            {
                memcpy(&(db_pdu.pdu[i]), therAppState->blether_ther_data.timestamp, 7);
                i+=7;
            }
            if (therAppState->blether_ther_data.flag & THER_TEMPERATURE_TYPE)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(therAppState->blether_ther_data.temptype), 1);
                i+=1;
            }


            db_pdu.len = i;
            bleprofile_WriteHandle(therAppState->blether_ther_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            //check client char cfg
            bleprofile_ReadHandle(therAppState->blether_ther_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            // this is the first indication
            if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
            {
                if (therAppState->blether_indication_sent == 0)
                {
                    bleprofile_sendIndication(therAppState->blether_ther_hdl,
                            (UINT8 *)db_pdu.pdu, db_pdu.len,blether_IndicationConf);
                    therAppState->blether_indication_sent = 1;
                }
                else
                {
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "No Confirmation blocks Indication", TVF_D(0));
                }
            }
        }

        therAppState->blether_indication_defer = 0;
    }
}

void blether_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;
    //load handle number

    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEALTH_THERMOMETER &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT)
        {
            therAppState->blether_ther_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_ther_hdl:%04x", TVF_D(therAppState->blether_ther_hdl));
            bleprofile_ReadHandle(therAppState->blether_ther_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            therAppState->blether_ther_client_hdl = bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_ther_client_hdl:%04x", TVF_D(therAppState->blether_ther_client_hdl));
            bleprofile_ReadHandle(therAppState->blether_ther_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEALTH_THERMOMETER &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_TEMPERATURE_TYPE)
        {
            therAppState->blether_tt_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_tt_hdl:%04x", TVF_D(therAppState->blether_tt_hdl));
            bleprofile_ReadHandle(therAppState->blether_tt_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEALTH_THERMOMETER &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE)
        {
            therAppState->blether_it_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_it_hdl:%04x", TVF_D(therAppState->blether_it_hdl));
            bleprofile_ReadHandle(therAppState->blether_it_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            therAppState->blether_it_client_hdl = bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_it_client_hdl:%04x", TVF_D(therAppState->blether_it_client_hdl));
            bleprofile_ReadHandle(therAppState->blether_it_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_HEALTH_THERMOMETER &&
            bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL)
        {
            therAppState->blether_mi_hdl = bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_mi_hdl:%04x", TVF_D(therAppState->blether_mi_hdl));
            bleprofile_ReadHandle(therAppState->blether_mi_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
            therAppState->blether_mi = db_pdu.pdu[0] + (db_pdu.pdu[1]<<8);

            therAppState->blether_mi_client_hdl = bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_mi_client_hdl:%04x", TVF_D(therAppState->blether_mi_client_hdl));
            bleprofile_ReadHandle(therAppState->blether_mi_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            therAppState->blether_mi_range_hdl = bleprofile_p_cfg->hdl[i]+2; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_mi_range_hdl:%04x", TVF_D(therAppState->blether_mi_range_hdl));
            bleprofile_ReadHandle(therAppState->blether_mi_range_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                 bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            therAppState->blether_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(therAppState->blether_ther_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif


}

void blether_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    therAppState->blether_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(therAppState->blether_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(therAppState->blether_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_connUp[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(therAppState->blether_remote_addr[5], therAppState->blether_remote_addr[4],
                   therAppState->blether_remote_addr[3], therAppState->blether_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_connUp[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(therAppState->blether_remote_addr[1], therAppState->blether_remote_addr[0],
                therAppState->blether_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
    {
        if ((therAppState->blether_hostinfo.serv[0] == UUID_SERVICE_HEALTH_THERMOMETER) &&
            (therAppState->blether_hostinfo.cha[0] == UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = therAppState->blether_hostinfo.cli_cha_desc[0] & 0xFF;
            db_cl_pdu.pdu[1] = therAppState->blether_hostinfo.cli_cha_desc[0] >>8;
        }
    }

    //reset client char cfg
    if (therAppState->blether_ther_client_hdl)
    {
        bleprofile_WriteHandle(therAppState->blether_ther_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
    {
        if ((therAppState->blether_hostinfo.serv[1] == UUID_SERVICE_HEALTH_THERMOMETER) &&
            (therAppState->blether_hostinfo.cha[1] == UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE))
        {
            db_cl_pdu.pdu[0] = therAppState->blether_hostinfo.cli_cha_desc[1] & 0xFF;
            db_cl_pdu.pdu[1] = therAppState->blether_hostinfo.cli_cha_desc[1] >>8;
        }
    }

    //reset client char cfg
    if (therAppState->blether_it_client_hdl)
    {
        bleprofile_WriteHandle(therAppState->blether_it_client_hdl, &db_cl_pdu);
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
    if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
    {
        if ((therAppState->blether_hostinfo.serv[2] == UUID_SERVICE_HEALTH_THERMOMETER) &&
            (therAppState->blether_hostinfo.cha[2] == UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL))
        {
            db_cl_pdu.pdu[0] = therAppState->blether_hostinfo.cli_cha_desc[2] & 0xFF;
            db_cl_pdu.pdu[1] = therAppState->blether_hostinfo.cli_cha_desc[2] >>8;
        }
    }

    //reset client char cfg
    if (therAppState->blether_mi_client_hdl)
    {
        bleprofile_WriteHandle(therAppState->blether_mi_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    if (bleprofile_p_cfg->encr_required == 0)
    {
        therAppState->blether_indication_enable = 1; //indication enable
        therAppState->blether_indication_defer = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether, indication on", TVF_D(0));
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blether_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_connDown[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(therAppState->blether_remote_addr[5], therAppState->blether_remote_addr[4],
                   therAppState->blether_remote_addr[3], therAppState->blether_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether_connDown[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(therAppState->blether_remote_addr[1], therAppState->blether_remote_addr[0],
                therAppState->blether_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

    // Save client characteristic descriptor to NVRAM
    if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
        UINT8 writtenbyte;

        if (therAppState->blether_ther_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_ther_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[0] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[0] = UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT;
            therAppState->blether_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (therAppState->blether_it_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_it_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[1] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[1] = UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE;
            therAppState->blether_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (therAppState->blether_mi_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_mi_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[2] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[2] = UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL;
            therAppState->blether_hostinfo.cli_cha_desc[2] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        {
            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
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
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, therAppState->blether_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2]: %02x %02x %02x %02x",
              TVF_BBBB(therAppState->blether_hostinfo.bdAddr[5], therAppState->blether_hostinfo.bdAddr[4],
                       therAppState->blether_hostinfo.bdAddr[3], therAppState->blether_hostinfo.bdAddr[2] ));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x, handle: %04x",
              TVF_BBW(therAppState->blether_hostinfo.bdAddr[1], therAppState->blether_hostinfo.bdAddr[0],
                      therAppState->blether_con_handle));
    }

    therAppState->blether_con_handle        = 0; //reset connection handle
    therAppState->blether_indication_enable = 0; //notification enable
    therAppState->blether_indication_defer  = 0;
    therAppState->blether_indication_sent   = 0;
    therAppState->blether_measurement_done  = 0;
}

void blether_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV stop!!!!", TVF_D(0));
}

void blether_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (therAppState->blether_apptimer_count)++;

                blether_Timeout(therAppState->blether_apptimer_count);
            }
            break;

    }
}

void blether_appFineTimerCb(UINT32 arg)
{
    (therAppState->blether_appfinetimer_count)++;

    blether_FineTimeout(therAppState->blether_appfinetimer_count);
}


void blether_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 writtenbyte;
        BLEPROFILE_DB_PDU db_cl_pdu;

        memcpy(therAppState->blether_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(therAppState->blether_remote_addr));
        memcpy(therAppState->blether_hostinfo.bdAddr, therAppState->blether_remote_addr, sizeof(BD_ADDR));

        if (therAppState->blether_ther_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_ther_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[0]          = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[0]           = UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT;
            therAppState->blether_hostinfo.cli_cha_desc[0]  = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (therAppState->blether_it_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_it_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[1]          = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[1]           = UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE;
            therAppState->blether_hostinfo.cli_cha_desc[1]  = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        if (therAppState->blether_mi_client_hdl)
        {
            bleprofile_ReadHandle(therAppState->blether_mi_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[2]          = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[2]           = UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL;
            therAppState->blether_hostinfo.cli_cha_desc[2]  = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blether_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether, encryption changed", TVF_D(0));

    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo)))
    {
        if (memcmp(therAppState->blether_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6) == 0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x",
                  TVF_BBBB(therAppState->blether_hostinfo.bdAddr[5], therAppState->blether_hostinfo.bdAddr[4],
                       therAppState->blether_hostinfo.bdAddr[3], therAppState->blether_hostinfo.bdAddr[2] ));
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0]: %02x %02x, handle: %04x",
                  TVF_BB(therAppState->blether_hostinfo.bdAddr[1], therAppState->blether_hostinfo.bdAddr[0] ));
        }
    }
    if (bleprofile_p_cfg->encr_required)
    {
        therAppState->blether_indication_enable = 1; //indication enable
        therAppState->blether_indication_defer  = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blether, indication on", TVF_D(0));
    }
}



void blether_IndicationConf(void)
{
    if (therAppState->blether_indication_sent)
    {
        therAppState->blether_indication_sent = 0;
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "THER Wrong Confirmation!!!", TVF_D(0));
        return;
    }

    if ((bleprofile_p_cfg->disc_required) & DISC_AFTER_CONFIRM)
    {
        if (therAppState->blether_con_handle && therAppState->blether_indication_enable && therAppState->blether_measurement_done)
        //Encryption and data measurement is done
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            therAppState->blether_con_handle        = 0;
            therAppState->blether_indication_enable = 0;
            therAppState->blether_indication_defer  = 0;
            therAppState->blether_measurement_done  = 0;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "THER Disconnect after confirm", TVF_D(0));
        }
   }
}

void blether_mi_IndicationConf(void)
{
    if (therAppState->blether_mi_indication_sent)
    {
        therAppState->blether_mi_indication_sent = 0;
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "THER mi:Wrong Confirmation!!!", TVF_D(0));
        return;
    }
}


void blether_transactionTimeout(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "THER ATT timeout", TVF_D(0));

    if ((bleprofile_p_cfg->disc_required) & DISC_ATT_TIMEOUT)
    {
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            therAppState->blether_con_handle        = 0;
            therAppState->blether_indication_enable = 0;
            therAppState->blether_indication_defer  = 0;
            therAppState->blether_measurement_done  = 0;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "THER Disconnect after ATT timeout", TVF_D(0));
        }
    }
}

int blether_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x", TVF_D(handle));

    if (therAppState->blether_ther_client_hdl && handle == therAppState->blether_ther_client_hdl)
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
        {
            BLEPROFILE_DB_PDU   db_cl_pdu;
            UINT8               writtenbyte;

            bleprofile_ReadHandle(therAppState->blether_ther_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[0] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[0] = UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT;
            therAppState->blether_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (therAppState->blether_it_client_hdl && handle == therAppState->blether_it_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(therAppState->blether_it_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_StopConnIdleTimer();
        }
        else
        {
            bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);
        }

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            therAppState->blether_hostinfo.serv[1] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[1] = UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE;
            therAppState->blether_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (therAppState->blether_mi_client_hdl && handle == therAppState->blether_mi_client_hdl)
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(therAppState->blether_remote_addr, therAppState->blether_hostinfo.bdAddr, 6) == 0)
        {
            BLEPROFILE_DB_PDU   db_cl_pdu;
            UINT8               writtenbyte;

            bleprofile_ReadHandle(therAppState->blether_mi_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            therAppState->blether_hostinfo.serv[2] = UUID_SERVICE_HEALTH_THERMOMETER;
            therAppState->blether_hostinfo.cha[2] = UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL;
            therAppState->blether_hostinfo.cli_cha_desc[2] = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLETHER_HOSTINFO), (UINT8 *)&(therAppState->blether_hostinfo));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }
    else if (therAppState->blether_mi_hdl && handle == therAppState->blether_mi_hdl)
    {
        BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;

        bleprofile_ReadHandle(therAppState->blether_mi_hdl, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        therAppState->blether_mi = db_pdu.pdu[0] + (db_pdu.pdu[1]<<8);

        bleprofile_ReadHandle(therAppState->blether_mi_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        // this is the first indication
        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
        {
            if (therAppState->blether_mi_indication_sent == 0)
            {
                bleprofile_sendIndication(therAppState->blether_mi_hdl,
                            (UINT8 *)db_pdu.pdu, db_pdu.len,blether_mi_IndicationConf);
                therAppState->blether_mi_indication_sent = 1;
            }
            else
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "No Confirmation blocks Indication", TVF_D(0));
            }
        }
    }

    return 0;
}
