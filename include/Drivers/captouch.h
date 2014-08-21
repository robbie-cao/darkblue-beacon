/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2013 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: captouch.h
*
*    This file defines a captouch driver
*
********************************************************************
*/

#ifndef __CAP_TOUCH_H__
#define __CAP_TOUCH_H__

/** \addtogroup HardwareDrivers*/
/*! @{ */
/**
* Defines a simple capacitive tough HW driver. The capacitive touch
* interface can be realized by connecting a pull-up resistor (recommend 100K)
* with a small shunt capacitor (recommend 2.4pF) connected to an ADC input
* so it forms an RC oscillator. The HW and the driver only support a polled
* mechanism to read a value proportional to the RC oscillation count (which
* depends on the effective capacitance, given that the R is constant).
*/

/// Internal bit definitions of the capacitive touch HW block control register.
enum
{
    HW_MIA_TOUCH_CTL_ENABLE_MASK                = 0x20000,
    HW_MIA_TOUCH_CTL_ENABLE                     = 0x20000,
    HW_MIA_TOUCH_CTL_DISABLE                    = 0x00000,
    HW_MIA_TOUCH_CTL_RESET_MASK                 = 0x10000,
    HW_MIA_TOUCH_CTL_RESET_ACTIVE               = 0x10000,
    HW_MIA_TOUCH_CTL_RESET_INACTIVE             = 0x00000,
    HW_MIA_TOUCH_CTL_MEASUREMENT_PERIOD_MASK    = 0x0FFFF,

    HW_MIA_TOUCH_INPUT_COUNT_MASK               = 0xFFFF,
};

/// Internal bit definitions of the capacitive touch HW block configuration register.
enum
{
    HW_LHL_CH_SEL_CFG_TOUCH_IO_ENABLE_MASK      = 0x8000,
    HW_LHL_CH_SEL_CFG_TOUCH_IO_ENABLE           = 0x8000,
    HW_LHL_CH_SEL_CFG_TOUCH_IO_DISABLE          = 0x0000,

    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_MASK          = 0x7C00,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_SHIFT         = 10,

    HW_LHL_ADC_CTL_TOUCH_INPUT_MASK             = 0x1F,
    HW_LHL_ADC_CTL_TOUCH_INPUT_SHIFT            = 0,

    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P38           = 1,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P37           = 2,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P36           = 3,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P35           = 4,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P34           = 5,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P33           = 6,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P32           = 7,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P31           = 8,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P30           = 9,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P29           = 10,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P28           = 11,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P23           = 12,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P22           = 13,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P21           = 14,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P19           = 16,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P18           = 17,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P17           = 18,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P16           = 19,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P15           = 20,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P14           = 21,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P13           = 22,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P12           = 23,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P11           = 24,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P10           = 25,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P9            = 26,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P8            = 27,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P1            = 28,
    HW_LHL_CH_SEL_CFG_TOUCH_INPUT_P0            = 29,
};

/// Internal R channel mux settings.
enum
{
    HW_MIA_ADC_R_CH_SEL_P0        = (1 << 3),
    HW_MIA_ADC_R_CH_SEL_P10       = (1 << 2),
    HW_MIA_ADC_R_CH_SEL_P33       = (1 << 1),
    HW_MIA_ADC_R_CH_SEL_MASK      = (0x0E)
};

/// Gets the current state of gpioPortPin.
/// \param gpioPortPin The port + pin to sense.
/// \param period The sampling period in number of 24MHz clock cycles.
/// \return The return value is the number of oscillations the capsense input oscillated in period.
UINT32 captouch_getCurrentState(UINT8 gpioPortPin, UINT32 period);


/* @}  */
#endif
