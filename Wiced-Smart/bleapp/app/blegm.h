#ifndef _BLEGM_H_
#define _BLEGM_H_
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
* File Name: blegm.h
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
enum blegm_flag
{
    GM_GLUCOSE_MEASUREMENT = 0x01,
    GM_UNIT_MMOLL = 0x02,
    GM_TIME_STAMP = 0x04,
    GM_DATE_AND_TIME_ADJUST = 0x08,
    GM_SAMPLE_LOCATION = 0x10,
    GM_MEAL = 0x20,
    GM_SENSOR_ANNUNCIATION = 0x40,
    GM_SEQUENCE_NUMBER = 0x80,
};

enum blegm_puart_rx_int
{
    GM_PUART_RX_INT_ENABLE = 0x01,
    GM_PUART_RX_FIFO_INT_ENABLE = 0x02,
    GM_PUART_RX_CTS_ENABLE = 0x04,
};

typedef PACKED struct
{
    UINT8 flag;
    SFLOAT glucosemeasurement;
    TIMESTAMP timestamp; //7bytes
    INT32 dateandtimeadjust;
    UINT8 samplelocationvalue;
    UINT8 mealvalue;
    UINT16 sensorannunciation;
    UINT16 sequencenumber;
}  BLEGM_GM_DATA;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blegm_Create(void);
void blegm_UARTtx (char *p_str);
void blegm_DUARTtx (char *p_str);
int blegm_PUARTTx(char *data, UINT8 len);
int blegm_PUARTTxMaxWait(char *data, UINT8 len, UINT8 ms);
int blegm_PUARTTxEchoMaxWait(char *data, UINT8 len, UINT8 tx_ms, UINT8 rx_ms, UINT8 flush);
int blegm_PUARTRx(char *data, UINT8 len);
int blegm_PUARTRxMaxWait(char *data, UINT8 len, UINT8 ms);
int blegm_ReadPUART(char *data);
void blegm_Sleep(UINT8 hund_us);
void blegm_SetOutput(UINT8 value);
void blegm_PUART_EnableRxInt(UINT8 rxpin, void (*userfn)(void*));
void blegm_PUART_EnableRxFIFOInt(void (*userfn)(void*));
void blegm_handleMeasurement(UINT8 *blegm_gm_data_ptr);

void blegm_timedHIDoffSetting(UINT32 min, UINT32 resolution);

void blegm_longtimedSleep(UINT16 min, UINT8 *p_longsleepwake);
void blegm_longtimedWake(UINT8 reason);

void blegm_startGMTimer( BLEAPP_TIMER_CB cb, UINT16 ms_timeout );
void blegm_stopGMTimer( void );

void blegm_GenerateADVData(UINT16 company_id, UINT16 seq_num);

extern const UINT8 blegm_db_data[];
extern const UINT16 blegm_db_size;
extern const BLE_PROFILE_CFG blegm_cfg;
extern const BLE_PROFILE_PUART_CFG blegm_puart_cfg;
extern const BLE_PROFILE_GPIO_CFG blegm_gpio_cfg;

#endif // end of #ifndef _BLEGM_H_


