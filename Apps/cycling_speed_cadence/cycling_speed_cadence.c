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
* BLE Cycling Speed and Cadence profile, service, application
*
* Refer to Bluetooth SIG Cycling Speed and Cadence Profile 1.0 Cycling Speed
* and Cadence Service 1.0 specifications for details.
*
* This sample implements Cycling Speed and Cadence Sensor.  During
* initialization the app registers with LE stack to receive various
* notifications including bonding complete, connection status change
* and peer writes.  When device is successfully bonded application
* saves peer's Bluetooth Device address to the NVRAM.  Bonded device can write
* into client configuration descriptor to receive notifications with CSC
* measurements.  Client configuration descriptors are also save in the
* NVRAM.  When a measurement is received (for example over fake UART), app
* checks if there is an active registration and sends measurement.  App
* also shows logic for processing commands received through the SC control
* point.
*
* Features demonstrated
*  - GATT database and Device configuration initialization
*  - Registration with LE stack for various events
*  - NVRAM read/write operation
*  - Processing control and data from the client
*  - Sending data to the client
*  - Support for connection parameters update
*  - Serializing events
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*/
#include "bleprofile.h"
#include "platform.h"
#include "cycling_speed_cadence.h"
#include "spar_utils.h"
#include "bleappevent.h"

/******************************************************
 *               Function Declarations
 ******************************************************/

static void   blecsc_Timeout( UINT32 count );
static void   blecsc_FineTimeout( UINT32 finecount );
static void   blecsc_handleUART( char *csc_char );
static void   blecsc_DBInit( void );
static void   blecsc_connUp( void );
static void   blecsc_connDown( void );
static void   blecsc_advStop( void );
static void   blecsc_appTimerCb( UINT32 arg );
static void   blecsc_appFineTimerCb( UINT32 arg );
static void   blecsc_smpBondResult( LESMP_PARING_RESULT result );
static void   blecsc_encryptionChanged( HCI_EVT_HDR *evt );
static UINT32 blecsc_RscButton(UINT32 function);
static void   blecsc_IndicationConf( void );
#if 0
static void   blecsc_FakeUART( char *csc_char, UINT32 count ); //This function is for CSC test
static UINT8  blecsc_DataParse( char *data );
#endif
static int    blecsc_writeCb( LEGATTDB_ENTRY_HDR *p );
static int    blecsc_checkClientConfigBeforeCP(void);
static int   blecsc_send_delayed_indication(void* data);
extern void   bleprofile_regAppEvtHandler( BLECM_APP_EVT_ENUM idx, BLECM_NO_PARAM_FUNC func );

/******************************************************
 *               Variables Definitions
 ******************************************************/

// GATT databse
const UINT8 blecsc_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','C','y','c','l','i','n','g',0x00,0x00,0x00,0x00,0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_CYCLING_SPEED_AND_CADENCE_SENSOR),

    // Handle 0x28: Cycling Speed and Cadence
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_CSC),

    // Handle 0x29: characteristic CSC Measurement, handle 0x2a characteristic value
#define HANDLE_CSC_MEASUREMENT_VALUE 0x2a
    CHARACTERISTIC_UUID16 (0x0029, HANDLE_CSC_MEASUREMENT_VALUE, UUID_CHARACTERISTIC_CSC_MEASUREMENT,
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 11),
        0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
#define HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR 0x2b
    CHAR_DESCRIPTOR_UUID16_WRITABLE (HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x2c: characteristic CSC Feature, handle 0x2d characteristic value
    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_CSC_FEATURE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x07,0x00,                  //instantaneous stride length, total distance

    // Handle 0x2e: characteristic Sensor Location, handle 0x2f characteristic value
#define HANDLE_CSC_SENSOR_LOCATION_VALUE 0x2f
    CHARACTERISTIC_UUID16 (0x002e, HANDLE_CSC_SENSOR_LOCATION_VALUE, UUID_CHARACTERISTIC_SENSOR_LOCATION, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x0B,                       //CSC_LOC_CHAINSTAY

    // Because of Apple application problem do not use consecutive handle
    // Handle 0x32: characteristic CSC Control Point, handle 0x33 characteristic value
#define HANDLE_CSC_CONTROL_POINT_VALUE 0x33
    CHARACTERISTIC_UUID16_WRITABLE (0x0032, HANDLE_CSC_CONTROL_POINT_VALUE, UUID_CHARACTERISTIC_CSC_CONTROL_POINT,
                                     LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE,
                                     LEGATTDB_PERM_WRITE_REQ,  19),
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,

    // Handle 0x34: Client Configuration descriptor
#define HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR 0x34
    CHAR_DESCRIPTOR_UUID16_WRITABLE (HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x3d: Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x3e: characteristic Manufacturer Name, handle 0x3f characteristic value
    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x40: characteristic Model Number, handle 0x41 characteristic value
    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '0','0','0','6',0x00,0x00,0x00,0x00,
};

const UINT16 blecsc_db_size = sizeof(blecsc_db_data);

const BLE_PROFILE_CFG blecsc_cfg =
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
    /*.local_name                     =*/ "BLE Cycling",    // [LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_CYCLING_SPEED_SENSOR),0x00,   // [COD_LEN];
    /*.ver                            =*/ "1.00",            //[VERSION_LEN];
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 120,  // second  0-> no timeout
    /*.powersave_timeout              =*/ 5,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x002f, 0x0033, 0x00}, // [HANDLE_NUM_MAX]
    /*.serv                           =*/ {UUID_SERVICE_CSC, UUID_SERVICE_CSC, UUID_SERVICE_CSC, UUID_SERVICE_CSC, 0x00},
    /*.cha                            =*/ {UUID_CHARACTERISTIC_CSC_MEASUREMENT, UUID_CHARACTERISTIC_CSC_FEATURE,
                                           UUID_CHARACTERISTIC_SENSOR_LOCATION, UUID_CHARACTERISTIC_CSC_CONTROL_POINT, 0x00},
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

const BLE_PROFILE_PUART_CFG blecsc_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ 32,       // GPIO pin number 20730A0 module need to use 32 instead
    /*.rxpin      =*/ 33,
};

const BLE_PROFILE_GPIO_CFG blecsc_gpio_cfg =
{
    /*.gpio_pin =*/
    {
    	GPIO_PIN_WP,      // This need to be used to enable/disable NVRAM write protect
    	GPIO_PIN_BUTTON,  // Button GPIO is configured to trigger either direction of interrupt.
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // not used
    },
    /*.gpio_flag =*/
    {
    	GPIO_SETTINGS_WP,
    	GPIO_SETTINGS_BUTTON,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
};


static UINT32  blecsc_apptimer_count       = 0;
static UINT32  blecsc_appfinetimer_count   = 0;
static UINT16  blecsc_con_handle           = 0;
static BD_ADDR blecsc_remote_addr;
static UINT8   blecsc_notification_enable  = 0;
static UINT8   blecsc_measurement_done     = 0;
static UINT8   blecsc_indication_sent      = 0;

//NVRAM save area
static BLECSC_HOSTINFO blecsc_hostinfo;

static BLECSC_CSC_DATA blecsc_csc_data;

static UINT8 blecsc_sup_senloc[CSC_SUPPORTED_SENLOC_MAX] =
{
    CSC_LOC_TOP_OF_SHOE,
    CSC_LOC_IN_SHOE,
    CSC_LOC_HIP,
    CSC_LOC_WHEEL,
    CSC_LOC_LEFT_CRANK,
    CSC_LOC_RIGHT_CRANK,
    CSC_LOC_LEFT_PEDAL,
    CSC_LOC_RIGHT_PEDAL,
    CSC_LOC_HUB,
    CSC_LOC_REAR_DROPOUT,
    CSC_LOC_CHAINSTAY
};

static UINT32 blecsc_wheel_rotations = 0;
static UINT16 blecsc_crank_rotations = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)blecsc_db_data, blecsc_db_size, (void *)&blecsc_cfg,
        (void *)&blecsc_puart_cfg, (void *)&blecsc_gpio_cfg, blecsc_Create);
}

void blecsc_Create(void)
{
    ble_trace0("blecsc_Create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blecsc_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blecsc_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blecsc_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blecsc_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blecsc_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blecsc_smpBondResult);
    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blecsc_writeCb);

    bleprofile_regButtonFunctionCb(blecsc_RscButton);

    //data init
    memset(&blecsc_csc_data, 0x00, sizeof(BLECSC_CSC_DATA));
    blecsc_csc_data.flag = 0;

    bleprofile_regTimerCb(blecsc_appFineTimerCb, blecsc_appTimerCb);
    bleprofile_StartTimer();

    blecsc_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}

void blecsc_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);
    bleprofile_pollPowersave();
}

void blecsc_FineTimeout(UINT32 count)
{
    char csc_char[READ_UART_LEN + 1];

    //ble_trace1("Finetimer:%d", finecount);

    //Reading
    bleprofile_ReadUART(csc_char);
    //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
    //      csc_char[0], csc_char[1], csc_char[2], csc_char[3], csc_char[4], csc_char[5]);

    if (csc_char[0] == 'D' && csc_char[1] == 'D') //download start
    {
        blecm_setFilterEnable(0);
        ble_trace0("CSA_filter disabled");
    }
    else if (csc_char[0] == 'A' && csc_char[1] == 'A') //download start
    {
        blecm_setFilterEnable(1);
        ble_trace0("CSA_filter enabled");

        blecsc_connDown();
    }
    else  //csc main reading
    {
        blecsc_handleUART(csc_char);
    }

    // button control
    bleprofile_ReadButton();
}

#if 0
void blecsc_FakeUART(char *csc_char, UINT32 count)
{
    UINT8 csc;

    //This is for test only
    if ((blecsc_notification_enable == 1) && (csc_char[0] == 0))
    {
        //faking data measurement
        csc = count & 0xFF;

        csc_char[0] = '0';
        csc_char[1] = '0';
        csc_char[2] = ',';

        if (csc >= 100)
        {
            csc_char[3] = '0' + (csc / 100);
            csc_char[4] = '0' + ((csc % 100) / 10);
            csc_char[5] = '0' + (csc % 10);
            csc_char[6] = ',';
        }
        else if (csc >= 10)
        {
            csc_char[3] = '0' + ((csc % 100) / 10);
            csc_char[4] = '0' + (csc % 10);
            csc_char[5] = ',';
        }
        else
        {
            csc_char[3] = '0' + (csc % 10);
            csc_char[4] = ',';
        }
    }

    //ble_trace1("csc=%d", csc);
    //ble_tracen(csc_char, 7);
}
#endif

void blecsc_handleUART(char *csc_char)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;

    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        blecsc_measurement_done = bleprofile_handleUARTCb((UINT8 *)csc_char, (UINT8 *)&blecsc_csc_data);
    }
    else
    {
#if 1
        if (bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            blecsc_csc_data.flag =  CSC_WHEEL_REVOLUTION_DATA_PRESENT | CSC_CRANK_REVOLUTION_DATA_PRESENT;

            blecsc_wheel_rotations += (blecsc_appfinetimer_count & 0x07);
            blecsc_csc_data.cumulative_wheel_revolutions = blecsc_wheel_rotations;

            blecsc_csc_data.last_wheel_event_time =
                                1024 * blecsc_appfinetimer_count * bleprofile_p_cfg->fine_timer_interval / 1000;

            blecsc_crank_rotations += (blecsc_appfinetimer_count & 0x03);
            blecsc_csc_data.cumulative_crank_revolutions = blecsc_crank_rotations;

            blecsc_csc_data.last_crank_event_time =
                                1024 * blecsc_appfinetimer_count * bleprofile_p_cfg->fine_timer_interval / 1000;

            blecsc_measurement_done = 1; //New measurement is done
        }
#endif

#if 0
        //Parse CSC data
        csc_len = blecsc_DataParse(csc_char);

        if (csc_len)
        {
            blecsc_csc_data.step = 0;
            //Writing DB with new value
            for (i = 0; i < csc_len; i++)
            {
                blecsc_csc_data.step += (csc_char[i] - '0');

                if (i < csc_len-1)
                {
                    blecsc_csc_data.step *= 10;
                }
            }
            blecsc_csc_data.flag = 0;

            blecsc_measurement_done = 1; //New measurement is done
        }
#endif
    }


    // send Notification
    if (blecsc_notification_enable && blecsc_measurement_done) //if connected and encrypted, old data is sent
    {
        int i = 0;

        // write partial based on flag
        // FLAG(1) - 1 bytes are mandatory
        memcpy(db_pdu.pdu, (UINT8 *)&blecsc_csc_data, 1);
        i = 1;

        // Optional item
        if (blecsc_csc_data.flag & CSC_WHEEL_REVOLUTION_DATA_PRESENT)
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blecsc_csc_data.cumulative_wheel_revolutions, 4);
            i += 4;

            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blecsc_csc_data.last_wheel_event_time, 2);
            i += 2;
        }

        if (blecsc_csc_data.flag & CSC_CRANK_REVOLUTION_DATA_PRESENT)
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blecsc_csc_data.cumulative_crank_revolutions, 2);
            i += 2;

            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blecsc_csc_data.last_crank_event_time, 2);
            i += 2;
        }

        db_pdu.len = i;
        bleprofile_WriteHandle(HANDLE_CSC_MEASUREMENT_VALUE, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        //check client char cfg
        bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if ((db_cl_pdu.len == 2) && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_sendNotification(HANDLE_CSC_MEASUREMENT_VALUE, (UINT8 *)db_pdu.pdu, db_pdu.len);
        }

        blecsc_measurement_done = 0; //enable new measurement
    }
}

#if 0
UINT8 blecsc_DataParse(char *data)
{
    char csc_char[4];
    int i;
    INT8 len;

    len = -1; // 0 : before finding first ',' 1-3: number written in csc_char
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
                csc_char[len] = 0; // null character

                if (len>0)
                {
                    memcpy(data, csc_char, 4); // copy back to original data

                    //ble_trace0(data); //echo
                }

                return len; //return number bytes written
            }
            else if (data[i] >= '0' && data[i] <='9') //number character
            {
                csc_char[len] = data[i];
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

void blecsc_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace1("\rblecsc_csc_hdl:%04x", HANDLE_CSC_MEASUREMENT_VALUE);
    bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_VALUE, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    ble_trace1("\rblecsc_csc_client_hdl:%04x", HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR);
    bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    ble_trace1("\rblecsc_csc_cp_hdl:%04x", HANDLE_CSC_CONTROL_POINT_VALUE);
    bleprofile_ReadHandle(HANDLE_CSC_CONTROL_POINT_VALUE, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    ble_trace1("\rblecsc_csc_cp_client_hdl:%04x", HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR);
    bleprofile_ReadHandle(HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    ble_trace1("\rblecsc_csc_bsl_hdl:%04x", HANDLE_CSC_SENSOR_LOCATION_VALUE);
    bleprofile_ReadHandle(HANDLE_CSC_SENSOR_LOCATION_VALUE, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    //init data with CSC
    bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_VALUE, &db_pdu);
    memcpy(&blecsc_csc_data, db_pdu.pdu, db_pdu.len);
    ble_tracen((char *)(&blecsc_csc_data), db_pdu.len);
}

// process connection up notification
void blecsc_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    blecsc_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(blecsc_remote_addr, (UINT8 *)emconninfo_getPeerAddr(), sizeof(blecsc_remote_addr));

    ble_trace3("\rblecsc_connUp: %08x%04x %d",
               (blecsc_remote_addr[5] << 24) + (blecsc_remote_addr[4] << 16) +
               (blecsc_remote_addr[3] << 8) + blecsc_remote_addr[2],
               (blecsc_remote_addr[1] << 8) + blecsc_remote_addr[0],
               blecsc_con_handle);

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);

    //using default value first
    db_cl_pdu.len    = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(blecsc_remote_addr, blecsc_hostinfo.bdAddr, 6) == 0)
    {
        if ((blecsc_hostinfo.serv[0] == UUID_SERVICE_CSC) &&
            (blecsc_hostinfo.cha[0]  == UUID_CHARACTERISTIC_CSC_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = blecsc_hostinfo.cli_cha_desc[0] & 0xFF;
            db_cl_pdu.pdu[1] = blecsc_hostinfo.cli_cha_desc[0] >>8;
        }
    }

    //reset client char cfg
    bleprofile_WriteHandle(HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if (memcmp(blecsc_remote_addr, blecsc_hostinfo.bdAddr, 6) == 0)
    {
        if ((blecsc_hostinfo.serv[1] == UUID_SERVICE_CSC) &&
            (blecsc_hostinfo.cha[1]  == UUID_CHARACTERISTIC_CSC_CONTROL_POINT))
        {
            db_cl_pdu.pdu[0] = blecsc_hostinfo.cli_cha_desc[1] & 0xFF;
            db_cl_pdu.pdu[1] = blecsc_hostinfo.cli_cha_desc[1] >> 8;
        }
    }

    //reset client char cfg
    bleprofile_WriteHandle(HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

    // If encryption is not required, we are ready to send notifications.
    // Otherwise we will need to wait until encryption is set.
    if (bleprofile_p_cfg->encr_required == 0)
    {
        blecsc_notification_enable = 1;             //notification enable
        ble_trace0("blecsc, notification on\n");
    }

    // One connection at a time.  Set advertisement state to not discoverable.
    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

// process connection down event
void blecsc_connDown(void)
{
    ble_trace3("\rblecsc_connDown: %08x%04x %d",
               (blecsc_remote_addr[5] << 24) + (blecsc_remote_addr[4] << 16) +
               (blecsc_remote_addr[3] << 8) + blecsc_remote_addr[2],
               (blecsc_remote_addr[1] << 8) + blecsc_remote_addr[0],
               blecsc_con_handle);

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);

    // Save client characteristic descriptors to NVRAM
    if (memcmp(blecsc_remote_addr, blecsc_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
        UINT8 writtenbyte;

        bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        blecsc_hostinfo.serv[0]         = UUID_SERVICE_CSC;
        blecsc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_CSC_MEASUREMENT;
        blecsc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

        bleprofile_ReadHandle(HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        blecsc_hostinfo.serv[1]         = UUID_SERVICE_CSC;
        blecsc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_CSC_CONTROL_POINT;
        blecsc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);
        ble_trace1("NVRAM write:%04x", writtenbyte);
    }

    bleprofile_Discoverable(bleprofile_p_cfg->default_adv, blecsc_hostinfo.bdAddr);

    ble_trace3("\rADV start: %08x%04x %d",
                    (blecsc_hostinfo.bdAddr[5] << 24) + (blecsc_hostinfo.bdAddr[4] << 16) +
                    (blecsc_hostinfo.bdAddr[3] << 8) + blecsc_hostinfo.bdAddr[2],
                    (blecsc_hostinfo.bdAddr[1] << 8) + blecsc_hostinfo.bdAddr[0],
                    blecsc_con_handle);

    blecsc_con_handle           = 0; //reset connection handle
    blecsc_notification_enable  = 0; //notification enable
    blecsc_measurement_done     = 0;
}

void blecsc_advStop(void)
{
    ble_trace0("ADV stop!!!!");
}

void blecsc_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                blecsc_apptimer_count++;

                blecsc_Timeout(blecsc_apptimer_count);
            }
            break;

    }
}

void blecsc_appFineTimerCb(UINT32 arg)
{
    blecsc_appfinetimer_count++;

    blecsc_FineTimeout(blecsc_appfinetimer_count);
}

void blecsc_smpBondResult(LESMP_PARING_RESULT  result)
{
    ble_trace1("blecsc, bond result %02x\n", result);

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
         // saving bd_addr in nvram
        UINT8 *bda;
        UINT8 writtenbyte;

        bda =(UINT8 *)emconninfo_getPeerAddr();

        memcpy(blecsc_hostinfo.bdAddr, bda, sizeof(BD_ADDR));

        blecsc_hostinfo.serv[0]         = UUID_SERVICE_CSC;
        blecsc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_CSC_MEASUREMENT;
        blecsc_hostinfo.cli_cha_desc[0] = 0;

        blecsc_hostinfo.serv[1]         = UUID_SERVICE_CSC;
        blecsc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_CSC_CONTROL_POINT;
        blecsc_hostinfo.cli_cha_desc[1] = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);
        ble_trace1("NVRAM write:%04x\n", writtenbyte);
    }
}

void blecsc_encryptionChanged(HCI_EVT_HDR *evt)
{
    ble_trace0("blecsc, encryption changed\n");

    if (bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo))
    {
        if (memcmp(blecsc_hostinfo.bdAddr, emconninfo_getPeerAddr(), 6) == 0)
        {
            ble_trace2("\rEncOn for Last paired device: %08x%04x",
                      (blecsc_hostinfo.bdAddr[5] << 24) + (blecsc_hostinfo.bdAddr[4] << 16) +
                      (blecsc_hostinfo.bdAddr[3] << 8) + blecsc_hostinfo.bdAddr[2],
                      (blecsc_hostinfo.bdAddr[1] << 8) + blecsc_hostinfo.bdAddr[0]);
        }
    }

    if (bleprofile_p_cfg->encr_required)
    {
        blecsc_notification_enable = 1; //notification enable

        ble_trace0("blecsc, notification on\n");
    }
}

UINT32 blecsc_RscButton(UINT32 function)
{
    if (function == BUTTON_DISCOVER)
    {
        ble_trace0("CSC: Discover button reset variable\n");
        blecsc_con_handle           = 0; //reset connection handle
        blecsc_notification_enable  = 0; //notification enable
        blecsc_measurement_done     = 0;
    }
    return 0;
}

// This function is called after LE stack has processed previous event, which was write command from the
// application and sent confirmation for the write.  Now it is safe to send indication.
int blecsc_send_delayed_indication(void* data)
{
	BLEPROFILE_DB_PDU *p_db_pdu = (BLEPROFILE_DB_PDU *)data;

	ble_tracen((char *)p_db_pdu->pdu, p_db_pdu->len);
	bleprofile_sendIndication(HANDLE_CSC_CONTROL_POINT_VALUE, (UINT8 *)p_db_pdu->pdu, p_db_pdu->len, blecsc_IndicationConf);

	// See the note above the return value below.
	cfa_mm_Free(data);

	// Note: The serialized call should return either BLE_APP_EVENT_NO_ACTION or
	// BLE_APP_EVENT_FREE_BUFFER in which case the stack will free data
	// using cfa_mm_Free.
	// The other option in this function would be to return BLE_APP_EVENT_FREE_BUFFER
	// and remove the cfa_mm_Free call above.
	return BLE_APP_EVENT_NO_ACTION;
}

int blecsc_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle  = legattdb_getHandle(p);
    int len        = legattdb_getAttrValueLen(p);
    UINT8 *attrPtr = legattdb_getAttrValue(p);

    ble_trace1("WriteCb: handle %04x\n", handle);

    if (handle == HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR)
    {
        // if client does not want to receive measurements anymore, start idle timer to disconnect,
        // otherwise make sure that timer is not running
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(HANDLE_CSC_MEASUREMENT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);

        // Save client characteristic descriptor to NVRAM
        if (memcmp(blecsc_remote_addr, blecsc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            blecsc_hostinfo.serv[0]         = UUID_SERVICE_CSC;
            blecsc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_CSC_MEASUREMENT;
            blecsc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);
            ble_trace1("NVRAM write:%04x\n", writtenbyte);
        }
    }
    else if (handle == HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);

        // Save client characteristic descriptor to NVRAM
        if (memcmp(blecsc_remote_addr, blecsc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            blecsc_hostinfo.serv[1]         = UUID_SERVICE_CSC;
            blecsc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_CSC_CONTROL_POINT;
            blecsc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLECSC_HOSTINFO), (UINT8 *)&blecsc_hostinfo);
            ble_trace1("NVRAM write:%04x", writtenbyte);
        }
    }
    else if (handle == HANDLE_CSC_CONTROL_POINT_VALUE)
    {
        BLECSC_CP_HDR       *cpHdr = (BLECSC_CP_HDR *) attrPtr ;
        BLEPROFILE_DB_PDU   db_pdu;
        BLECSC_CP_RSP_HDR   *cprspHdr = (BLECSC_CP_RSP_HDR *)(db_pdu.pdu);
        UINT8               response;
        UINT8               length = 3; //default is 3

        if (blecsc_checkClientConfigBeforeCP())
        {
            if (cpHdr->opcode == CSC_CP_SET_CUMULATIVE_VALUE)
            {
                // this is set cumulative value procedure
                if (len < 5)
                {
                    response = CSC_CP_INVALID_PARAMETER;
                }
                else
                {
                    blecsc_wheel_rotations = cpHdr->parameter[0] + (cpHdr->parameter[1] << 8) + (cpHdr->parameter[2] << 16) + (cpHdr->parameter[3] << 24);

                    response = CSC_CP_SUCCESS;
                }
            }
            else if (cpHdr->opcode == CSC_CP_START_SENSOR_CALIBRATION)
            {
                // This may time consuming job when the real sensor is used.
                response = CSC_CP_OPCODE_NOT_SUPPORTED; //CSC_CP_SUCCESS;
            }
            else if (cpHdr->opcode == CSC_CP_UPDATE_SENSOR_LOCATION)
            {
                //check location value
                if ((cpHdr-> parameter[0] >= CSC_LOC_START) && (cpHdr-> parameter[0] <= CSC_LOC_END))
                {
                    //write GATT DB with changed value
                    BLEPROFILE_DB_PDU db_pdu_bsc;
                    db_pdu_bsc.pdu[0] = cpHdr-> parameter[0];
                    db_pdu_bsc.len = 1;

                    bleprofile_WriteHandle(HANDLE_CSC_SENSOR_LOCATION_VALUE, &db_pdu_bsc);
                    ble_tracen((char *)db_pdu_bsc.pdu, db_pdu_bsc.len);

                    response = CSC_CP_SUCCESS;
                }
                else
                {
                    response = CSC_CP_INVALID_PARAMETER;
                }
            }
            else if (cpHdr->opcode == CSC_CP_REQUEST_SUPPORTED_SENSOR_LOCATIONS)
            {
                // support all cases
                int i;

                for (i = 0; i < CSC_SUPPORTED_SENLOC_MAX; i++)
                {
                    cprspHdr->response_parameter[i] = blecsc_sup_senloc[i];
                    length++;
                }
                response = CSC_CP_SUCCESS;
            }
            else
            {
                // error response
                response = CSC_CP_OPCODE_NOT_SUPPORTED;
            }

            // client char cfg is already checked

            cprspHdr->opcode         = CSC_CP_RESPONSE_CODE;
            cprspHdr->request_opcode = cpHdr->opcode;
            cprspHdr->response_value = response;

            if (blecsc_indication_sent == 0)
            {
            	// some clients do not like if we send indication before write confirm.
            	// delay sending indication for later
            	BLEPROFILE_DB_PDU *p_db_pdu = (BLEPROFILE_DB_PDU *)cfa_mm_Alloc(sizeof(BLEPROFILE_DB_PDU));
                if (p_db_pdu != NULL)
                {
                    blecsc_indication_sent = 1;
					p_db_pdu->len = length < sizeof (p_db_pdu->pdu) ? length : sizeof (p_db_pdu->pdu);
					memcpy(p_db_pdu->pdu, cprspHdr, p_db_pdu->len);
					bleappevt_serialize(blecsc_send_delayed_indication, p_db_pdu);
                }
            }
            else
            {
                ble_trace0("No Confirmation, Indication not sent");
                return BLECSC_CP_PROCEDURE_ALREADY_IN_PROGRESS;
            }
        }
        else
        {
            return BLECSC_CP_CLIENT_CHAR_CONF_IMPROPERLY;
        }
    }

    return 0;
}

int blecsc_checkClientConfigBeforeCP(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;
    bleprofile_ReadHandle (HANDLE_CSC_CONTROL_POINT_CLIENT_CONFIGURATION_DESCRIPTOR, &db_cl_pdu);
    ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

    if ((db_cl_pdu.len == 2) && (db_cl_pdu.pdu[0] & CCC_INDICATION))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Process Indication Confirm message from the client
void blecsc_IndicationConf(void)
{
    if (blecsc_indication_sent)
    {
        blecsc_indication_sent = 0;
    }
    else
    {
        ble_trace0("WS Wrong Confirmation!!!");
        return;
    }
}


