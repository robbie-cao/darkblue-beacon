#ifndef _CYCLING_SPEED_CADENCE_H_
#define _CYCLING_SPEED_CADENCE_H_
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
* This file provides definitions and function prototypes for the BLE
* Cycling Speed and Cadence profile, service, application
*
* Refer to Bluetooth SIG Cycling Speed and Cadence Profile 1.0 Cycling Speed
* and Cadence Service 1.0 specifications for details.
*
*/
#include "bleprofile.h"
#include "sparcommon.h"

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

#define CSC_SUPPORTED_SENLOC_MAX 11
#define BLECSC_CP_PROCEDURE_ALREADY_IN_PROGRESS 0x80
#define BLECSC_CP_CLIENT_CHAR_CONF_IMPROPERLY   0x81

//flag enum
enum blecsc_flag
{
    CSC_WHEEL_REVOLUTION_DATA_PRESENT = 0x01,
    CSC_CRANK_REVOLUTION_DATA_PRESENT = 0x02,
};

//flag feature
enum blecsc_feature
{
    CSC_WHEEL_REVOLUTION_DATA_SUPPORTED     = 0x01,
    CSC_CRANK_REVOLUTION_DATA_SUPPORTED     = 0x02,
    CSC_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED = 0x04,
};

// sensor location
enum blecsc_sensorlocation
{
    CSC_LOC_START        = 0x01,
    CSC_LOC_TOP_OF_SHOE  = 0x01,
    CSC_LOC_IN_SHOE      = 0x02,
    CSC_LOC_HIP          = 0x03,
    CSC_LOC_WHEEL        = 0x04,
    CSC_LOC_LEFT_CRANK   = 0x05,
    CSC_LOC_RIGHT_CRANK  = 0x06,
    CSC_LOC_LEFT_PEDAL   = 0x07,
    CSC_LOC_RIGHT_PEDAL  = 0x08,
    CSC_LOC_HUB          = 0x09,
    CSC_LOC_REAR_DROPOUT = 0x0A,
    CSC_LOC_CHAINSTAY    = 0x0B,
    CSC_LOC_END          = 0x0B,
};

// CP Op Code
enum blecsc_opcode
{
    CSC_CP_SET_CUMULATIVE_VALUE               = 0x01,
    CSC_CP_START_SENSOR_CALIBRATION           = 0x02,
    CSC_CP_UPDATE_SENSOR_LOCATION             = 0x03,
    CSC_CP_REQUEST_SUPPORTED_SENSOR_LOCATIONS = 0x04,
    CSC_CP_RESPONSE_CODE                      = 0x10,
};

// CP Response Values
enum blecsc_responsevalues
{
    CSC_CP_SUCCESS              = 0x01,
    CSC_CP_OPCODE_NOT_SUPPORTED = 0x02,
    CSC_CP_INVALID_PARAMETER    = 0x03,
    CSC_CP_OPERATION_FAILED     = 0x04,
};

typedef UINT8 UINT24[3];

typedef PACKED struct
{
    UINT8  flag;
    UINT32 cumulative_wheel_revolutions;
    UINT16 last_wheel_event_time;
    UINT16 cumulative_crank_revolutions;
    UINT16 last_crank_event_time;
}  BLECSC_CSC_DATA;

//host information for NVRAM
typedef PACKED struct
//typedef struct
{
    // BD address of the bonded host
    BD_ADDR  bdAddr;
    UINT16   serv[2];
    UINT16   cha[2];
    UINT16   cli_cha_desc[2];
}  BLECSC_HOSTINFO;

typedef PACKED struct
{
    UINT8     opcode;
    UINT8     parameter[18];
} BLECSC_CP_HDR;

typedef PACKED struct
{
    UINT8     opcode;
    UINT8     request_opcode;
    UINT8     response_value;
    UINT8     response_parameter[16];
} BLECSC_CP_RSP_HDR;


#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecsc_Create( void );

extern const UINT8                 blecsc_db_data[];
extern const UINT16                blecsc_db_size;
extern const BLE_PROFILE_CFG       blecsc_cfg;
extern const BLE_PROFILE_PUART_CFG blecsc_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG  blecsc_gpio_cfg;

#endif // end of #ifndef _BLECSC_H_

