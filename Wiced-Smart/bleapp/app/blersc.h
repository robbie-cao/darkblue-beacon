#ifndef _BLERSC_H_
#define _BLERSC_H_
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
* This file provides definitions and function prototypes required for 
* implementation of a sample Running Speed and Cadence Sensor
*
*/
#include "bleprofile.h"

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

#define SUPPORTED_SENLOC_MAX                    3
#define BLERSC_CP_PROCEDURE_ALREADY_IN_PROGRESS 0x80
#define BLERSC_CP_CLIENT_CHAR_CONF_IMPROPERLY   0x81

//flag enum
enum blersc_flag
{
    RSC_INSTANTANEOUS_STRIDE_LENGTH_PRESENT     = 0x01,
    RSC_TOTAL_DISTANCE_PRESENT                  = 0x02,
    RSC_WALKING_OR_RUNNING_STATUS               = 0x04,    
};

// CP Op Code
enum blersc_opcode
{
    RSC_CP_RESET_TOTAL_DISTANCE                 = 0x01,
    RSC_CP_START_SENSOR_CALIBRATION             = 0x02,
    RSC_CP_UPDATE_SENSOR_LOCATION               = 0x03,
    RSC_CP_REQUEST_SUPPORTED_SENSOR_LOCATIONS   = 0x04,
    RSC_CP_RESPONSE_CODE                        = 0x10,
};

// CP Response Values
enum blersc_responsevalues
{
    RSC_CP_SUCCESS                              = 0x01,
    RSC_CP_OPCODE_NOT_SUPPORTED                 = 0x02,
    RSC_CP_INVALID_PARAMETER                    = 0x03,
    RSC_CP_OPERATION_FAILED                     = 0x04,
};

// sensor location
enum blersc_sensorlocation
{
    RSC_LOC_START                               = 0x01,
    RSC_LOC_TOP_OF_SHOE                         = 0x01,
    RSC_LOC_IN_SHOE                             = 0x02,
    RSC_LOC_HIP                                 = 0x03,
    RSC_LOC_END                                 = 0x03,
};

typedef UINT8 UINT24[3];

typedef PACKED struct
{
    UINT8       flag;
    UINT16      instantaneous_speed;
    UINT8       instantaneous_cadence;
    UINT16      instantaneous_stride_length;
    UINT24      total_distance;
}  BLERSC_RSC_DATA;

//host information for NVRAM
typedef PACKED struct
//typedef struct
{
    // BD address of the bonded host
    BD_ADDR     bdAddr;
    UINT16      serv[2];
    UINT16      cha[2];
    UINT16      cli_cha_desc[2];	
}  BLERSC_HOSTINFO;

typedef PACKED struct
{
    UINT8       opcode;
    UINT8       parameter[18];
} BLERSC_CP_HDR;

typedef PACKED struct
{
    UINT8       opcode;
    UINT8       request_opcode;
    UINT8       response_value;
    UINT8       response_parameter[16];
} BLERSC_CP_RSP_HDR;


#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blersc_Create(void);

extern const UINT8                  blersc_db_data[];
extern const UINT16                 blersc_db_size;
extern const BLE_PROFILE_CFG        blersc_cfg;
extern const BLE_PROFILE_PUART_CFG  blersc_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG   blersc_gpio_cfg;

#endif // end of #ifndef _BLERSC_H_

