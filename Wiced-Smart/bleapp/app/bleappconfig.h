/*******************************************************************************
* ------------------------------------------------------------------------------
*
* Copyright (c), 2011 BROADCOM Corp.
*
*          ALL RIGHTS RESERVED
*
* This file contains all configuration items used by ble applications. 
********************************************************************************/
#ifndef __BLE_APP_CONFIG__
#define __BLE_APP_CONFIG__

#include "types.h"
#include "bleprofile.h"
#include "hidddriversconfig.h"

#pragma pack(1)
/// Miscellaneous configuration items for applications.
/// Currently, only the application dedicated area in the EEPROM is defined here
/// Items that do not logically fit anywhere maybe added to this.
typedef PACKED struct
{
    /// Port and Pin to which EEPROM WP is connected
    /// Most significant 3 bits - port;
    /// Least significant 5 bits - pin;
    /// 0xFF when WP is not configured
    BYTE bleapputils_eepromWpPortPin;

    /// Location of the application deicated area in the EEPROM
    UINT16 bleapputils_eepromCustAreaOffset;

    /// Size of the application dedicated area in the EEPROM
    UINT16 bleapputils_eepromCustAreaLen;
} MiscBleAppConfig;

#pragma pack()

extern BLE_PROFILE_CFG bleprofile_cfg;
extern BLE_PROFILE_PUART_CFG bleprofile_puart_cfg;
extern BLE_PROFILE_GPIO_CFG bleprofile_gpio_cfg;
extern BLE_PROFILE_PWM_CFG bleprofile_pwm_cfg;
extern BLEAPP_INIT_CFG bleapp_init_cfg;
extern BLEAPP_DB_CFG bleapp_db_cfg;
extern UINT8* bleapp_p_db;
extern BLEAPP_SELECT_LPO_CFG bleapp_select_lpo_cfg;
extern BLEAPP_CPU_CLOCK_CFG bleapp_cpu_clock_cfg;
extern BLEBAT_BATMON_CFG blebat_batmon_cfg;
extern UINT8 bleapp_trace_enable;
extern UINT8 bleapp_hidoff_enable;
extern UINT32 bleapp_max_sleep;
#endif

