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
* This file implements the Glucose profile, service, application.
*
* Refer to Bluetooth SIG Glucose Profile 1.0. and Glucose Service
* 1.0 specifications for details.
*
* The file implements a sample Glucose sensor.  This file replaces ROM's
* processing of the fine timer and GATT database, keeping rest of the
* functionality untouched.  ROM code can be reviewed at
* WICED-Smart-SDK\Wiced-Smart\bleapp\app\blebgm.c.
*
* Features demonstrated
*  - Replacing ROM application create functions with local one
*  - Accessing ROM functions
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/

#include "bleprofile.h"
#include "bleapp.h"
#include "blebgm.h"
#include "lesmpkeys.h"
#include "spar_utils.h"
#include "bleappevent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////

static void    glucose_meter_create(void);
static int     glucose_meter_write_handler(LEGATTDB_ENTRY_HDR *p);
static void    glucose_meter_advStop(void);

// Forward declaration of the ROM function.
extern void   blebgm_connUp(void);
extern void   blebgm_connDown(void);
extern void   blebgm_appTimerCb(UINT32 arg);
extern void   blebgm_appFineTimerCb(UINT32 arg);
extern void   blebgm_smpBondResult(LESMP_PARING_RESULT  result);
extern void   blebgm_encryptionChanged(HCI_EVT_HDR *evt);
extern void   blebgm_IndicationConf(void);
extern void   blebgm_transactionTimeout(void);
extern UINT32 blebgm_buttonCb(UINT32 function);

extern int    blebgmhandleConnParamUpdateRsp(void *l2capHdr);

typedef int (*LEL2CAP_MSGHANDLER)(void*);
extern LEL2CAP_MSGHANDLER lel2cap_handleConnParamUpdateRsp;
extern BLEBGM_APP_STORAGE *blebgm_getHostData(UINT8 *adr, UINT8 adrType);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 glucose_meter_db_data[]=
{
    // GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
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
        0x04,0x04                  // features that we support.
};

// modify configuration to set 5 seconds idle timeout
const BLE_PROFILE_CFG glucose_meter_cfg =
{
    /* .fine_timer_interval            =*/ 100, // UINT16 ; //ms
    /* .default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /* .button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /* .high_undirect_adv_interval     =*/ 32,   // slots
    /* .low_undirect_adv_interval      =*/ 2048, // slots
    /* .high_undirect_adv_duration     =*/ 30,   // seconds
    /* .low_undirect_adv_duration      =*/ 30,  // seconds
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
    /* .con_idle_timeout               =*/ 5,    // second  0-> no timeout
    /* .powersave_timeout              =*/ 5,    // second  0-> no timeout
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


/******************************************************
 *               Variables Definitions
 ******************************************************/

#define BLEBGM_IOP_ADVANCE_DB

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

extern tBgmAppState *bgmAppState;
extern int blebgm_iopDataEntries;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

// Application initialization
APPLICATION_INIT()
{
	// Register the GATT DB, configurations and the application create function.
    bleapp_set_cfg((UINT8 *)glucose_meter_db_data, sizeof(glucose_meter_db_data), (void *)&glucose_meter_cfg,
        (void *)&blebgm_puart_cfg, (void *)&blebgm_gpio_cfg, glucose_meter_create);
}

// The application create function.  Do not call blebgm Create because we need to overwrite
// write callback
void glucose_meter_create(void)
{
    ble_trace0("blebgm_Create");
    ble_trace0(bleprofile_p_cfg->ver);

    bgmAppState = (tBgmAppState *)cfa_mm_Sbrk(sizeof(tBgmAppState));
    memset(bgmAppState, 0x00, sizeof(tBgmAppState));

    //initialize the default value of bgmAppState
    bgmAppState->blebgm_null_operator_nsrr = 1;
    bgmAppState->blebgm_racpState = BLEBGM_RACP_IDLE;

#ifdef BLEBGM_IOP_ADVANCE_DB
    blebgm_create_iopdb();
#endif
    bgmAppState->blebgm_iopContextData[blebgm_iopDataEntries-1].medication = 0xd030;

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
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, glucose_meter_advStop);

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

    // use RAM version of the write handler
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)glucose_meter_write_handler);

    bleprofile_regTimerCb(blebgm_appFineTimerCb, blebgm_appTimerCb);
    bleprofile_StartTimer();

    // load the Bonded host info.
    blebgm_loadAppData();

    blebgm_connDown();
}


UINT32 glucose_meter_checkGetRecordformat(UINT8 *ptr, INT32 len)
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
                        if ((len != (3 + 2 * 2)) ||
                            ((ptr[3] + (ptr[4] << 8)) > (ptr[5] + (ptr[6] << 8))))
                        {
                            status = BLEBGM_RACP_RSP_CODE_INVALID_OPERAND;
                        }
                    }
                    else if (ptr[2] == BLEBGM_RACP_FILTER_TYPE_FACING_TIME)
                    {
                        if ((len != (3 + 7 * 2)) ||
                        	(blebgm_getTimeStampInseconds(&ptr[3], 0, 0) > blebgm_getTimeStampInseconds(&ptr[10], 0, 0)))
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


UINT32 glucose_meter_checkRACPformat(UINT8 *ptr, INT32 len)
{
    UINT32 status = 0;

    ble_trace1("chkRacpFmt %d", ptr[0]);
    ble_tracen((char *)ptr, len);

    switch(ptr[0])
    {
        case BLEBGM_RACP_REPORT_RECORDS:
            status = glucose_meter_checkGetRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_CLEAR_RECORDS:
            status = glucose_meter_checkGetRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_ABORT_REPORTING:
            status = blebgm_checkAbortReportingformat(ptr, len);
            break;

        case BLEBGM_RACP_REQ_NUM_OF_RECORDS:
            status = glucose_meter_checkGetRecordformat(ptr, len);
            break;

        case BLEBGM_RACP_RSP_NUM_OF_RECORDS:
        case BLEBGM_RACP_RSP_CODE:
        default:
            status = BLEBGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED;

            break;
    }
    return status;
}


int glucose_meter_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle  = legattdb_getHandle(p);
    int len        = legattdb_getAttrValueLen(p);
    UINT8 *attrPtr = legattdb_getAttrValue(p);

    ble_trace3("GM Write handle =0x%04x, length = 0x%04x racpHandle:%x", handle, len, bgmAppState->blebgm_racpHandle);
    ble_tracen((char *) attrPtr, len);

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
                    if (racpHdr->opCode != BLEBGM_RACP_ABORT_REPORTING)
                    {
                        return BLEBGM_RACP_RSP_PROCEDURE_ALREADY_IN_PROGRESS;
                    }

                default:
                    //
                    ble_trace0("RACPReqUnknownState");
                    break;
            }

            // We need to check the racp operation for packet
            // integrity.
            bgmAppState->blebgm_racpPktFormatStatus = glucose_meter_checkRACPformat(attrPtr, len);

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

void glucose_meter_advStop(void)
{
    ble_trace0("ADV stop");
}
