/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: blecgm.c
*
* Abstract: This file implements the BLE continuous glucose monitor profile, service, application
*
* Functions:
*
*******************************************************************************/
#include "blecgm.h"
#include "lesmpkeys.h"
#include "gpiodriver.h"
#include "dbfw_app.h"

//#include "blecgmspar.h" //only for spar application

//#include "uart.h"

#define FID    FID_BLEAPP_APP__BLECGM_C


// This is compilation flag enables a dummy database to be used
// for IOP testing purposes. When the real glucose meter functionality are
// available, it should be disabled.
#define BLECGM_IOP_ADVANCE_DB

#ifdef BLECGM_IOP_ADVANCE_DB_BIG
#ifdef BLE_P1
#define BLECGM_IOP_ADVANCE_DB_SIZE  20 //Flash can support 500 //10
#else
#define BLECGM_IOP_ADVANCE_DB_SIZE  200 //Flash can support 500 //10
#endif
#else
#define BLECGM_IOP_ADVANCE_DB_SIZE  10
#endif

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecgm_Timeout(UINT32 count);
void blecgm_FineTimeout(UINT32 finecount);

void blecgm_DBInit(void);
void blecgm_connUp(void);
void blecgm_connDown(void);
void blecgm_advStop(void);
void blecgm_reportRecordsCont(void);
void blecgm_IndicationConf(void);
void blecgm_transactionTimeout(void);
void blecgm_handleRACPStates(void );
void blecgm_loadAppData(void);
void blecgm_appTimerCb(UINT32 arg);
void blecgm_appFineTimerCb(UINT32 arg);
void blecgm_smpBondResult(LESMP_PARING_RESULT  result);
void blecgm_encryptionChanged(HCI_EVT_HDR *evt);
void blecgm_clearAllConf( int idx );
void blecgm_setsessionstarttime(UINT8 *ptr, INT32 len);
void blecgm_getthedateformat(UINT8 *tmp, UINT64 seconds, UINT16 year);
void blecgm_reportRecordNumber(BLEPROFILE_DB_PDU  *);
void blecgm_reportRecords(BLEPROFILE_DB_PDU  *);
void blecgm_clearRecords(BLEPROFILE_DB_PDU  *);
void blecgm_PUARTInitTxWaterlevel(UINT8 puart_tx_waterlevel);
void blecgm_PUARTSetTxWaterlevel(UINT8 puart_tx_waterlevel);
void blecgm_formRACPRecordCntInd(BLEPROFILE_DB_PDU *pdu,UINT8 Operator, UINT16 count );
void blecgm_sendAsManyMeasurements(BLEPROFILE_DB_PDU *);
void blecgm_racpStatusIndication( UINT8 reqCode, UINT8 status);
void blecgm_sendnotification(UINT16 timeoffset_minute, UINT32 count);
void blecgm_clientConfFlagToCheckAndStore(UINT16 flag, UINT16 flagToCheck, UINT16 flagToStore);

int blecgm_setupTargetAdrInScanRsp(void);
int blecgm_writeCb(LEGATTDB_ENTRY_HDR *p);
int blecgm_PUARTRxMaxFail(char *data, UINT8 len, UINT8 max);
int blecgm_startMeasurementReport(BLEPROFILE_DB_PDU  *);
int blecgm_reportNextMeasurement(BLEPROFILE_DB_PDU  *);
int blecgm_reportMeasurement(BLECGM_GLUCOSE_MEASUREMENT *ptr);
int blecgm_filterConditionMatched(BLECGM_GLUCOSE_MEASUREMENT *ptr );
int blecgm_checkClientConfigBeforeRACP(UINT16 handle);
int blecgm_lookupAppData(UINT8 *adr, UINT8 adrType );
int blecgm_addHost( UINT8 *adr, UINT8 adrType );

extern void utilslib_delayUs(UINT32 delay);
extern void wdog_restart(void);

UINT8 blecgm_getcgmcprspopcode(UINT8 *ptr, INT32 len, UINT8 *p_err);
UINT32 blecgm_addMeasurement(UINT16 offset, UINT32 count);

BLECGM_APP_STORAGE *blecgm_getHostData( UINT8 *adr, UINT8 adrType );
BLECGM_APP_STORAGE *blecgm_getCurHostData(void);

#ifdef BLECGM_IOP_ADVANCE_DB
void blecgm_create_iopdb(void);
#endif

#ifdef BLECGM_ASCPENABLE
void blecgm_PermAuthEnable(UINT8 Authentication);
#endif

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
#include "lel2cap.h"

#define GAP_CONN_PARAM_TIMEOUT 30
int blecgmhandleConnParamUpdateRsp(LEL2CAP_HDR *l2capHdr);

typedef int (*LEL2CAP_MSGHANDLER)(LEL2CAP_HDR*);
extern LEL2CAP_MSGHANDLER lel2cap_handleConnParamUpdateRsp;
#endif

#ifdef BLECGM_ASCPENABLE
//code to check the Authentication for ASCP
UINT8 blecgm_Authenticationcode[19] = {0x01,0x02,0x03,0x04,0x05};
UINT8 blecgm_Authcodelength = 5;
#endif


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blecgm_db_data[]=
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
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x12, // length
        0x10, // max length
        0x00, 0x2a, // uuid
	0x42,0x4c,0x45,0x20,0x43,0x47,0x4d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// "BLE CGM"
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
	0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7,
///////////////////////////////////////////////////////
// Primary service GLUCOSE METER.
0x00, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // Primary Service uuid
    UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT&0x00FF,
    (UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT&0xFF00)>>8,     // Glucose Service
0x01, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // Characteristic Declaration uuid
	0x10,0x02,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_MEASUREMENT&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_MEASUREMENT&0xFF00)>>8,  
0x02, 0x01, // <<<<<<<<<<<<<<<<<<Handle for reporting Gluclose Measurement
        0,//LEGATTDB_PERM_READABLE,
        0x0F, // length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_MEASUREMENT&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_MEASUREMENT&0xFF00)>>8, // uuid of Glucose Measurement
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x03, 0x01, // <<<<< Client Characteristic Configuration.
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,
// Glucose Features.
0x04, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x05,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_FEATURES&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_FEATURES&0xFF00)>>8,
0x05, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        6, // length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_FEATURES&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_FEATURES&0xFF00)>>8,   // UUID for Glucose Features.
        0x01, 0x00, 0x2E,0xF1,     // features that we support.
// Glucose Status.
0x06, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x07,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_STATUS&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_STATUS&0xFF00)>>8,
0x07, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        5, // length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_STATUS&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_STATUS&0xFF00)>>8,   // UUID for Glucose Status.
        0x00,0x00,0x00,
// Glucose Session Start Time.
0x08, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x0E,0x09,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_SESSION&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_SESSION&0xFF00)>>8,
0x09, 0x01, // handle
        LEGATTDB_PERM_READABLE       
       |LEGATTDB_PERM_WRITE_CMD
       |LEGATTDB_PERM_WRITE_REQ,
        0x0B, // length
        0x09, // max write length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_SESSION&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_SESSION&0xFF00)>>8,   // UUID for 
        0xDD,0x07,0x01,0x01,0x00,0x00,0x00,0x00,0x00,  //default start time is 2013-1-1-0-0-0.
// Glucose Session Run Time.
0x0A, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x0B,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RUNTIME&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RUNTIME&0xFF00)>>8,
0x0B, 0x01, // handle
        LEGATTDB_PERM_READABLE,       
        0x04, // length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RUNTIME&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RUNTIME&0xFF00)>>8,   // UUID for 
        0x00,0x18,
// RACP
0x0C, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x2C,0x0D,0x01,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RACP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RACP&0xFF00)>>8,    // UUID_RACP
0x0D, 0x01, // handle
       LEGATTDB_PERM_WRITE_CMD
       |LEGATTDB_PERM_WRITE_REQ
       |LEGATTDB_PERM_VARIABLE_LENGTH,
        0x14, // length
        0x12, // max write length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RACP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RACP&0xFF00)>>8, // uuid, Data Access Control point.
	0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,
0x0E, 0x01, // <<<<< Client Characteristic Configuration.
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,
#ifdef BLECGM_ASCPENABLE
0x0F, 0x01, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x2C,0x00,0x02,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_ASCP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_ASCP&0xFF00)>>8,    // UUID_ASCP
0x00, 0x02, // handle
       LEGATTDB_PERM_WRITE_CMD
       |LEGATTDB_PERM_WRITE_REQ
       |LEGATTDB_PERM_VARIABLE_LENGTH,
        0x16, // length
        0x14, // max write length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_ASCP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_ASCP&0xFF00)>>8,
	0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
0x01, 0x02, // <<<<< Client Characteristic Configuration.
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,
#endif
0x02, 0x02, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x2C,0x03,0x02,
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_CGMCP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_CGMCP&0xFF00)>>8,    // UUID_CGMCP
0x03, 0x02, // handle
       LEGATTDB_PERM_WRITE_CMD
       |LEGATTDB_PERM_WRITE_REQ
       |LEGATTDB_PERM_VARIABLE_LENGTH,
        0x15, // length
        0x13, // max write length
    UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_CGMCP&0x00FF,
    (UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_CGMCP&0xFF00)>>8,
	0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,
0x04, 0x02, // <<<<< Client Characteristic Configuration.
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,

};

const UINT16 blecgm_db_size = sizeof(blecgm_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blecgm_cfg =
{
    100, // UINT16 fine_timer_interval; //ms
    4, // HIGH_UNDIRECTED_DISCOVERABLE, // UINT8 default_adv; //default adv
    0, // UINT8 button_adv_toggle; //pairing button make adv toggle (if 1) or always on (if 0)
    32, //UINT16 high_undirect_adv_interval; // frames, 1.25 ms
    2048, //UINT16 low_undirect_adv_interval; //frames, 1.25 ms.
    30, // UINT16 high_undirect_adv_duration; //seconds
    300, // UINT16 low_undirected_adv_duration; //seconds
    0, //UINT16 high_direct_adv_interval; //seconds
    0,  //UINT16 low_direct_adv_interval; //seconds
    0,  // UINT16 high_direct_adv_duration; //seconds
    0, //  UINT16 low_direct_adv_duration; //seconds
    "BLE CGM", //char local_name[LOCAL_NAME_LEN_MAX];
    "\x00\x00\x00", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
#ifdef BLE_SECURITY_REQUEST
    SECURITY_ENABLED|SECURITY_REQUEST,
#else
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
#endif
    1, // UINT8 disc_required;// if 1, disconnection after confirmation
    1, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x04,  //  UINT8 tx_power_level; //dbm
    0, // UINT8 con_idle_timeout; //second   0-> no timeout
    0, //    UINT8 powersave_timeout; //second  0-> no timeout
    {0x102,
      0x105,
      0x107,
      0x10D,
      0x203}, // UINT16 hdl[HANDLE_NUM_MAX];   //GATT HANDLE number
    {UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT,
      UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT,
      UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT,
      UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT,
      UUID_SERVICE_CONTINUOUS_GLUECOSE_MEASUREMENT}, // UINT16 serv[HANDLE_NUM_MAX];  //GATT service UUID
    {UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_MEASUREMENT,
      UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_FEATURES,
      UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_STATUS,
      UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_RACP,
     UUID_CHARACTERISTIC_CONTINUOUS_GLUCOSE_CGMCP}, // UINT16 cha[HANDLE_NUM_MAX];   // GATT characteristic UUID
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
    1, // UINT8 button_client_timeout; // seconds
    3, //UINT8 button_discover_timeout; // seconds
    0, //UINT8 button_filter_timeout; // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_timeout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blecgm_puart_cfg =
{
    115200, // UINT32 baudrate;
    32, //24, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    33, //25, // UINT8  rxpin; //GPIO pin number
};

#ifdef BLE_P1
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blecgm_gpio_cfg =
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
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blecgm_gpio_cfg =
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
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blecgm_gpio_cfg =
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


#ifdef BLECGM_IOP_ADVANCE_DB
UINT16 blecgm_iop_advance_db_size = BLECGM_IOP_ADVANCE_DB_SIZE;
#endif

 int blecgm_noofEntries = 0;

typedef struct
{
    // NVRAM storage for application.
    BLECGM_APP_STORAGE blecgm_appStorage[BLECGM_MAX_BONDED_HOST];

#ifdef BLECGM_IOP_ADVANCE_DB
    BLECGM_GLUCOSE_MEASUREMENT *blecgm_iopData;
    int *blecgm_iopDataValid;
#endif

    // current operation.
    int blecgm_racpOperation;
    // current operator.
    int blecgm_racpOperator;
    // current filter type.
    int blecgm_racpFilterType;

    UINT32 blecgm_racpPktFormatStatus;

    // this is the currently reporting time offset.
    UINT16 blecgm_racpCurTimeOffset;
    // use max of 7 bytes to hold the time stamp info.
    UINT8 blecgm_racpFilterMin[TIMESTAMP_LEN];
    // use max of 7 bytes to hold the time stamp info.
    UINT8 blecgm_racpFilterMax[TIMESTAMP_LEN];

    // RACP operation related information.
    BLECGM_RACP_STATE blecgm_racpState;

    BOOL8 blecgm_racpOperationAbort;

    //this is for counting sent notification
    UINT16 blecgm_racpNotificationCnt;
#ifdef BLECGM_ASCPENABLE
    UINT8 blecgm_measurementHandleperm;
    UINT8 blecgm_statusHandleperm;
    UINT8 blecgm_sessionHandleperm;
    UINT8 blecgm_racpHandleperm;
    UINT8 blecgm_cgmcpHandleperm;

    UINT8 blecgm_measurementCCCHandleperm;
    UINT8 blecgm_racpCCCHandleperm;
    UINT8 blecgm_cgmcpCCCHandleperm;
#endif
    // This is the RACP handle.
    UINT16 blecgm_measurementHandle;
    UINT16 blecgm_statusHandle;
    UINT16 blecgm_sessionHandle;
    UINT16 blecgm_racpHandle;
    UINT16 blecgm_cgmcpHandle;
    UINT16 blecgm_featureHandle;

    UINT16 blecgm_measurementCCCHandle;
    UINT16 blecgm_racpCCCHandle;
    UINT16 blecgm_cgmcpCCCHandle;
#ifdef BLECGM_ASCPENABLE
    UINT32 blecgm_ascpPktFormatStatus;
    UINT16 blecgm_ascpCCCHandle;
    UINT16 blecgm_ascpHandle;
    UINT8 blecgm_ascpauthsuccess;
#endif

    // Currently indicated Measurement.
    BLECGM_GLUCOSE_MEASUREMENT *blecgm_curMeasureRecPtr;
    BLECGM_CGMCP_CALIBRATION blecgm_calibrationrecord;

    UINT32 blecgm_apptimer_count;
    UINT32 blecgm_appfinetimer_count;
    BD_ADDR blecgm_remote_addr;

    UINT8 blecgm_bat_enable;
#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    UINT32 blecgm_conparam_timeout;
    UINT8 blecgm_conparam_timeout_enable;
#endif
    UINT8 blecgm_communicationinterval;
    UINT16 blecgm_sessionnum;
    UINT16 blecgm_sessiontimeout;
    UINT16 blecgm_highalert;
    UINT16 blecgm_lowalert; 
    UINT16 blecgm_hyperlevel;
    UINT16 blecgm_hypolevel;
    UINT64 blecgm_timeinsec;
 
    UINT8 blecgm_null_operator_nsrr;

} tCgmAppState;

tCgmAppState *cgmAppState = NULL;


//////////////////////////////////////////
//   database access functions.
int blecgm_dbGetRecordCntSinceTimeOffset( int );
int blecgm_dbGetRecordCntlessThanTimeOffset( int );
int blecgm_dbGetRecordCntInRange( int , int);
int blecgm_dbClearRecordRange(int TimeOffsetMin, int TimeOffsetMax);
int blecgm_dbClearAllRecords(void);
int blecgm_dbClearLastRecords(void);
int blecgm_dbClearFirstRecords(void);
int blecgm_dbClearLessOrEqualRecords(int TimeOffsetMin);
int blecgm_dbClearLargerOrEqualRecords(int TimeOffsetMax);

BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetFirstMeasurement(void);
BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetNextMeasurement(int TimeOffset);
BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetLastMeasurement(void);
BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetMeasurement(int idx );
BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetMeasurement(int TimeOffset);


#ifdef BLECGM_FACETIME
void blecgm_findBaseTime(UINT8 *ptr);
int blecgm_dbClearLargerOrEqualRecordsTime(TIMESTAMP *TimeMin);
int blecgm_dbClearLessOrEqualRecordsTime(TIMESTAMP *TimeMax);
int blecgm_dbClearRecordRangeTime(TIMESTAMP *TimeMin, TIMESTAMP *TimeMax);
int blecgm_dbGetRecordCntInRangeTime(TIMESTAMP *startTime, TIMESTAMP *endTime);
int blecgm_dbGetRecordCntlessThanTime(TIMESTAMP *endTime);
int blecgm_dbGetRecordCntSinceTime(TIMESTAMP *startTime);
#endif

//Output GPIO
UINT8 blecgm_gpio_output = 4;//should be same as blecgm_gpio_cfg
UINT8 blecgm_PUART_RxInt_Enable = 0;
UINT8 blecgm_puart_tx_waterlevel=0;

void (*blecgm_PUART_RxIntUserCb)(void*)=NULL;

#define PUART_TX_WATERLEVEL  1 //13 // 1

void blecgm_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0], bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[2], bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    cgmAppState = (tCgmAppState *)cfa_mm_Sbrk(sizeof(tCgmAppState));
    memset(cgmAppState, 0x00, sizeof(tCgmAppState));

    //initialize the default value of cgmAppState
    cgmAppState->blecgm_null_operator_nsrr = 1;
    cgmAppState->blecgm_racpState = BLECGM_RACP_IDLE;

#ifdef BLECGM_IOP_ADVANCE_DB
    blecgm_create_iopdb();
#endif

#if 1
    // dump the database to debug uart.
    legattdb_dumpDb();
#endif

    if(bleprofile_p_cfg->test_enable)
    {
        //patient alert check.
        cgmAppState->blecgm_lowalert = 0x04;
        cgmAppState->blecgm_highalert = 0x40;
        cgmAppState->blecgm_hyperlevel = 0xC8; // generel hyper level 200 mg/dl.
        cgmAppState->blecgm_hypolevel = 0x48;//  generel hypo level 72 mg/dl. 
    }

    //blecm_enableRandomAddressResolution();

    // this would generate the adv payload and scan rsp payload.
    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);


    blecgm_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blecgm_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blecgm_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blecgm_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blecgm_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blecgm_smpBondResult);

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
             | LESMP_KEY_DISTRIBUTION_SIGN_KEY
    );

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
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blecgm_IndicationConf);
#endif

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    lel2cap_handleConnParamUpdateRsp = blecgmhandleConnParamUpdateRsp;
#endif

    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blecgm_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blecgm_writeCb);

    bleprofile_regTimerCb(blecgm_appFineTimerCb, blecgm_appTimerCb);
    bleprofile_StartTimer();

    // load the Bonded host info.
    blecgm_loadAppData();

    blecgm_connDown();
}

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
int blecgmhandleConnParamUpdateRsp(LEL2CAP_HDR *l2capHdr)
{
    LEL2CAP_COMMAND_HDR *cmdPkt = (LEL2CAP_COMMAND_HDR *) l2capHdr;
    UINT16 *p_result;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdateRsp", TVF_D(0));

    //disable timeout
    cgmAppState->blecgm_conparam_timeout_enable = 0;

    if(cmdPkt->len == 2)
    {
        p_result = (UINT16 *)(cmdPkt+1);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "result = %d", TVF_D(*p_result));

        if(*p_result == 0)
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

#ifdef BLECGM_IOP_ADVANCE_DB
void blecgm_create_iopdb(void)
{
    //malloc
    cgmAppState->blecgm_iopData =
        cfa_mm_Sbrk((UINT32)(sizeof(BLECGM_GLUCOSE_MEASUREMENT)*blecgm_iop_advance_db_size));

    cgmAppState->blecgm_iopDataValid =
        cfa_mm_Sbrk((UINT32)(sizeof(int)*blecgm_iop_advance_db_size));

    {
        extern UINT32 cfa_mm_MemFreeBytes(void);

        // free_ram is the number of bytes free. at this point.
        UINT32 free_ram = cfa_mm_MemFreeBytes();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "FreeRAM=%d", TVF_D(free_ram));
    }

    //copy
    memset(cgmAppState->blecgm_iopData, 0, sizeof(BLECGM_GLUCOSE_MEASUREMENT)*blecgm_iop_advance_db_size);
    memset(cgmAppState->blecgm_iopDataValid, 0, sizeof(int)*blecgm_iop_advance_db_size);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_create_iopdb: %d", TVF_D(blecgm_noofEntries));
}
#endif

void blecgm_Timeout(UINT32 count)
{
    if(cgmAppState->blecgm_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();

    if((cgmAppState->blecgm_sessionnum)&&(cgmAppState->blecgm_communicationinterval))
    {
        UINT8 interval = cgmAppState->blecgm_communicationinterval;

        cgmAppState->blecgm_sessiontimeout++;
        if(interval == 0xff)
        {
            //fastest communication interval is 1 minute.
            interval = 1;
        }
        //if timer expires, add one record
        if(cgmAppState->blecgm_sessiontimeout == interval*60) //sessiontimeout is second, communicationinterval is minute
        {
            UINT16 timeoffset_minute = (cgmAppState->blecgm_sessionnum * interval)+(cgmAppState->blecgm_sessiontimeout/60);
            cgmAppState->blecgm_timeinsec = (cgmAppState->blecgm_sessionnum * interval*60)+cgmAppState->blecgm_sessiontimeout;

             //send notification.
            blecgm_sendnotification(timeoffset_minute, count);    
        }
    }

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    #define BT_ERROR_CODE_LE_UNACCEPTABLE_CONNECTION_INTERVAL 0x3B

    //check time out
    if(cgmAppState->blecgm_conparam_timeout_enable &&
       cgmAppState->blecgm_apptimer_count == cgmAppState->blecgm_conparam_timeout)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Disc-conn param TO", TVF_D(0));
        blecm_disconnect(BT_ERROR_CODE_LE_UNACCEPTABLE_CONNECTION_INTERVAL);
        cgmAppState->blecgm_conparam_timeout_enable = 0;
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
        bleprofile_DeleteNVRAM( STACKNVRAM_LOCAL_KEYS);
        bleprofile_DeleteNVRAM(STACKNVRAM_BOND_INDEX );
        for(i=0; i<lesmpkeys_maxBondedDevices;  i++)
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

        status = bleprofile_WriteNVRAM( STACKNVRAM_LOCAL_KEYS,
                                sizeof(LESMPKEYS_LOCAL_ROOT_KEY),
                                (UINT8 *)&local_key);
        if( status != sizeof(LESMPKEYS_LOCAL_ROOT_KEY))
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
        if( status != sizeof(LESMPKEYS_BOND_IDX) * lesmpkeys_maxBondedDevices)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Bondindex Write Failed: %d/%d", TVF_WW(status, sizeof(LESMPKEYS_BOND_IDX) * lesmpkeys_maxBondedDevices));
        }
        else
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Bondindex Write Success: %d", TVF_D(status));
        }

        for(i=0; i<number;  i++)
        {
            status = bleprofile_WriteNVRAM(STACKNVRAM_FIRST_BONDED_IDX + i ,
                                size,
                                     (UINT8 *)&bondinfo);
            if( status != size)
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Failed: %d/%d", TVF_BBW(i, status, size));
            }
            else
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bondinfo[%d] Write Success: %d", TVF_BB(i, status));
            }
        }
        for(i=number; i<number+number2;  i++)
        {
            status = bleprofile_WriteNVRAM(STACKNVRAM_FIRST_BONDED_IDX + i ,
                                size2,
                                     (UINT8 *)&bondinfo);
            if( status != size2)
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

void blecgm_sendnotification(UINT16 timeoffset_minute, UINT32 count)
{
    BLEPROFILE_DB_PDU dbPdu;
    BLECGM_GLUCOSE_MEASUREMENT *measurementPtr;
    
    //create a new record.
    UINT32 data_added = blecgm_addMeasurement(timeoffset_minute, count);

    if(data_added && (blecgm_checkClientConfigBeforeRACP(cgmAppState->blecgm_measurementHandle) == 0))
    {
        measurementPtr = blecgm_dbGetLastMeasurement();
     
        dbPdu.len = measurementPtr->size;
        
        // copy the measurement data.
        BT_MEMCPY(dbPdu.pdu, measurementPtr, measurementPtr->size);
#if 1
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Notification handle = %d", TVF_D(cgmAppState->blecgm_measurementHandle));//debug only
#endif
        bleprofile_sendNotification(
                cgmAppState->blecgm_measurementHandle,dbPdu.pdu,dbPdu.len);
            
        cgmAppState->blecgm_sessionnum++;

        cgmAppState->blecgm_sessiontimeout=0;  
    }
}

void blecgm_FineTimeout(UINT32 finecount)
{
    if(bleprofile_p_cfg->test_enable)
    {
        // let the RACP get a chance to do things.
        blecgm_handleRACPStates();
    }

    // button control
    bleprofile_ReadButton();
}

void blecgm_DBInit(void)
{
    int i;

    //load handle number
    for(i=0; i<HANDLE_NUM_MAX; i++)
    {
        if (bleprofile_p_cfg->serv[i] == uuid_service_continuous_glucose_measurement &&
            bleprofile_p_cfg->cha[i]  == uuid_characteristic_continuous_glucose_measurement)
        {
            cgmAppState->blecgm_measurementHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_measurementHandle:%04x", TVF_D(cgmAppState->blecgm_measurementHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_measurementHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_measurementHandle);
#endif              
            cgmAppState->blecgm_measurementCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_measurementCCCHandle:%04x", TVF_D(cgmAppState->blecgm_measurementCCCHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_measurementCCCHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_measurementCCCHandle);
#endif
        }
        else if (bleprofile_p_cfg->serv[i] == uuid_service_continuous_glucose_measurement &&
                 bleprofile_p_cfg->cha[i]  == uuid_characteristic_continuous_glucose_status)
        {
            cgmAppState->blecgm_statusHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_statusHandle:%04x", TVF_D(cgmAppState->blecgm_statusHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_statusHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_statusHandle);
#endif
     
           //session hadle should always be incremented by 2. All the handles must be saved.
           //All the handles are needed to set the permissions of the GATT DB after the authorization of ASCP.
            cgmAppState->blecgm_sessionHandle=bleprofile_p_cfg->hdl[i]+2; 
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_sessionHandle:%04x", TVF_D(cgmAppState->blecgm_sessionHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_sessionHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_sessionHandle);
#endif
        }
        else if (bleprofile_p_cfg->serv[i] == uuid_service_continuous_glucose_measurement &&
                 bleprofile_p_cfg->cha[i]  == uuid_characteristic_continuous_glucose_racp)
        {
            cgmAppState->blecgm_racpHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_racpHandle:%04x", TVF_D(cgmAppState->blecgm_racpHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_racpHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_racpHandle);
#endif
            cgmAppState->blecgm_racpCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_racpCCCHandle:%04x", TVF_D(cgmAppState->blecgm_racpCCCHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_racpCCCHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_racpCCCHandle);
#endif
        }
        else if (bleprofile_p_cfg->serv[i] == uuid_service_continuous_glucose_measurement &&
                 bleprofile_p_cfg->cha[i]  == uuid_characteristic_continuous_glucose_cgmcp)
        {
            cgmAppState->blecgm_cgmcpHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_cgmcpHandle:%04x", TVF_D(cgmAppState->blecgm_cgmcpHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_cgmcpHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_cgmcpHandle);
#endif

            cgmAppState->blecgm_cgmcpCCCHandle=bleprofile_p_cfg->hdl[i]+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_cgmcpCCCHandle:%04x", TVF_D(cgmAppState->blecgm_cgmcpCCCHandle));
#ifdef BLECGM_ASCPENABLE
            cgmAppState->blecgm_cgmcpCCCHandleperm =  legattdb_readPermByHandle(cgmAppState->blecgm_cgmcpCCCHandle);
#endif
        }
        else if (bleprofile_p_cfg->serv[i] == uuid_service_continuous_glucose_measurement &&
                 bleprofile_p_cfg->cha[i]  == uuid_characteristic_continuous_glucose_features)
        {
            cgmAppState->blecgm_featureHandle=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_featureHandle:%04x", TVF_D(cgmAppState->blecgm_featureHandle));
        }
        else if(bleprofile_p_cfg->serv[i]==UUID_SERVICE_BATTERY &&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            cgmAppState->blecgm_bat_enable = 1;
            blebat_Init();
        }
#ifdef BLECGM_ASCPENABLE
        {
            cgmAppState->blecgm_ascpHandle = 0x200;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_ascpHandle:%04x", TVF_D(cgmAppState->blecgm_ascpHandle));

            cgmAppState->blecgm_ascpCCCHandle = cgmAppState->blecgm_ascpHandle+1; //Choose next one.
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_ascpCCCHandle:%04x", TVF_D(cgmAppState->blecgm_ascpCCCHandle));
        }
#endif
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(blecgm_gm_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif

#ifdef BLECGM_ASCPENABLE
    //authentication is not done yet. default value should be 0.
    blecgm_PermAuthEnable(cgmAppState->blecgm_ascpauthsuccess);
#endif

}

#ifdef BLECGM_ASCPENABLE
void blecgm_PermAuthEnable(UINT8 Authentication)
{
    //check for authorization.
    if(Authentication)
    { 
        legattdb_writePermByHandle(cgmAppState->blecgm_measurementHandle, cgmAppState->blecgm_measurementHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_measurementCCCHandle, cgmAppState->blecgm_measurementCCCHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_statusHandle, cgmAppState->blecgm_statusHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_sessionHandle, cgmAppState->blecgm_sessionHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_racpHandle, cgmAppState->blecgm_racpHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_racpCCCHandle, cgmAppState->blecgm_racpCCCHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_cgmcpHandle, cgmAppState->blecgm_cgmcpHandleperm);
        legattdb_writePermByHandle(cgmAppState->blecgm_cgmcpCCCHandle, cgmAppState->blecgm_cgmcpCCCHandleperm);
    }
    else
    {
        //if not authorized, set the permissions as NONE.
        //LEGATTDB_PERM_AUTH_WRITABLE is set only for the writable permission by default.
        legattdb_writePermByHandle(cgmAppState->blecgm_measurementHandle, LEGATTDB_PERM_NONE);
        legattdb_writePermByHandle(cgmAppState->blecgm_measurementCCCHandle, LEGATTDB_PERM_AUTH_WRITABLE);
        legattdb_writePermByHandle(cgmAppState->blecgm_statusHandle, LEGATTDB_PERM_NONE);
        legattdb_writePermByHandle(cgmAppState->blecgm_sessionHandle, LEGATTDB_PERM_AUTH_WRITABLE);
        legattdb_writePermByHandle(cgmAppState->blecgm_racpHandle, LEGATTDB_PERM_AUTH_WRITABLE);
        legattdb_writePermByHandle(cgmAppState->blecgm_racpCCCHandle, LEGATTDB_PERM_AUTH_WRITABLE);
        legattdb_writePermByHandle(cgmAppState->blecgm_cgmcpHandle, LEGATTDB_PERM_AUTH_WRITABLE);
        legattdb_writePermByHandle(cgmAppState->blecgm_cgmcpCCCHandle, LEGATTDB_PERM_AUTH_WRITABLE);
    }
}
#endif

void blecgm_connUp(void)
{
    int adrType   = lesmpkeys_getPeerAdrType();
    UINT8 *bdAdr  = lesmpkeys_getPeerAdr();

#ifdef BLE_CONNECTION_PARAMETER_UPDATE
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParam:Interval: %d, Latency: %d", TVF_WW(emconninfo_getConnInterval(), emconninfo_getSlaveLatency()));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParam: Supervision TO%d", TVF_D(emconninfo_getSupervisionTimeout()));

    //UINT16 minInterval, UINT16 maxInterval, UINT16 slaveLatency, UINT16 timeout
    lel2cap_sendConnParamUpdateReq(16, 32, 0, 100);
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdateReq:16,32,0,100", TVF_D(0));

    // save timeout
    cgmAppState->blecgm_conparam_timeout = cgmAppState->blecgm_apptimer_count + GAP_CONN_PARAM_TIMEOUT;
    cgmAppState->blecgm_conparam_timeout_enable = 1;
#endif

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);

    // clear this pointer.
    cgmAppState->blecgm_curMeasureRecPtr = NULL;
    
    // set the state to idle.
    cgmAppState->blecgm_racpState = BLECGM_RACP_IDLE ;

    if((bleprofile_p_cfg->encr_required&SECURITY_REQUEST) == SECURITY_REQUEST)
    {
        lesmp_sendSecurityRequest();
    }

}

void blecgm_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "connDown:", TVF_D(0));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "DiscReason:%02x", TVF_D(emconinfo_getDiscReason()));

    // clear this pointer.
    cgmAppState->blecgm_curMeasureRecPtr = NULL;

    // set the state to idle.
    cgmAppState->blecgm_racpState = BLECGM_RACP_IDLE ;

    // We may not want to blinded do this. This function call is used for
    // IOP test cases.
    // Setup the Target Address in the scan response payload.
    blecgm_setupTargetAdrInScanRsp();

    // Mandatory discovery mode
    if(bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, NULL );
    }

#ifdef BLECGM_ASCPENABLE
    //need authorization after the connection down.
    cgmAppState->blecgm_ascpauthsuccess = 0;
            
    //set the permission to NONE.
    blecgm_PermAuthEnable(cgmAppState->blecgm_ascpauthsuccess);
#endif
}

void blecgm_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADVStop", TVF_D(0));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "StartAdv", TVF_D(0));
    blecgm_connDown( );
}

void blecgm_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (cgmAppState->blecgm_apptimer_count)++;
                blecgm_Timeout(cgmAppState->blecgm_apptimer_count);
            }
            break;
    }
}

void blecgm_appFineTimerCb(UINT32 arg)
{
    (cgmAppState->blecgm_appfinetimer_count)++;

    blecgm_FineTimeout(cgmAppState->blecgm_appfinetimer_count);
}

void blecgm_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm, bond result %02x", TVF_D(result));

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
        int idx;
        int adrType   = lesmpkeys_getPeerAdrType();
        UINT8 *bdAdr  = lesmpkeys_getPeerAdr();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "adr type = %02x", TVF_D(adrType));
        ble_tracen((char *)bdAdr, 6 );

#if 0
        {
            //select one device for the next connection

            BLECM_SELECT_ADDR select_addr[1];
            int i;

            BT_MEMCPY(select_addr[0].addr, bdAdr, 6);
            select_addr[0].type = adrType;

            ble_trace1("index = %d", lesmpkeys_find(bdAdr, adrType));

            for(i=0; i<5; i++)
            {
                if(blecm_findAddress(i, (UINT8 *)bdAdr, &adrType))
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

                //lesmp_logBytes("\rFind Bonded Info:", bdAddr, 6 );
                bondedIndexBlock = (LESMPKEYS_BOND_IDX*)lesmpkeys_getBondedInfoIdx();

                if( bondedIndexBlock )
                {
                    LESMPKEYS_BOND_IDX *curPtr = bondedIndexBlock;

                    for(i=0;i< lesmpkeys_maxBondedDevices ; i++)
                    {
                        ble_trace3("index:%d, adrType:%d, idx :%02x", i, curPtr->adrType, curPtr->idx);
                        ble_tracen(curPtr->address, 6);

                        // move to next block.
                        curPtr ++;
                    }

                    cfa_mm_Free(bondedIndexBlock);
                }
            }
#endif
           //blecm_enableAddressSelection();
           //blecm_disableAddressSelection();
           //blecm_SelectAddress(select_addr, 1);
           //blecm_enableRandomAddressResolution();
        }
#endif
        // do we have record of this device.
        idx = blecgm_lookupAppData( bdAdr, adrType );

        if( idx != -1 )
        {
            // We rebonded with a known device.
            blecgm_clearAllConf(idx);
        }
        else
        {
            // this is a new host.
            blecgm_addHost( bdAdr, adrType );
        }
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "smpFailed,discLink", TVF_D(0));
        blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
    }
}

void blecgm_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "encChanged", TVF_D(0));

#if 0
    // TBD
    if(bleprofile_ReadNVRAM(
           VS_BLE_HOST_LIST,
           sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blecgm_hostinfo))
    {
        if(memcmp(blecgm_hostinfo.bdAddr, emconninfo_getPeerAddr(), 6)==0)
        {
            ble_trace2("\rEncOnLastPairedDev:%08x%04x",
                      (blecgm_hostinfo.bdAddr[5]<<24)+(blecgm_hostinfo.bdAddr[4]<<16)+
                      (blecgm_hostinfo.bdAddr[3]<<8)+blecgm_hostinfo.bdAddr[2],
                      (blecgm_hostinfo.bdAddr[1]<<8)+blecgm_hostinfo.bdAddr[0]);
        }
    }

    if(bleprofile_p_cfg->encr_required)
    {
        blecgm_indication_enable = 1; //indication enable
        blecgm_indication_defer = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "IndOn", TVF_D(0));
    }
#endif
}

void blecgm_IndicationConf(void)
{
    if(cgmAppState->blecgm_racpState  == BLECGM_RACP_ACTIVE )
    {
        // only when the racp state is active, we need to do something.
        switch( cgmAppState->blecgm_racpOperation)
        {
            case BLECGM_RACP_REQ_NUM_OF_RECORDS:
                // we got Indication Confirmation.
                cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
                break;
            case BLECGM_RACP_REPORT_RECORDS:
                // We are reporting records.
                // The only Indication Conf we should be receiving is
                // the Indication for completion.
                cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
                break;
            case BLECGM_RACP_CLEAR_RECORDS:
                cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
                break;
            case BLECGM_RACP_ABORT_REPORTING:
                cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
                break;
        }
    }
    else if ( cgmAppState->blecgm_racpState == BLECGM_RACP_COMPLETE )
    {
        // we are expecting this.
        if(cgmAppState->blecgm_racpOperationAbort)
        {
            // we need to Indicate Abort success.
            blecgm_racpStatusIndication(BLECGM_RACP_ABORT_REPORTING,
                                BLECGM_RACP_RSP_CODE_SUCCESS );
            cgmAppState->blecgm_racpOperationAbort=FALSE;
        }
        else
        {
            cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
        }
    }
}

void blecgm_transactionTimeout(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ATT TO", TVF_D(0));

    if((bleprofile_p_cfg->disc_required) & DISC_ATT_TIMEOUT)
    {
        blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "DiscATT TO", TVF_D(0));
    }
}

int blecgm_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);
    int len = legattdb_getAttrValueLen(p);
    UINT8 *attrPtr = legattdb_getAttrValue(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "GM Write handle =0x%04x, length = 0x%04x", TVF_WW(handle, len));
    ble_tracen ((char *) attrPtr, len );

    if( handle == cgmAppState->blecgm_racpHandle)
    {
        if(bleprofile_p_cfg->test_enable)
        {
            BLECGM_RACP_HDR *racpHdr = (BLECGM_RACP_HDR *) attrPtr ;

            switch ( cgmAppState->blecgm_racpState )
            {
                case BLECGM_RACP_IDLE:
                    // We need to check for Client Characteristic Configuration
                    // before we accept the racp operation.
                    if(blecgm_checkClientConfigBeforeRACP(handle))
                    {
                        // RACP operation prediction has not met.
                        return BLECGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY;
                    }
                    break;

                case BLECGM_RACP_COMPLETE:
                case BLECGM_RACP_ACTIVE:
                case BLECGM_RACP_PEND:
                    // We got request while we are processing
                    // request. Abort has already been checked so this is
                    // not abort.
                    return BLECGM_RACP_RSP_PROCEDURE_ALREADY_IN_PROGRESS;

                default:
                    //
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RACPReqUnknownState", TVF_D(0));
                    break;
            }

            // We need to check the racp operation for packet
            // integrity.
            cgmAppState->blecgm_racpPktFormatStatus = blecgm_checkRACPformat(attrPtr, len);

            if( !(cgmAppState->blecgm_racpPktFormatStatus) )
            {
                // only if there are no errors we want to proceed with the RACP
                // operation.

                // look for abort first.
                if( racpHdr->opCode == BLECGM_RACP_ABORT_REPORTING )
                {
                    // Here is an abort.
                    cgmAppState->blecgm_racpOperationAbort = TRUE;
                }
                else
                {
                    // we got a new request.
                    cgmAppState->blecgm_racpState  = BLECGM_RACP_PEND;
                    // reset count
                    cgmAppState->blecgm_racpNotificationCnt = 0;
                }
            }
        }
    }
#ifdef BLECGM_ASCPENABLE
    else if( handle == cgmAppState->blecgm_ascpHandle )
    {
        if(bleprofile_p_cfg->test_enable)
        {
            UINT8 tmp[3];
            BLECGM_ASCP_HDR *ascpHdr = (BLECGM_ASCP_HDR *) attrPtr ;

            // We need to check for Client Characteristic Configuration
            // before we accept the ascp operation.
            if(blecgm_checkClientConfigBeforeRACP(handle))
            {
                // ASCP operation prediction has not met.
                return BLECGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY;
            }

            if((len>=5)&&(len<=20))  //length of the characteristics.
            {
                cgmAppState->blecgm_ascpPktFormatStatus = blecgm_checkASCPformat(ascpHdr, len);
            }
            else
            {
                cgmAppState->blecgm_ascpPktFormatStatus = BLECGM_ASCP_RSP_CODE_INVALID_OPERAND;
            }
            //set the indication
            tmp[0] = BLECGM_ASCP_AUTH_RESPONSE;
            tmp[1] = attrPtr[0];
            tmp[2] = cgmAppState->blecgm_ascpPktFormatStatus;
#if 1
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ASCP handle = %d", TVF_D(cgmAppState->blecgm_ascpHandle));//debug only
#endif
            bleprofile_sendIndication( cgmAppState->blecgm_ascpHandle, tmp, 3, NULL);//Indication Confirmation cb is not needed.

        }
    }
#endif
    else if( handle == cgmAppState->blecgm_cgmcpHandle )
    {
        if(bleprofile_p_cfg->test_enable)
        {
            UINT8 length;
            UINT8 tmp[20];
            UINT8 err_code = BLECGM_CGMCP_RSP_CODE_SUCCESS;//set to success by default.
            UINT8 rspopcode;
           
            // We need to check for Client Characteristic Configuration
            // before we accept the cgmcp operation.
            if(blecgm_checkClientConfigBeforeRACP(handle))
            {
                // CGMCP operation prediction has not met.
                return BLECGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY;
            }

            rspopcode = blecgm_getcgmcprspopcode(attrPtr, len, &err_code);
           
            if(rspopcode == BLECGM_CGMCP_RSP_OPCODE)
            {
                tmp[0] = rspopcode;
                tmp[1] = attrPtr[0];
                tmp[2] = err_code;
                length = 3;
            }
            else
            {
                tmp[0] = rspopcode;
                //set the read rsp value.
                switch(rspopcode)
                {
                    case BLECGM_CGMCP_RSP_GET_COMMUNICATION_INTERVAL:
                        tmp[1] = cgmAppState->blecgm_communicationinterval;
                        length = 2;//op code lenght is added.
                    break;

                    case BLECGM_CGMCP_RSP_GET_CALIBRATION_VALUE:
                        // we only use one byte status
                        memcpy(&tmp[1],(UINT8*)&(cgmAppState->blecgm_calibrationrecord), sizeof(BLECGM_CGMCP_CALIBRATION)-8);
                        length = sizeof(BLECGM_CGMCP_CALIBRATION)-8+1;//op code lenght is added.
                    break;

                    case BLECGM_CGMCP_RSP_GET_HIGH_ALERT:
                        memcpy(&tmp[1], (UINT8*)&cgmAppState->blecgm_highalert, 2);
                        length = 3;//op code lenght is added.
                    break;

                    case BLECGM_CGMCP_RSP_GET_LOW_ALERT:
                        memcpy(&tmp[1],(UINT8*)&cgmAppState->blecgm_lowalert, 2);
                        length = 3;//op code lenght is added.
                    break;
                }
            }
#if 1
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CGMCP handle=%d", TVF_D(cgmAppState->blecgm_cgmcpHandle));//debug only
#endif
            bleprofile_sendIndication(cgmAppState->blecgm_cgmcpHandle, tmp, length, NULL);//Indication Confirmation cb is not needed.
        }
    }
    else if(handle ==cgmAppState->blecgm_sessionHandle)
    {
        BLEPROFILE_DB_PDU db_pdu;

        bleprofile_ReadHandle(cgmAppState->blecgm_featureHandle,&db_pdu);

        if(db_pdu.pdu[1]& 0x10)//crc is enabled
        {
            if(len == 11)
            {
                blecgm_setsessionstarttime(attrPtr, len);
            }
        }
        else if(len == 9)
        {
            blecgm_setsessionstarttime(attrPtr, len);
        }
    }
    else if( handle == cgmAppState->blecgm_measurementCCCHandle)
    {
        blecgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_NOTIFICATION,
                BLECGM_APP_FLAG_MEASUREMENT_CONF);
    }
    else if( handle == cgmAppState->blecgm_racpCCCHandle )
    {
        blecgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_INDICATION ,
                BLECGM_APP_FLAG_RACP_CONF );
    }
#ifdef BLECGM_ASCPENABLE
    else if( handle == cgmAppState->blecgm_ascpCCCHandle )
    {
        blecgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_INDICATION ,
                BLECGM_APP_FLAG_RACP_CONF );
    }
#endif
    else if( handle == cgmAppState->blecgm_cgmcpCCCHandle )
    {
        blecgm_clientConfFlagToCheckAndStore(*attrPtr,
                LEATT_CLIENT_CONFIG_INDICATION ,
                BLECGM_APP_FLAG_RACP_CONF );
    }

    return 0;
}

void blecgm_setsessionstarttime(UINT8 *ptr, INT32 len)
{
    BLEPROFILE_DB_PDU dbPdu;

    UINT64 totalnoofsec;

    //year shouldnot be zero.
    UINT8 tmp[9] = {1, 0, 0, 0, 0, 0, 0, 0, 0};

    UINT16 year = (ptr[1] << 8)|(ptr[0]);

    UINT64 timeinsec;

    //do not copy year.
    memcpy(&(tmp[2]), &(ptr[2]), len-2);

    timeinsec = blecgm_getTimeStampInseconds(tmp, 0);
    
    if(timeinsec < (cgmAppState->blecgm_timeinsec))
    {
        //session started previous year.
        year--;

        if(year%400 == 0)
        {
            //leap year.
            totalnoofsec = (366 * 24 * 3600);
        }
        else if(year%100 == 0)
        {
            totalnoofsec = (365 * 24 * 3600);
        }
        else if(year%4 == 0)
        {
            //leap year.
            totalnoofsec = (366 * 24 * 3600);
        }
        else
        {
            totalnoofsec = (365 * 24 * 3600);
        }

        timeinsec+=totalnoofsec; 
    }
  
    tmp[0] = year&0x00FF;
    tmp[1] = (year&0xFF00)>>8;

    timeinsec -= (cgmAppState->blecgm_timeinsec);

    blecgm_getthedateformat(&(tmp[2]),timeinsec,year);
 
    dbPdu.len = len;
    BT_MEMCPY(dbPdu.pdu, &tmp, dbPdu.len);
    bleprofile_WriteHandle(cgmAppState->blecgm_sessionHandle, &dbPdu);
}

void blecgm_getthedateformat(UINT8 *tmp, UINT64 seconds, UINT16 year)
{
    UINT8 noofdays[12] = 
        {31, 28, 31, 30, 
         31, 30, 31, 31, 
         30, 31, 30, 31};

    UINT64 modulus;
    int i;

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
    
    tmp[4] = (UINT8)(seconds%60); //second
    modulus = seconds/60;
    tmp[3] = (modulus%60); //minute
    modulus = modulus/60;
    tmp[2] = (modulus%24); //hour
    modulus = modulus/24;  //days

    for(i=0;i<12;i++)
    {
        if(modulus > noofdays[i])
        {
            modulus -= noofdays[i];
        }
        else
        {
            tmp[0] = i+1;
            break;
        }
    }
    tmp[1] = modulus+1;
}


UINT8 blecgm_getcgmcprspopcode(UINT8 *ptr, INT32 len, UINT8 *p_err)
{
    UINT8 status = 0;
    
    switch(ptr[0]) //OP code
    {
        case BLECGM_CGMCP_SET_COMMUNICATION_INTERVAL:
               if(len==2)
                {
                    cgmAppState->blecgm_communicationinterval = ptr[1];
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;//error code is set.
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        case BLECGM_CGMCP_GET_COMMUNICATION_INTERVAL:
               if(len==1)
                {
                    status = BLECGM_CGMCP_RSP_GET_COMMUNICATION_INTERVAL;
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                    status = BLECGM_CGMCP_RSP_OPCODE;//
                }
            break;
        case BLECGM_CGMCP_SET_CALIBRATION_VALUE:
                if((len>=2)&&(len<=20))
                {
                    UINT16 prev_RN = cgmAppState->blecgm_calibrationrecord.DataRecordNumber;
                    memcpy((UINT8*)&(cgmAppState->blecgm_calibrationrecord), &ptr[1], len-1);
                    cgmAppState->blecgm_calibrationrecord.DataRecordNumber = prev_RN+1;
                    cgmAppState->blecgm_calibrationrecord.Status[0] = 0;
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        case BLECGM_CGMCP_GET_CALIBRATION_VALUE:
               if(len==3)
                {
                    //only one record at this time. will be extended later.
                    if((cgmAppState->blecgm_calibrationrecord.DataRecordNumber == ptr[1]+(ptr[2]<<8))||
                       (ptr[1]+(ptr[2]<<8)==0xFFFF))
                    {
                        status = BLECGM_CGMCP_RSP_GET_CALIBRATION_VALUE;
                    }
                    else
                    {
                        *p_err = BLECGM_CGMCP_RSP_CODE_PARAMETER_OUT_OF_RANGE;
                        status = BLECGM_CGMCP_RSP_OPCODE;
                    }
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                    status = BLECGM_CGMCP_RSP_OPCODE;
                }
            break;
        case BLECGM_CGMCP_SET_HIGH_ALERT:
               if(len==3)
                {
                    cgmAppState->blecgm_highalert = ptr[1]+(ptr[2]<<8); // 
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        case BLECGM_CGMCP_GET_HIGH_ALERT:
               if(len==1)
                {
                    status = BLECGM_CGMCP_RSP_GET_HIGH_ALERT;
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                    status = BLECGM_CGMCP_RSP_OPCODE;
                }
            break;
        case BLECGM_CGMCP_SET_LOW_ALERT:
               if(len==3)
                {
                    cgmAppState->blecgm_lowalert =  ptr[1]+(ptr[2]<<8);
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        case BLECGM_CGMCP_GET_LOW_ALERT:
               if(len==1)
                {
                    status = BLECGM_CGMCP_RSP_GET_LOW_ALERT;
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                    status = BLECGM_CGMCP_RSP_OPCODE;
                }
            break;
        case BLECGM_CGMCP_START_SESSION:
                if(len == 1)
                {
                    // delete all record
                    blecgm_dbClearAllRecords();
                    
                    if(cgmAppState->blecgm_communicationinterval)
                    {
                        // send the first record.
                        blecgm_sendnotification(0,cgmAppState->blecgm_apptimer_count); //offset is 0 when we start the session.
                    }
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_PARAMETER_OUT_OF_RANGE;
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        case BLECGM_CGMCP_STOP_SESSION:
                if(len == 1)
                {
                    cgmAppState->blecgm_sessionnum = 0;
                    cgmAppState->blecgm_sessiontimeout = 0;
                }
                else
                {
                    *p_err = BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND;
                }
            status = BLECGM_CGMCP_RSP_OPCODE;
            break;
        default:
            status = BLECGM_CGMCP_RSP_OPCODE;
            *p_err = BLECGM_CGMCP_RSP_CODE_OP_CODE_NOT_SUPPORTED;
            break;
    }

    return status;
}

void blecgm_reportRecordNumber( BLEPROFILE_DB_PDU *dbPdu)
{
    int sendInd = FALSE;
    int recCount = 0;
#if 0
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RptReNum", TVF_D(0));//debug only
#endif
    // maximum 20 bytes.
    ble_tracen ( (char *)dbPdu->pdu, dbPdu->len);

    switch(cgmAppState->blecgm_racpOperator)
    {
        case BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            // report all records.
            // fix me.
            {
                int i;
                for(i = 0; i< blecgm_noofEntries; i++)
                {
                    if(cgmAppState->blecgm_iopDataValid[i] )
                    {
                        recCount ++;
                    }
                }
            }
            blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator, recCount);
            sendInd = TRUE;
            break;
        case BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            // store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2];
            if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
            {
                recCount = blecgm_dbGetRecordCntSinceTimeOffset(
                                    dbPdu->pdu[3] | (dbPdu->pdu[4] << 8 ) );

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#ifdef BLECGM_FACETIME
            else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blecgm_dbGetRecordCntSinceTime((TIMESTAMP *)& (dbPdu->pdu[3]));

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#endif
            else
            {
                // It is something we don't understand need error response?
                // TBD
            }
            break;
        case BLECGM_RACP_OPERATOR_LESS_OR_EQUAL:
            // store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2];
            if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
            {
                recCount = blecgm_dbGetRecordCntlessThanTimeOffset(
                        dbPdu->pdu[3] | (dbPdu->pdu[4] << 8 ) );

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#ifdef BLECGM_FACETIME
            else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blecgm_dbGetRecordCntlessThanTime((TIMESTAMP *)& (dbPdu->pdu[3]));

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#endif
            else
            {
                // It is something we don't understand need error response?
                // TBD
            }
            break;
        case BLECGM_RACP_OPERATOR_IN_RANGE:
            // store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2];
            if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
            {
                recCount = blecgm_dbGetRecordCntInRange (
                        dbPdu->pdu[3] | (dbPdu->pdu[4] << 8 ),
                        dbPdu->pdu[5] | (dbPdu->pdu[6] << 8 )
                        );

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#ifdef BLECGM_FACETIME
            else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
            {
                recCount = blecgm_dbGetRecordCntInRangeTime (
                        (TIMESTAMP *)& (dbPdu->pdu[3]),
                        (TIMESTAMP *)& (dbPdu->pdu[10])
                        );

                blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator,recCount );
                sendInd = TRUE;
            }
#endif
            else
            {
                // It is something we don't understand need error response?
                // TBD
            }
            break;
        case BLECGM_RACP_OPERATOR_FIRST_RECORD:
        case BLECGM_RACP_OPERATOR_LAST_RECORD:
            blecgm_formRACPRecordCntInd(dbPdu,cgmAppState->blecgm_racpOperator, 1);
            sendInd = TRUE;
            break;
        default:
            break;
    }

    if( sendInd )
    {
#if 1
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RACP handle = %d", TVF_D(cgmAppState->blecgm_racpHandle));//debug only
#endif
        bleprofile_sendIndication( cgmAppState->blecgm_racpHandle,
                (UINT8 *)dbPdu->pdu, dbPdu->len, blecgm_IndicationConf);
    }
}

int blecgm_reportMeasurement(BLECGM_GLUCOSE_MEASUREMENT *measurementPtr)
{
    BLEPROFILE_DB_PDU dbPdu;

    if(measurementPtr)
    {
        dbPdu.len = measurementPtr->size;

        // copy the measurement data.
        BT_MEMCPY(dbPdu.pdu, measurementPtr, measurementPtr->size);
#if 1
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Notification handle = %d", TVF_D(cgmAppState->blecgm_measurementHandle));//debug only
#endif
        // We get something to send out.
        bleprofile_sendNotification(
                cgmAppState->blecgm_measurementHandle,dbPdu.pdu,dbPdu.len);

       cgmAppState->blecgm_racpNotificationCnt++;
    }

    return 0;
}

int blecgm_reportNextMeasurement(BLEPROFILE_DB_PDU  *dbPdu)
{
    BLECGM_GLUCOSE_MEASUREMENT *measurementPtr = NULL;

    if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS)
    {
        // retrieve the time offset.
        int timeOffset = cgmAppState->blecgm_racpCurTimeOffset ;

        measurementPtr = blecgm_dbGetNextMeasurement(timeOffset);

        if(measurementPtr)
        {
            blecgm_reportMeasurement(measurementPtr);
        }
        else
        {
            // not more record.
            return BLECGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
        // We need to store the time offset for the more than one
        // record query.
        cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;
    }
    else if (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        // retrieve the time offset.
        int timeOffset = cgmAppState->blecgm_racpCurTimeOffset ;

        measurementPtr = blecgm_dbGetNextMeasurement(timeOffset);

        if(measurementPtr)
        {
            // we need to check the condition.
            if( blecgm_filterConditionMatched(measurementPtr) )
            {
                blecgm_reportMeasurement(measurementPtr);
            }
            else
            {
                // This is out of range.
            }
        // We need to store the time offset for the more than one
        // record query.
        cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;
        }
        else
        {
            // not more record.
            return BLECGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }
    else if (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL)
    {
        // retrieve the time offset.
        int timeOffset = cgmAppState->blecgm_racpCurTimeOffset ;

        measurementPtr = blecgm_dbGetNextMeasurement(timeOffset);

        if(measurementPtr)
        {
            // we need to check the condition.
            if( blecgm_filterConditionMatched(measurementPtr) )
            {
                blecgm_reportMeasurement(measurementPtr);
            }
            // We need to store the time offset for the more than one
            // record query.
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;
        }
        else
        {
            // not more record.
            return BLECGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }
    else if (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE)
    {
        // retrieve the time offset.
        int timeOffset = cgmAppState->blecgm_racpCurTimeOffset ;

        measurementPtr = blecgm_dbGetNextMeasurement(timeOffset);

        if(measurementPtr)
        {
            // we need to check the condition.
            if( blecgm_filterConditionMatched(measurementPtr) )
            {
                blecgm_reportMeasurement(measurementPtr);
            }

            // We need to store the time offset for the more than one
            // record query.
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;
        }
        else
        {
            // not more record.
            return BLECGM_GET_DB_RECORD_NO_MORE_RECORD;
        }
    }

    return BLECGM_GET_DB_RECORD_SUCCESS;
}

int blecgm_startMeasurementReport(BLEPROFILE_DB_PDU  *dbPdu)
{
    BLECGM_GLUCOSE_MEASUREMENT *measurementPtr = NULL;

    if( (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_FIRST_RECORD)
                        ||
        (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS)
                        ||
        (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
                        ||
        (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL)
                        ||
        (cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE)
      )
    {
        measurementPtr = blecgm_dbGetFirstMeasurement();
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LAST_RECORD)
    {
        measurementPtr = blecgm_dbGetLastMeasurement();
    }

    if( !measurementPtr )
    {
        // we don't have anything in the db.
        return BLECGM_GET_DB_RECORD_NO_MORE_RECORD;
    }

    if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            // store the time offset.
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            // we need to check the condition.
            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#endif
        else
        {
            // undefined filter type.
        }
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL)
    {
        if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            // store the time offset.
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            // we need to check the condition.
            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#endif
        else
        {
            // undefined filter type.
        }
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE)
    {
        if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            // store the time offset.
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            // we need to check the condition.
            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType  == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;

            if( !blecgm_filterConditionMatched(measurementPtr) )
            {
                // This is does not match the condition.
                // Error Message.
                measurementPtr  = NULL;
            }
        }
#endif
        else
        {
            // undefined filter type.
        }

    }

    if( measurementPtr )
    {
        blecgm_reportMeasurement(measurementPtr);

        if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS)
        {
            cgmAppState->blecgm_racpCurTimeOffset = measurementPtr->timeOffset ;
        }
    }

    return BLECGM_GET_DB_RECORD_SUCCESS;
}

void blecgm_clearRecords(BLEPROFILE_DB_PDU  *dbPdu)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ClrRec", TVF_D(0));
    ble_tracen ( (char *)dbPdu->pdu, dbPdu->len);

    cgmAppState->blecgm_racpFilterType = dbPdu->pdu[2];

    if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE)
    {
        if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            // time offset filtering.
            int TimeOffsetMin = dbPdu->pdu[3] | (dbPdu->pdu[4]<<8 ) ;
            int TimeOffsetMax = dbPdu->pdu[5] | (dbPdu->pdu[6]<<8 ) ;

            if( blecgm_dbClearRecordRange(TimeOffsetMin, TimeOffsetMax) )
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // timestamp filtering.
            if( blecgm_dbClearRecordRangeTime((TIMESTAMP *)&(dbPdu->pdu[3]),
                                                                   (TIMESTAMP *)&(dbPdu->pdu[10])))
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#endif
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS)
    {
        // clear all stored records.
        if( blecgm_dbClearAllRecords() )
        {
            // no records found.
            // indicate completion with error.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                    BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
        }
        else
        {
            // indicate completion.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                    BLECGM_RACP_RSP_CODE_SUCCESS );
        }

        // wait for Indication Confirmation.
        cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_FIRST_RECORD)
    {
        // clear first record.
        if( blecgm_dbClearFirstRecords() )
        {
            // no records found.
            // indicate completion with error.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                   BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
        }
        else
        {
            // indicate completion.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                    BLECGM_RACP_RSP_CODE_SUCCESS );
        }

        // wait for Indication Confirmation.
        cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LAST_RECORD )
    {
        // clear lastRecord.
        if( blecgm_dbClearLastRecords() )
        {
            // no records found.
            // indicate completion with error.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                    BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
        }
        else
        {
            // indicate completion.
            blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                    BLECGM_RACP_RSP_CODE_SUCCESS );
        }
 
        // wait for Indication Confirmation.
        cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
    }
    else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
    {
        if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            int TimeOffsetMin = dbPdu->pdu[3] | (dbPdu->pdu[4]<<8 ) ;
            // clear less or equal.
            if( blecgm_dbClearLessOrEqualRecords(TimeOffsetMin) )
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // clear less or equal.
            if( blecgm_dbClearLessOrEqualRecordsTime((TIMESTAMP *)&(dbPdu->pdu[3])) )
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#endif
    }
    else if(cgmAppState->blecgm_racpOperator ==BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL )
    {
        if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            int TimeOffsetMin = dbPdu->pdu[3] | (dbPdu->pdu[4]<<8 ) ;
            // clear less or equal.
            if( blecgm_dbClearLargerOrEqualRecords(TimeOffsetMin) )
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#ifdef BLECGM_FACETIME
        else if(cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            // clear less or equal.
            if( blecgm_dbClearLargerOrEqualRecordsTime((TIMESTAMP *)&(dbPdu->pdu[3])) )
            {
                // no records found.
                // indicate completion with error.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND);
            }
            else
            {
                // indicate completion.
                blecgm_racpStatusIndication(BLECGM_RACP_CLEAR_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            // wait for Indication Confirmation.
            cgmAppState->blecgm_racpState = BLECGM_RACP_COMPLETE ;
        }
#endif
    }
}

void blecgm_reportRecords(BLEPROFILE_DB_PDU  *dbPdu)
{
    switch(cgmAppState->blecgm_racpOperator)
    {
        case BLECGM_RACP_OPERATOR_FIRST_RECORD:
        case BLECGM_RACP_OPERATOR_LAST_RECORD:
            if(blecgm_startMeasurementReport(dbPdu)
                                       == BLECGM_GET_DB_RECORD_SUCCESS )
            {
                // We are done.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                                BLECGM_RACP_RSP_CODE_SUCCESS );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            else
            {
                // We don't have anything there.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            break;
        case BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            if( blecgm_startMeasurementReport(dbPdu)
                                        == BLECGM_GET_DB_RECORD_SUCCESS )
            {
                blecgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            break;
        case BLECGM_RACP_OPERATOR_LESS_OR_EQUAL:
            // store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2] ;
            // Copy the filter Max.
            BT_MEMCPY(cgmAppState->blecgm_racpFilterMax,dbPdu->pdu + 3,TIMESTAMP_LEN);

            if( blecgm_startMeasurementReport(dbPdu)
                                        == BLECGM_GET_DB_RECORD_SUCCESS )
            {
                blecgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            break;
        case BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            // Store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2] ;
            // Copy the filter Max.
            BT_MEMCPY(cgmAppState->blecgm_racpFilterMin,dbPdu->pdu + 3,TIMESTAMP_LEN);
            if( blecgm_startMeasurementReport(dbPdu)
                                        == BLECGM_GET_DB_RECORD_SUCCESS )
            {
                blecgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            break;
        case BLECGM_RACP_OPERATOR_IN_RANGE:
            // Store the filter type here.
            cgmAppState->blecgm_racpFilterType  = dbPdu->pdu[2] ;
            //
            if( cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
            {
                BT_MEMCPY(cgmAppState->blecgm_racpFilterMin,dbPdu->pdu + 3, 2);
                BT_MEMCPY(cgmAppState->blecgm_racpFilterMax,dbPdu->pdu + 5, 2);
            }
            else
            {
                BT_MEMCPY(cgmAppState->blecgm_racpFilterMin,dbPdu->pdu + 3,TIMESTAMP_LEN );
                BT_MEMCPY(cgmAppState->blecgm_racpFilterMax,dbPdu->pdu + 3 + TIMESTAMP_LEN,
                                    TIMESTAMP_LEN );
            }

            if( blecgm_startMeasurementReport(dbPdu)
                                        == BLECGM_GET_DB_RECORD_SUCCESS )
            {
                blecgm_sendAsManyMeasurements(dbPdu);
            }
            else
            {
                // We don't have anything there.
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
                // waiting for the Infication Confirmation.
                cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;
            }
            break;
        default:
            break;
    }
}

// This function is intended to continue reporting.
void blecgm_reportRecordsCont(void)
{
    BLEPROFILE_DB_PDU dbPdu;

    switch( cgmAppState->blecgm_racpOperator)
    {
        case BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS:
        case BLECGM_RACP_OPERATOR_LESS_OR_EQUAL:
        case BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL:
        case BLECGM_RACP_OPERATOR_IN_RANGE:
            blecgm_sendAsManyMeasurements(&dbPdu);
            break;
        case BLECGM_RACP_OPERATOR_LAST_RECORD:
        case BLECGM_RACP_OPERATOR_FIRST_RECORD:
            break;
    }
}

void blecgm_handleRACPStates(void)
{
    // take care of the error reporting first.
    if( cgmAppState->blecgm_racpPktFormatStatus )
    {
        BLEPROFILE_DB_PDU db_pdu;

        bleprofile_ReadHandle(cgmAppState->blecgm_racpHandle,&db_pdu);

        if(blecgm_checkClientConfigBeforeRACP(cgmAppState->blecgm_racpHandle) == 0)
        {
            // the racp packet has error.
            blecgm_racpStatusIndication( db_pdu.pdu[0],cgmAppState->blecgm_racpPktFormatStatus);
        }

        // clear the error status flag.
        cgmAppState->blecgm_racpPktFormatStatus  = 0;
        return;
    }

    // if there are abort, we need to check them.
    if( cgmAppState->blecgm_racpOperationAbort )
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "racpAbort", TVF_D(0));

#if 0
        if( !cgmAppState->blecgm_racpOperation )
        {
            // We are trying to abort while not in operation.
            blecgm_racpStatusIndication(BLECGM_RACP_ABORT_REPORTING,
                                BLECGM_RACP_RSP_CODE_ABORT_UNSUCCESSFUL);
        }
        else
#endif
        {
            // we need to Indicate Abort success.
            blecgm_racpStatusIndication(BLECGM_RACP_ABORT_REPORTING,
                                BLECGM_RACP_RSP_CODE_SUCCESS );

        }

        // clear this flag.
        cgmAppState->blecgm_racpOperationAbort  = FALSE;

        // go back to idle state.
        cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
    }
    else
    {
        if( cgmAppState->blecgm_racpState  != BLECGM_RACP_IDLE  )
        {
            if( cgmAppState->blecgm_racpState  == BLECGM_RACP_PEND )
            {
                // we have something do to.
                BLEPROFILE_DB_PDU db_pdu;
                // get the data.
                bleprofile_ReadHandle(cgmAppState->blecgm_racpHandle,&db_pdu);
                // this means a new request comes in.
                // change the state to indicate we started to look into it.
                cgmAppState->blecgm_racpState  = BLECGM_RACP_ACTIVE;

                //////////////////////////////////////////////////////////////
                // Taking a RACP operation.
                // Load the opcode and operator.
                // RACP opcode is at offset 0.
                cgmAppState->blecgm_racpOperation  = db_pdu.pdu[0];
                cgmAppState->blecgm_racpOperator   = db_pdu.pdu[1];
                // clear the filter stuffs.
                cgmAppState->blecgm_racpFilterType = 0;
                BT_MEMSET( &(cgmAppState->blecgm_racpFilterMin),0x0, TIMESTAMP_LEN );
                BT_MEMSET( &(cgmAppState->blecgm_racpFilterMax),0x0, TIMESTAMP_LEN );

                switch( cgmAppState->blecgm_racpOperation )
                {
                    case BLECGM_RACP_REPORT_RECORDS:
                        blecgm_reportRecords(&db_pdu);
                        break;
                    case BLECGM_RACP_CLEAR_RECORDS:
                        blecgm_clearRecords(&db_pdu);
                        break;
                    case BLECGM_RACP_ABORT_REPORTING:
                        break;
                    case BLECGM_RACP_REQ_NUM_OF_RECORDS:
                        blecgm_reportRecordNumber(&db_pdu);
                        break;

                    case BLECGM_RACP_RESERVED_0:
                        // this is reserved.
                        // we need to at least clear the states.
                    default:
                        // Give error msg.
                        blecgm_racpStatusIndication(cgmAppState->blecgm_racpOperation ,
                                BLECGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED );

                        cgmAppState->blecgm_racpState  = BLECGM_RACP_IDLE;
                        break;
                }
            }
            else if (cgmAppState->blecgm_racpState  == BLECGM_RACP_ACTIVE)
            {
                // RACP is already in active state.
                if( cgmAppState->blecgm_racpOperation == BLECGM_RACP_REPORT_RECORDS )
                {
                    blecgm_reportRecordsCont();
                }
            }
        }
    }
}

int blecgm_dbGetRecordCntInRange( int start, int end)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            if((curPtr->timeOffset<= end)
                    &&
                (curPtr->timeOffset >= start))
            {
                cnt++;
            }
        }
        // move to next record.
        curPtr++;
    }

    return cnt;
}

#ifdef BLECGM_FACETIME
int blecgm_dbGetRecordCntInRangeTime(TIMESTAMP *startTime, TIMESTAMP *endTime)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            UINT8 *pBaseTime;
            UINT8 BaseTime[7];
            blecgm_findBaseTime(BaseTime);
            pBaseTime = BaseTime;

            if((blecgm_getTimeStampInseconds(pBaseTime, curPtr->timeOffset)
                <=
                blecgm_getTimeStampInseconds((UINT8 *)endTime, 0))
                &&
                (blecgm_getTimeStampInseconds(pBaseTime, curPtr->timeOffset)
                >=
                blecgm_getTimeStampInseconds((UINT8 *)startTime, 0)))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blecgm_dbGetRecordCntlessThanTime(TIMESTAMP *endTime)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            UINT8 *pBaseTime;
            UINT8 BaseTime[7];
            blecgm_findBaseTime(BaseTime);
            pBaseTime = BaseTime;

            if(blecgm_getTimeStampInseconds(pBaseTime, curPtr->timeOffset)
                <=
                blecgm_getTimeStampInseconds((UINT8 *)endTime, 0))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

void blecgm_findBaseTime(UINT8 *ptr)
{
    BLEPROFILE_DB_PDU db_pdu;

    //7bytes base time.
    db_pdu.len = 7;
    
    bleprofile_ReadHandle(cgmAppState->blecgm_sessionHandle, &db_pdu);

    memcpy(ptr, db_pdu.pdu, 7);
}

int blecgm_dbGetRecordCntSinceTime(TIMESTAMP *startTime)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            UINT8 *pBaseTime;
            UINT8 BaseTime[7];
            blecgm_findBaseTime(BaseTime);
            pBaseTime = BaseTime;

            if(blecgm_getTimeStampInseconds(pBaseTime, curPtr->timeOffset) >=
                blecgm_getTimeStampInseconds((UINT8 *)startTime, 0))
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}
#endif

int blecgm_dbGetRecordCntlessThanTimeOffset( int endTimeOffset)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            if(curPtr->timeOffset <= endTimeOffset)
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

int blecgm_dbGetRecordCntSinceTimeOffset( int startTimeOffset)
{
    int cnt = 0;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            if(curPtr->timeOffset >=  startTimeOffset)
            {
                cnt++;
            }
        }

        // move to next record.
        curPtr++;
    }

    return cnt;
}

BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetMeasurement(int TimeOffset)
{
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i < blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            if( curPtr->timeOffset == TimeOffset )
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

BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetNextMeasurement(int TimeOffset)
{
    int i;
    int nextValid = FALSE;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i < blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            if( nextValid)
            {
                return curPtr;
            }
            else if( curPtr->timeOffset == TimeOffset )
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

BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetFirstMeasurement(void)
{
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            return curPtr;
        }
        // move on to next one.
        curPtr++;
    }
    return NULL;
}

BLECGM_GLUCOSE_MEASUREMENT *blecgm_dbGetLastMeasurement(void)
{
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = &(cgmAppState->blecgm_iopData[blecgm_noofEntries-1]);

    for(i = blecgm_noofEntries-1 ; i>= 0; i--)
    {
        if(cgmAppState->blecgm_iopDataValid[i] )
        {
            return curPtr;
        }
        // move on to next one.
        curPtr--;
    }
    return NULL;
}

void blecgm_formRACPRecordCntInd(BLEPROFILE_DB_PDU *dbPdu,UINT8 Optor, UINT16 count )
{
    UINT8 operator = Optor;

    if(cgmAppState->blecgm_null_operator_nsrr)
    {
        operator = NULL;
    }
    // form the response message.
    dbPdu->pdu[0] = BLECGM_RACP_RSP_NUM_OF_RECORDS;
    dbPdu->pdu[1] = operator;
    dbPdu->pdu[2] = count & 0xff ;
    dbPdu->pdu[3] = (count >>8 ) & 0xff ;
    // packet length is 3 bytes.
    dbPdu->len = 4;
}

void blecgm_racpStatusIndication( UINT8 reqOpCode, UINT8 status)
{
    UINT8 tmp[4];

    tmp[0] = BLECGM_RACP_RSP_CODE;
    tmp[1] = NULL;
    tmp[2] = reqOpCode;
    tmp[3] = status;
#if 1
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RACP handle=%d", TVF_D(cgmAppState->blecgm_racpHandle));//debug only
#endif
    bleprofile_sendIndication( cgmAppState->blecgm_racpHandle, tmp, 4,blecgm_IndicationConf);
}

// This function returns true if the filter condition matched.
int blecgm_filterConditionMatched(BLECGM_GLUCOSE_MEASUREMENT *ptr )
{
    if( ptr)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Filter Condition %d, %d", TVF_WW(cgmAppState->blecgm_racpFilterType,cgmAppState->blecgm_racpOperator));
        if( cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
        {
            if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
            {
                // time offset less than or equal to.
                if( ptr->timeOffset <=
                        (cgmAppState->blecgm_racpFilterMax[0] | (cgmAppState->blecgm_racpFilterMax[1] << 8) ) )
                {
                    return TRUE;
                }
            }
            else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL)
            {
                // time offset less than or equal to.
                if( ptr->timeOffset >=
                        (cgmAppState->blecgm_racpFilterMin[0] | (cgmAppState->blecgm_racpFilterMin[1] << 8) ) )
                {
                    return TRUE;
                }
            }
            else if( cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE )
            {
                // time offset less than or equal to.
                if( (ptr->timeOffset >=
                    (cgmAppState->blecgm_racpFilterMin[0] | (cgmAppState->blecgm_racpFilterMin[1] << 8)))
                        &&
                    (ptr->timeOffset <=
                    (cgmAppState->blecgm_racpFilterMax[0] | (cgmAppState->blecgm_racpFilterMax[1] << 8)))
                  )
                {
                    return TRUE;
                }
            }
        }
#ifdef BLECGM_FACETIME
        else if( cgmAppState->blecgm_racpFilterType == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
        {
            UINT8 *pBaseTime;
            UINT8 BaseTime[7];
            UINT64 timeInSeconds;

            blecgm_findBaseTime(BaseTime);
            pBaseTime = BaseTime;

            timeInSeconds =
                    blecgm_getTimeStampInseconds(pBaseTime,ptr->timeOffset);

            if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL)
            {
                UINT64 refTimeInSeconds =
                    blecgm_getTimeStampInseconds( &(cgmAppState->blecgm_racpFilterMin[0]),0 );

                if( timeInSeconds >=refTimeInSeconds )
                {
                    return TRUE;
                }
            }
            else if(cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_LESS_OR_EQUAL)
            {
                UINT64 refTimeInSeconds =
                    blecgm_getTimeStampInseconds( &(cgmAppState->blecgm_racpFilterMax[0]),0 );

                if( timeInSeconds <= refTimeInSeconds )
                {
                    return TRUE;
                }
            }
            else if( cgmAppState->blecgm_racpOperator == BLECGM_RACP_OPERATOR_IN_RANGE )
            {
                UINT64 minTimeInSeconds =
                    blecgm_getTimeStampInseconds( &(cgmAppState->blecgm_racpFilterMin[0]),0 );
                UINT64 maxTimeInSeconds =
                    blecgm_getTimeStampInseconds( &(cgmAppState->blecgm_racpFilterMax[0]),0 );

                if( (timeInSeconds >=minTimeInSeconds )
                        &&
                        (timeInSeconds <= maxTimeInSeconds )
                  )
                {
                    return TRUE;
                }
            }
        }
#endif
        else
        {
            // this is unknown filter type.
        }
    }
    // default is false.
    return FALSE;
}

int blecgm_dbClearFirstRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(  cgmAppState->blecgm_iopDataValid[i] )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;

            // got one.
            break;
        }
    }

    return noRecordFound;
}

int blecgm_dbClearLessOrEqualRecords(int TimeOffsetMax)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(  curPtr->timeOffset <= TimeOffsetMax )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;

}

#ifdef BLECGM_FACETIME
int blecgm_dbClearLessOrEqualRecordsTime(TIMESTAMP *TimeMax)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    UINT8 *pBaseTime;
    UINT8 BaseTime[7];

    blecgm_findBaseTime(BaseTime);
    pBaseTime = BaseTime;

    for(i = 0; i< blecgm_noofEntries; i++)
    {

        UINT64 timeInSeconds =
                blecgm_getTimeStampInseconds(pBaseTime, curPtr->timeOffset);
        UINT64 maxTimeInSeconds =
                blecgm_getTimeStampInseconds( (UINT8 *)TimeMax,0 );
   
        if(timeInSeconds <= maxTimeInSeconds )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;

}

int blecgm_dbClearLargerOrEqualRecordsTime(TIMESTAMP *TimeMin)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    UINT8 *pBaseTime;
    UINT8 BaseTime[7];

    blecgm_findBaseTime(BaseTime);
    pBaseTime = BaseTime;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        UINT64 timeInSeconds =
                blecgm_getTimeStampInseconds(pBaseTime,curPtr->timeOffset);
        UINT64 minTimeInSeconds =
                blecgm_getTimeStampInseconds(  (UINT8 *)TimeMin,0 );

        if(timeInSeconds >=minTimeInSeconds )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

int blecgm_dbClearRecordRangeTime(TIMESTAMP *TimeMin, TIMESTAMP *TimeMax)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    UINT8 *pBaseTime;
    UINT8 BaseTime[7];

    blecgm_findBaseTime(BaseTime);
    pBaseTime = BaseTime;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        UINT64 timeInSeconds =
                blecgm_getTimeStampInseconds(
                            pBaseTime,curPtr->timeOffset);
        UINT64 minTimeInSeconds =
                blecgm_getTimeStampInseconds( (UINT8 *)TimeMin,0 );
        UINT64 maxTimeInSeconds =
                    blecgm_getTimeStampInseconds( (UINT8 *)TimeMax,0 );

        if( (timeInSeconds >=minTimeInSeconds )
                        &&
            (timeInSeconds <= maxTimeInSeconds )
           )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

#endif

int blecgm_dbClearLargerOrEqualRecords(int TimeOffsetMin)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(  curPtr->timeOffset >= TimeOffsetMin )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

int blecgm_dbClearLastRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for(i = blecgm_noofEntries-1;i >=0; i--)
    {
        if(  cgmAppState->blecgm_iopDataValid[i] )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
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
int blecgm_dbClearAllRecords(void)
{
    int noRecordFound = TRUE;
    int i;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if(  cgmAppState->blecgm_iopDataValid[i] )
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }
    }
    //set to first record after cleared all the records
    blecgm_noofEntries = 0;

    return noRecordFound;

}

int blecgm_dbClearRecordRange(int TimeOffsetMin, int TimeOffsetMax)
{
    int noRecordFound = TRUE;
    int i;
    BLECGM_GLUCOSE_MEASUREMENT *curPtr = cgmAppState->blecgm_iopData;

    for(i = 0; i< blecgm_noofEntries; i++)
    {
        if( ( curPtr->timeOffset >= TimeOffsetMin ) && ( curPtr->timeOffset <= TimeOffsetMax ))
        {
            // clear the flag.
            cgmAppState->blecgm_iopDataValid[i] = FALSE;
            // we find at least one.
            noRecordFound = FALSE;
        }

        // move on to next one.
        curPtr++;
    }

    return noRecordFound;
}

UINT32 blecgm_addMeasurement(UINT16 offset, UINT32 count)
{
#ifdef BLECGM_IOP_ADVANCE_DB
    int i;

    if( !(cgmAppState->blecgm_iopDataValid[blecgm_noofEntries]) )
    {
        UINT32 temp;
        // the last one is availble to reuse.
        // this record becomes valid.
        cgmAppState->blecgm_iopDataValid[blecgm_noofEntries]=TRUE;

        // increase the time offset of last record by one.
        cgmAppState->blecgm_iopData[blecgm_noofEntries].size = 0x0F;
        //SFLOAT is 12 bit.
        cgmAppState->blecgm_iopData[blecgm_noofEntries].glucoseConcentration = count&0x0FFF;
        cgmAppState->blecgm_iopData[blecgm_noofEntries].timeOffset = offset;
        cgmAppState->blecgm_iopData[blecgm_noofEntries].qualality = 0x32;//setting as 50% signal quality
        cgmAppState->blecgm_iopData[blecgm_noofEntries].flag = BLECGM_M_F_QUALITY_PRESENT;
          
        //status is set only for the patient alert.
        if((count&0x0FFF)<cgmAppState->blecgm_lowalert)
        {
            temp |= BLECGM_SENSOR_STATUS_LOWER_PATIENT_LEVEL;
        }
        else if((count&0x0FFF)>cgmAppState->blecgm_highalert)
        {
            temp |= BLECGM_SENSOR_STATUS_HIGHER_PATIENT_LEVEL;
        }

        if((count&0x0FFF)<cgmAppState->blecgm_hypolevel)
        {
            temp |= BLECGM_SENSOR_STATUS_LOWER_HYPO_LEVEL;
        }
        else if((count&0x0FFF)>cgmAppState->blecgm_hyperlevel)
        {
            temp |= BLECGM_SENSOR_STATUS_HIGHER_HYPER_LEVEL;
        }

        if(temp)
        {
            memcpy(&cgmAppState->blecgm_iopData[blecgm_noofEntries].sensorStatus,(UINT8*)&temp,3);
            cgmAppState->blecgm_iopData[blecgm_noofEntries].flag |= BLECGM_M_F_STATUS_ANNUNCIATION_WARNING;
        }
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_addMeasurement, last one. put %d idx back.", TVF_D(blecgm_noofEntries));
        blecgm_noofEntries++;

        if(blecgm_noofEntries == blecgm_iop_advance_db_size)
        {
            blecgm_noofEntries--;
        }

        return 1;
    }
    else
    {
        for(i = blecgm_noofEntries-1; i >= 0; i--)
        {
            if(!(cgmAppState->blecgm_iopDataValid[i]) )
            {
                // we find a available spot.
                int j;
                UINT32 temp;
                // slide all the records down.
                for (j = i; j < blecgm_noofEntries; j++)
                {
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_addMeasurement, %d idx is deleted.", TVF_D(i));
                    BT_MEMCPY(&(cgmAppState->blecgm_iopData[j]),
                            &(cgmAppState->blecgm_iopData[j+1]),
                            sizeof(BLECGM_GLUCOSE_MEASUREMENT  ) );
                }
                // this record becomes valid.
                cgmAppState->blecgm_iopDataValid[i]=TRUE;

                // increase the time offset of last record by one.
                cgmAppState->blecgm_iopDataValid[blecgm_noofEntries]=TRUE;

                // increase the time offset of last record by one.
                cgmAppState->blecgm_iopData[blecgm_noofEntries].size = sizeof(BLECGM_GLUCOSE_MEASUREMENT);
                //SFLOAT is 12 bit.
                cgmAppState->blecgm_iopData[blecgm_noofEntries].glucoseConcentration = count&0x0FFF;
                cgmAppState->blecgm_iopData[blecgm_noofEntries].timeOffset = offset;
                cgmAppState->blecgm_iopData[blecgm_noofEntries].qualality = 0x32;//setting as 50% signal quality
                cgmAppState->blecgm_iopData[blecgm_noofEntries].flag = BLECGM_M_F_QUALITY_PRESENT;

                //status is set only for the patient alert.
                if((count&0x0FFF)<cgmAppState->blecgm_lowalert)
                {
                    temp |= BLECGM_SENSOR_STATUS_LOWER_PATIENT_LEVEL;
                }
                else if((count&0x0FFF)>cgmAppState->blecgm_highalert)
                {
                    temp |= BLECGM_SENSOR_STATUS_HIGHER_PATIENT_LEVEL;
                }

                if((count&0x0FFF)<cgmAppState->blecgm_hypolevel)
                {
                    temp |= BLECGM_SENSOR_STATUS_LOWER_HYPO_LEVEL;
                }
                else if((count&0x0FFF)>cgmAppState->blecgm_hyperlevel)
                {
                    temp |= BLECGM_SENSOR_STATUS_HIGHER_HYPER_LEVEL;
                }

                if(temp)
                {
                    memcpy(&cgmAppState->blecgm_iopData[blecgm_noofEntries].sensorStatus,(UINT8*)&temp,3);
                    cgmAppState->blecgm_iopData[blecgm_noofEntries].flag |= BLECGM_M_F_STATUS_ANNUNCIATION_WARNING;
                }
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecgm_addMeasurement, put %d idx back.", TVF_D(blecgm_noofEntries));

                return 1;
            }
        }
    }

#endif

    return 0;
}

UINT64 blecgm_getTimeStampInseconds( UINT8 *baseTime, UINT16 timeOffset) //timeoffset in minute
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

    tmp += timeOffset; //adding minutes

    tmp *= 60; //making second

    tmp += ptr[6]; //adding seconds
#if 1
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "tmp_l = %08x", TVF_D(tmp&0xFFFFFFFF));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "tmp_h = %08x", TVF_D(tmp>>32));
#endif
    return tmp;
}

UINT32 blecgm_checkRACPformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    ble_tracen( (char *)ptr, len );

    switch( ptr[0] )
    {
        case BLECGM_RACP_REPORT_RECORDS:
            status = blecgm_checkGetRecordformat(ptr, len);
            break;

        case BLECGM_RACP_CLEAR_RECORDS:
            status = blecgm_checkClearRecordformat(ptr, len);
            break;

        case BLECGM_RACP_ABORT_REPORTING:
            status = blecgm_checkAbortReportingformat(ptr, len);
            break;

        case BLECGM_RACP_REQ_NUM_OF_RECORDS:
            status = blecgm_checkNumRecordformat(ptr, len);
            break;

        case BLECGM_RACP_RSP_NUM_OF_RECORDS:
        case BLECGM_RACP_RSP_CODE:
        default:
            status = BLECGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED;

            break;
    }
    return status;
}

#ifdef BLECGM_ASCPENABLE
UINT32 blecgm_checkASCPformat(BLECGM_ASCP_HDR *ptr, INT32 len)
{
    UINT32 status = 0;

    switch(ptr->opCode)
    {
        case BLECGM_ASCP_REQ_AUTH:
            status = blecgm_checkAuthentication(ptr, len);
            break;

        case BLECGM_ASCP_SET_AUTH_CODE:
            status = blecgm_setauthcode(ptr, len);
            break;

        case BLECGM_ASCP_SET_PRIMARY_COLLECTOR:
            status = blecgm_setprimarycollector(ptr, len);
            break;
        default:
            status = BLECGM_ASCP_RSP_CODE_OP_CODE_NOT_SUPPORTED;
            break;
    }
    return status;
}

UINT32 blecgm_checkAuthentication(BLECGM_ASCP_HDR *ptr, INT32 len)
{
    // 1st byte is opcode.
    if(blecgm_Authcodelength == len-1)
    {   
        //check the authentication code.
        if(memcmp(ptr->operandValue, blecgm_Authenticationcode, len-1))
        {
            return BLECGM_ASCP_RSP_CODE_AUTH_FAILED;
        }
        else
        {
            //set the authentication result.
            cgmAppState->blecgm_ascpauthsuccess = 1;
            
            //set the permission.
            blecgm_PermAuthEnable(cgmAppState->blecgm_ascpauthsuccess);

            return BLECGM_ASCP_RSP_CODE_SUCCESS;
        }
    }
    else
    {
        return BLECGM_ASCP_RSP_CODE_AUTH_FAILED;
    }
    
}

UINT32 blecgm_setauthcode(BLECGM_ASCP_HDR *ptr, INT32 len)
{
    //check the authentication result
    if(cgmAppState->blecgm_ascpauthsuccess)
    {
        //remove the len for op code
        blecgm_Authcodelength = len-1;
        // 1st byte is opcode.
        memcpy(blecgm_Authenticationcode, ptr->operandValue, blecgm_Authcodelength);
        return BLECGM_ASCP_RSP_CODE_SUCCESS;
    }
    else
    {
        return BLECGM_ASCP_RSP_CODE_INSUFFICIENT_AUTH;
    }
}

UINT32 blecgm_setprimarycollector(BLECGM_ASCP_HDR *ptr, INT32 len)
{
    //check the authentication result
    if(cgmAppState->blecgm_ascpauthsuccess)
    {
        //determine from NVRAM write.
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "TO BE DETERMINED", TVF_D(0));
        return BLECGM_ASCP_RSP_CODE_SUCCESS;
    }
    else
    {
        return BLECGM_ASCP_RSP_CODE_INSUFFICIENT_AUTH;
    }
}
#endif

UINT32 blecgm_checkGetRecordformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    if(len < 2)
    {
        status = BLECGM_RACP_RSP_CODE_INVALID_OPERATOR;
    }
    else
    {
        switch(ptr[1])
        {
            case BLECGM_RACP_OPERATOR_NULL:
                status = BLECGM_RACP_RSP_CODE_INVALID_OPERATOR;
                break;
            case BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            case BLECGM_RACP_OPERATOR_FIRST_RECORD:
            case BLECGM_RACP_OPERATOR_LAST_RECORD:
                if(len > 2)
                {
                    status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            case BLECGM_RACP_OPERATOR_LESS_OR_EQUAL:
            case BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL:
                if(len > 2)
                {
                    // check the filter type.
                    if(
#ifdef BLECGM_FACETIME
                     (ptr[2] > BLECGM_RACP_FILTER_TYPE_FACING_TIME)
                        ||
                       (ptr[2] < BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
#else
                       (ptr[2] != BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
#endif
                      )
                    {
                        // undefined filter type.
                        status = BLECGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED;
                    }

                    if(ptr[2] == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
                    {
                        if(len != (3+2))
                        {
                            status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
#ifdef BLECGM_FACETIME
                    else if(ptr[2] == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
                    {
                        if(len != (3+7))
                        {
                            status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
#endif
                }
                else
                {
                    status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            case BLECGM_RACP_OPERATOR_IN_RANGE:
                if(len > 2)
                {
                    // check the filter type.
                    if(
#ifdef BLECGM_FACETIME
                    (ptr[2] > BLECGM_RACP_FILTER_TYPE_FACING_TIME)
                        ||
                       (ptr[2] < BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
#else
                       (ptr[2] != BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
#endif
                      )
                    {
                        // undefined filter type.
                        status = BLECGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED;
                    }

                    if(ptr[2] == BLECGM_RACP_FILTER_TYPE_TIME_OFFSET)
                    {
                        if(len != (3+2*2))
                        {
                            status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
#ifdef BLECGM_FACETIME
                    else if(ptr[2] == BLECGM_RACP_FILTER_TYPE_FACING_TIME)
                    {
                        if(len != (3+7*2))
                        {
                            status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
#endif
                }
                else
                {
                    status = BLECGM_RACP_RSP_CODE_INVALID_OPERAND;
                }
                break;
            default:
                // This is out of range.
                status = BLECGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

UINT32 blecgm_checkAbortReportingformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    if(len < 2)
    {
        status = BLECGM_RACP_RSP_CODE_INVALID_OPERATOR;
    }
    else
    {
        switch(ptr[1])
        {
            case BLECGM_RACP_OPERATOR_NULL:
                break;

            case BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS:
            case BLECGM_RACP_OPERATOR_LESS_OR_EQUAL:
            case BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL:
            case BLECGM_RACP_OPERATOR_IN_RANGE:
            case BLECGM_RACP_OPERATOR_FIRST_RECORD:
            case BLECGM_RACP_OPERATOR_LAST_RECORD:
                status = BLECGM_RACP_RSP_CODE_INVALID_OPERATOR;
                break;
            default:
                // This is out of range.
                status = BLECGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

UINT32 blecgm_checkClearRecordformat(UINT8 *ptr, INT32 len)
{
    return blecgm_checkGetRecordformat(ptr, len);
}

UINT32 blecgm_checkNumRecordformat(UINT8 *ptr, INT32 len)
{
    return blecgm_checkGetRecordformat(ptr, len);
}

int  blecgm_checkClientConfigBeforeRACP(UINT16 handle)
{
    int status = 0;
    int value = 1;
    BLEPROFILE_DB_PDU dbPdu;

    // check the notification configuration for measurement.
    bleprofile_ReadHandle(cgmAppState->blecgm_measurementCCCHandle ,&dbPdu);
    if( (dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_NOTIFICATION) )
    {
        // Notification is set.
        status |= BLECGM_CCC_MEASUREMENT;
    }

    // check RACP.
    bleprofile_ReadHandle(cgmAppState->blecgm_racpCCCHandle,&dbPdu);
    if( (dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_INDICATION) )
    {
        // RACP setting is good.
        status |= BLECGM_CCC_RACP;
    }
#ifdef BLECGM_ASCPENABLE
    // check ASCP.
    bleprofile_ReadHandle(cgmAppState->blecgm_ascpCCCHandle,&dbPdu);
    if( (dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_INDICATION) )
    {
        // ASCP setting is good.
        status |= BLECGM_CCC_ASCP;
    }
#endif
    // check CGMCP.
    bleprofile_ReadHandle(cgmAppState->blecgm_cgmcpCCCHandle,&dbPdu);
    if( (dbPdu.pdu[0] & LEATT_CLIENT_CONFIG_INDICATION) )
    {
        // CGMCP setting is good.
        status |= BLECGM_CCC_CGMCP;
    }
    if(handle == cgmAppState->blecgm_racpHandle)
    {
        if((status & BLECGM_CCC_RACP)
            &&
            (status & BLECGM_CCC_MEASUREMENT))
        {
            value = 0;
        }
    }
#ifdef BLECGM_ASCPENABLE
    else if(handle == cgmAppState->blecgm_ascpHandle)
    {
        if(status & BLECGM_CCC_ASCP)
        {
            value = 0;
        }
    }
#endif
    else if(handle == cgmAppState->blecgm_cgmcpHandle)
    {
        if(status & BLECGM_CCC_CGMCP)
        {
            value = 0;
        }
    }
    else if(handle == cgmAppState->blecgm_measurementHandle)
    {
        if(status & BLECGM_CCC_MEASUREMENT)
        {
            value = 0;
        }
    }

    return value;
}

void blecgm_sendAsManyMeasurements(BLEPROFILE_DB_PDU *dbPdu)
{
    while( blecm_getAvailableTxBuffers() > 1 ) // two notifications can be sent at the same time
    {
        if( blecgm_reportNextMeasurement(dbPdu)
                == BLECGM_GET_DB_RECORD_NO_MORE_RECORD )
        {
            // we are done.
            if(cgmAppState->blecgm_racpNotificationCnt)
            {
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_SUCCESS );
            }
            else
            {
                blecgm_racpStatusIndication(BLECGM_RACP_REPORT_RECORDS,
                        BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND );
            }
            // waiting for the Infication Confirmation.
            cgmAppState->blecgm_racpState   = BLECGM_RACP_COMPLETE ;

            // No more data to send, break out of the loop.
            break;
        }
    }
}

void blecgm_loadAppData(void)
{
    int tmp ;

    tmp = bleprofile_ReadNVRAM(
            VS_BLE_HOST_LIST,
            sizeof(cgmAppState->blecgm_appStorage), (UINT8 *)cgmAppState->blecgm_appStorage);

    if( tmp != sizeof(cgmAppState->blecgm_appStorage) )
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "loadAppDataHadNothing", TVF_D(0));

        // clear the memory area for it.
        BT_MEMSET((UINT8 *)cgmAppState->blecgm_appStorage, 0x0, sizeof(cgmAppState->blecgm_appStorage));
    }
}

BLECGM_APP_STORAGE *blecgm_getHostData( UINT8 *adr, UINT8 adrType )
{
    BLECGM_APP_STORAGE *hostData = NULL;
    int idx = blecgm_lookupAppData(adr, adrType);

    if( idx != -1 )
    {
        hostData = &(cgmAppState->blecgm_appStorage[idx]);
    }

    return hostData;
}


int blecgm_lookupAppData(UINT8 *adr, UINT8 adrType )
{
    int i;
    BLECGM_APP_STORAGE *curPtr = cgmAppState->blecgm_appStorage;

    for(i=0; i< BLECGM_MAX_BONDED_HOST;i++)
    {
        if(curPtr->misc & BLECGM_APP_FLAG_VALID )
        {
            // a valid record.
            // check address type first.
            int tmpAdrType = curPtr->misc & BLECGM_APP_FLAG_ADR_TYPE;
            if( (tmpAdrType && adrType  )
                    ||
                (!tmpAdrType && !adrType  )
              )
            {
                // the adr type are the same. we compare the 6 bytes of
                // address.
                if(!BT_MEMCMP(curPtr->bdAdr, adr, 6) )
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

void blecgm_clearAllConf( int idx )
{
    if( (idx >= 0) && (idx < BLECGM_MAX_BONDED_HOST) )
    {
        // keep the address type and clear everything else.
        cgmAppState->blecgm_appStorage[idx].misc &= BLECGM_APP_FLAG_ADR_TYPE;
    }
}

int blecgm_addHost( UINT8 *adr, UINT8 adrType )
{
    int i;
    BLECGM_APP_STORAGE *curPtr = cgmAppState->blecgm_appStorage;
    int len;

    // look for available spot.
    for(i=0; i < BLECGM_MAX_BONDED_HOST;i++)
    {
        if(!(curPtr->misc & BLECGM_APP_FLAG_VALID ))
        {
            // This is available.
            break;
        }
        curPtr++;
    }

    if( i >= BLECGM_MAX_BONDED_HOST )
    {
        // We don't have valid index any more.
        // Fix me.
        // we use index 0.
        i = 0;
    }

    // set the address.
    BT_MEMCPY( &(cgmAppState->blecgm_appStorage[i]), adr, BLECGM_BD_ADR_LEN);

    if(adrType)
    {
        // random address.
        cgmAppState->blecgm_appStorage[i].misc
            = BLECGM_APP_FLAG_ADR_TYPE|BLECGM_APP_FLAG_VALID;
    }
    else
    {
        // public address.
        cgmAppState->blecgm_appStorage[i].misc = BLECGM_APP_FLAG_VALID;
    }

    len = bleprofile_WriteNVRAM(
            VS_BLE_HOST_LIST, sizeof(cgmAppState->blecgm_appStorage),
            (UINT8 *)(cgmAppState->blecgm_appStorage));

    if( len != sizeof(cgmAppState->blecgm_appStorage))
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "addHost write nvram problem, %d bytes", TVF_D(len));
    }

    return i;
}

BLECGM_APP_STORAGE *blecgm_getCurHostData(void)
{
    int adrType   = lesmpkeys_getPeerAdrType();
    UINT8 *bdAdr  = lesmpkeys_getPeerAdr();
    int idx;

    idx = blecgm_lookupAppData( bdAdr, adrType );
    if( idx != -1 )
    {
        return &(cgmAppState->blecgm_appStorage[idx]);
    }
    return NULL;
}

int blecgm_setupTargetAdrInScanRsp(void)
{
    int i;
    BLECGM_APP_STORAGE *curAppData = (cgmAppState->blecgm_appStorage);

    BLECM_SELECT_ADDR select_addr[4]; //Max 4 can be supported
    UINT8 TargetNum = 0;

    // The 31 bytes of Scan response data can hold up to 4 entries.
    for(i=0; i < 4 ;i++)
    {

        if(curAppData->misc & BLECGM_APP_FLAG_VALID )
        {
            // fill in the address.
            BT_MEMCPY(select_addr[TargetNum].addr, curAppData->bdAdr,BLECGM_BD_ADR_LEN );
            select_addr[TargetNum].type = (curAppData->misc & BLECGM_APP_FLAG_ADR_TYPE)>>15;

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

void blecgm_clientConfFlagToCheckAndStore(UINT16 flag,
        UINT16 flagToCheck, UINT16 flagToStore)
{
    BLECGM_APP_STORAGE *curPtr = blecgm_getCurHostData();
    int len;

    if( curPtr )
    {
        if( flag & flagToCheck)
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
                VS_BLE_HOST_LIST, sizeof(cgmAppState->blecgm_appStorage),
                (UINT8 *)(cgmAppState->blecgm_appStorage));

        if( len != sizeof(cgmAppState->blecgm_appStorage))
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "storing appData failed. %d bytes", TVF_D(len));
        }
    }
}
