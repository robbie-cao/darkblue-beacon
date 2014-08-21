#ifndef _BLECGM_H_
#define _BLECGM_H_
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
* File Name: blecgm.h
*
* Abstract: This file implements the BLE Blood Pressure Profile and Service
*
* Functions:
*
*******************************************************************************/
#include "bleprofile.h"


// Module, Test, Securuty setting
//#define BLE_P1
#define BLE_P2

//#define BLETEST_ENABLE
//#define BLE_SECURITY_REQUEST
#define BLE_AUTH_WRITE
#define BLE_CONNECTION_PARAMETER_UPDATE
#define BLECGM_IOP_ADVANCE_DB_BIG
#define BLECGM_IOP_ADVANCE_DB_SIMULATE_DATA


//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

// this has been defined in many places.
#define BLECGM_BD_ADR_LEN         6

//////////////////////////////////////////////////////////////////////////////
//    GLUCOSE_MEANSUREMENT, D07R01.
////////////// Flags.
#define BLECGM_M_F_TREND_INFORMATION_PRESENT                0x01
#define BLECGM_M_F_QUALITY_PRESENT                          0x02
#define BLECGM_M_F_STATUS_ANNUNCIATION_WARNING              0x20 
#define BLECGM_M_F_STATUS_ANNUNCIATION_CALORTEMP            0x40
#define BLECGM_M_F_SENSOR_STATUS_ANNUNCIATION               0x80
#define BLECGM_M_F_RFU_a                                    0x04
#define BLECGM_M_F_RFU_b                                    0x08
#define BLECGM_M_F_RFU_c                                    0x10



////////////// timeOffset
#define BLECGM_TIME_OFFSET_OVER_RUN                         0x7fff
#define BLECGM_TIME_OFFSET_UNDER_RUN                        0x8000

////////////// type_sample_location
#define BLECGM_TYPE_SAMPLE_LOC_RESERVED                     0x00

#define BLECGM_TYPE_TYPE_MASK                               0xf
#define BLECGM_TYPE_CAPILLARY_WHOLE_BLOOD                   0x1
#define BLECGM_TYPE_CAPILLARY_PLASMA                        0x2
#define BLECGM_TYPE_VENOUS_WHILE_BLOOD                      0x3
#define BLECGM_TYPE_VENOUS_PLASMA                           0x4
#define BLECGM_TYPE_ARTERIAL_WHOLE_BLOOD                    0x5
#define BLECGM_TYPE_ARTERIAL_PLASMA                         0x6
#define BLECGM_TYPE_UNDETERMINED_WHOLE_BLOOD                0x7
#define BLECGM_TYPE_UNDETERMINED_PLASMA                     0x8
#define BLECGM_TYPE_INTERSTITIAL_FLUID                      0x9
#define BLECGM_TYPE_CONTROL_SOLUTION                        0xa

#define BLECGM_SAMPLE_LOC_SAMPLE_LOC_MASK                   0xf0
#define BLECGM_SAMPLE_LOC_FINGER                            0x1
#define BLECGM_SAMPLE_LOC_ALTERNATE_SITE_TEST               0x2
#define BLECGM_SAMPLE_LOC_EARLOBE                           0x3
#define BLECGM_SAMPLE_LOC_CONTROL_SOLUTION                  0x4
#define BLECGM_SAMPLE_LOC_RFU_5                             0x5
#define BLECGM_SAMPLE_LOC_RFU_6                             0x6
#define BLECGM_SAMPLE_LOC_RFU_7                             0x7
#define BLECGM_SAMPLE_LOC_RFU_8                             0x8
#define BLECGM_SAMPLE_LOC_RFU_9                             0x9
#define BLECGM_SAMPLE_LOC_RFU_a                             0xa
#define BLECGM_SAMPLE_LOC_RFU_b                             0xb
#define BLECGM_SAMPLE_LOC_RFU_c                             0xc
#define BLECGM_SAMPLE_LOC_RFU_d                             0xd
#define BLECGM_SAMPLE_LOC_RFU_e                             0xe
#define BLECGM_SAMPLE_LOC_NOT_AVAILABLE                     0xf


////////////// Sensor Status Annunciation
#define BLECGM_SENSOR_STATUS_SESSION_STOPPED                  0x000001
#define BLECGM_SENSOR_STATUS_DEVICE_BATTERY_LOW               0x000002
#define BLECGM_SENSOR_STATUS_SENSOR_TYPE_INCORRECT            0x000004
#define BLECGM_SENSOR_STATUS_MALFUNCTION                      0x000008
#define BLECGM_SENSOR_STATUS_DEVICE_SPECIFIC_ALERT            0x000010
#define BLECGM_SENSOR_STATUS_GENERAL_DEVICE_FAULT             0x000020
#define BLECGM_SENSOR_STATUS_TIME_SYNCHRONIZATION_REQUIRED    0x000100
#define BLECGM_SENSOR_STATUS_CALIBRATION_NOT_ALLOWED          0x000200
#define BLECGM_SENSOR_STATUS_CALIBRATION_RECOMMENDED          0x000400
#define BLECGM_SENSOR_STATUS_CALIBRATION_REQUIRED             0x000800
#define BLECGM_SENSOR_STATUS_HIGH_TEMPERATURE                 0x001000
#define BLECGM_SENSOR_STATUS_LOW_TEMPERATURE                  0x002000
#define BLECGM_SENSOR_STATUS_LOWER_PATIENT_LEVEL              0x010000
#define BLECGM_SENSOR_STATUS_HIGHER_PATIENT_LEVEL             0x020000
#define BLECGM_SENSOR_STATUS_LOWER_HYPO_LEVEL                 0x040000
#define BLECGM_SENSOR_STATUS_HIGHER_HYPER_LEVEL               0x080000
#define BLECGM_SENSOR_STATUS_DECREASE_RATE_EXCEEDED           0x100000
#define BLECGM_SENSOR_STATUS_INCREASE_RATE_EXCEEDED           0x200000
#define BLECGM_SENSOR_STATUS_LOWER_DEVICE_PROCESS             0x400000
#define BLECGM_SENSOR_STATUS_HIGHER_DEVICE_PROCESS            0x800000


#define BLECGM_SENSOR_STATUS_RFU_a                            0x000040
#define BLECGM_SENSOR_STATUS_RFU_b                            0x000080
#define BLECGM_SENSOR_STATUS_RFU_c                            0x004000
#define BLECGM_SENSOR_STATUS_RFU_d                            0x008000


typedef PACKED struct
{
   UINT8     size;
   UINT8     flag;
   SFLOAT    glucoseConcentration; // 2 bytes
   UINT16    timeOffset; // 2 bytes
   UINT8     sensorStatus[3]; // 3 bytes
   SFLOAT    trendinformation;
   SFLOAT    qualality;
} BLECGM_GLUCOSE_MEASUREMENT;





//////////////////////////////////////////////////////////////////////////////
//    GLUCOSE FEATUES.
#define BLECGM_GLUCOSE_FEATURES_CALIBRATION_SUPPORTED                     0x000001
#define BLECGM_GLUCOSE_FEATURES_PATIENT_HIGHORLOW_ALERT_SUPPORTED         0x000002
#define BLECGM_GLUCOSE_FEATURES_HYPO_ALERT_SUPPORTED                      0x000004
#define BLECGM_GLUCOSE_FEATURES_HYPER_ALERT_SUPPORTED                     0x000008
#define BLECGM_GLUCOSE_FEATURES_RATE_INCREASEORDECREASE_ALERT_SUPPORTED   0x000010
#define BLECGM_GLUCOSE_FEATURES_DEVICE_SPECIFIC_ALERT_SUPPORTED           0x000020
#define BLECGM_GLUCOSE_FEATURES_SENSOR_MALFUNCTION_DETECTION_SUPPORTED    0x000040
#define BLECGM_GLUCOSE_FEATURES_SENSOR_TEMPERATURE_DETECTION_SUPPORTED    0x000080
#define BLECGM_GLUCOSE_FEATURES_SENSOR_RESULT_DETECTION_SUPPORTED         0x000100
#define BLECGM_GLUCOSE_FEATURES_LOW_BATTERY_DETECTION_SUPPORTED           0x000200
#define BLECGM_GLUCOSE_FEATURES_SENSOR_TYPE_ERROR_DETECTION_SUPPORTED     0x000400
#define BLECGM_GLUCOSE_FEATURES_GENERAL_DEVICE_FAULT_SUPPORTED            0x000800
#define BLECGM_GLUCOSE_FEATURES_E2ECRC_SUPPORTED                          0x001000
#define BLECGM_GLUCOSE_FEATURES_MULTIPLE_BOND_SUPPORTED                   0x002000
#define BLECGM_GLUCOSE_FEATURES_MULTIPLE_SESSION_SUPPORTED                0x004000
#define BLECGM_GLUCOSE_FEATURES_TREND_INFORMATION_SUPPORTED               0x008000
#define BLECGM_GLUCOSE_FEATURES_QUALITY_SUPPORTED                         0x010000

#define BLECGM_SENSOR_FEATURES_RFU_a                                      0x020000
#define BLECGM_SENSOR_FEATURES_RFU_b                                      0x040000
#define BLECGM_SENSOR_FEATURES_RFU_c                                      0x080000
#define BLECGM_SENSOR_FEATURES_RFU_d                                      0x100000
#define BLECGM_SENSOR_FEATURES_RFU_e                                      0x200000
#define BLECGM_SENSOR_FEATURES_RFU_f                                      0x400000
#define BLECGM_SENSOR_FEATURES_RFU_g                                      0x800000

//////////////////////////////////////////////////////////////////////////////
//                      Record Access Control Point.

// RACP Op Code.
#define BLECGM_RACP_RESERVED_0                                            0x00
#define BLECGM_RACP_REPORT_RECORDS                                        0x01
#define BLECGM_RACP_CLEAR_RECORDS                                         0x02
#define BLECGM_RACP_ABORT_REPORTING                                       0x03
#define BLECGM_RACP_REQ_NUM_OF_RECORDS                                    0x04
#define BLECGM_RACP_RSP_NUM_OF_RECORDS                                    0x05
#define BLECGM_RACP_RSP_CODE                                              0x06


// RACP Operator.
#define BLECGM_RACP_OPERATOR_NULL                                         0x00
#define BLECGM_RACP_OPERATOR_ALL_STORED_RECORDS                           0x01
#define BLECGM_RACP_OPERATOR_LESS_OR_EQUAL                                0x02
#define BLECGM_RACP_OPERATOR_LARGER_OR_EQUAL                              0x03
#define BLECGM_RACP_OPERATOR_IN_RANGE                                     0x04
#define BLECGM_RACP_OPERATOR_FIRST_RECORD                                 0x05
#define BLECGM_RACP_OPERATOR_LAST_RECORD                                  0x06


// RACP response code.
// Defined in Glucose Characteristic D09R01.
#define BLECGM_RACP_RSP_CODE_RESERVED                                     0x00
#define BLECGM_RACP_RSP_CODE_SUCCESS                                      0x01
#define BLECGM_RACP_RSP_CODE_OP_CODE_NOT_SUPPORTED                        0x02
#define BLECGM_RACP_RSP_CODE_INVALID_OPERATOR                             0x03
#define BLECGM_RACP_RSP_CODE_OPERATOR_NOT_SUPPORTED                       0x04
#define BLECGM_RACP_RSP_CODE_INVALID_OPERAND                              0x05
#define BLECGM_RACP_RSP_CODE_NO_RECORDS_FOUND                             0x06
#define BLECGM_RACP_RSP_CODE_ABORT_UNSUCCESSFUL                           0x07
#define BLECGM_RACP_RSP_CODE_PROCEDURE_NOT_COMPLETED                      0x08
#define BLECGM_RACP_RSP_CODE_FILTER_TYPE_NOT_SUPPORTED                    0x09
// Defined in Glucose Service D09r08
#define BLECGM_RACP_RSP_PROCEDURE_ALREADY_IN_PROGRESS                     0x80
#define BLECGM_RACP_RSP_CLIENT_CHAR_CONF_IMPROPERLY                       0x81


// RACP Filter Type.
#define BLECGM_RACP_FILTER_TYPE_RESERVED                                  0x00
#define BLECGM_RACP_FILTER_TYPE_TIME_OFFSET                               0x01
#ifdef BLECGM_FACETIME
#define BLECGM_RACP_FILTER_TYPE_FACING_TIME                               0x02
#endif

#ifdef BLECGM_ASCPENABLE
//////////////////////////////////////////////////////////////////////////////
//                      Application Security Control Point.

#define BLECGM_ASCP_OPERAND_MAXLEN                                        19 //max length refer Characteristic D09R05.
// ASCP Op Code.
#define BLECGM_ASCP_RESERVED_0                                            0x00
#define BLECGM_ASCP_REQ_AUTH                                              0x01
#define BLECGM_ASCP_SET_AUTH_CODE                                         0x02
#define BLECGM_ASCP_SET_PRIMARY_COLLECTOR                                 0x03
#define BLECGM_ASCP_AUTH_RESPONSE                                         0x04

// ASCP response code.
// Defined in Glucose Characteristic D09R05.
#define BLECGM_ASCP_RSP_CODE_RESERVED                                     0x00
#define BLECGM_ASCP_RSP_CODE_SUCCESS                                      0x01
#define BLECGM_ASCP_RSP_CODE_OP_CODE_NOT_SUPPORTED                        0x02
#define BLECGM_ASCP_RSP_CODE_INVALID_OPERAND                              0x03
#define BLECGM_ASCP_RSP_CODE_AUTH_FAILED                                  0x04
#define BLECGM_ASCP_RSP_CODE_INSUFFICIENT_AUTH                            0x05
#endif


//////////////////////////////////////////////////////////////////////////////
//                      CGM Control Point.

// CGMCP Op Code.
#define BLECGM_CGMCP_RESERVED_0                                          0x00
#define BLECGM_CGMCP_SET_COMMUNICATION_INTERVAL                          0x01
#define BLECGM_CGMCP_GET_COMMUNICATION_INTERVAL                          0x02
#define BLECGM_CGMCP_SET_CALIBRATION_VALUE                               0x04
#define BLECGM_CGMCP_GET_CALIBRATION_VALUE                               0x05
#define BLECGM_CGMCP_SET_HIGH_ALERT                                      0x07
#define BLECGM_CGMCP_GET_HIGH_ALERT                                      0x08
#define BLECGM_CGMCP_SET_LOW_ALERT                                       0x0A
#define BLECGM_CGMCP_GET_LOW_ALERT                                       0x0B
#define BLECGM_CGMCP_START_SESSION                                       0x0D
#define BLECGM_CGMCP_STOP_SESSION                                        0x0E

// CGMCP response op code.
// Defined in Glucose Characteristic D09R05.
#define BLECGM_CGMCP_RSP_GET_COMMUNICATION_INTERVAL                       0x03
#define BLECGM_CGMCP_RSP_GET_CALIBRATION_VALUE                            0x06
#define BLECGM_CGMCP_RSP_GET_HIGH_ALERT                                   0x09
#define BLECGM_CGMCP_RSP_GET_LOW_ALERT                                    0x0C
#define BLECGM_CGMCP_RSP_OPCODE                                           0x0F

// CGMCP response code.
// Defined in Glucose Characteristic D09R05.
#define BLECGM_CGMCP_RSP_CODE_RESERVED                                     0x00
#define BLECGM_CGMCP_RSP_CODE_SUCCESS                                      0x01
#define BLECGM_CGMCP_RSP_CODE_OP_CODE_NOT_SUPPORTED                        0x02
#define BLECGM_CGMCP_RSP_CODE_INVALID_OPERAND                              0x03
#define BLECGM_CGMCP_RSP_CODE_PROCEDURE_NOT_COMPLETED                      0x04
#define BLECGM_CGMCP_RSP_CODE_PARAMETER_OUT_OF_RANGE                       0x05


typedef PACKED struct
{
    UINT8     opCode;
    UINT8     operatorValue;
} BLECGM_RACP_HDR;

#ifdef BLECGM_ASCPENABLE
typedef PACKED struct
{
    UINT8     opCode;
    UINT8     operandValue[BLECGM_ASCP_OPERAND_MAXLEN]; 
} BLECGM_ASCP_HDR;
#endif

typedef PACKED struct
{
 SFLOAT Concentration;
 UINT16 Time;
 UINT8 TypeSampleLocation;
 UINT16 NextCalibrationTime;
 UINT16 DataRecordNumber;
 UINT8 Status[9];
} BLECGM_CGMCP_CALIBRATION;

typedef enum
{
  BLECGM_RACP_IDLE,
  BLECGM_RACP_PEND,
  BLECGM_RACP_ACTIVE,  // this should be doing the buck of activities.
  BLECGM_RACP_COMPLETE // this should be waiting for Infication Conf.

} BLECGM_RACP_STATE;

typedef enum
{
    BLECGM_CCC_MEASUREMENT = 0x01,
    BLECGM_CCC_RACP = 0x02,
#ifdef BLECGM_ASCPENABLE
    BLECGM_CCC_ASCP = 0x04,
#endif
    BLECGM_CCC_CGMCP = 0x8,
} BLECGM_CCC_BITMAP;

//These bit maps are defined for use in the misc field of BLECGM_APP_STORAGE
#define BLECGM_APP_FLAG_VALID                                     0x8000
#define BLECGM_APP_FLAG_ADR_TYPE                                  0x4000
#define BLECGM_APP_FLAG_MEASUREMENT_CONF                          0x0001
#define BLECGM_APP_FLAG_RACP_CONF                                 0x0002
typedef PACKED struct
{
   // BD address of the bonded host
    UINT8    bdAdr[BLECGM_BD_ADR_LEN];
    UINT16   misc; // one of the 16 bits is used to store the adrtype.
                   // one of the 16 bits is used for valid or not flag.
                   // other 15 bits can be used for other purposes.
}  BLECGM_APP_STORAGE;

//////////////////////////////////////////////////////////////////////////////
//       Glucose Service defined Error code on type of ATT.
///////////////////////////////////////////////////////////////////////////
//            Some constants
#define BLECGM_GET_DB_RECORD_SUCCESS                                      0x0
#define BLECGM_GET_DB_RECORD_FAIL                                         0x1
#define BLECGM_GET_DB_RECORD_NO_MORE_RECORD                               0x2



//We support a maximum of 4 hosts bonded.
#define BLECGM_MAX_BONDED_HOST                                            0x4



#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecgm_Create(void);
void blecgm_UARTtx (char *p_str);
void blecgm_DUARTtx (char *p_str);
int blecgm_PUARTTx(char *data, UINT8 len);
int blecgm_PUARTTxMaxWait(char *data, UINT8 len, UINT8 ms);
			int blecgm_PUARTTxEchoMaxWait(char *data, UINT8 len, UINT8 tx_ms, UINT8 rx_ms, UINT8 flush);
int blecgm_PUARTRx(char *data, UINT8 len);
int blecgm_PUARTRxMaxWait(char *data, UINT8 len, UINT8 ms);
int blecgm_ReadPUART(char *data);
void blecgm_Sleep(UINT8 hund_us);
void blecgm_SetOutput(UINT8 value);
void blecgm_PUART_EnableRxInt(UINT8 rxpin, void (*userfn)(void*));
void blecgm_handleMeasurement(UINT8 *blecgm_gm_data_ptr);

UINT64 blecgm_getTimeStampInseconds(UINT8* baseTime, UINT16 timeOffset);
UINT32 blecgm_checkRACPformat(UINT8 *ptr, INT32 len);
UINT32 blecgm_checkGetRecordformat(UINT8 *ptr, INT32 len);
UINT32 blecgm_checkAbortReportingformat(UINT8 *ptr, INT32 len);
UINT32 blecgm_checkClearRecordformat(UINT8 *ptr, INT32 len);
UINT32 blecgm_checkNumRecordformat(UINT8 *ptr, INT32 len);

#ifdef BLECGM_ASCPENABLE
UINT32 blecgm_checkASCPformat(BLECGM_ASCP_HDR *ptr, INT32 len);
UINT32 blecgm_checkAuthentication(BLECGM_ASCP_HDR *ptr, INT32 len);
UINT32 blecgm_setauthcode(BLECGM_ASCP_HDR *ptr, INT32 len);
UINT32 blecgm_setprimarycollector(BLECGM_ASCP_HDR *ptr, INT32 len);
#endif

extern const UINT8 blecgm_db_data[];
extern const UINT16 blecgm_db_size;
extern const BLE_PROFILE_CFG blecgm_cfg;
extern const BLE_PROFILE_PUART_CFG blecgm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blecgm_gpio_cfg;

#endif // end of #ifndef _BLECGM_H_
