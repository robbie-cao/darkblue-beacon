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
* This file implements the Glucose profile, service, application.
*
* Refer to Bluetooth SIG Glucose Profile 1.0. and Glucose Service
* 1.0 specifications for details.
*
* The file implements a sample Glucose sensor.
* During initialization the app registers with LE stack
* to receive various notifications including bonding complete, connection
* status change and peer write.  When device is successfully bonded
* application saves peer's Bluetooth Device address to the NVRAM.  Bonded
* device can write into client configuration descriptor for glucose 
* measurement characteristic to receive notification or indication when 
* data is received.  That information is also save in the NVRAM.
* Simulation allows to send indication or notification on timer or GPIO
* triggering.
*
*/
#include "blebgm.h"
#include "lesmpkeys.h"
#include "gpiodriver.h"
#include "dbfw_app.h"

//#include "blebgmspar.h" //only for spar application

//#include "uart.h"

#define FID    FID_BLEAPP_APP__BLEBGM_C

// This is compilation flag enables a dummy database to be used
// for IOP testing purposes. When the real glucose meter functionality are
// available, it should be disabled.
#define BLEBGM_IOP_ADVANCE_DB

#ifdef BLEBGM_IOP_ADVANCE_DB_BIG
#ifdef BLE_P1
#define BLEBGM_IOP_ADVANCE_DB_SIZE  20 //Flash can support 500 //10
#else
#define BLEBGM_IOP_ADVANCE_DB_SIZE  200 //Flash can support 500 //10
#endif
#else
#define BLEBGM_IOP_ADVANCE_DB_SIZE  10
#endif

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blebgm_Timeout(UINT32 count);
void blebgm_FineTimeout(UINT32 finecount);

void blebgm_DBInit(void);
void blebgm_connUp(void);
void blebgm_connDown(void);
void blebgm_advStop(void);
void blebgm_appTimerCb(UINT32 arg);
void blebgm_appFineTimerCb(UINT32 arg);
void blebgm_smpBondResult(LESMP_PARING_RESULT  result);
void blebgm_encryptionChanged(HCI_EVT_HDR *evt);
void blebgm_IndicationConf(void);
void blebgm_transactionTimeout(void);


int blebgm_writeCb(LEGATTDB_ENTRY_HDR *p);

void blebgm_PUARTInitTxWaterlevel(UINT8 puart_tx_waterlevel);
void blebgm_PUARTSetTxWaterlevel(UINT8 puart_tx_waterlevel);
int blebgm_PUARTRxMaxFail(char *data, UINT8 len, UINT8 max);

void blebgm_handleRACPStates(void);
void blebgm_reportRecordNumber(BLEPROFILE_DB_PDU  *);
void blebgm_reportRecords(BLEPROFILE_DB_PDU  *);
void blebgm_clearRecords(BLEPROFILE_DB_PDU  *);
int  blebgm_startMeasurementReport(BLEPROFILE_DB_PDU  *);
int  blebgm_reportNextMeasurement(BLEPROFILE_DB_PDU  *);
int  blebgm_reportMeasurement(BLEBGM_GLUCOSE_MEASUREMENT *ptr);

void blebgm_reportRecordsCont(void);

void blebgm_formMeasurePkt(BLEPROFILE_DB_PDU *pdu,
                                BLEBGM_GLUCOSE_MEASUREMENT *ptr);
void blebgm_formContextIndPkt(BLEPROFILE_DB_PDU *pdu,
                                BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *ptr);

void blebgm_formRACPRecordCntInd(BLEPROFILE_DB_PDU *pdu,UINT8 Operator, UINT16 count);

void blebgm_racpStatusIndication(UINT8 reqCode, UINT8 status);


int blebgm_filterConditionMatched(BLEBGM_GLUCOSE_MEASUREMENT *ptr);

UINT32 blebgm_buttonCb(UINT32 function);


int  blebgm_checkClientConfigBeforeRACP(void);


void blebgm_sendAsManyMeasurements(BLEPROFILE_DB_PDU *);


extern void utilslib_delayUs(UINT32 delay);
extern void wdog_restart(void);

void                blebgm_loadAppData(void);
int                 blebgm_lookupAppData(UINT8 *adr, UINT8 adrType);
void                blebgm_clearAllConf(int idx);
int                 blebgm_addHost(UINT8 *adr, UINT8 adrType);
BLEBGM_APP_STORAGE  *blebgm_getHostData(UINT8 *adr, UINT8 adrType);
BLEBGM_APP_STORAGE  *blebgm_getCurHostData(void);

void                blebgm_clientConfFlagToCheckAndStore(UINT16 flag, UINT16 flagToCheck, UINT16 flagToStore);

int                 blebgm_setupTargetAdrInScanRsp(void);

#ifdef BLEBGM_IOP_ADVANCE_DB
void                blebgm_create_iopdb(void);
#endif


#ifdef BLE_CONNECTION_PARAMETER_UPDATE
#include "lel2cap.h"

#define GAP_CONN_PARAM_TIMEOUT 30
int blebgmhandleConnParamUpdateRsp(LEL2CAP_HDR *l2capHdr);

typedef int (*LEL2CAP_MSGHANDLER)(LEL2CAP_HDR*);
extern LEL2CAP_MSGHANDLER lel2cap_handleConnParamUpdateRsp;
#endif

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blebgm_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 4), 
        0x00, 0x00, 0x00, 0x00,

    // GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),
    
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','G','l','u','c','o','s','e','m','e','t','e','r',    // "BLE Glucosemeter"

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8 (APPEARANCE_GENERIC_GLUCOSE_METER),

    // Device Info service
    PRIMARY_SERVICE_UUID16 (0x002d, UUID_SERVICE_DEVICE_INFORMATION),

    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    CHARACTERISTIC_UUID16 (0x0030, 0x0031, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0032, 0x0033, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7,

    // Primary service GLUCOSE METER.
    PRIMARY_SERVICE_UUID16 (0x0100, UUID_SERVICE_GLUECOSE_CONCENTRATION),

    CHARACTERISTIC_UUID16 (0x0101, 0x0102, UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 3),  
        0x00,0x00,0x00,
    
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0103, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0104, 0x0105, UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT_CONTEXT, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE, 3),  
        0x00,0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0106, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

#ifdef BLE_AUTH_WRITE       
    CHARACTERISTIC_UUID16_WRITABLE (0x0107, 0x0108, UUID_CHARACTERISTIC_GLUCOSE_RACP,
                           LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE, 
                           LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_READABLE | 
                           LEGATTDB_PERM_AUTH_WRITABLE | LEGATTDB_PERM_VARIABLE_LENGTH, 18),
#else
    CHARACTERISTIC_UUID16_WRITABLE (0x0107, 0x0108, UUID_CHARACTERISTIC_GLUCOSE_RACP,
                           LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE, 
                           LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_READABLE | LEGATTDB_PERM_VARIABLE_LENGTH, 18),
#endif
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
        0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0109, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    CHARACTERISTIC_UUID16 (0x010a, 0x010b, UUID_CHARACTERISTIC_GLUCOSE_FEATURES, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),  
        0x00,0x04                  // features that we support.
};

const UINT16 blebgm_db_size = sizeof(blebgm_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blebgm_cfg =
{
    /* .fine_timer_interval            =*/ 100, // UINT16 ; //ms
    /* .default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /* .button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /* .high_undirect_adv_interval     =*/ 32,   // slots
    /* .low_undirect_adv_interval      =*/ 2048, // slots
    /* .high_undirect_adv_duration     =*/ 30,   // seconds
    /* .low_undirect_adv_duration      =*/ 300,  // seconds
    /* .high_direct_adv_interval       =*/ 0,    // seconds
    /* .low_direct_adv_interval        =*/ 0,    // seconds
    /* .high_direct_adv_duration       =*/ 0,    // seconds
    /* .low_direct_adv_duration        =*/ 0,    // seconds
    /* .local_name                     =*/ "BLE Glucosemeter",   // [LOCAL_NAME_LEN_MAX];
    /* .cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_GLUCOSE_METER), 0x00, // [COD_LEN];
    /* .ver                            =*/ "1.00",               // [VERSION_LEN];
#ifdef BLE_SECURITY_REQUEST
    /* .encr_required                  =*/ SECURITY_ENABLED|SECURITY_REQUEST,
#else
    /* .encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
#endif
    /* .disc_required                  =*/ 1,    // if 1, disconnection after confirmation
    /* .test_enable                    =*/ 1,    // if 1 TEST MODE is enabled 
    /* .tx_power_level                 =*/ 0x04, // dbm
    /* .con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /* .powersave_timeout              =*/ 0,    // second  0-> no timeout
    /* .hdl                            =*/ {0x102, 0x105, 0x108, 0x10b, 0x00},                                               // GATT HANDLE number
    /* .serv                           =*/ {UUID_SERVICE_GLUECOSE_CONCENTRATION, UUID_SERVICE_GLUECOSE_CONCENTRATION,
                                            UUID_SERVICE_GLUECOSE_CONCENTRATION, UUID_SERVICE_GLUECOSE_CONCENTRATION, 0x00}, // GATT service UUID
    /* .cha                            =*/ {UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT, UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT_CONTEXT,
                                            UUID_CHARACTERISTIC_GLUCOSE_RACP, UUID_CHARACTERISTIC_GLUCOSE_FEATURES, 0x00},   // GATT characteristic UUID
    /* .findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /* .findme_alert_level             =*/ 0,    // alert level of find me
    /* .client_grouptype_enable        =*/ 0,    // if 1 grouptype read can be used
    /* .linkloss_button_enable         =*/ 0,    // if 1 linkloss button is enable
    /* .pathloss_check_interval        =*/ 0,    // second
    /* .alert_interval                 =*/ 0,    // interval of alert
    /* .high_alert_num                 =*/ 0,    // number of alert for each interval
    /* .mild_alert_num                 =*/ 0,    // number of alert for each interval
    /* .status_led_enable              =*/ 0,    // if 1 status LED is enable
    /* .status_led_interval            =*/ 0,    // second
    /* .status_led_con_blink           =*/ 0,    // blink num of connection
    /* .status_led_dir_adv_blink       =*/ 0,    // blink num of dir adv
    /* .status_led_un_adv_blink        =*/ 0,    // blink num of undir adv
    /* .led_on_ms                      =*/ 0,    // led blink on duration in ms
    /* .led_off_ms                     =*/ 0,    // led blink off duration in ms
    /* .buz_on_ms                      =*/ 0,    // buzzer on duration in ms
    /* .button_power_timeout           =*/ 0,    // seconds
    /* .button_client_timeout          =*/ 1,    // seconds
    /* .button_discover_timeout        =*/ 3,    // seconds
    /* .button_filter_timeout          =*/ 0,    // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_timeout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blebgm_puart_cfg =
{
    /* .baudrate   =*/ 115200,
    /* .txpin      =*/ 24,     // 20730A0 module need to use 32 instead, normally it is 31
    /* .rxpin      =*/ 25,     // GPIO pin number
};

#ifdef BLE_P1
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blebgm_gpio_cfg =
{
    {1, 14, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON1|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON2|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON3|GPIO_INT,
      GPIO_OUTPUT|GPIO_INIT_LOW,
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
#elif defined(BLE_P2)
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blebgm_gpio_cfg =
{
    {1, 0, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON1|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON2|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON3|GPIO_INT,
      GPIO_OUTPUT|GPIO_INIT_LOW,
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
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blebgm_gpio_cfg =
{
    {31, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON1|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON2|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON3|GPIO_INT,
      GPIO_OUTPUT|GPIO_INIT_LOW,
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


#ifdef BLEBGM_IOP_ADVANCE_DB
UINT16 blebgm_iop_advance_db_size = BLEBGM_IOP_ADVANCE_DB_SIZE;

//use ROM variable

//Flag, bit[0] -> time offset
//      bit[1] -> Glucose Concentration and Type Location
//      bit[2] -> Glucose Unit, 0 means mg/dL, 1 means  mmol/L
//      bit[3] -> Sensor Status Annunciation.
//      bit[4] -> Context Information
PLACE_IN_DROM const BLEBGM_GLUCOSE_MEASUREMENT iopData [10] =
{
    {
        0x01,   // flag, Time offset
        0x01,   // sequence number.
        0xdb,0x07,0x09,0x0a,0x06,0x00,0x00, // timestamp
        0x0000, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x00,   // flag, no time offset.
        0x02,   // sequence number.
        0xdb,0x07,0x09,0x0a,0x12,0x00,0x00, // timestamp
        0x0000, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x03,   // flag
        0x03,   // sequence number.
        0xdb,0x07,0x09,0x0b,0x06,0x00,0x00, // timestamp
        0x0000, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x1b,   // flag
        0x04,   // sequence number.
        0xdb,0x07,0x09,0x0b,0x12,0x00,0x00, // timestamp
        0x003c, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x04,   // flag
        0x05,   // sequence number.
        0xdb,0x07,0x09,0x0c,0x06,0x00,0x00, // timestamp
        0x003c, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x05,   // flag
        0x06,   // sequence number.
        0xdb,0x07,0x09,0x0c,0x12,0x00,0x00, // timestamp
        0x003c, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x1b,   // flag
        0x07,   // sequence number.
        0xdb,0x07,0x09,0x0d,0x06,0x00,0x00, // timestamp
        0xffc4, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x06,   // flag
        0x08,   // sequence number.
        0xdb,0x07,0x09,0x0d,0x12,0x00,0x00, // timestamp
        0xffc4, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x1a,   // flag
        0x09,   // sequence number.
        0xdb,0x07,0x09,0x0e,0x06,0x00,0x00, // timestamp
        0xffc4, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    },
    {
        0x1a,   // flag
        0x0a,   // sequence number.
        0xdb,0x07,0x09,0x0e,0x12,0x00,0x00, // timestamp
        0xffc4, // timeOffset.
        0xb010, // glucose concentration.
        0x00, // type sample location.
        0x00, // sensor status.
    }
};

// Flags.
// bit[0] -> Carbohydrates ID
// bit[1] -> MealFlag
// bit[2] -> Tester-Health
// bit[3] -> Excercise Duration and intensity
// bit[4] -> Medication Flag
// bit[5] -> HbA1c Flag
// bit[6] -> Extended Flags.
//
PLACE_IN_DROM const BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT
iopContextData[10] =
{
    {
        0xff,   // flags.
        0x0001, // seqNum.
        0x00,   // extendedFlags.
        0x01,   // carb ID.
        0x0005, // carb
        0x03,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0002, // seqNum.
        0x00,   // extendedFlags.
        0x03,   // carb ID.
        0x0005, // carb
        0x01,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0003, // seqNum.
        0x00,   // extendedFlags.
        0x01,   // carb ID.
        0x0005, // carb
        0x03,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0004, // seqNum.
        0x00,   // extendedFlags.
        0x03,   // carb ID.
        0x0005, // carb
        0x01,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0005, // seqNum.
        0x00,   // extendedFlags.
        0x01,   // carb ID.
        0x0005, // carb
        0x03,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0006, // seqNum.
        0x00,   // extendedFlags.
        0x03,   // carb ID.
        0x0005, // carb
        0x01,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0007, // seqNum.
        0x00,   // extendedFlags.
        0x01,   // carb ID.
        0x0005, // carb
        0x03,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0008, // seqNum.
        0x00,   // extendedFlags.
        0x03,   // carb ID.
        0x0005, // carb
        0x01,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x0009, // seqNum.
        0x00,   // extendedFlags.
        0x01,   // carb ID.
        0x0005, // carb
        0x03,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    },
    {
        0xff,   // flags.
        0x000a, // seqNum.
        0x00,   // extendedFlags.
        0x03,   // carb ID.
        0x0005, // carb
        0x01,   // meal.
        0x51,   // tester health.
        0x0000, // exercise duration
        0x00,   // exercise intensity.
        0x01,   // medication ID.
        0x0002, // medication.
        0x0008, // HbA1c
    }
};
#endif

int blebgm_iopDataEntries = 10;
typedef struct
{
    // NVRAM storage for application.
    BLEBGM_APP_STORAGE blebgm_appStorage[BLEBGM_MAX_BONDED_HOST];

#ifdef BLEBGM_IOP_ADVANCE_DB
    BLEBGM_GLUCOSE_MEASUREMENT *blebgm_iopData;
    BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *blebgm_iopContextData;
    int *blebgm_iopDataValid;
#endif

    // current operation.
    int blebgm_racpOperation;
    // current operator.
    int blebgm_racpOperator;
    // current filter type.
    int blebgm_racpFilterType;

    UINT32 blebgm_racpPktFormatStatus;



    // this is the currently reporting sequence number.
    UINT16 blebgm_racpCurSeqNum;
    // use max of 7 bytes to hold the time stamp info.
    UINT8 blebgm_racpFilterMin[TIMESTAMP_LEN];
    // use max of 7 bytes to hold the time stamp info.
    UINT8 blebgm_racpFilterMax[TIMESTAMP_LEN];

    // RACP operation related information.
    BLEBGM_RACP_STATE blebgm_racpState;

    BOOL8 blebgm_racpOperationAbort;

    //this is for counting sent notification
    UINT16 blebgm_racpNotificationCnt;



    // This is the RACP handle.
    int blebgm_measurementHandle;
    int blebgm_contextHandle;
    int blebgm_racpHandle;
    int blebgm_featureHandle;

    int blebgm_measurementCCCHandle;
    int blebgm_contextCCCHandle;
    int blebgm_racpCCCHandle;


    // Currently indicated Measurement.
    BLEBGM_GLUCOSE_MEASUREMENT *blebgm_curMeasureRecPtr;



    UINT32 blebgm_apptimer_count;
    UINT32 blebgm_appfinetimer_count;
    BD_ADDR blebgm_remote_addr;

    UINT8 blebgm_bat_enable;
#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    UINT32 blebgm_conparam_timeout;
    UINT8 blebgm_conparam_timeout_enable;
#endif
    UINT8 blebgm_null_operator_nsrr;
} tBgmAppState;

tBgmAppState *bgmAppState = NULL;


//////////////////////////////////////////
//   database access functions.
int blebgm_dbGetRecordCntSinceSeq(int);
int blebgm_dbGetRecordCntSinceTime(TIMESTAMP *startTime);
int blebgm_dbGetRecordCntlessThanSeq(int);
int blebgm_dbGetRecordCntlessThanTime(TIMESTAMP *endTime);
int blebgm_dbGetRecordCntInRange(int , int);
int blebgm_dbGetRecordCntInRangeTime(TIMESTAMP *startTime, TIMESTAMP *endTime);
BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetFirstMeasurement(void);
BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetNextMeasurement(int seqNum);
BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetLastMeasurement(void);
BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetMeasurement(int idx);
BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *blebgm_dbGetContext(UINT16 seqNum);

//
BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetMeasurement(int seqNum);

int blebgm_dbClearRecordRange(int seqNumMin, int seqNumMax);
int blebgm_dbClearRecordRangeTime(TIMESTAMP *TimeMin, TIMESTAMP *TimeMax);
int blebgm_dbClearAllRecords(void);

int blebgm_dbClearLastRecords(void);
int blebgm_dbClearFirstRecords(void);
int blebgm_dbClearLessOrEqualRecords(int seqMin);
int blebgm_dbClearLessOrEqualRecordsTime(TIMESTAMP *TimeMax);
int blebgm_dbClearLargerOrEqualRecords(int seqMax);
int blebgm_dbClearLargerOrEqualRecordsTime(TIMESTAMP *TimeMin);

//Output GPIO
UINT8 blebgm_gpio_output = 4;//should be same as blebgm_gpio_cfg

void (*blebgm_PUART_RxIntUserCb)(void*)=NULL;
UINT8 blebgm_PUART_RxInt_Enable = 0;

UINT8 blebgm_puart_tx_waterlevel=0;

#define PUART_TX_WATERLEVEL  1 //13 // 1

void blebgm_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0], bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[2], bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    bgmAppState = (tBgmAppState *)cfa_mm_Sbrk(sizeof(tBgmAppState));
    memset(bgmAppState, 0x00, sizeof(tBgmAppState));

    //initialize the default value of bgmAppState
    bgmAppState->blebgm_null_operator_nsrr = 1;
    bgmAppState->blebgm_racpState = BLEBGM_RACP_IDLE;

#ifdef BLEBGM_IOP_ADVANCE_DB
    blebgm_create_iopdb();
#endif

#if 0
    // dump the database to debug uart.
    legattdb_dumpDb();
#endif

    //blecm_enableRandomAddressResolution();

    // this would generate the adv payload and scan rsp payload.
    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);


    blebgm_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blebgm_connUp);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blebgm_connDown);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blebgm_advStop);

    // register buttone callback.
    bleprofile_regButtonFunctionCb(blebgm_buttonCb);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blebgm_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blebgm_smpBondResult);

#if 0
    // smp pairing with PIN code
    lesmp_setPairingParam(
             LESMP_IO_CAP_DISP_ONLY,           // IOCapability,
	         LESMP_OOB_AUTH_DATA_NOT_PRESENT,   // OOBDataFlag,
	         LESMP_AUTH_FLAG_BONDING|LESMP_AUTH_REQ_FLAG_MITM, // AuthReq,
	         LESMP_MAX_KEY_SIZE,                // MaxEncKeySize,
             // InitiatorKeyDistrib,
	         LESMP_KEY_DISTRIBUTION_ENC_KEY
             | LESMP_KEY_DISTRIBUTION_ID_KEY
             | LESMP_KEY_DISTRIBUTION_SIGN_KEY,
             // ResponderKeyDistrib
	         LESMP_KEY_DISTRIBUTION_ENC_KEY
             | LESMP_KEY_DISTRIBUTION_ID_KEY
             | LESMP_KEY_DISTRIBUTION_SIGN_KEY);

    lesmp_setJustWorksNotPermitted();

    // 16 bytes key
    {
        UINT8 passKey[16];
	    memset(passKey, 0, 16);
	    passKey[0] = 0x00;
	    passKey[1] = 0x11;

    //    lesmp_setPassKey(passKey, 16);
    }
#endif

#if 1
    {
        extern void lesmp_setReturnInsufficientAuthentication(void);
        lesmp_setReturnInsufficientAuthentication();
    }
#endif

#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blebgm_IndicationConf);
#endif

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    lel2cap_handleConnParamUpdateRsp = blebgmhandleConnParamUpdateRsp;
#endif

    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blebgm_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blebgm_writeCb);

    bleprofile_regTimerCb(blebgm_appFineTimerCb, blebgm_appTimerCb);
    bleprofile_StartTimer();

    // load the Bonded host info.
    blebgm_loadAppData();

    blebgm_connDown();
}

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
int blebgmhandleConnParamUpdateRsp(LEL2CAP_HDR *l2capHdr)
{
    LEL2CAP_COMMAND_HDR *cmdPkt = (LEL2CAP_COMMAND_HDR *) l2capHdr;
    UINT16 *p_result;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdateRsp", TVF_D(0));

    //disable timeout
    bgmAppState->blebgm_conparam_timeout_enable = 0;

    if (cmdPkt->len == 2)
    {
        p_result = (UINT16 *)(cmdPkt+1);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "result = %d", TVF_D(*p_result));

        if (*p_result == 0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Accepted", TVF_D(0));
        }
        else
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Rejected", TVF_D(0));
        }

        return *p_result;
    }

    return -1;
}
#endif

#ifdef BLEBGM_IOP_ADVANCE_DB
void blebgm_create_iopdb(void)
{
    //malloc
    bgmAppState->blebgm_iopData =
        cfa_mm_Sbrk((UINT32)(sizeof(BLEBGM_GLUCOSE_MEASUREMENT)*blebgm_iop_advance_db_size));

    bgmAppState->blebgm_iopContextData =
        cfa_mm_Sbrk((UINT32)(sizeof(BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT)*blebgm_iop_advance_db_size));

    bgmAppState->blebgm_iopDataValid =
        cfa_mm_Sbrk((UINT32)(sizeof(int)*blebgm_iop_advance_db_size));

    {
        extern UINT32 cfa_mm_MemFreeBytes(void);

        // free_ram is the number of bytes free. at this point.
        UINT32 free_ram = cfa_mm_MemFreeBytes();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "FreeRAM=%d", TVF_D(free_ram));
    }

    //copy
    memcpy(bgmAppState->blebgm_iopData, iopData, sizeof(BLEBGM_GLUCOSE_MEASUREMENT)*blebgm_iopDataEntries);
    memcpy(bgmAppState->blebgm_iopContextData, iopContextData, sizeof(BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT)*blebgm_iopDataEntries);
    {
        int i;
        for (i = 0; i < blebgm_iopDataEntries; i++)
        {
            bgmAppState->blebgm_iopDataValid[i] = 1;
        }
    }


    if (blebgm_iopDataEntries != blebgm_iop_advance_db_size)
    {
        int i;

        //add data
        //for (i = blebgm_iopDataEntries; i < blebgm_iop_advance_db_size; i++)
        for (i = blebgm_iopDataEntries; i < blebgm_iop_advance_db_size; i++)
        {
            // this record becomes valid.
            bgmAppState->blebgm_iopDataValid[i]=TRUE;

            //copy contents from others (i mod 10)
            BT_MEMCPY(&(bgmAppState->blebgm_iopData[i]),
                                &(bgmAppState->blebgm_iopData[i%10]),
                                sizeof(BLEBGM_GLUCOSE_MEASUREMENT));

            BT_MEMCPY(&(bgmAppState->blebgm_iopContextData[i]),
                                &(bgmAppState->blebgm_iopContextData[i%10]),
                                sizeof(BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT));

            // increase the sequence number of last record by one.
            bgmAppState->blebgm_iopData[i].seqNum = (bgmAppState->blebgm_iopData[i-1].seqNum)+1;
            bgmAppState->blebgm_iopContextData[i].seqNum = bgmAppState->blebgm_iopData[i].seqNum;

            // copy timestamp from last one
            BT_MEMCPY(&(bgmAppState->blebgm_iopData[i].BaseTime), &(bgmAppState->blebgm_iopData[i-1].BaseTime), sizeof(TIMESTAMP));
            //increase the timestamp
            //YYYYMMDDhhmmss
            bgmAppState->blebgm_iopData[i].BaseTime[3]++;
            if (bgmAppState->blebgm_iopData[i].BaseTime[3] > 28)
            {
                bgmAppState->blebgm_iopData[i].BaseTime[3]=1;
                bgmAppState->blebgm_iopData[i].BaseTime[2]++;

                if (bgmAppState->blebgm_iopData[i].BaseTime[2] > 12)
                {
                    bgmAppState->blebgm_iopData[i].BaseTime[2]=1;
                    bgmAppState->blebgm_iopData[i].BaseTime[0]++;

                    if (bgmAppState->blebgm_iopData[i].BaseTime[0] == 0)
                    {
                        bgmAppState->blebgm_iopData[i].BaseTime[1]++;
                    }
                }
            }
#ifdef BLEBGM_IOP_ADVANCE_DB_SIMULATE_DATA
            //changing data with seq number
            bgmAppState->blebgm_iopData[i].flags = (bgmAppState->blebgm_iopData[i].seqNum)&0x1F; //5bit  0x00-0x1F
            bgmAppState->blebgm_iopData[i].timeOffset = (INT16)(bgmAppState->blebgm_iopData[i].seqNum); //uint16
            bgmAppState->blebgm_iopData[i].glucoseConcentration = (UINT16)(bgmAppState->blebgm_iopData[i].seqNum); //uint16
            bgmAppState->blebgm_iopData[i].glucoseConcentration %= 256;
            if (bgmAppState->blebgm_iopData[i].flags & 0x04)
            {
                //mol/L
                // 10^-3 (0xD)
                bgmAppState->blebgm_iopData[i].glucoseConcentration |= 0xD000;
            }
            else
            {
                //kg/L
                // 10^-5 (0xB)
                bgmAppState->blebgm_iopData[i].glucoseConcentration |= 0xB000;
            }
            bgmAppState->blebgm_iopData[i].typeSampleLocation = (bgmAppState->blebgm_iopData[i].seqNum)&0xFF; //uint8
            bgmAppState->blebgm_iopData[i].sensorStatus = (bgmAppState->blebgm_iopData[i].seqNum)&0x0FFF; //12bit 0x00-0x0FFF
            bgmAppState->blebgm_iopContextData[i].flags = (bgmAppState->blebgm_iopContextData[i].seqNum)&0xFF; // 8bit
            bgmAppState->blebgm_iopContextData[i].carbID = (bgmAppState->blebgm_iopContextData[i].seqNum)&0x07; // 0-7
            bgmAppState->blebgm_iopContextData[i].carb = bgmAppState->blebgm_iopContextData[i].seqNum; // 2 bytes, grams.
            bgmAppState->blebgm_iopContextData[i].meal = (bgmAppState->blebgm_iopContextData[i].seqNum)&0x07; // 0-5
            bgmAppState->blebgm_iopContextData[i].testerHealth = (bgmAppState->blebgm_iopContextData[i].seqNum)&0xFF; // 1 byte.
            bgmAppState->blebgm_iopContextData[i].exerciseDuration = bgmAppState->blebgm_iopContextData[i].seqNum; // 2 byte.
            bgmAppState->blebgm_iopContextData[i].exerciseIntensity = (bgmAppState->blebgm_iopContextData[i].seqNum)&0xFF;// 1 byte.
            bgmAppState->blebgm_iopContextData[i].medicationID = (bgmAppState->blebgm_iopContextData[i].seqNum)&0x07; // 0-5
            bgmAppState->blebgm_iopContextData[i].medication = bgmAppState->blebgm_iopContextData[i].seqNum; // 2 bytes.
            bgmAppState->blebgm_iopContextData[i].HbA1c  = bgmAppState->blebgm_iopContextData[i].seqNum; // 2 bytes.
#endif
        }
        blebgm_iopDataEntries = blebgm_iop_advance_db_size;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_create_iopdb: %d", TVF_D(blebgm_iopDataEntries));
    }
}
#endif

void blebgm_Timeout(UINT32 count)
{
    if (bgmAppState->blebgm_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    #define BT_ERROR_CODE_LE_UNACCEPTABLE_CONNECTION_INTERVAL 0x3B

    //check time out
    if (bgmAppState->blebgm_conparam_timeout_enable &&
       bgmAppState->blebgm_apptimer_count == bgmAppState->blebgm_conparam_timeout)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Disc-conn param TO", TVF_D(0));
        blecm_disconnect(BT_ERROR_CODE_LE_UNACCEPTABLE_CONNECTION_INTERVAL);
        bgmAppState->blebgm_conparam_timeout_enable = 0;
    }

#if 0
    //debug only
    ble_trace3("ConnParam in Timeout: %d, %d, %d",
                emconninfo_getConnInterval(),
                emconninfo_getSlaveLatency(),
                emconninfo_getSupervisionTimeout());
#endif
#endif

#if 0
    //NVRAM test
    {
        extern UINT8 lesmpkeys_maxBondedDevices;
        int i;
        bleprofile_DeleteNVRAM(STACKNVRAM_LOCAL_KEYS);
        bleprofile_DeleteNVRAM(STACKNVRAM_BOND_INDEX);
        for (i = 0; i < lesmpkeys_maxBondedDevices;  i++)
        {
	     bleprofile_DeleteNVRAM(STACKNVRAM_FIRST_BONDED_IDX + i);
        }
    }
#endif

#if 0
    //NVRAM test
    {
        #define NUMBER 10
        UINT8 status;
        LESMPKEYS_LOCAL_ROOT_KEY local_key;
        extern UINT8 lesmpkeys_maxBondedDevices;
        LESMPKEYS_BOND_IDX bondindex[NUMBER];
        LESMPKEYS_BONDED_INFO bondinfo;
        int size = sizeof(LESMPKEYS_BONDED_INFO); //40+count; //40; //sizeof(LESMPKEYS_BONDED_INFO);
        int size2 = (40+count)&0xFF;
        int number = lesmpkeys_maxBondedDevices; //NUMBER; //lesmpkeys_maxBondedDevices;
        int number2 = NUMBER - lesmpkeys_maxBondedDevices;
        int i;

        status = bleprofile_WriteNVRAM(STACKNVRAM_LOCAL_KEYS,
                                sizeof(LESMPKEYS_LOCAL_ROOT_KEY),
                                (UINT8 *)&local_key);
        if (status != sizeof(LESMPKEYS_LOCAL_ROOT_KEY))
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Localkey Write Failed: %d/%d", TVF_WW(status, sizeof(LESMPKEYS_LOCAL_ROOT_KEY)));
        }
        else
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Localkey Write Success: %d", TVF_D(status));
        }

        status = bleprofile_WriteNVRAM(STACKNVRAM_BOND_INDEX ,
                sizeof(LESMPKEYS_BOND_IDX) * lesmpkeys_maxBondedDevices,
                                     (UINT8 *)bondindex);
        if (status != sizeof(LESMPKEYS_BOND_IDX) * lesmpkeys_maxBondedDevices)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Bondindex Write Failed: %d/%d", TVF_WW(status, sizeof(LESMPKEYS_BOND_IDX) * lesmpkeys_maxBondedDevices));
        }
        else
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Bondindex Write Success: %d", TVF_D(status));
        }

        for (i = 0; i < number;  i++)
        {
	     status = bleprofile_WriteNVRAM(STACKNVRAM_FIRST_BONDED_IDX + i ,
	 	                       size,
                                     (UINT8 *)&bondinfo);
            if (status != size)
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Failed: %d/%d", TVF_BBW(i, status, size));
            }
            else
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Success: %d", TVF_BB(i, status));
            }
	 }
        for (i = number; i < number+number2;  i++)
        {
	     status = bleprofile_WriteNVRAM(STACKNVRAM_FIRST_BONDED_IDX + i ,
	 	                       size2,
                                     (UINT8 *)&bondinfo);
            if (status != size2)
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Failed: %d/%d", TVF_BBW(i, status, size2));
            }
            else
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Success: %d", TVF_BB(i, status));
            }
	 }
    }
#endif
}

void blebgm_FineTimeout(UINT32 finecount)
{
    if (bleprofile_p_cfg->test_enable)
    {
        // let the RACP get a chance to do things.
        blebgm_handleRACPStates();
    }

    // button control
    bleprofile_ReadButton();
}

void blebgm_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;

    //load handle number
    for (i = 0; i < HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i]==UUID_SERVICE_GLUECOSE_CONCENTRATION&&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT)
        {
            bgmAppState->blebgm_measurementHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_measurementHandle:%04x", TVF_D(bgmAppState->blebgm_measurementHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_measurementHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bgmAppState->blebgm_measurementCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_measurementCCCHandle:%04x", TVF_D(bgmAppState->blebgm_measurementCCCHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_measurementCCCHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i]==UUID_SERVICE_GLUECOSE_CONCENTRATION&&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT_CONTEXT)
        {
            bgmAppState->blebgm_contextHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_contextHandle:%04x", TVF_D(bgmAppState->blebgm_contextHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_contextHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bgmAppState->blebgm_contextCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_contextCCCHandle:%04x", TVF_D(bgmAppState->blebgm_contextCCCHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_contextCCCHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i]==UUID_SERVICE_GLUECOSE_CONCENTRATION&&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_GLUCOSE_RACP)
        {
            bgmAppState->blebgm_racpHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_racpHandle:%04x", TVF_D(bgmAppState->blebgm_racpHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_racpHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bgmAppState->blebgm_racpCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_racpCCCHandle:%04x", TVF_D(bgmAppState->blebgm_racpCCCHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_racpCCCHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i]==UUID_SERVICE_GLUECOSE_CONCENTRATION&&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_GLUCOSE_FEATURES)
        {
            bgmAppState->blebgm_featureHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_featureHandle:%04x", TVF_D(bgmAppState->blebgm_featureHandle));
            bleprofile_ReadHandle(bgmAppState->blebgm_featureHandle, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if (bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                bleprofile_p_cfg->cha[i]  == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            bgmAppState->blebgm_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(blebgm_gm_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif

}

void blebgm_connUp(void)
{
    int adrType   = lesmpkeys_getPeerAdrType();
    UINT8 *bdAdr  = lesmpkeys_getPeerAdr();
    BLEBGM_APP_STORAGE *hostData;
    UINT8     measurement = 0;
    UINT8     measurementContext = 0;
    UINT8     racp= 0;
    BLEPROFILE_DB_PDU dbPdu;

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParam:Interval: %d, Latency: %d", TVF_WW(emconninfo_getConnInterval(), emconninfo_getSlaveLatency()));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParam: Supervision TO%d", TVF_D(emconninfo_getSupervisionTimeout()));

    //UINT16 minInterval, UINT16 maxInterval, UINT16 slaveLatency, UINT16 timeout
    lel2cap_sendConnParamUpdateReq(16, 32, 0, 100);
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdateReq:16,32,0,100", TVF_D(0));
    //lel2cap_sendConnParamUpdateReq(0, 0, 0, 100);

    // save timeout
    bgmAppState->blebgm_conparam_timeout = bgmAppState->blebgm_apptimer_count + GAP_CONN_PARAM_TIMEOUT;
    bgmAppState->blebgm_conparam_timeout_enable = 1;
#endif

    // setup the client configuration.
    hostData = blebgm_getHostData(bdAdr, adrType);

    if (hostData)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "GotHostData", TVF_D(0));

        if (hostData->misc & BLEBGM_APP_FLAG_MEASUREMENT_CONF)
        {
            // measurement client configuration is set.
            measurement = LEATT_CLIENT_CONFIG_NOTIFICATION;
        }

        if (hostData->misc & BLEBGM_APP_FLAG_MEASUREMENT_CONTEXT_CONF)
        {
            // measurement context client configuration is set.
            measurementContext = LEATT_CLIENT_CONFIG_NOTIFICATION;
        }

        if (hostData->misc & BLEBGM_APP_FLAG_RACP_CONF)
        {
            // RACP client configuration is set.
            racp= LEATT_CLIENT_CONFIG_INDICATION;
        }
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NoHostData", TVF_D(0));
    }

    // 2 bytes.
    dbPdu.len = 2;
    dbPdu.pdu[0] = measurement;
    dbPdu.pdu[1] = 0x00;
    bleprofile_WriteHandle(bgmAppState->blebgm_measurementCCCHandle, &dbPdu);

    dbPdu.pdu[0] = measurementContext;
    bleprofile_WriteHandle(bgmAppState->blebgm_contextCCCHandle, &dbPdu);

    dbPdu.pdu[0] = racp;
    bleprofile_WriteHandle(bgmAppState->blebgm_racpCCCHandle, &dbPdu);

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);

    // clear this pointer.
    bgmAppState->blebgm_curMeasureRecPtr = NULL;

    // set the state to idle.
    bgmAppState->blebgm_racpState = BLEBGM_RACP_IDLE;

    if ((bleprofile_p_cfg->encr_required&SECURITY_REQUEST) == SECURITY_REQUEST)
    {
        lesmp_sendSecurityRequest();
    }

}

void blebgm_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "connDown:", TVF_D(0));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "DiscReason:%02x", TVF_D(emconinfo_getDiscReason()));

    // clear this pointer.
    bgmAppState->blebgm_curMeasureRecPtr = NULL;

    // set the state to idle.
    bgmAppState->blebgm_racpState = BLEBGM_RACP_IDLE;


    // We may not want to blinded do this. This function call is used for
    // IOP test cases.
    // Setup the Target Address in the scan response payload.
    blebgm_setupTargetAdrInScanRsp();

    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, NULL);
    }

}

void blebgm_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADVStop", TVF_D(0));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "StartAdv", TVF_D(0));
    blebgm_connDown();
}

void blebgm_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (bgmAppState->blebgm_apptimer_count)++;

                blebgm_Timeout(bgmAppState->blebgm_apptimer_count);
            }
            break;
    }
}

void blebgm_appFineTimerCb(UINT32 arg)
{
    (bgmAppState->blebgm_appfinetimer_count)++;

    blebgm_FineTimeout(bgmAppState->blebgm_appfinetimer_count);
}


void blebgm_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        int idx;
        int adrType   = lesmpkeys_getPeerAdrType();
        UINT8 *bdAdr  = lesmpkeys_getPeerAdr();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "adr type = %02x", TVF_D(adrType));
        ble_tracen((char *)bdAdr, 6);

#if 0
{
        //select one device for the next connection

        BLECM_SELECT_ADDR select_addr[1];
        int i;

        BT_MEMCPY(select_addr[0].addr, bdAdr, 6);
        select_addr[0].type = adrType;

        ble_trace1("index = %d", lesmpkeys_find(bdAdr, adrType));

        for (i = 0; i < 5; i++)
        {
            if (blecm_findAddress(i, (UINT8 *)bdAdr, &adrType))
            {
                ble_trace2("index:%d, adrType:%d", i, adrType);
                ble_tracen(bdAdr, 6);
            }
        }

#if 0
//This will override BondedInfo with last index.
//This should not be called unless debugging purpose
        {
            LESMPKEYS_BOND_IDX *bondedIndexBlock;

            extern UINT8 lesmpkeys_maxBondedDevices;

            //lesmp_logBytes("\rFind Bonded Info:", bdAddr, 6);
            bondedIndexBlock = (LESMPKEYS_BOND_IDX*)lesmpkeys_getBondedInfoIdx();

            if (bondedIndexBlock)
            {
                LESMPKEYS_BOND_IDX *curPtr = bondedIndexBlock;

                for (i = 0;i < lesmpkeys_maxBondedDevices; i++)
                {
                    ble_trace3("index:%d, adrType:%d, idx :%02x", i, curPtr->adrType, curPtr->idx);
                    ble_tracen(curPtr->address, 6);

                    // move to next block.
                    curPtr ++;
                }

                //
                cfa_mm_Free(bondedIndexBlock);
            }
        }
#endif

//        blecm_enableAddressSelection();
        //blecm_disableAddressSelection();
//        blecm_SelectAddress(select_addr, 1);

//        blecm_enableRandomAddressResolution();
}

#endif

        // do we have record of this device.
        idx = blebgm_lookupAppData(bdAdr, adrType);

        if (idx != -1)
        {
            // We rebonded with a known device.
            blebgm_clearAllConf(idx);
        }
        else
        {
            // this is a new host.
            blebgm_addHost(bdAdr, adrType);
        }
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "smpFailed,discLink", TVF_D(0));
        blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
    }
}

void blebgm_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "encChanged", TVF_D(0));

#if 0
// TBD
    if (bleprofile_ReadNVRAM(
           VS_BLE_HOST_LIST,
           sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blebgm_hostinfo))
    {
        if (memcmp(blebgm_hostinfo.bdAddr, emconninfo_getPeerAddr(), 6)==0)
        {
            ble_trace2("\rEncOnLastPairedDev:%08x%04x",
                      (blebgm_hostinfo.bdAddr[5] <<24 ) + (blebgm_hostinfo.bdAddr[4] << 16) +
                      (blebgm_hostinfo.bdAddr[3] << 8) + blebgm_hostinfo.bdAddr[2],
                      (blebgm_hostinfo.bdAddr[1] << 8) + blebgm_hostinfo.bdAddr[0]);
        }
    }

    if (bleprofile_p_cfg->encr_required)
    {
        blebgm_indication_enable = 1; //indication enable
        blebgm_indication_defer = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "IndOn", TVF_D(0));
    }
#endif
}



void blebgm_IndicationConf(void)
{

    if (bgmAppState->blebgm_racpState  == BLEBGM_RACP_ACTIVE)
    {
        // only when the racp state is active, we need to do something.
        switch(bgmAppState->blebgm_racpOperation)
        {
            case BLEBGM_RACP_REQ_NUM_OF_RECORDS:
                // we got Indication Confirmation.
                bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
                break;
            case BLEBGM_RACP_REPORT_RECORDS:
                // We are reporting records.
                // The only Indication Conf we should be receiving is
                // the Indication for completion.
                bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
                break;
            case BLEBGM_RACP_CLEAR_RECORDS:
                bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
                break;
            case BLEBGM_RACP_ABORT_REPORTING:
                bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
                break;
        }

    }
    else if (bgmAppState->blebgm_racpState == BLEBGM_RACP_COMPLETE)
    {
        // we are expecting this.

        if (bgmAppState->blebgm_racpOperationAbort)
        {
            // we need to Indicate Abort success.
            blebgm_racpStatusIndication(BLEBGM_RACP_ABORT_REPORTING,
                                BLEBGM_RACP_RSP_CODE_SUCCESS);
            bgmAppState->blebgm_racpOperationAbort=FALSE;
        }
        else
        {
            bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
        }

    }
}

void blebgm_transactionTimeout(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ATT TO", TVF_D(0));

    if ((bleprofile_p_cfg->disc_required) & DISC_ATT_TIMEOUT)
    {
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "DiscATT TO", TVF_D(0));
        }
    }
}

int blebgm_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);
    int len = legattdb_getAttrValueLen(p);
    UINT8 *attrPtr = legattdb_getAttrValue(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "GM Write handle =0x%04x, length = 0x%04x", TVF_WW(handle, len));
    ble_tracen ((char *) attrPtr, len);

    if (handle == bgmAppState->blebgm_racpHandle)
    {
        if (bleprofile_p_cfg->test_enable)
        {
            BLEBGM_RACP_HDR *racpHdr = (BLEBGM_RACP_HDR *) attrPtr;

            switch (bgmAppState->blebgm_racpState)
            {
                case BLEBGM_RACP_IDLE:
                    // We need to check for Client Characteristic Configuration
                    // before we accept the racp operation.
                    if (!(blebgm_checkClientConfigBeforeRACP()==
                         (BLEBGM_CCC_MEASUREMENT|
                          BLEBGM_CCC_MEASUREMENT_CONTEXT|
                          BLEBGM_CCC_RACP)))
                    {
                        // RACP operation prediction has not met.
                        return BLEBGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY;
                    }
                    break;

                case BLEBGM_RACP_COMPLETE:
                case BLEBGM_RACP_ACTIVE:
                case BLEBGM_RACP_PEND:
                    // We got request while we are processing
                    // request. Abort has already been checked so this is
                    // not abort.
                    return BLEBGM_RACP_RSP_PROCEDURE_ALREADY_IN_PROGRESS;

                default:
                    //
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RACPReqUnknownState", TVF_D(0));
                    break;
            }

            // We need to check the racp operation for packet
            // integrity.
            bgmAppState->blebgm_racpPktFormatStatus = blebgm_checkRACPformat(attrPtr, len);

            if (!(bgmAppState->blebgm_racpPktFormatStatus))
            {
                // only if there are no errors we want to proceed with the RACP
                // operation.

                // look for abort first.
                if (racpHdr->opCode == BLEBGM_RACP_ABORT_REPORTING)
                {
                    // Here is an abort.
                    bgmAppState->blebgm_racpOperationAbort = TRUE;
                }
                else
                {
                    // we got a new request.
                    bgmAppState->blebgm_racpState  = BLEBGM_RACP_PEND;
                    // reset count
                    bgmAppState->blebgm_racpNotificationCnt = 0;
                }
            }
        }
    }
    else if (handle == bgmAppState->blebgm_measurementCCCHandle)
    {
        blebgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_NOTIFICATION,
                BLEBGM_APP_FLAG_MEASUREMENT_CONF);
    }
    else if (handle == bgmAppState->blebgm_contextCCCHandle)
    {
        blebgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_NOTIFICATION,
                BLEBGM_APP_FLAG_MEASUREMENT_CONTEXT_CONF);
    }
    else if (handle == bgmAppState->blebgm_racpCCCHandle)
    {
        blebgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_INDICATION ,
                BLEBGM_APP_FLAG_RACP_CONF);
    }

    return 0;
}

void blebgm_reportRecordNumber(BLEPROFILE_DB_PDU *dbPdu)
{
    int sendInd = FALSE;
    int recCount = 0;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RptReNum", TVF_D(0));
    // maximum 20 bytes.
    ble_tracen ((char *)dbPdu->pdu, dbPdu->len);

    switch(bgmAppState->blebgm_racpOperator)
    {
        case BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            // report all records.
            // fix me.
            {
                int i;
                for (i = 0; i < blebgm_iopDataEntries; i++)
                {
                    if (bgmAppState->blebgm_iopDataValid[i])
                    {
                        recCount ++;
                    }
                }
            }
            blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator, recCount);
            sendInd = TRUE;
            break;
        case BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            // store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            //
            if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
            {
                recCount = blebgm_dbGetRecordCntSinceSeq(
                                    dbPdu->pdu[3] | (dbPdu->pdu[4] << 8));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator,recCount);
                sendInd = TRUE;
            }
            else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blebgm_dbGetRecordCntSinceTime((TIMESTAMP *)& (dbPdu->pdu[3]));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator,recCount);
                sendInd = TRUE;
            }
            else
            {
                // It is something we don't understand need error response?
                // TBD

            }
            break;
        case BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL:
            // store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            //
            if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
            {
                recCount = blebgm_dbGetRecordCntlessThanSeq(
                        dbPdu->pdu[3] | (dbPdu->pdu[4] << 8));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator,recCount);
                sendInd = TRUE;
            }
            else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blebgm_dbGetRecordCntlessThanTime((TIMESTAMP *)& (dbPdu->pdu[3]));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator,recCount);
                sendInd = TRUE;
            }
            else
            {
                // It is something we don't understand need error response?
                // TBD

            }
            break;
        case BLEBGM_RACP_OPERATOR_IN_RANGE:
            // store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            //
            if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
            {
                recCount = blebgm_dbGetRecordCntInRange (
                        dbPdu->pdu[3] | (dbPdu->pdu[4] << 8),
                        dbPdu->pdu[5] | (dbPdu->pdu[6] << 8));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator,recCount);
                sendInd = TRUE;
            }
            else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blebgm_dbGetRecordCntInRangeTime (
                        (TIMESTAMP *)& (dbPdu->pdu[3]),
                        (TIMESTAMP *)& (dbPdu->pdu[10]));

                blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator, recCount);
                sendInd = TRUE;
            }
            else
            {
                // It is something we don't understand need error response?
                // TBD

            }
            break;
        case BLEBGM_RACP_OPERATOR_FIRST_RECORD:
        case BLEBGM_RACP_OPERATOR_LAST_RECORD:
            blebgm_formRACPRecordCntInd(dbPdu,bgmAppState->blebgm_racpOperator, 1);
            sendInd = TRUE;
            break;
        default:

            break;
    }

    if (sendInd)
    {
        bleprofile_sendIndication(bgmAppState->blebgm_racpHandle,
                (UINT8 *)dbPdu->pdu, dbPdu->len, blebgm_IndicationConf);
    }
}

int blebgm_reportMeasurement(BLEBGM_GLUCOSE_MEASUREMENT *measurementPtr)
{
    BLEPROFILE_DB_PDU dbPdu;
    if (measurementPtr)
    {
        blebgm_formMeasurePkt(&dbPdu, measurementPtr);

        // We get something to send out.
        bleprofile_sendNotification(
                bgmAppState->blebgm_measurementHandle,dbPdu.pdu,dbPdu.len);

        if (measurementPtr->flags & BLEBGM_M_F_CONTEXT_INFORMATION)
        {
            // we have an context with this measurement.
            BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *ptr =
                        blebgm_dbGetContext(measurementPtr->seqNum);

            if (ptr)
            {
                blebgm_formContextIndPkt(&dbPdu ,ptr);

                // Notify.
                bleprofile_sendNotification(
                        bgmAppState->blebgm_contextHandle,dbPdu.pdu,dbPdu.len);
            }
            else
            {
                // this is not consistant.

            }
        }

        bgmAppState->blebgm_racpNotificationCnt++;
    }

    return 0;
}

int blebgm_reportNextMeasurement(BLEPROFILE_DB_PDU  *dbPdu)
{
    BLEBGM_GLUCOSE_MEASUREMENT *measurementPtr = NULL;

    if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS)
    {
        // retrieve the sequence number.
        int seqNum = bgmAppState->blebgm_racpCurSeqNum;

        measurementPtr = blebgm_dbGetNextMeasurement(seqNum);

        if (measurementPtr)
        {
            blebgm_reportMeasurement(measurementPtr);
        }
        else
        {
            // not more record.
            return BLEBGM_GET_DB_RECORD_NO_MORE_RECORD;
        }

        // We need to store the sequence number for the more than one
        // record query.
        bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        // retrieve the sequence number.
        int seqNum = bgmAppState->blebgm_racpCurSeqNum;

        measurementPtr = blebgm_dbGetNextMeasurement(seqNum);

        if (measurementPtr)
        {
            // we need to check the condition.
            if (blebgm_filterConditionMatched(measurementPtr))
            {
                blebgm_reportMeasurement(measurementPtr);
            }
            else
            {
                // This is out of range.

            }
            // We need to store the sequence number for the more than one
            // record query.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;
        }
        else
        {
            // not more record.
            return BLEBGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL)
    {
        // retrieve the sequence number.
        int seqNum = bgmAppState->blebgm_racpCurSeqNum;

        measurementPtr = blebgm_dbGetNextMeasurement(seqNum);

        if (measurementPtr)
        {
            // we need to check the condition.
            if (blebgm_filterConditionMatched(measurementPtr))
            {
                blebgm_reportMeasurement(measurementPtr);
            }

            // We need to store the sequence number for the more than one
            // record query.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;
        }
        else
        {
            // not more record.
            return BLEBGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE)
    {
        // retrieve the sequence number.
        int seqNum = bgmAppState->blebgm_racpCurSeqNum;

        measurementPtr = blebgm_dbGetNextMeasurement(seqNum);

        if (measurementPtr)
        {
            // we need to check the condition.
            if (blebgm_filterConditionMatched(measurementPtr))
            {
                blebgm_reportMeasurement(measurementPtr);
            }

            // We need to store the sequence number for the more than one
            // record query.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;
        }
        else
        {
            // not more record.
            return BLEBGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }

    return BLEBGM_GET_DB_RECORD_SUCCESS;
}

int blebgm_startMeasurementReport(BLEPROFILE_DB_PDU  *dbPdu)
{
    BLEBGM_GLUCOSE_MEASUREMENT *measurementPtr = NULL;

    if ((bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_FIRST_RECORD) ||
        (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS) ||
        (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL) ||
        (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL) ||
        (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE))
    {
        //
        measurementPtr = blebgm_dbGetFirstMeasurement();
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LAST_RECORD)
    {
        measurementPtr = blebgm_dbGetLastMeasurement();
    }


    if (!measurementPtr)
    {
        // we don't have anything in the db.
        return BLEBGM_GET_DB_RECORD_NO_MORE_RECORD;
    }

    if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            // store the sequence number.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            // we need to check the condition.
            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else
        {
            // undefined filter type.
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL)
    {
        if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            // store the sequence number.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            // we need to check the condition.
            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else
        {
            // undefined filter type.
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE)
    {
        if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            // store the sequence number.
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            // we need to check the condition.
            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else if (bgmAppState->blebgm_racpFilterType  == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;

            if (!blebgm_filterConditionMatched(measurementPtr))
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
        else
        {
            // undefined filter type.
        }

    }


    if (measurementPtr)
    {
        blebgm_reportMeasurement(measurementPtr);


        if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS)
        {
            bgmAppState->blebgm_racpCurSeqNum = measurementPtr->seqNum;
        }
    }

    return BLEBGM_GET_DB_RECORD_SUCCESS;
}

void blebgm_clearRecords(BLEPROFILE_DB_PDU  *dbPdu)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ClrRec", TVF_D(0));
    ble_tracen ((char *)dbPdu->pdu, dbPdu->len);


    bgmAppState->blebgm_racpFilterType = dbPdu->pdu[2];

    if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE)
    {
        if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            // sequence number filtering.
            int seqMin = dbPdu->pdu[3] | (dbPdu->pdu[4] << 8);
            int seqMax = dbPdu->pdu[5] | (dbPdu->pdu[6] << 8);

            if (blebgm_dbClearRecordRange(seqMin, seqMax))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);

            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
        else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // timestamp filtering.

            if (blebgm_dbClearRecordRangeTime((TIMESTAMP *)&(dbPdu->pdu[3]),
                                                                   (TIMESTAMP *)&(dbPdu->pdu[10])))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);

            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS)
    {
        {
            // clear all stored records.
            if (blebgm_dbClearAllRecords())
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_FIRST_RECORD)
    {
        {
            // clear first record.
            if (blebgm_dbClearFirstRecords())
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;

        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LAST_RECORD)
    {
        {
            // clear lastRecord.
            if (blebgm_dbClearLastRecords())
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
    }
    else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            int seqMin = dbPdu->pdu[3] | (dbPdu->pdu[4] << 8);
            // clear less or equal.
            if (blebgm_dbClearLessOrEqualRecords(seqMin))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
        else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // clear less or equal.
            if (blebgm_dbClearLessOrEqualRecordsTime((TIMESTAMP *)&(dbPdu->pdu[3])))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
    }
    else if (bgmAppState->blebgm_racpOperator ==BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL)
    {
        if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            int seqMin = dbPdu->pdu[3] | (dbPdu->pdu[4] << 8);
            // clear less or equal.
            if (blebgm_dbClearLargerOrEqualRecords(seqMin))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
        else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // clear less or equal.
            if (blebgm_dbClearLargerOrEqualRecordsTime((TIMESTAMP *)&(dbPdu->pdu[3])))
            {
                // no records found.
                // indicate completion with error.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blebgm_racpStatusIndication(BLEBGM_RACP_CLEAR_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);

            }
            // wait for Indication Confirmation.
            bgmAppState->blebgm_racpState = BLEBGM_RACP_COMPLETE;
        }
    }
}

void blebgm_reportRecords(BLEPROFILE_DB_PDU  *dbPdu)
{
    switch(bgmAppState->blebgm_racpOperator)
    {
        case BLEBGM_RACP_OPERATOR_FIRST_RECORD:
        case BLEBGM_RACP_OPERATOR_LAST_RECORD:
            if (blebgm_startMeasurementReport(dbPdu)
                                       == BLEBGM_GET_DB_RECORD_SUCCESS)
            {
                // We are done.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                                BLEBGM_RACP_RSP_CODE_SUCCESS);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }
            else
            {
                // We don't have anything there.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }
            break;
        case BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            if (blebgm_startMeasurementReport(dbPdu)
                                        == BLEBGM_GET_DB_RECORD_SUCCESS)
            {
                blebgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }
            break;
        case BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL:
            // store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            // Copy the filter Max.
            BT_MEMCPY(bgmAppState->blebgm_racpFilterMax,dbPdu->pdu + 3,TIMESTAMP_LEN);

            if (blebgm_startMeasurementReport(dbPdu)
                                        == BLEBGM_GET_DB_RECORD_SUCCESS)
            {
                blebgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }
            break;
        case BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            // Store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            // Copy the filter Max.
            BT_MEMCPY(bgmAppState->blebgm_racpFilterMin,dbPdu->pdu + 3,TIMESTAMP_LEN);
            if (blebgm_startMeasurementReport(dbPdu)
                                        == BLEBGM_GET_DB_RECORD_SUCCESS)
            {
                blebgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }
            break;
        case BLEBGM_RACP_OPERATOR_IN_RANGE:
            // Store the filter type here.
            bgmAppState->blebgm_racpFilterType  = dbPdu->pdu[2];
            //
            if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
            {
                BT_MEMCPY(bgmAppState->blebgm_racpFilterMin,dbPdu->pdu + 3, 2);
                BT_MEMCPY(bgmAppState->blebgm_racpFilterMax,dbPdu->pdu + 5, 2);
            }
            else
            {
                BT_MEMCPY(bgmAppState->blebgm_racpFilterMin,dbPdu->pdu + 3, TIMESTAMP_LEN);
                BT_MEMCPY(bgmAppState->blebgm_racpFilterMax,dbPdu->pdu + 3 + TIMESTAMP_LEN,
                                    TIMESTAMP_LEN);
            }

            if (blebgm_startMeasurementReport(dbPdu)
                                        == BLEBGM_GET_DB_RECORD_SUCCESS)
            {
                blebgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
                // waiting for the Infication Confirmation.
                bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;
            }

            break;
        default:
            break;
    }
}

// This function is intended to continue reporting.
void blebgm_reportRecordsCont(void)
{
    BLEPROFILE_DB_PDU dbPdu;

    switch(bgmAppState->blebgm_racpOperator)
    {
        case BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS:
        case BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL:
        case BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL:
        case BLEBGM_RACP_OPERATOR_IN_RANGE:
            blebgm_sendAsManyMeasurements(&dbPdu);
            break;
        case BLEBGM_RACP_OPERATOR_LAST_RECORD:
        case BLEBGM_RACP_OPERATOR_FIRST_RECORD:
            break;
    }
}

void blebgm_handleRACPStates(void)
{
#if 0
    ble_trace3("racp state = %02x, pdu check %02x. tx buffer = %d",
            bgmAppState->blebgm_racpState,bgmAppState->blebgm_racpPktFormatStatus,
            blecm_getAvailableTxBuffers());
#endif

    // take care of the error reporting first.
    if (bgmAppState->blebgm_racpPktFormatStatus)
    {
        BLEPROFILE_DB_PDU db_pdu;

        bleprofile_ReadHandle(bgmAppState->blebgm_racpHandle,&db_pdu);

        if (blebgm_checkClientConfigBeforeRACP() | BLEBGM_CCC_RACP)
        {
            // the racp packet has error.
            blebgm_racpStatusIndication(db_pdu.pdu[0], bgmAppState->blebgm_racpPktFormatStatus);
        }

        // clear the error status flag.
        bgmAppState->blebgm_racpPktFormatStatus  = 0;
        return;
    }

    // if there are abort, we need to check them.
    if (bgmAppState->blebgm_racpOperationAbort)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "racpSbort", TVF_D(0));

#if 0
        if (!bgmAppState->blebgm_racpOperation)
        {
            // We are trying to abort while not in operation.
            blebgm_racpStatusIndication(BLEBGM_RACP_ABORT_REPORTING,
                                BLEBGM_RACP_RSP_CODE_ABORT_UNSUCCESSFUL);
        }
        else
#endif
        {
            // we need to Indicate Abort success.
            blebgm_racpStatusIndication(BLEBGM_RACP_ABORT_REPORTING,
                                BLEBGM_RACP_RSP_CODE_SUCCESS);

        }

        // clear this flag.
        bgmAppState->blebgm_racpOperationAbort  = FALSE;

        // go back to idle state.
        bgmAppState->blebgm_racpState           = BLEBGM_RACP_IDLE;
    }
    else
    {
        if (bgmAppState->blebgm_racpState != BLEBGM_RACP_IDLE)
        {
            if (bgmAppState->blebgm_racpState == BLEBGM_RACP_PEND)
            {
                // we have something do to.
                BLEPROFILE_DB_PDU db_pdu;
                // get the data.
                bleprofile_ReadHandle(bgmAppState->blebgm_racpHandle,&db_pdu);
                // this means a new request comes in.
                // change the state to indicate we started to look into it.
                bgmAppState->blebgm_racpState      = BLEBGM_RACP_ACTIVE;

                //////////////////////////////////////////////////////////////
                // Taking a RACP operation.
                // Load the opcode and operator.
                // RACP opcode is at offset 0.
                bgmAppState->blebgm_racpOperation  = db_pdu.pdu[0];
                bgmAppState->blebgm_racpOperator   = db_pdu.pdu[1];
                // clear the filter stuffs.
                bgmAppState->blebgm_racpFilterType = 0;
                BT_MEMSET(&(bgmAppState->blebgm_racpFilterMin),0x0, TIMESTAMP_LEN);
                BT_MEMSET(&(bgmAppState->blebgm_racpFilterMax),0x0, TIMESTAMP_LEN);

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "racp operation opcode = %02x ", TVF_D(bgmAppState->blebgm_racpOperation));
                switch(bgmAppState->blebgm_racpOperation)
                {
                    case BLEBGM_RACP_REPORT_RECORDS:
                        blebgm_reportRecords(&db_pdu);
                        break;
                    case BLEBGM_RACP_CLEAR_RECORDS:
                        blebgm_clearRecords(&db_pdu);
                        break;
                    case BLEBGM_RACP_ABORT_REPORTING:
                        break;
                    case BLEBGM_RACP_REQ_NUM_OF_RECORDS:
                        blebgm_reportRecordNumber(&db_pdu);
                        break;

                    case BLEBGM_RACP_RESERVED_0:
                        // this is reserved.
                        // we need to at least clear the states.
                    default:
                        // Give error msg.
                        blebgm_racpStatusIndication(bgmAppState->blebgm_racpOperation ,
                                BLEBGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED);

                        bgmAppState->blebgm_racpState  = BLEBGM_RACP_IDLE;
                        break;
                }
            }
            else if (bgmAppState->blebgm_racpState  == BLEBGM_RACP_ACTIVE)
            {
                // RACP is already in active state.
                if (bgmAppState->blebgm_racpOperation == BLEBGM_RACP_REPORT_RECORDS)
                {
                    blebgm_reportRecordsCont();
                }
            }
        }
    }
}

int blebgm_dbGetRecordCntInRange(int start, int end)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if ((curPtr->seqNum <= end) &&
                (curPtr->seqNum >= start))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blebgm_dbGetRecordCntInRangeTime(TIMESTAMP *startTime, TIMESTAMP *endTime)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if ((blebgm_getTimeStampInseconds((UINT8 *)curPtr->BaseTime, curPtr->timeOffset, curPtr->flags) <=
                    blebgm_getTimeStampInseconds((UINT8 *)endTime, 0, 0)) &&
                (blebgm_getTimeStampInseconds((UINT8 *)curPtr->BaseTime, curPtr->timeOffset, curPtr->flags) >=
                    blebgm_getTimeStampInseconds((UINT8 *)startTime, 0, 0)))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blebgm_dbGetRecordCntlessThanSeq(int endSeq)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (curPtr->seqNum <= endSeq)
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;

}

int blebgm_dbGetRecordCntlessThanTime(TIMESTAMP *endTime)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (blebgm_getTimeStampInseconds((UINT8 *)curPtr->BaseTime, curPtr->timeOffset, curPtr->flags) <=
                    blebgm_getTimeStampInseconds((UINT8 *)endTime, 0, 0))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blebgm_dbGetRecordCntSinceSeq(int startSeq)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (curPtr->seqNum >= startSeq)
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blebgm_dbGetRecordCntSinceTime(TIMESTAMP *startTime)
{
    int cnt = 0;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (blebgm_getTimeStampInseconds((UINT8 *)curPtr->BaseTime, curPtr->timeOffset, curPtr->flags) >=
                    blebgm_getTimeStampInseconds((UINT8 *)startTime, 0, 0))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetMeasurement(int seqNum)
{
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (curPtr->seqNum == seqNum)
            {
                // We found the specified record.
                return curPtr;
            }
        }

        // move to next record.
        curPtr++;
    }

    return NULL;
}

BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetNextMeasurement(int seqNum)
{
    int i;
    int nextValid = FALSE;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            if (nextValid)
            {
                return curPtr;
            }
            else if (curPtr->seqNum == seqNum)
            {
                // We found the specified record.
                nextValid = TRUE;
            }
        }

        // move to next record.
        curPtr++;
    }

    return NULL;
}

BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetFirstMeasurement(void)
{
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "get first measurement, %d", TVF_D(i));
            return curPtr;
        }
        // move on to next one.
        curPtr++;
    }
    return NULL;
}

BLEBGM_GLUCOSE_MEASUREMENT *blebgm_dbGetLastMeasurement(void)
{
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = &(bgmAppState->blebgm_iopData[blebgm_iopDataEntries-1]);

    for (i = blebgm_iopDataEntries-1; i>= 0; i--)
    {
        if (bgmAppState->blebgm_iopDataValid[i])
        {
            return curPtr;
        }
        // move on to next one.
        curPtr--;
    }
    return NULL;
}


void blebgm_formMeasurePkt(BLEPROFILE_DB_PDU *pdu, BLEBGM_GLUCOSE_MEASUREMENT *ptr)
{
    UINT8 *curPtr;
    int pktSize = 10; // minimum is 10 bytes.


    // get the mandatory portion.
    BT_MEMCPY(pdu->pdu, ptr, pktSize);
    //
    curPtr = pdu->pdu + pktSize;

    if (ptr->flags & BLEBGM_M_F_TIME_OFFSET_FIELD_PRESENT)
    {
        // 2 byte.
        pktSize +=2;

        *curPtr++ = (ptr->timeOffset & 0xff);
        *curPtr++ = ((ptr->timeOffset >> 8) & 0xff);
    }


    // we need to determin the size of the packet.
    if (ptr->flags & BLEBGM_M_F_CON_AND_TYPE_SAMPLE_LOCATION_PRESENT)
    {
        // 1 byte (type and sample location) + 2 bytes(concentration).
        pktSize += 1 + 2;

        *curPtr++ = (ptr->glucoseConcentration & 0xff);
        *curPtr++ = ((ptr->glucoseConcentration >> 8) & 0xff);

        *curPtr++ = ptr->typeSampleLocation;
    }

    if (ptr->flags & BLEBGM_M_F_SENSOR_STATUS_ANNUNCIATION)
    {
        // 2 byte.
        pktSize +=2;

        *curPtr++ = (ptr->sensorStatus        & 0xff);
        *curPtr++ = ((ptr->sensorStatus >> 8) & 0xff);
    }

    pdu->len = pktSize;
}

void blebgm_formContextIndPkt(BLEPROFILE_DB_PDU *pdu,
                                BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *ptr)
{
    UINT8 *curPtr;
    int pktSize = 3; // minimum is 3 bytes.

    // get the mandatory portion.
    BT_MEMCPY(pdu->pdu, ptr, pktSize);
    //
    curPtr = pdu->pdu + pktSize;

    // we need to determin the size of the packet.
    if (ptr->flags & BLEBGM_M_C_F_EXTENDED_FLAG_PRESENT)
    {
        // 1 byte
        pktSize++;

        *curPtr++ = ptr->extendedFlags;
    }

    if (ptr->flags & BLEBGM_M_C_F_CARB_ID_PRESENT)
    {
        // 1 byte ID, 2 bytes carb.
        pktSize+= 3;

        *curPtr++ = ptr->carbID;
        *curPtr++ = (ptr->carb       & 0xff);
        *curPtr++ = ((ptr->carb>> 8) & 0xff);
    }

    if (ptr->flags & BLEBGM_M_C_F_MEAL_FIELD_PRESENT)
    {
        // 1 byte.
        pktSize++;

        *curPtr++ = ptr->meal;
    }

    if (ptr->flags & BLEBGM_M_C_F_TESTER_HEALTH_PRESENT)
    {
        // 1 byte.
        pktSize++;

        *curPtr++ = ptr->testerHealth;
    }


    if (ptr->flags & BLEBGM_M_C_F_EXERCISE_FIELD_PRESENT)
    {
        // 2+1 byte.
        pktSize +=3;

        *curPtr++ = (ptr->exerciseDuration       & 0xff);
        *curPtr++ = ((ptr->exerciseDuration>> 8) & 0xff);
        *curPtr++ = ptr->exerciseIntensity;
    }

    if (ptr->flags & BLEBGM_M_C_F_MEDICAION_PRESENT)
    {
        // 1+2 byte.
        pktSize += 3;

        *curPtr++ = ptr->medicationID;
        *curPtr++ = (ptr->medication       & 0xff);
        *curPtr++ = ((ptr->medication>> 8) & 0xff);
    }

    if (ptr->flags & BLEBGM_M_C_F_HB_A1C_PRESENT)
    {
        // 2 byte.
        pktSize+=2;

        *curPtr++ = (ptr->HbA1c        & 0xff);
        *curPtr++ = ((ptr->HbA1c>> 8) & 0xff);
    }

    pdu->len = pktSize;
}

BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *blebgm_dbGetContext(UINT16 seqNum)
{
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT *curPtr = &(bgmAppState->blebgm_iopContextData[0]);

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if (curPtr->seqNum == seqNum)
        {
            // this is the one.
            return curPtr;
        }

        // move on to next one.
        curPtr++;
    }
    return NULL;
}

void blebgm_formRACPRecordCntInd(BLEPROFILE_DB_PDU *dbPdu,UINT8 Optor, UINT16 count)
{
    UINT8 operator = Optor;

    if (bgmAppState->blebgm_null_operator_nsrr)
    {
        operator = NULL;
    }

    // form the response message.
    dbPdu->pdu[0] = BLEBGM_RACP_RSP_NUM_OF_RECORDS;
    dbPdu->pdu[1] = operator;
    dbPdu->pdu[2] = count & 0xff;
    dbPdu->pdu[3] = (count >>8) & 0xff;
    // packet length is 3 bytes.
    dbPdu->len = 4;
}

void blebgm_racpStatusIndication(UINT8 reqOpCode, UINT8 status)
{
    UINT8 tmp[4];

    tmp[0] = BLEBGM_RACP_RSP_CODE;
    tmp[1] = NULL;
    tmp[2] = reqOpCode;
    tmp[3] = status;

    bleprofile_sendIndication(bgmAppState->blebgm_racpHandle, tmp, 4,blebgm_IndicationConf);
}

// This function returns true if the filter condition matched.
int blebgm_filterConditionMatched(BLEBGM_GLUCOSE_MEASUREMENT *ptr)
{
    if (ptr)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Filter Condition %d, %d", TVF_WW(bgmAppState->blebgm_racpFilterType,bgmAppState->blebgm_racpOperator));
        if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
        {
            if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL)
            {
                // Sequence number less than or equal to.
                if (ptr->seqNum <=
                        (bgmAppState->blebgm_racpFilterMax[0] | (bgmAppState->blebgm_racpFilterMax[1] << 8)))
                {
                    return TRUE;
                }
            }
            else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL)
            {
                // Sequence number less than or equal to.
                if (ptr->seqNum >=
                        (bgmAppState->blebgm_racpFilterMin[0] | (bgmAppState->blebgm_racpFilterMin[1] << 8)))
                {
                    return TRUE;
                }
            }
            else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE)
            {
                // Sequence number less than or equal to.
                if ((ptr->seqNum >=
                        (bgmAppState->blebgm_racpFilterMin[0] | (bgmAppState->blebgm_racpFilterMin[1] << 8))) &&
                    (ptr->seqNum <=
                        (bgmAppState->blebgm_racpFilterMax[0] | (bgmAppState->blebgm_racpFilterMax[1] << 8))))
                {
                    return TRUE;
                }
            }
        }
        else if (bgmAppState->blebgm_racpFilterType == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL)
            {
                UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8 *) &ptr->BaseTime,ptr->timeOffset, ptr->flags);

                UINT64 refTimeInSeconds =
                    blebgm_getTimeStampInseconds( &(bgmAppState->blebgm_racpFilterMin[0]),0, 0);

                if (timeInSeconds >=refTimeInSeconds)
                {
                    return TRUE;
                }
            }
            else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL)
            {
                UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8*) &ptr->BaseTime,ptr->timeOffset, ptr->flags);

                UINT64 refTimeInSeconds =
                    blebgm_getTimeStampInseconds( &(bgmAppState->blebgm_racpFilterMax[0]),0,0);

                if (timeInSeconds <= refTimeInSeconds)
                {
                    return TRUE;
                }
            }
            else if (bgmAppState->blebgm_racpOperator == BLEBGM_RACP_OPERATOR_IN_RANGE)
            {
                UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8 *) &ptr->BaseTime,ptr->timeOffset, ptr->flags);
                UINT64 minTimeInSeconds =
                    blebgm_getTimeStampInseconds( &(bgmAppState->blebgm_racpFilterMin[0]),0,0);
                UINT64 maxTimeInSeconds =
                    blebgm_getTimeStampInseconds( &(bgmAppState->blebgm_racpFilterMax[0]),0,0);

                if ((timeInSeconds >= minTimeInSeconds) &&
                    (timeInSeconds <= maxTimeInSeconds))
                {
                    return TRUE;
                }
            }
        }
        else
        {
            // this is unknown filter type.
        }
    }
    // default is false.
    return FALSE;
}

int blebgm_dbClearFirstRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if ( bgmAppState->blebgm_iopDataValid[i])
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;

            // got one.
            break;
        }
    }

    return noRecordFound;
}

int blebgm_dbClearLessOrEqualRecords(int seqMax)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if ( curPtr->seqNum <= seqMax)
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;

}

int blebgm_dbClearLessOrEqualRecordsTime(TIMESTAMP *TimeMax)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8 *) &curPtr->BaseTime, curPtr->timeOffset,curPtr->flags);
        UINT64 maxTimeInSeconds =
                    blebgm_getTimeStampInseconds( (UINT8 *)TimeMax,0,0);

        if (timeInSeconds <= maxTimeInSeconds)
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;

}

int blebgm_dbClearLargerOrEqualRecords(int seqMin)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if ( curPtr->seqNum >= seqMin)
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}


int blebgm_dbClearLargerOrEqualRecordsTime(TIMESTAMP *TimeMin)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8 *) &curPtr->BaseTime,curPtr->timeOffset,curPtr->flags);
        UINT64 minTimeInSeconds =
                    blebgm_getTimeStampInseconds(  (UINT8 *)TimeMin,0,0);

        if (timeInSeconds >=minTimeInSeconds)
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}


int blebgm_dbClearLastRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for (i = blebgm_iopDataEntries-1;i >=0; i--)
    {
        if ( bgmAppState->blebgm_iopDataValid[i])
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;

            // got one.
            break;
        }
    }

    return noRecordFound;
}


// return FALSE if there are records found and cleared.
// return TRUE if there are no records found.
int blebgm_dbClearAllRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if ( bgmAppState->blebgm_iopDataValid[i])
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }
    }

    return noRecordFound;
}

int blebgm_dbClearRecordRange(int seqMin, int seqMax)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        if ((curPtr->seqNum >= seqMin) && (curPtr->seqNum <= seqMax))
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

int blebgm_dbClearRecordRangeTime(TIMESTAMP *TimeMin, TIMESTAMP *TimeMax)
{
    int noRecordFound = TRUE;
    int i;
    BLEBGM_GLUCOSE_MEASUREMENT *curPtr = bgmAppState->blebgm_iopData;

    for (i = 0; i < blebgm_iopDataEntries; i++)
    {
        UINT64 timeInSeconds =
                    blebgm_getTimeStampInseconds(
                            (UINT8 *) &curPtr->BaseTime,curPtr->timeOffset,curPtr->flags);
        UINT64 minTimeInSeconds =
                    blebgm_getTimeStampInseconds( (UINT8 *)TimeMin,0,0);
        UINT64 maxTimeInSeconds =
                    blebgm_getTimeStampInseconds( (UINT8 *)TimeMax,0,0);

        if ((timeInSeconds >= minTimeInSeconds) &&
            (timeInSeconds <= maxTimeInSeconds))
        {
            // clear the flag.
            bgmAppState->blebgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

UINT32 blebgm_buttonCb(UINT32 function)
{
    switch(function)
    {
        case BUTTON_CLIENT:
#ifdef BLEBGM_IOP_ADVANCE_DB
    {
        int i;

        // for IOP test. This button will add one more records.
        if (!(bgmAppState->blebgm_iopDataValid[blebgm_iopDataEntries-1]))
        {
            // the last one is availble to reuse.
            // this record becomes valid.
            bgmAppState->blebgm_iopDataValid[blebgm_iopDataEntries-1] = TRUE;

            // increase the sequence number of last record by one.

            bgmAppState->blebgm_iopData[blebgm_iopDataEntries -1].seqNum++;

            bgmAppState->blebgm_iopContextData[blebgm_iopDataEntries -1].seqNum =
                bgmAppState->blebgm_iopData[blebgm_iopDataEntries -1].seqNum;

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_buttonCb, last one. put %d idx back.", TVF_D(blebgm_iopDataEntries -1));
        }
        else
        {
            for (i = blebgm_iopDataEntries-2; i >= 0; i--)
            {
                if (!(bgmAppState->blebgm_iopDataValid[i]))
                {
                    // we find a available spot.
                    int j;
                    // slide all the records down.
                    for (j = i; j < blebgm_iopDataEntries - 1; j++)
                    {

                        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_buttonCb, %d idx is deleted.", TVF_D(i));
                        BT_MEMCPY(&(bgmAppState->blebgm_iopData[j]),
                                &(bgmAppState->blebgm_iopData[j+1]),
                                sizeof(BLEBGM_GLUCOSE_MEASUREMENT));

                        BT_MEMCPY(&(bgmAppState->blebgm_iopContextData[j]),
                                &(bgmAppState->blebgm_iopContextData[j+1]),
                                sizeof(BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT));
                    }
                    // this record becomes valid.
                    bgmAppState->blebgm_iopDataValid[i]=TRUE;

                    // increase the sequence number of last record by one.

                    bgmAppState->blebgm_iopData[blebgm_iopDataEntries -1].seqNum++;

                    bgmAppState->blebgm_iopContextData[blebgm_iopDataEntries -1].seqNum =
                        bgmAppState->blebgm_iopData[blebgm_iopDataEntries -1].seqNum;

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebgm_buttonCb, put %d idx back.", TVF_D(blebgm_iopDataEntries -1));
                    break;
                }
            }
        }
    }
#endif
            break;
        default:
            break;
    }

    return 0;
}

UINT64 blebgm_getTimeStampInseconds( UINT8 *baseTime, INT16 timeOffset, UINT8 flags)
{
    UINT64 tmp = 0;
    int i;
    UINT8 noofdays[12] = 
        {31, 28, 31, 30, 
         31, 30, 31, 31, 
         30, 31, 30, 31};

    UINT8 *ptr = (UINT8 *)baseTime;
    // TimeStamp is 7 bytes. yy,m,d,h,m,s.

    UINT16 year = (ptr[1] << 8)|(ptr[0]);
  
    if(year%400 == 0)
    {
        //leap year.
        noofdays[1]++;
    }
    else if(year%100 == 0)
    {
    }
    else if(year%4 == 0)
    {
        //leap year.
        noofdays[1]++;
    }

    // temp has total number of days before this year
    tmp = (year-1)*365 + (year-1)/4 - (year-1)/100 + (year-1)/400;
    
    // tmp has total number of day before this month
    if(ptr[2]>1)
    {
        for(i=1; i<ptr[2];i++)
        {
            tmp += noofdays[i-1]; 
        }
        
    }

    //add up this month's date
    tmp += (ptr[3]-1); //remove today

    tmp *= 24; //make hours

    tmp += ptr[4]; //adding hour

    tmp *= 60; //making minute

    tmp += ptr[5];

    //check timeoffset is valid of not
    if(flags & BLEBGM_M_F_TIME_OFFSET_FIELD_PRESENT)
    {
        if(timeOffset>=0)
        {
            tmp += (UINT16) timeOffset; //adding minutes
        }
        else
        {
            tmp -= (0x7FFF-(UINT16)(timeOffset&0x7FFF)+1);
        }
    }

    tmp *= 60; //making second

    tmp += ptr[6]; //adding seconds

#if 0
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "%d/%d/%d", TVF_BBW(ptr[2], ptr[3], ptr[0]+(ptr[1]<<8)));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "%d:%d:%d", TVF_BBBB(ptr[4],ptr[5],ptr[6],0));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "%d", TVF_WW(timeOffset,0));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "sec_low=%08x", TVF_D(tmp&0xFFFFFFFF));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "sec_high=%08x", TVF_D(tmp>>32));
#endif

    return tmp;
}

UINT32 blebgm_checkRACPformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "chkRacpFmt", TVF_D(0));
    ble_tracen((char *)ptr, len);

    switch(ptr[0])
    {
        case BLEBGM_RACP_REPORT_RECORDS:
            status = blebgm_checkGetRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_CLEAR_RECORDS:
            status = blebgm_checkClearRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_ABORT_REPORTING:
            status = blebgm_checkAbortReportingformat(ptr, len);
            break;

        case BLEBGM_RACP_REQ_NUM_OF_RECORDS:
            status = blebgm_checkNumRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_RSP_NUM_OF_RECORDS:
        case BLEBGM_RACP_RSP_CODE:
        default:
            status = BLEBGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED;

            break;
    }
    return status;
}

UINT32 blebgm_checkGetRecordformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    if (len < 2)
    {
        status = BLEBGM_RACP_RSP_CODE_INVALID_OPERATOR;
    }
    else
    {
        switch(ptr[1])
        {
            case BLEBGM_RACP_OPERATOR_NULL:
                status = BLEBGM_RACP_RSP_CODE_INVALID_OPERATOR;
                break;
            case BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            case BLEBGM_RACP_OPERATOR_FIRST_RECORD:
            case BLEBGM_RACP_OPERATOR_LAST_RECORD:
                if (len > 2)
                {
                    status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            case BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL:
            case BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL:
                if (len > 2)
                {
                    // check the filter type.
                    if ((ptr[2] > BLEBGM_RACP_FILTER_TYPE_FACING_TIME) ||
                        (ptr[2] < BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER))
                    {
                        // undefined filter type.
                        status = BLEBGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED;
                    }

                    if (ptr[2] == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
                    {
                        if (len != (3 + 2))
                        {
                            status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
                    else if (ptr[2] == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
                    {
                        if (len != (3 + 7))
                        {
                            status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
                }
                else
                {
                    status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            case BLEBGM_RACP_OPERATOR_IN_RANGE:
                if (len > 2)
                {
                    // check the filter type.
                    if ((ptr[2] > BLEBGM_RACP_FILTER_TYPE_FACING_TIME) ||
                        (ptr[2] < BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER))
                    {
                        // undefined filter type.
                        status = BLEBGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED;
                    }

                    if (ptr[2] == BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER)
                    {
                        if (len != (3 + 2 * 2))
                        {
                            status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
                    else if (ptr[2] == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
                    {
                        if (len != (3 + 7 * 2))
                        {
                            status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
                }
                else
                {
                    status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            default:
                // This is out of range.
                status = BLEBGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

UINT32 blebgm_checkAbortReportingformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    if (len < 2)
    {
        status = BLEBGM_RACP_RSP_CODE_INVALID_OPERATOR;
    }
    else
    {
        switch(ptr[1])
        {
            case BLEBGM_RACP_OPERATOR_NULL:
                break;

            case BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            case BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL:
            case BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            case BLEBGM_RACP_OPERATOR_IN_RANGE:
            case BLEBGM_RACP_OPERATOR_FIRST_RECORD:
            case BLEBGM_RACP_OPERATOR_LAST_RECORD:
                status = BLEBGM_RACP_RSP_CODE_INVALID_OPERATOR;
                break;
            default:
                // This is out of range.
                status = BLEBGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

UINT32 blebgm_checkClearRecordformat(UINT8 *ptr, INT32 len)
{
    return blebgm_checkGetRecordformat(ptr, len);
}

UINT32 blebgm_checkNumRecordformat(UINT8 *ptr, INT32 len)
{
    return blebgm_checkGetRecordformat(ptr, len);
}

int  blebgm_checkClientConfigBeforeRACP(void)
{
    int               status = 0;
    BLEPROFILE_DB_PDU dbPdu;

    // check the notification configuration for measurement.
    bleprofile_ReadHandle(bgmAppState->blebgm_measurementCCCHandle ,&dbPdu);
    if ((dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_NOTIFICATION))
    {
        // Notification is set.
        status |= BLEBGM_CCC_MEASUREMENT;
    }

    // check measurement context.
    bleprofile_ReadHandle(bgmAppState->blebgm_contextCCCHandle ,&dbPdu);
    if ((dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_NOTIFICATION))
    {
        // context is good.
        status |= BLEBGM_CCC_MEASUREMENT_CONTEXT;
    }

    // check RACP.
    bleprofile_ReadHandle(bgmAppState->blebgm_racpCCCHandle,&dbPdu);
    if ((dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_INDICATION))
    {
        // RACP setting is good.
        status |= BLEBGM_CCC_RACP;
    }

    return status;
}

void blebgm_sendAsManyMeasurements(BLEPROFILE_DB_PDU *dbPdu)
{
    while (blecm_getAvailableTxBuffers() > 1) // two notifications can be sent at the same time
    {
        if (blebgm_reportNextMeasurement(dbPdu)
                == BLEBGM_GET_DB_RECORD_NO_MORE_RECORD)
        {
            // we are done.
            if (bgmAppState->blebgm_racpNotificationCnt)
            {
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_SUCCESS);
            }
            else
            {
                blebgm_racpStatusIndication(BLEBGM_RACP_REPORT_RECORDS,
                        BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            // waiting for the Infication Confirmation.
            bgmAppState->blebgm_racpState   = BLEBGM_RACP_COMPLETE;

            // No more data to send, break out of the loop.
            break;
        }
    }
}

void blebgm_loadAppData(void)
{
    int tmp;

    tmp = bleprofile_ReadNVRAM(
            VS_BLE_HOST_LIST,
            sizeof(bgmAppState->blebgm_appStorage), (UINT8 *)bgmAppState->blebgm_appStorage);

    if (tmp != sizeof(bgmAppState->blebgm_appStorage))
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "loadAppDataHadNothing", TVF_D(0));

        // clear the memory area for it.
        BT_MEMSET((UINT8 *)bgmAppState->blebgm_appStorage, 0x0, sizeof(bgmAppState->blebgm_appStorage));
    }
}

BLEBGM_APP_STORAGE *blebgm_getHostData(UINT8 *adr, UINT8 adrType)
{
    BLEBGM_APP_STORAGE *hostData = NULL;
    int idx = blebgm_lookupAppData(adr, adrType);

    if (idx != -1)
    {
        hostData = &(bgmAppState->blebgm_appStorage[idx]);
    }

    return hostData;
}


int blebgm_lookupAppData(UINT8 *adr, UINT8 adrType)
{
    int i;
    BLEBGM_APP_STORAGE *curPtr = bgmAppState->blebgm_appStorage;

    for (i = 0; i < BLEBGM_MAX_BONDED_HOST; i++)
    {
        if (curPtr->misc & BLEBGM_APP_FLAG_VALID)
        {
            // a valid record.
            // check address type first.
            int tmpAdrType = curPtr->misc & BLEBGM_APP_FLAG_ADR_TYPE;
            if (( tmpAdrType &&  adrType) ||
                (!tmpAdrType && !adrType))
            {
                // the adr type are the same. we compare the 6 bytes of
                // address.
                if (!BT_MEMCMP(curPtr->bdAdr, adr, 6))
                {
                    // this is the match.
                    return i;
                }
            }
        }
        // move to next entry;
        curPtr++;
    }
    // not found.
    return -1;
}

void blebgm_clearAllConf(int idx)
{
    if ((idx >= 0) && (idx < BLEBGM_MAX_BONDED_HOST))
    {
        // keep the address type and clear everything else.
        bgmAppState->blebgm_appStorage[idx].misc &= BLEBGM_APP_FLAG_ADR_TYPE;
    }
}

int blebgm_addHost(UINT8 *adr, UINT8 adrType)
{
    int i;
    BLEBGM_APP_STORAGE *curPtr = bgmAppState->blebgm_appStorage;
    int len;

    // look for available spot.
    for (i = 0; i < BLEBGM_MAX_BONDED_HOST;i++)
    {
        if (!(curPtr->misc & BLEBGM_APP_FLAG_VALID))
        {
            // This is available.
            break;
        }
        curPtr++;
    }

    if (i >= BLEBGM_MAX_BONDED_HOST)
    {
        // We don't have valid index any more.
        // Fix me.
        // we use index 0.
        i = 0;
    }

    // set the address.
    BT_MEMCPY(&(bgmAppState->blebgm_appStorage[i]), adr, BLEBGM_BD_ADR_LEN);

    if (adrType)
    {
        // random address.
        bgmAppState->blebgm_appStorage[i].misc
            = BLEBGM_APP_FLAG_ADR_TYPE|BLEBGM_APP_FLAG_VALID;
    }
    else
    {
        // public address.
        bgmAppState->blebgm_appStorage[i].misc = BLEBGM_APP_FLAG_VALID;
    }

    len = bleprofile_WriteNVRAM(
            VS_BLE_HOST_LIST, sizeof(bgmAppState->blebgm_appStorage),
            (UINT8 *)(bgmAppState->blebgm_appStorage));

    if (len != sizeof(bgmAppState->blebgm_appStorage))
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "addHost write nvram problem, %d bytes", TVF_D(len));
    }

    return i;
}

BLEBGM_APP_STORAGE *blebgm_getCurHostData(void)
{
    int     adrType = lesmpkeys_getPeerAdrType();
    UINT8   *bdAdr  = lesmpkeys_getPeerAdr();
    int idx;

    idx = blebgm_lookupAppData(bdAdr, adrType);
    if (idx != -1)
    {
        return &(bgmAppState->blebgm_appStorage[idx]);
    }
    return NULL;
}

int blebgm_setupTargetAdrInScanRsp(void)
{
    int i;
    BLEBGM_APP_STORAGE *curAppData = (bgmAppState->blebgm_appStorage);

    BLECM_SELECT_ADDR select_addr[4]; //Max 4 can be supported
    UINT8 TargetNum = 0;

    // The 31 bytes of Scan response data can hold up to 4 entries.
    for (i = 0; i < 4; i++)
    {

        if (curAppData->misc & BLEBGM_APP_FLAG_VALID)
        {
            // fill in the address.
            BT_MEMCPY(select_addr[TargetNum].addr, curAppData->bdAdr,BLEBGM_BD_ADR_LEN);
            select_addr[TargetNum].type = (curAppData->misc & BLEBGM_APP_FLAG_ADR_TYPE)>>15;

            TargetNum++;
        }
        curAppData++;
    }

    blecm_enableAddressSelection(); //turn on addr selection
    //blecm_SelectAddress(select_addr, TargetNum);
    blecm_SelectTargetAddress(select_addr, TargetNum); //set target addr
    blecm_disableAddressSelection(); //disable addr selection

    return TargetNum;
}

void blebgm_clientConfFlagToCheckAndStore(UINT16 flag,
        UINT16 flagToCheck, UINT16 flagToStore)
{
    BLEBGM_APP_STORAGE *curPtr = blebgm_getCurHostData();
    int len;

    if (curPtr)
    {
        if (flag & flagToCheck)
        {
            // notification is on.
            curPtr->misc |= flagToStore;
        }
        else
        {
            // notification is off.
            curPtr->misc &= ~flagToStore;
        }

        len = bleprofile_WriteNVRAM(
                VS_BLE_HOST_LIST, sizeof(bgmAppState->blebgm_appStorage),
                (UINT8 *)(bgmAppState->blebgm_appStorage));

        if (len != sizeof(bgmAppState->blebgm_appStorage))
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "storing appData failed. %d bytes", TVF_D(len));
        }
    }
}
