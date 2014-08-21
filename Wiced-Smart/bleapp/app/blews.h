#ifndef _BLEWS_H_
#define _BLEWS_H_
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
* File Name: blews.h
*
* Abstract: This file implements the BLE Blood Pressure profile, service, application
*
* Functions:
*
*******************************************************************************/
#include "bleprofile.h"

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

//flag enum
enum blews_flag
{
    WS_UNIT_LBS = 0x0001,
    WS_TIME_AND_DATE = 0x0002,
    WS_TIME_STAMP = 0x0004,
    WS_BMI = 0x0008,
    WS_HEIGHT = 0x0010,
    WS_UNIT_INCH = 0x0020,
    WS_PERSON_ID = 0x0040,
    WS_DATE_OF_BIRTH = 0x0080,
    WS_GENDER = 0x0100,
    WS_BODY_FAT_PERCENTAGE = 0x0200,
    WS_BASAL_METABOLISM = 0x0400,
    WS_SKELETAL_MUSCLE_PERCENTAGE = 0x0800,
};


typedef PACKED struct
{
    UINT8 flag; // UINT16 flag;
    FLOAT32 weight;
    TIMESTAMP timeanddate; //7bytes
    TIMESTAMP timestamp; //7bytes
    FLOAT32 bmi;
    FLOAT32 height;
    UINT8 personid;
    TIMESTAMP dateofbirth;
    UINT8 gender;
    SFLOAT bodyfatpercentage;
    FLOAT32 basalmetabolism;
    SFLOAT skeletalmusclepercentage;
}  BLEWS_WS_DATA;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blews_Create(void);

extern const UINT8 blews_db_data[];
extern const UINT16 blews_db_size;
extern const BLE_PROFILE_CFG blews_cfg;
extern const BLE_PROFILE_PUART_CFG blews_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blews_gpio_cfg;

#endif // end of #ifndef _BLEWS_H_

