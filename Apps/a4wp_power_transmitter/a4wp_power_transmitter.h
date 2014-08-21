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
* BLE Vendor Specific Device
*
* This file provides definitions and function prototypes for A4WP PTU device
* (Multi-connection)
*
*/
#ifndef _BLEPTUM_H_
#define _BLEPTUM_H_

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

// A4WP Service UUID
#define UUID_SERVCLASS_A4WP                     0xFFFE

// following definitions are the GPIOs used in the application
#define BLEPTUM_GPIO_PIN_WP                     1
#define BLEPTUM_GPIO_PIN_UART_TX                24
#define BLEPTUM_GPIO_PIN_UART_RX                25
#define BLEPTUM_GPIO_PIN_ADC1                   14
#define BLEPTUM_GPIO_PIN_ADC2                   15
#define BLEPTUM_GPIO_PIN_ADC3                   0
#define BLEPTUM_GPIO_PIN_LED1                   2
#define BLEPTUM_GPIO_PIN_CTR1                   4
#define BLEPTUM_GPIO_PIN_PWM1                   26
#define BLEPTUM_GPIO_PIN_PWM_BASE               26

//following definitions represent the ADC channels
#define BLEPTUM_ADC_CH1                         ADC_INPUT_P14
#define BLEPTUM_ADC_CH2                         ADC_INPUT_P15
#define BLEPTUM_ADC_CH3                         ADC_INPUT_P0

//defines the PWM Id(s) used by the pwm interfaces
#define BLEPTUM_PWM1_ID                         (BLEPTUM_GPIO_PIN_PWM1 - BLEPTUM_GPIO_PIN_PWM_BASE)

// A4WP characteristic offset values w.r.t primary service handle
#define BLEPTUM_PRU_CONTROL_HDL_OFFSET          2
#define BLEPTUM_PTU_STATIC_HDL_OFFSET           4
#define BLEPTUM_PRU_ALERT_HDL_OFFSET            6
#define BLEPTUM_PRU_ALERT_CCC_HDL_OFFSET        7
#define BLEPTUM_PRU_STATIC_HDL_OFFSET           9
#define BLEPTUM_PRU_DYNAMIC_HDL_OFFSET          11

// A4WP PTU static parameter length (in octets)
#define BLEPTUM_PTU_STATIC_PARAM_LEN            17

// A4WP PRU control parameter length (in octets)
#define BLEPTUM_PRU_CONTROL_PARAM_LEN           5

// CCC parameter length
#define BLEPTUM_CCC_PARAM_LEN                   2

// Max Number of connections allowed in PTU.
#define BLEPTUM_MAX_CON                         2

// Application level ATT transaction timeout (in seconds)
#define BLEPTUM_TRANSACTION_TIMEOUT             30

// extracts the 12bit connection handle
#define BLEPTUM_LEL2CAP_GET_ACL_CONN_HANDLE(a)  ((a) & 0x0fff)

// mimimum RSSI value of PRU devices used to filter during LE scan.
#define BLEPTUM_MIN_RSSI                        -60

// HCI connection timeout error
#define BLEPTUM_HCI_ERR_CONNECTION_TIMEOUT      0x08

/*
 * Following definitions can be used for additional app configurable options
 * Setting these masks in ptum_app_state->bleptu_config will enable the feature.
 */
#define BLEPTUM_SMP_PAIRING                     0x01
#define BLEPTUM_SMP_ERASE_KEY                   0x02

// enables the LE disconnection procedure when application level transation timer expires.
#define BLEPTUM_TRANS_TO_DISCONNECTION          0x01

// enable following feature to force internal HCI reset with connection timeout
//#define BLEPTUM_HCI_RESET                     0x02

// enable following feature to force watchdog reset with connection timeout
//#define BLEPTUM_WDTO                          0x04

// following feature flags enable the ADC, PWM, LED and Control GPIO sample codes
//#define BLEPTUM_PWM_TEST
//#define BLEPTUM_ADC_TEST

/* Following feature flags enables the characteristic value interpretation as debug traces.
 * - PRU static parameters
 * - PRU Dynamic parameters
 * - PRU Alert parameters
 */
#define BLEPTUM_PRU_STATIC_INTERPRET
#define BLEPTUM_PRU_DYNAMIC_INTERPRET
#define BLEPTUM_PRU_ALERT_INTERPRET

// following definitions are used to interpret the parameters during decode for prints
// refer the A4WP BSS 1.2 document for values used here.
#define BLEPTUM_TRACE_PRU_POWER_UNKNOWN         31
#define BLEPTUM_TRACE_PRU_GAIN_UNKNOWN          7
#define BLEPTUM_TRACE_PRU_POWER_START_BIT       3
#define BLEPTUM_TRACE_NO_IMP                    0
#define BLEPTUM_TRACE_MAX_IMP                   5
#define BLEPTUM_TRACE_NO_CATEGORY               0
#define BLEPTUM_TRACE_MAX_CATEGORY              5
#define BLEPTUM_TRACE_BYTE_SHIFT                8
#define BLEPTUM_TRACE_MAX_SOURCE_IMP            18
#define BLEPTUM_TRACE_MID_SOURCE_IMP            10
#define BLEPTUM_TRACE_PERMIT                    0
#define BLEPTUM_TRACE_MAX_POWER                 0
#define BLEPTUM_TRACE_POWER_33_PERCENT          0x01
#define BLEPTUM_TRACE_POWER_66_PERCENT          0x02
#define BLEPTUM_TRACE_PERMIT_WITH_WAIT          0x01
#define BLEPTUM_TRACE_PERMIT_ERROR              0x80
#define BLEPTUM_TRACE_PERMIT_POWER_LIMIT        0x81
#define BLEPTUM_TRACE_PERMIT_DEVICE_LIMIT       0x82
#define BLEPTUM_TRACE_PERMIT_CLASS_LIMIT        0x83

// bleptum state enum
enum bleptum_state
{
    BLEPTUM_IDLE                        = 0x00,
    BLEPTUM_SMP_DONE                    = 0x01,
    BLEPTUM_DISCOVER_DONE               = 0x02,

    BLEPTUM_W4_READ_PRU_STATIC          = 0x21,
    BLEPTUM_READ_PRU_STATIC_DONE        = 0x22,
    BLEPTUM_WRITE_PTU_STATIC_DONE       = 0x23,
    BLEPTUM_W4_1ST_READ_PRU_DYNAMIC     = 0x24,
    BLEPTUM_READ_1ST_PRU_DYNAMIC_DONE   = 0x25,
    BLEPTUM_WRITE_PRU_CONTROL_DONE      = 0x26,
    BLEPTUM_WRITE_CCC_DONE              = 0x27,
    BLEPTUM_W4_READ_PRU_DYNAMIC         = 0x28,
    BLEPTUM_READ_PRU_DYNAMIC_DONE       = 0x29,

    BLEPTUM_FAIL = 0xFF
};

// following structure member details are as per in A4WP BSS 1.2.
// A4WP PRU ADV payload format for service data
PACKED struct t_BLEPTUM_ADV_PAYLOAD
{
    UINT16  service_uuid;
    UINT16  gatt_primary_service_handle;
    UINT8   pru_rssi_parameters;
    UINT8   adv_flag;
};
typedef struct t_BLEPTUM_ADV_PAYLOAD BLEPTUM_ADV_PAYLOAD;

// A4WP PRU static parameter data format
PACKED struct t_BLEPTUM_PRU_STATIC
{
    UINT8   optional_fields;
    UINT8   protocol_rev;
    UINT8   rfu_1;   //RFU
    UINT8   pru_category;
    UINT8   pru_information;
    UINT8   hw_rev;
    UINT8   fw_rev;
    UINT8   maximum_power_desired;
    UINT16  v_rect_min_static;
    UINT16  v_rect_high_static;
    UINT16  v_rect_set;
    UINT16  delta_r1;
    UINT8   rfu_2[4];
};
typedef struct t_BLEPTUM_PRU_STATIC BLEPTUM_PRU_STATIC;

// A4WP PRU dynamic parameter data format
PACKED struct t_BLEPTUM_PRU_DYNAMIC
{
    UINT8   optional_fields;
    UINT16  v_rect;
    UINT16  i_rect;
    UINT16  v_out;
    UINT16  i_out;
    UINT8   temperature;
    UINT16  v_rect_min_dyn;
    UINT16  v_rect_set_dyn;
    UINT16  v_rect_high_dyn;
    UINT8   pru_alert;
    UINT8   rfu[3];
};
typedef struct t_BLEPTUM_PRU_DYNAMIC BLEPTUM_PRU_DYNAMIC;

// A4WP pru_information bit fields of PRU static parameter
typedef union t_BLEPTUM_PRU_INFO
{ 
    UINT8 pru_information;
    struct
    {
        UINT8 reserved          : 4;
        UINT8 power_back_off    : 1;
        UINT8 power_control     : 1;
        UINT8 sep_BTLE          : 1;
        UINT8 nfc_reciever      : 1;
    } bitmap;
} BLEPTUM_PRU_INFO;

// optional field bit map of A4WP PRU dynamic parameter
typedef union t_DYNAMIC_OPTIONAL_FIELDS
{
    UINT8 dyn_opt_fields;
    struct
    {
        UINT8 rfu               : 2;
        UINT8 pru_wait          : 1;
        UINT8 v_rect_high       : 1;
        UINT8 v_rect_set        : 1;
        UINT8 v_rect_min        : 1;
        UINT8 temperature       : 1;
        UINT8 i_out             : 1;
        UINT8 v_out             : 1;
    } bitmap;
} DYNAMIC_OPTIONAL_FIELDS;

// PRU alert bit fields in dynamic parameter
typedef union t_PRU_ALERT
{
    UINT8 pru_alert;
    struct
    {
        UINT8 rfu               : 1;
        UINT8 pru_charge_port   : 1;
        UINT8 wcd               : 1;
        UINT8 cc                : 1;
        UINT8 self_protection   : 1;
        UINT8 ot                : 1;
        UINT8 oc                : 1;
        UINT8 ov                : 1;
    } bitmap;
} PRU_ALERT;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

// application state details; this will be sharable entry for all connection
typedef struct t_BLEPTUM_APP_STATE
{
    // maximum number of slave connections allowed
    UINT8               bleptu_max_con;

    // active connections at run time
    UINT8               bleptu_num_con;

    // configuration option bitmask
    UINT8               bleptu_config;

    // application timer count (seconds)
    UINT32              bleptu_apptimer_count;

    // fine timer count (ticks)
    UINT32              bleptu_appfinetimer_count;

    // control characteristic value used to write to PRU
    UINT8               bleptu_pru_control[5];

    // PTU static parameter used to write to PRU
    UINT8               bleptu_ptu_static[17];

    // alert client characteristic configuration value
    UINT16              bleptu_pru_alert_ccc;
} BLEPTUM_APP_STATE;

// per connection specific information; this will be non-sharable entry for each connection
typedef struct t_BLEPTUM_INFO
{
    // LE connection handle
    UINT16              bleptu_conhandle;

    // BD address of slave/remote device
    BD_ADDR             bleptu_remote_addr;

    // address type of slave/remote device
    UINT8               bleptu_remote_addr_type;

    // application connection state w.r.t PRU.
    UINT8               bleptu_state;

    // stores the device connection information for the device
    EMCONINFO_DEVINFO   bleptu_dev_info;

    // SMP information details
    LESMP_INFO          bleptu_smp_info;

    // PRU control characteristic handle (for write)
    UINT16              bleptu_pru_control_hdl;

    // PTU static parameter characteristic handle (for write)
    UINT16              bleptu_ptu_static_hdl;

    // PRU alert characteristic handle (notification/indications)
    UINT16              bleptu_pru_alert_hdl;

    // alert client configuration descriptor handle to enable notifications
    UINT16              bleptu_pru_alert_ccc_hdl;

    // PRU static parameter handle (to read)
    UINT16              bleptu_pru_static_hdl;

    // PRU dynamic parameter handle (to read)
    UINT16              bleptu_pru_dynamic_hdl;

    // flag to enable the transation timer
    UINT8               bleptu_timeout_enable;

    // transation timeout value w.r.t application timer count.
    UINT32              bleptu_timeout;
} BLEPTUM_INFO;

#endif //_BLEPTUM_H_
