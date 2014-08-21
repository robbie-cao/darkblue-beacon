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
* Define functions to high-level driver functions
*/
#ifndef __MIA_DRIVER_H__
#define __MIA_DRIVER_H__

#include "types.h"
//#include "keyscan.h"
#include "quadraturedriver.h"


/**  \addtogroup mia
 *  \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines a MIA driver. The BCM applications or other applications use this
* driver to obtain the status from and control the behavior of the MIA driver.
*/
/// Mia constants are defined below
enum
{
    // The following will need to be redefined depending on how HW muxes pin functions

    ///Mask for P2_6(GPIO_22). 00 in the mask will be normal P2_6 or GPIO_22.
    MIA_P2_OPSR_SEL_PORT_2_6_BIT_MASK    = 0xC0,
    /// This value in the above mask would set KSO14 output.
    MIA_P2_OPSR_SEL_PORT_2_6_IS_KSO14    = 0x40,
    /// Mask for P2_7(GPIO_23).  00 value in the mask would set normal GPIO_23.
    MIA_OPSR_SEL0_PORT_2_7_BIT_MASK      = 0x03,
    /// This value in the above mask would set KSO15 output.
    MIA_OPSR_SEL0_PORT_2_7_IS_KSO15      = 0x01,
    /// Mask for P3_0(GPIO_24).  00 value in the mask would set normal GPIO_24.
    MIA_OPSR_SEL0_PORT_3_0_BIT_MASK      = 0x0C,
    /// This value in the above mask would set KSO16 output.
    MIA_OPSR_SEL0_PORT_3_0_IS_KSO16      = 0x04,
    /// Mask for P3_1.  0 value in the mask would set normal GPIO_25.
    MIA_OPSR_SEL0_PORT_3_1_BIT_MASK      = 0x10,
    /// This value in the above mask would set KSO17 output.
    MIA_OPSR_SEL0_PORT_3_1_IS_KSO17      = 0x10,
    /// Mask for P3_2.  0 value in the mask would set normal GPIO_26.
    MIA_OPSR_SEL0_PORT_3_2_BIT_MASK      = 0x60,
    /// This value in the above mask would set KSO18 output.
    MIA_OPSR_SEL0_PORT_3_2_IS_KSO18      = 0x20,
    /// Mask for P3_3.  0 value in the mask would set normal GPIO_27.
    MIA_OPSR_SEL1_PORT_3_3_BIT_MASK      = 0x03,
    /// This value in the above mask would set KSO19 output.
    MIA_OPSR_SEL1_PORT_3_3_IS_KSO19      = 0x01,
};

/* @} */

/**  \addtogroup mia
 *  \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines the BCM miadriver class. Mia driver class encapsulates the mia clock freeze
* and other functionalities for other mia drivers.
*/

/// MIA status and control bits.
enum
{
    HW_MIA_CTRL_CLK_FREEZE_MASK     = 0x0001,
    HW_MIA_CTRL_CLK_FREEZE_REQ      = 0x0001,
    HW_MIA_CTRL_CLK_UNFREEZE_REQ    = 0x0000,

    HW_MIA_STATUS_CLK_FROZEN_MASK   = 0x0001,
    HW_MIA_STATUS_CLK_FROZEN        = 0x0001,
    HW_MIA_STATUS_CLK_RUNNING       = 0x0000,
    HW_MIA_STATUS_KEYCODE_SET_MASK  = 0x0002,
    HW_MIA_STATUS_KEYCODE_SET       = 0x0002,
    //HW_MIA_STATUS_QUAD_SET_MASK     = 0x0010,
    HW_MIA_STATUS_QUAD_SET          = 0x0010,
    HW_MIA_STATUS_EVT_SET_MASK      = 0x0012,
    HW_MIA_STATUS_EVT_SET           = 0x0012,


    HW_LHL_CTL_HID_OFF_MASK         = 0x0001,
    HW_LHL_CTL_KS_EN_MASK           = 0x0002,
    HW_LHL_CTL_QD_EN_MASK           = 0x0004,
    HW_LHL_CTL_KSI_USED_MASK        = 0xE000,
    HW_LHL_CTL_HID_OFF              = 0x0001,
    HW_LHL_CTL_KS_EN                = 0x0002,
    HW_LHL_CTL_QD_EN                = 0x0004,
    HW_LHL_CTL_KSI_USED_SHIFT_COUNT = 13,

    HW_LHL_STATUS_HID_OFF_WAKEUP_MASK    = 0x0001,
    HW_LHL_STATUS_GPIO_ASYNC_WAKEUP_MASK = 0x0002,
    /// 1 if timer expired, 0 otherwise
    HW_LHL_STATUS_HID_OFF_WAKEUP    = 0x0001,
    /// 1 if GPIO interrupt occurred, 0 otherwise
    HW_LHL_STATUS_GPIO_ASYNC_WAKEUP = 0x0002,

    HW_LDO_CTL_EN_HV_POK_MASK       = 0x8000,
    HW_LDO_CTL_EN_LHL_POK_MASK      = 0x4000,
    HW_LDO_CTL_EN_MIA_FB_DIS_MASK   = 0x0200,
    HW_LDO_CTL_EN_MIA_PASSTH_MASK   = 0x0100,
    HW_LDO_CTL_EN_FW_REG_OFF_MASK   = 0x0002,
    HW_LDO_CTL_EN_CTL_EN_MASK       = 0x0001,
    HW_LDO_CTL_EN_HV_POK            = 0x8000,
    HW_LDO_CTL_EN_LHL_POK           = 0x4000,
    HW_LDO_CTL_EN_MIA_FB_DIS        = 0x0200,
    HW_LDO_CTL_EN_MIA_PASSTH        = 0x0100,
    HW_LDO_CTL_EN_FW_REG_OFF        = 0x0002,
    HW_LDO_CTL_EN_BG_PD_EN          = 0x0002,
    HW_LDO_CTL_EN_CTL_EN            = 0x0001,

    HW_HID_OFF_CNT1_EN_MASK         = 0x8000,
    HW_HID_OFF_CNT1_EN              = 0x8000,

    HW_LHL_ADC_CTL_RTC_TERM_CNT_STATUS_EN = 0x1000,
    HW_LHL_ADC_CTL_RTC_CNT_RESET    = 0x2000,
    HW_LHL_ADC_CTL_RTC_TIMER_FN_EN  = 0x4000,
    HW_LHL_ADC_CTL_RTC_CNT_EN       = 0x8000,

    HW_LHL_LDO_CTL_EN_LHL_RTC_CLOCK_SEL_MASK = 0x0040,
    HW_LHL_LDO_CTL_EN_LHL_MIA_CLOCK_SEL_MASK = 0x0080,
    HW_LHL_LDO_CTL_EN_LHL_RTC_CLOCK_SEL_32KHZ = 0x0040,
    HW_LHL_LDO_CTL_EN_LHL_MIA_CLOCK_SEL_32KHZ = 0x0080,
    HW_LHL_LDO_CTL_EN_LHL_RTC_CLOCK_SEL_128KHZ = 0x0000,
    HW_LHL_LDO_CTL_EN_LHL_MIA_CLOCK_SEL_128KHZ = 0x0000,

    HW_HIDOFF_CNT0_MASK             = 0xFC00, // bits 10:15 are valid
    HW_HIDOFF_CNT0_REG_SHIFT        = 0x000A, // shift in the value to this position in register
    HW_HIDOFF_CNT0_RTC_SHIFT        = 0x000D, // shift out 13 bits from rtc value

    HW_HIDOFF_CNT1_MASK             = 0xFFFF,
    HW_HIDOFF_CNT1_RTC_SHIFT        = 0x0013  // shift out 19 bits

};

/// MIA LPO control bits.
enum
{
        HW_LPO_POWER_DOWN_EN            = 0x0080,
        HW_LPO_POWER_DOWN_SEL_MASK      = 0x0300,
        HW_LPO_POWER_DOWN_SEL_AUTO      = 0x0100,
        HW_LPO_POWER_DOWN_SEL_FORCE     = 0x0200
};

/// MIA always-on bits.
enum
{
    KEEPSTATE0_HID_OFF_MASK         = 0x0001,
    KEEPSTATE0_SOFT_POWER_OFF_MASK  = 0x0002,
    KEEPSTATE0_FUNC_LOCK_STATE_MASK = 0x0004,
    KEEPSTATE0_HID_OFF              = 0x0001,
    KEEPSTATE0_SOFT_POWER_OFF       = 0x0002,
    KEEPSTATE0_FUNC_LOCK_STATE_ON   = 0x0004,
    KEEPSTATE0_FUNC_LOCK_STATE_OFF  = 0x0000
};

/// MIA timed wake none.
enum
{
    HID_OFF_WAKEUP_TIME_NONE = 0
};

/// MIA LPO selection.
enum
{
    MIA_LPO_NONE = 0,
    MIA_LPO_LHL_ON = 1,
    MIA_LPO_32K_ON = 2
};

/// Reference clock for timed wake from deep sleep.
typedef enum 
{
    /// Use the 32 KHz clock for the RTC to give a range of ~64 ms - ~36 Hrs
    HID_OFF_TIMED_WAKE_CLK_SRC_32KHZ,

    /// Use the 128 KHz clock for the RTC to give a range of ~256 ms - ~144 Hrs
    HID_OFF_TIMED_WAKE_CLK_SRC_128KHZ
} MiaTimedWakeRefClock;

#pragma pack(1)
/// Internal state of the MIA driver.
typedef PACKED struct
{
    /// Pointer to the timed wake setup function
    void (*mia_virtualSetupTimedWake)(UINT32 wakeupTime, MiaTimedWakeRefClock ref);

    /// Whether mia HW is initialized or not
    UINT8 inited            : 1;

    /// Whether KS driver is registered with this instance
    UINT8 keyscanRegistered : 1;

    /// Whether Quad driver is registered with this instance.
    UINT8 quadRegistered    : 1;

    /// Whether interrupt is enabled/disabled
    UINT8 lhlInterruptState : 1;

    /// Whether interrupt is enabled/disabled
    UINT8 miaInterruptState : 1;

    /// Whether lpo 32k was running
    UINT8 lpo32kWasRunning : 1;

    /// Whether reason for reset was power on
    UINT8 resetReasonPor : 1;

} MiaState;
#pragma pack()

/// Initializes the MIA HW block.
void mia_init(void);

/// Notify other drivers that the MIA clock was frozen. Internal.
void mia_clkFreezeNotifyDriver(void);

/// Notify other drivers that the MIA clock was unfrozen. Internal.
void mia_clkUnfreezeNotifyDriver(void);

/// Interrupt handler (ISR context) for LHL power domain. Internal.
void lhl_interruptHandler(void);

/// Keyscan interrupt handler (ISR context). Internal.
void ks_interruptHandler(void); 

/// Quadrature sensor interrupt handler (ISR context). Internal.
void quad_interruptHandler(void);

/// Unfreeze a frozen MIA clock. Internal.
void mia_unfreezeClk(void);

/// Freeze the MIA clock. Internal.
void mia_freezeClk(void);

/// Poll all HW blocks on the MIA domain if one or more events are detected.
/// Keyscan and Quadrature HW blocks are part of the MIA power domain. Events
/// from either or both are processed by freezing the MIA clock, pulling events
/// out of the HW block and then unfreezing the MIA clock.
void mia_pollHardware(void);

/// Retrieves pending events from HW and also flushes KS events.
void mia_flushAllEvents(void);

/// Register the keyscan driver for event notification. Internal.
void mia_notificationRegisterKeyscan(void);

/// Register the quadrature driver for event notification. Internal.
void mia_notificationRegisterQuad(void);

/// Dispatches a GPIO interrupt to the GPIO driver in thread context. Internal.
int mia_handleLhlInterrupt(void *unused);

/// Dispatches a keyscan interrupt to the keyscan driver in thread context. Internal.
int mia_handleKsInterrupt(void *unused);

/// Dispatches a quadrature interrupt to the quadrature driver in thread context. Internal.
int mia_handleQuadInterrupt(void *unused);

/// Get the 16 bits stored in the always on register 0.
UINT16 mia_getKeepstate0(void); 

/// Get the 16 bits stored in the always on register 1.
UINT16 mia_getKeepstate1(void);

/// Set the 16 bits stored in the always on register 0.
void mia_setKeepstate0(UINT16 newVal);

/// Set the 16 bits stored in the always on register 0.
void mia_setKeepstate1(UINT16 newVal);

/// OR in bits set in the parameter to always on register 0.
void mia_addKeepstate0(UINT16 newVal);

/// OR in bits set in the parameter to always on register 1.
void mia_addKeepstate1(UINT16 newVal);

/// Remove bits set in the parameter to always on register 0.
void mia_delKeepstate0(UINT16 newVal);

/// Remove bits set in the parameter to always on register 1.
void mia_delKeepstate1(UINT16 newVal);

/// Check if the reason for booting is PoR or wake from deep sleep.
/// \return TRUE if waking from PoR; else waking from deep-sleep.
BOOL32 mia_isResetReasonPor(void);

/// Enables keyscan before entring deep-sleep. Internal.
void mia_enableKeyscanPriorToHidOff(void);

/// Turn off internal LPOs if possible. Internal.
void mia_turnOffLhlLpoIfPossible(void);

/// Set up timed wake from deep slee.
/// \param wakeupTimeInMs The number of mS to wake in after entering deep sleep.
/// \param ref The reference clock to use.
void mia_setupTimedWake(UINT32 wakeupTimeInMs, MiaTimedWakeRefClock ref);

/// Enter deep sleep.
/// \param wakeupTimeInMs The number of mS to wake in after entering deep sleep.
/// \param ref The reference clock to use.
void mia_enterHidOff(UINT32 wakeupTime, MiaTimedWakeRefClock ref);

/// Enable or disable keyscan and quadrature interrupts.
/// \param enable When TRUE, interrupts are enabled; else disabled.
void mia_enableMiaInterrupt(BOOL32 enable);

/// Enable or disable GPIO interrupts.
/// \param enable When TRUE, interrupts are enabled; else disabled.
void mia_enableLhlInterrupt(BOOL32 enable);

/// Get the MIA interrupt state.  This state may not be the
/// actual hardware MIA interrupt setting, since the interrupt
/// context interrupt handler will disable the MIA interrupt
/// without updating the state variable, which is a good thing
/// since it makes it easy to restore the correct hardware
/// setting from this state variable when done handling the
/// serialized interrupt by calling mia_restoreMiaInterruptState().
/// \return
///     - true if MIA interrupts should be enabled.
///     - false if MIA interrupts should be disabled
BOOL32 mia_getMiaInterruptState(void);

/// Get the LHL interrupt state.  This state may not be the
/// actual hardware LHL interrupt setting, since the interrupt
/// context interrupt handler will disable the LHL interrupt
/// without updating the state variable, which is a good thing
/// since it makes it easy to restore the correct hardware
/// setting from this state variable when done handling the
/// serialized interrupt by calling restoreLhlInterruptSetting().
/// \return
///     - true if lhl interrupts should be enabled.
///     - false if lhl interrupts should be disabled
BOOL32 mia_getLhlInterruptState(void);

/// Restore the possibly overridden MIA interrupt setting to
/// that of the MIA interrupt state.  See getMiaInterruptState()
/// for more details.
void mia_restoreMiaInterruptState(void);

/// Restore the possibly overridden LHL interrupt setting to
/// that of the LHL interrupt state.  See getLhlInterruptState()
/// for more details.
void mia_restoreLhlInterruptState(void);

/// Turns off all unused LPOs. Internal.
UINT8 mia_turnoffUnusedLpos(void);

///Turns off external 32K and internal 128K before deep-sleep. Internal.
UINT8 mia_turnOffLhlAnd32KLposForHidoff(void);

/* @} */

#endif
