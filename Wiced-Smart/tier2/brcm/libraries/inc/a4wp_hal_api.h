/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2007 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: a4wp_hal_api.h
*
*    Abstract: The a4wp pru application can choose to configure the 
*        BRCM PMU(59350) chip and to read the various PRU dynamic parameters
*        from PMU.
*
********************************************************************
*/
#ifndef _A4WP_HAL_API_H_
#define _A4WP_HAL_API_H_

#include "types.h"

// The PMU register types that the application can try to read/write
typedef enum t_a4wp_reg_type
{
    A4WP_REG_VRECT,             // Voltage at diode output
    A4WP_REG_IRECT,             // Current at diode output 
    A4WP_REG_VOUT,              // Voltage at charge/battery port, Vbuck
    A4WP_REG_IOUT,              // Current at charge/battery port Ibuck
    A4WP_REG_TEMP,              // Temperature of PRU,off-chip Tntc
    A4WP_REG_TEMP_ONCHIP,       // Temperature of PRU, on-chip Tjunc
    A4WP_REG_CTRL,              // Control of PRU
    A4WP_REG_IOCON,             // IO Control
    A4WP_REG_DEVID,             // Device ID (read only)
} A4WP_HAL_REG;

typedef UINT8 A4WP_HAL_REG_t;

// Alert types notified from the A4WP HAL layer.
typedef enum a4wp_hal_alert_type
{
    A4WP_HAL_ALERT_TRANSITION1          = 0x01,
    A4WP_HAL_ALERT_TRANSITION2          = 0x02,
    A4WP_HAL_ALERT_TA_DETECT            = 0x04,
    A4WP_HAL_ALERT_CHARGE_CMPLT         = 0x08,
    A4WP_HAL_ALERT_PRU_SELF_PROTECTION  = 0x10,
    A4WP_HAL_ALERT_OTEMP                = 0x20,
    A4WP_HAL_ALERT_OCURR                = 0x40,
    A4WP_HAL_ALERT_OVLTG                = 0x80,

    A4WP_HAL_SPEC_ALERT_MASK            = 0xFF,
    //Local ALERTS
    A4WP_HAL_ALERT_OVD_ON               = 0x00000100, // Detune ON
    A4WP_HAL_ALERT_WPT_REG_OFF          = 0x00000200,
    A4WP_HAL_ALERT_WPT_REG_ON           = 0x00000400,
    A4WP_HAL_ALERT_BT_REG_OFF           = 0x00000800,
    A4WP_HAL_ALERT_SELF_PROT            = 0x00001000,
    A4WP_HAL_ALERT_OTNTC                = 0x00002000, // Off-chip sensor OTP trigger
    A4WP_HAL_ALERT_OTP1                 = 0x00004000, // On-chip sensor 1 OTP trigger
    A4WP_HAL_ALERT_OTP2                 = 0x00008000, // On-chip sensor 2 OTP trigger
    A4WP_HAL_ALERT_OTR1                 = 0x00010000, // On-chip sensor 1 OTP recovery 
    A4WP_HAL_ALERT_OTR2                 = 0x00020000, // On-chip sensor 2 OTP recovery
    A4WP_HAL_ALERT_LOWVLTG              = 0x00040000,
    A4WP_HAL_ALERT_BT_REG_ON            = 0x00080000,
    A4WP_HAL_LOCAL_ALERT_MASK           = 0x7FFFFF00,
} A4WP_HAL_ALERT_ENUM;

typedef UINT32 A4WP_HAL_ALERT_t;

// A4WP HAL layer status values.
typedef enum a4wp_hal_status_type
{
    A4WP_HAL_STATUS_SUCCESS,        // Successfull 
    A4WP_HAL_STATUS_FAILURE,        // Failed to read from HW
    A4WP_HAL_STATUS_INVALID,        // Invalid request
    A4WP_HAL_STATUS_INTERNAL_ERROR, // Internal error leading to failure.
} A4WP_HAL_STATUS;

// Register mask for A4WP_REG_CTRL register write
#define A4WP_CTRL_CHARGING_EN           0x80000000  // To enable write to System Control Register Upper Byte (0x06)
#define A4WP_CTRL_CHARGING_MASK         0x00010000  // Enable/disable mask for charge enable (BUCK)
#define A4WP_CTRL_PWR_EN                0x10000000  // To enable write to I/O Control Register (0x07)
#define A4WP_CTRL_PWR_MASK              0x00F00000  // PWR mask
#define A4WP_CTRL_PWR_GPIO4_MASK        0x00800000  // GPIO4 mask
#define A4WP_CTRL_PWR_GPIO3_MASK        0x00400000  // GPIO3 mask
#define A4WP_CTRL_PWR_DETUNE_MASK       0x00300000  // Detune mask
#define A4WP_CTRL_PWR_SHIFT             20          // Shift value (left) to get to the PWR mask position

// A4WP HAL WPT protocol values.
typedef enum a4wp_hal_wpt_protocol_type
{
    A4WP_HAL_WPT_PROTOCOL_NONE,     // No active protocol
    A4WP_HAL_WPT_PROTOCOL_WPC,      // WPC protocol
    A4WP_HAL_WPT_PROTOCOL_PMA,      // PMA protocol
    A4WP_HAL_WPT_PROTOCOL_A4WP,     // A4WP protocol
} A4WP_HAL_WPT_PROTOCOL_TYPE;

/* Callback function type for the alerts from PMU.
 * The alert values in the callback function will be one of the A4WP_HAL_ALERT_ENUM type.
 */
typedef A4WP_HAL_STATUS (*A4WP_HAL_NOTIFY_t)(A4WP_HAL_ALERT_t alert);

/* The I2C and Serial flash lines are multiplexed in 20732. Serial flash is the default mode when
 * detected with boot up. So when the patch is loaded from Serial flash, it has to be re-configured to use
 * with I2C lines (to communicate with BRCM PMU).
 * The application can use this function to configure the MUX to use for I2C lines.
 */
void a4wp_hal_i2c_mux_configure(void);

/* Allows the app to configure and initialize the PMU interfaces.
 * Params:
 *      alert_cb :        Application can register for the alert notifications from PMU.
 *      gpio_button:      Specify the GPIO button type configured for interrupt.
 *                        (GPIO_BUTTON1, GPIO_BUTTON2 or GPIO_BUTTON3)
 *      enable_i_out_wa:  Enable/disable the PMU workaround for Iout calculation
 */
UINT8 a4wp_hal_init(A4WP_HAL_NOTIFY_t alert_cb, UINT16 gpio_button, BOOL32 enable_i_out_wa);

/* Allows the app to read the specific register value from A4WP_HAL_REG type.
 * Application can use this value to send over to PTU.
 * Params:
 *      reg:              PMU register address type, can be one of the A4WP_HAL_REG type.
 */
UINT32 a4wp_hal_reg_read(A4WP_HAL_REG_t reg);

/* Allows the app to write the PMU register. Only the A4WP_REG_CTRL register can be 
 * updated with application needed values.
 * Params:
 *      reg:              PMU register address type, can be only A4WP_REG_CTRL.
 *      value:            Control write value, to enable the BUCK.
 */
void a4wp_hal_reg_write(A4WP_HAL_REG_t reg, UINT32 value);

/* Allows the app to read from the PMU register. This interface is to directly read any of
 * the PMU(59350) register. When application wants to know some specific register value, 
 * this function can be used.
 * Note: Application needs to know the appropriate PMU registers before using this
 *       function and these details are not provided here.
 * Params:
 *      reg_addr:         PMU register address.
 *      p_data:           Buffer for the values read from PMU.
 *      length:           Number of octets to read from PMU
 */
void a4wp_hal_read_pmu_reg(UINT16 reg_addr, UINT8 *p_data, UINT16 length);

/* Allows the app to write to specific PMU register. This interface is to directly write some
 * specific value to PMU(59350) register.
 * Note: Application needs to know the appropriate PMU registers before using this
 *       function and these details are not provided here.
 * Params:
 *      reg_addr:         PMU register address.
 *      p_data:           Buffer for the values read from PMU.
 *      length:           Number of octets to read from PMU.
 */
void a4wp_hal_write_pmu_reg(UINT16 reg_addr, UINT8 *p_data, UINT16 length);

/* Allows the app to know the PMU mode for the tri-mode support.
 * Note: Application need to handle only A4WP protocol to setup the BLE related configurations.
 * Params: None
 * Returns: A4WP_HAL_WPT_PROTOCOL_TYPE value.
 */
A4WP_HAL_WPT_PROTOCOL_TYPE a4wp_hal_get_wpt_protocol(void);

#endif   // _A4WP_HAL_API_H_

