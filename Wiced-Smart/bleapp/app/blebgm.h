#ifndef _BLEBGM_H_
#define _BLEBGM_H_
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
* This file provides definitions and function prototypes for implementation
* of the sample Glucose sensor.
*
* Refer to Bluetooth SIG Glucose Profile 1.0. and Glucose Service
* 1.0 specifications for details.
*
*/
#include "bleprofile.h"


// Module, Test, Securuty setting
//#define BLE_P1
#define BLE_P2

//#define BLETEST_ENABLE
//#define BLE_SECURITY_REQUEST
#define BLE_AUTH_WRITE
#define BLE_CONNECTION_PARAMETER_UPDATE
//#define BLEBGM_IOP_ADVANCE_DB_BIG
#define BLEBGM_IOP_ADVANCE_DB_SIMULATE_DATA


//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

// this has been defined in many places.
#define BLEBGM_BD_ADR_LEN         6

//////////////////////////////////////////////////////////////////////////////
//    GLUCOSE_MEANSUREMENT, D07R01.
////////////// Flags.
#define BLEBGM_M_F_TIME_OFFSET_FIELD_PRESENT                0x01
#define BLEBGM_M_F_CON_AND_TYPE_SAMPLE_LOCATION_PRESENT     0x02
#define BLEBGM_M_F_UNIT_IN_MMOL_PER_LITER                   0x04
#define BLEBGM_M_F_SENSOR_STATUS_ANNUNCIATION               0x08
#define BLEBGM_M_F_CONTEXT_INFORMATION                      0x10

////////////// timeOffset
#define BLEBGM_TIME_OFFSET_OVER_RUN                         0x7fff
#define BLEBGM_TIME_OFFSET_UNDER_RUN                        0x8000

////////////// type_sample_location
#define BLEBGM_TYPE_SAMPLE_LOC_RESERVED                     0x00

#define BLEBGM_TYPE_TYPE_MASK                               0xf
#define BLEBGM_TYPE_CAPILLARY_WHOLE_BLOOD                   0x1
#define BLEBGM_TYPE_CAPILLARY_PLASMA                        0x2
#define BLEBGM_TYPE_VENOUS_WHILE_BLOOD                      0x3
#define BLEBGM_TYPE_VENOUS_PLASMA                           0x4
#define BLEBGM_TYPE_ARTERIAL_WHOLE_BLOOD                    0x5
#define BLEBGM_TYPE_ARTERIAL_PLASMA                         0x6
#define BLEBGM_TYPE_UNDETERMINED_WHOLE_BLOOD                0x7
#define BLEBGM_TYPE_UNDETERMINED_PLASMA                     0x8
#define BLEBGM_TYPE_INTERSTITIAL_FLUID                      0x9
#define BLEBGM_TYPE_CONTROL_SOLUTION                        0xa

#define BLEBGM_SAMPLE_LOC_SAMPLE_LOC_MASK                   0xf0
#define BLEBGM_SAMPLE_LOC_FINGER                            0x1
#define BLEBGM_SAMPLE_LOC_ALTERNATE_SITE_TEST               0x2
#define BLEBGM_SAMPLE_LOC_EARLOBE                           0x3
#define BLEBGM_SAMPLE_LOC_CONTROL_SOLUTION                  0x4
#define BLEBGM_SAMPLE_LOC_RFU_5                             0x5
#define BLEBGM_SAMPLE_LOC_RFU_6                             0x6
#define BLEBGM_SAMPLE_LOC_RFU_7                             0x7
#define BLEBGM_SAMPLE_LOC_RFU_8                             0x8
#define BLEBGM_SAMPLE_LOC_RFU_9                             0x9
#define BLEBGM_SAMPLE_LOC_RFU_a                             0xa
#define BLEBGM_SAMPLE_LOC_RFU_b                             0xb
#define BLEBGM_SAMPLE_LOC_RFU_c                             0xc
#define BLEBGM_SAMPLE_LOC_RFU_d                             0xd
#define BLEBGM_SAMPLE_LOC_RFU_e                             0xe
#define BLEBGM_SAMPLE_LOC_NOT_AVAILABLE                     0xf


////////////// Sensor Status Annunciation
#define BLEBGM_SENSOR_STATUS_BATTERY_LOW_AT_MEASUREMENT       0x0001
#define BLEBGM_SENSOR_STATUS_MULFUNCTION_AT_MEASUREMENT       0x0002
#define BLEBGM_SENSOR_STATUS_SAMPLE_SIZE_NOT_ENOUGH           0x0004
#define BLEBGM_SENSOR_STATUS_STRIP_WAS_NOT_INSERTED_PROPERLY  0x0008
#define BLEBGM_SENSOR_STATUS_STRIP_WAS_NOT_RIGHT_TYPE         0x0010
#define BLEBGM_SENSOR_STATUS_STRIP_RESULT_TOO_HIGH            0x0020
#define BLEBGM_SENSOR_STATUS_STRIP_RESULT_TOO_LOW             0x0040
#define BLEBGM_SENSOR_STATUS_STRIP_TEMP_TOO_HIGH              0x0080
#define BLEBGM_SENSOR_STATUS_STRIP_TEMP_TOO_LOW               0x0100
#define BLEBGM_SENSOR_STATUS_STRIP_READ_INTERRUPTED           0x0200
#define BLEBGM_SENSOR_STATUS_STRIP_GENERAL_DEVICE_FAULT       0x0400
#define BLEBGM_SENSOR_STATUS_STRIP_TIME_FAULT                 0x0800


typedef PACKED struct
{
    UINT8     flags;
    UINT16    seqNum;               // 2 bytes
    TIMESTAMP BaseTime;             // 7 bytes
    INT16     timeOffset;           // 2 bytes
    SFLOAT    glucoseConcentration; // 2 bytes
    UINT8     typeSampleLocation;   // 1 bytes
    UINT16    sensorStatus;         // 2 bytes
} BLEBGM_GLUCOSE_MEASUREMENT;



//////////////////////////////////////////////////////////////////////////////
//    GLUCOSE_MEANSUREMENT CONTEXT.

//    Flags.
#define BLEBGM_M_C_F_CARB_ID_PRESENT                          0x01
#define BLEBGM_M_C_F_MEAL_FIELD_PRESENT                       0x02
#define BLEBGM_M_C_F_TESTER_HEALTH_PRESENT                    0x04
#define BLEBGM_M_C_F_EXERCISE_FIELD_PRESENT                   0x08
#define BLEBGM_M_C_F_MEDICAION_PRESENT                        0x10
#define BLEBGM_M_C_F_MEDICAION_UNIT_IN_MILL                   0x20
#define BLEBGM_M_C_F_HB_A1C_PRESENT                           0x40
#define BLEBGM_M_C_F_EXTENDED_FLAG_PRESENT                    0x80


//    Carb ID.
#define BLEBGM_CARB_ID_BREAKFAST                              0x01
#define BLEBGM_CARB_ID_LUNCH                                  0x02
#define BLEBGM_CARB_ID_DINNER                                 0x03
#define BLEBGM_CARB_ID_SNACK                                  0x04
#define BLEBGM_CARB_ID_DRINK                                  0x05
#define BLEBGM_CARB_ID_SUPPER                                 0x06
#define BLEBGM_CARB_ID_BRUNCH                                 0x07

//    Meal Field.
#define BLEBGM_PREPRANDIAL                                    0x01
#define BLEBGM_POSTPRANDIAL                                   0x02
#define BLEBGM_FASTING                                        0x03
#define BLEBGM_CASUAL                                         0x04

//    Tester Health
#define BLEBGM_SELF                                           0x01
#define BLEBGM_HEALFH_CARE_PROF                               0x02
#define BLEBGM_LAB_TEST                                       0x03
// 0x4 - 0xe are reserved.
#define BLEBGM_TESTER_VALUE_NOT_AVAILABLE                     0x0f


//    Health Nibble.
#define BLEBGM_HEALTH_MINOR_HEALTH_ISSUE                      0x01
#define BLEBGM_HEALTH_MAJOR_HEALTH_ISSUE                      0x02
#define BLEBGM_HEALTH_DURING_MENSES                           0x03
#define BLEBGM_HEALTH_UNDER_STRESS                            0x04
#define BLEBGM_HEALTH_NO_HEALTH_ISSUE                         0x05
// 0x6 - 0xe are reserved.
#define BLEBGM_HEALTH_VALUE_NOT_AVAILABLE                     0x0f


typedef PACKED struct
{
    UINT8     flags;
    UINT16    seqNum;           // 2 bytes
    UINT8     extendedFlags;    // 1 byte.
    UINT8     carbID;           // 1 byte.
    SFLOAT    carb;             // 2 bytes, grams.
    UINT8     meal;             // 1 byte.
    UINT8     testerHealth;     // 1 byte.
    UINT16    exerciseDuration; // 2 byte.
    UINT8     exerciseIntensity;// 1 byte.
    UINT8     medicationID;     // 1 byte.
    SFLOAT    medication;       // 2 bytes.
    SFLOAT    HbA1c;            // 2 bytes.
} BLEBGM_GLUCOSE_MEASUREMENT_CONTEXT;



//////////////////////////////////////////////////////////////////////////////
//    GLUCOSE FEATUES.
#define BLEBGM_GLUCOSE_FEATURES_LOW_BATTERY_DURING_MEASUREMENT_SUPPORTED  0x0001
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_MALFUNCTION_SUPPORTED              0x0002
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_SAMPLE_SIZE_SUPPORTED              0x0004
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_STRIP_INSERTION_ERROR_SUPPORTED    0x0008
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_STRIP_TYPE_ERROR_SUPPORTED         0x0010
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_HIGH_LOW_DETECTION_SUPPORTED       0x0020
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_TEMP_HIG_LOW_DETECTION_SUPPORTED   0x0040
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_READ_INT_DETECTION_SUPPORTED       0x0080
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_GENERAL_DEVICE_FAULT_SUPPORTED     0x0100
#define BLEBGM_GLUCOSE_FEATURES_SENSOR_TIME_FAULT_SUPPORTED               0x0200



//////////////////////////////////////////////////////////////////////////////
//                      Record Access Control Point.


// RACP Op Code.
#define BLEBGM_RACP_RESERVED_0                                            0x00
#define BLEBGM_RACP_REPORT_RECORDS                                        0x01
#define BLEBGM_RACP_CLEAR_RECORDS                                         0x02
#define BLEBGM_RACP_ABORT_REPORTING                                       0x03
#define BLEBGM_RACP_REQ_NUM_OF_RECORDS                                    0x04
#define BLEBGM_RACP_RSP_NUM_OF_RECORDS                                    0x05
#define BLEBGM_RACP_RSP_CODE                                              0x06


// RACP Operator.
#define BLEBGM_RACP_OPERATOR_NULL                                         0x00
#define BLEBGM_RACP_OPERATOR_ALL_STORED_RECORDS                           0x01
#define BLEBGM_RACP_OPERATOR_LESS_OR_EQUAL                                0x02
#define BLEBGM_RACP_OPERATOR_LARGER_OR_EQUAL                              0x03
#define BLEBGM_RACP_OPERATOR_IN_RANGE                                     0x04
#define BLEBGM_RACP_OPERATOR_FIRST_RECORD                                 0x05
#define BLEBGM_RACP_OPERATOR_LAST_RECORD                                  0x06


// RACP response code.
// Defined in Glucose Characteristic D09R01.
#define BLEBGM_RACP_RSP_CODE_RESERVED                                     0x00
#define BLEBGM_RACP_RSP_CODE_SUCCESS                                      0x01
#define BLEBGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED                        0x02
#define BLEBGM_RACP_RSP_CODE_INVALID_OPERATOR                             0x03
#define BLEBGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED                       0x04
#define BLEBGM_RACP_RSP_CODE_INVALID_OPERAND                              0x05
#define BLEBGM_RACP_RSP_CODE_NO_RECORDS_FOUND                             0x06
#define BLEBGM_RACP_RSP_CODE_ABORT_UNSUCCESSFUL                           0x07
#define BLEBGM_RACP_RSP_CODE_PROCEDURE_NOT_COMPLETED                      0x08
#define BLEBGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED                    0x09
// Defined in Glucose Service D09r08
#define BLEBGM_RACP_RSP_PROCEDURE_ALREADY_IN_PROGRESS                     0x80
#define BLEBGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY                       0x81


// RACP Filter Type.
#define BLEBGM_RACP_FILTER_TYPE_RESERVED                                  0x00
#define BLEBGM_RACP_FILTER_TYPE_SEQUENCE_NUMBER                           0x01
#define BLEBGM_RACP_FILTER_TYPE_FACING_TIME                               0x02


typedef PACKED struct
{
    UINT8     opCode;
    UINT8     operatorValue;
} BLEBGM_RACP_HDR;


typedef enum
{
  BLEBGM_RACP_IDLE,
  BLEBGM_RACP_PEND,
  BLEBGM_RACP_ACTIVE,  // this should be doing the buck of activities.
  BLEBGM_RACP_COMPLETE // this should be waiting for Infication Conf.

} BLEBGM_RACP_STATE;

typedef enum
{
    BLEBGM_CCC_MEASUREMENT         = 0x01,
    BLEBGM_CCC_MEASUREMENT_CONTEXT = 0x02,		
    BLEBGM_CCC_RACP                = 0x04
} BLEBGM_CCC_BITMAP;

//These bit maps are defined for use in the misc field of BLEBGM_APP_STORAGE
#define BLEBGM_APP_FLAG_VALID                                     0x8000
#define BLEBGM_APP_FLAG_ADR_TYPE                                  0x4000
#define BLEBGM_APP_FLAG_MEASUREMENT_CONF                          0x0001
#define BLEBGM_APP_FLAG_MEASUREMENT_CONTEXT_CONF                  0x0002
#define BLEBGM_APP_FLAG_RACP_CONF                                 0x0004
typedef PACKED struct
{
    // BD address of the bonded host
    UINT8    bdAdr[BLEBGM_BD_ADR_LEN];
    UINT16   misc; // one of the 16 bits is used to store the adrtype.
                   // one of the 16 bits is used for valid or not flag.
                   // other 15 bits can be used for other purposes.
}  BLEBGM_APP_STORAGE;


//////////////////////////////////////////////////////////////////////////////
//       Glucose Service defined Error code on type of ATT.




////////////////////////////////////////////////////////////////////////////
//            Some constants
#define BLEBGM_GET_DB_RECORD_SUCCESS                                      0x0
#define BLEBGM_GET_DB_RECORD_FAIL                                         0x1
#define BLEBGM_GET_DB_RECORD_NO_MORE_RECORD                               0x2



//We support a maximum of 4 hosts bonded.
#define BLEBGM_MAX_BONDED_HOST                                            0x4



#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blebgm_Create(void);
void blebgm_UARTtx (char *p_str);
void blebgm_DUARTtx (char *p_str);
int blebgm_PUARTTx(char *data, UINT8 len);
int blebgm_PUARTTxMaxWait(char *data, UINT8 len, UINT8 ms);
int blebgm_PUARTTxEchoMaxWait(char *data, UINT8 len, UINT8 tx_ms, UINT8 rx_ms, UINT8 flush);
int blebgm_PUARTRx(char *data, UINT8 len);
int blebgm_PUARTRxMaxWait(char *data, UINT8 len, UINT8 ms);
int blebgm_ReadPUART(char *data);
void blebgm_Sleep(UINT8 hund_us);
void blebgm_SetOutput(UINT8 value);
void blebgm_PUART_EnableRxInt(UINT8 rxpin, void (*userfn)(void*));
void blebgm_handleMeasurement(UINT8 *blebgm_gm_data_ptr);

UINT64 blebgm_getTimeStampInseconds(UINT8* baseTime, INT16 timeOffset, UINT8 flags);
UINT32 blebgm_checkRACPformat(UINT8 *ptr, INT32 len);
UINT32 blebgm_checkGetRecordformat(UINT8 *ptr, INT32 len);
UINT32 blebgm_checkAbortReportingformat(UINT8 *ptr, INT32 len);
UINT32 blebgm_checkClearRecordformat(UINT8 *ptr, INT32 len);
UINT32 blebgm_checkNumRecordformat(UINT8 *ptr, INT32 len);


extern const UINT8 blebgm_db_data[];
extern const UINT16 blebgm_db_size;
extern const BLE_PROFILE_CFG blebgm_cfg;
extern const BLE_PROFILE_PUART_CFG blebgm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blebgm_gpio_cfg;

#endif // end of #ifndef _BLEBGM_H_


