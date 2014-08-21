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
* Defines functions to access auxiliary clock peripheral, which can be used to output a
* free running clock signal to an GPIO or as a reference clock to the on-chip PWM.
*/
#ifndef __ACLK_H__
#define __ACLK_H__

/*****************************************************************************/
/** @defgroup HardwareDrivers       Peripheral Drivers
 *
 *  WICED Smart Peripheral Drivers functions
 */
/*****************************************************************************/

/** \addtogroup  ACLK
* \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines an Aclk driver.
*/
enum
{
    HW_MIA_ACLK_CTL_ACLK0_SHIFT                     = 0,
    HW_MIA_ACLK_CTL_ACLK0_DIV_MASK                  = 0x000000FF,
    HW_MIA_ACLK_CTL_ACLK0_DIV_SHIFT                 = 0,
    HW_MIA_ACLK_CTL_ACLK0_POST_DIV_MASK             = 0x00000700,
    HW_MIA_ACLK_CTL_ACLK0_POST_DIV_SHIFT            = 8,
    HW_MIA_ACLK_CTL_ACLK0_ENABLE_MASK               = 0x00001000,
    HW_MIA_ACLK_CTL_ACLK0_ENABLE                    = 0x00001000,
    HW_MIA_ACLK_CTL_ACLK0_DISABLE                   = 0x00000000,
    HW_MIA_ACLK_CTL_ACLK0_CLK_SRC_SEL_MASK          = 0x00008000,
    HW_MIA_ACLK_CTL_ACLK0_CLK_SRC_24_MHZ            = 0x00008000,
    HW_MIA_ACLK_CTL_ACLK0_CLK_SRC_1_MHZ             = 0x00000000,
    HW_MIA_ACLK_CTL_ACLK0_MASK_ALL  = (HW_MIA_ACLK_CTL_ACLK0_DIV_MASK
                                       | HW_MIA_ACLK_CTL_ACLK0_POST_DIV_MASK
                                       | HW_MIA_ACLK_CTL_ACLK0_ENABLE_MASK
                                       | HW_MIA_ACLK_CTL_ACLK0_CLK_SRC_SEL_MASK),
    
    HW_MIA_ACLK_CTL_ACLK1_SHIFT                     = 16,
    HW_MIA_ACLK_CTL_ACLK1_DIV_MASK                  = 0x00FF0000,
    HW_MIA_ACLK_CTL_ACLK1_DIV_SHIFT                 = 16,
    HW_MIA_ACLK_CTL_ACLK1_POST_DIV_MASK             = 0x07000000,
    HW_MIA_ACLK_CTL_ACLK1_POST_DIV_SHIFT            = 24,
    HW_MIA_ACLK_CTL_ACLK1_ENABLE_MASK               = 0x10000000,
    HW_MIA_ACLK_CTL_ACLK1_ENABLE                    = 0x10000000,
    HW_MIA_ACLK_CTL_ACLK1_DISABLE                   = 0x00000000,
    HW_MIA_ACLK_CTL_ACLK1_CLK_SRC_SEL_MASK          = (int)0x80000000,
    HW_MIA_ACLK_CTL_ACLK1_CLK_SRC_24_MHZ            = (int)0x80000000,
    HW_MIA_ACLK_CTL_ACLK1_CLK_SRC_1_MHZ             = 0x00000000,
    HW_MIA_ACLK_CTL_ACLK1_MASK_ALL = (HW_MIA_ACLK_CTL_ACLK1_DIV_MASK
                                      | HW_MIA_ACLK_CTL_ACLK1_POST_DIV_MASK
                                      | HW_MIA_ACLK_CTL_ACLK1_ENABLE_MASK
                                      | HW_MIA_ACLK_CTL_ACLK1_CLK_SRC_SEL_MASK),
};

/// ACLK channel selection.
enum CLK_SRC_SEL
{
	/// Channel 0.
    ACLK0,

    // Channel 1.
    ACLK1,
};

/// Internal reference clock for the ACLK generator circuit.
enum CLK_SRC_FREQ_SEL
{
	/// Internal 1 MHz reference.
    ACLK_FREQ_1_MHZ,

    // Internal 24 MHz reference.
    ACLK_FREQ_24_MHZ
};

/// Configures an ACLK reference channel.
/// \param frequency The desired frequency.
/// \param src The ACLK channel we are configuring.
/// \freqSel The internal reference fequency to use to generated the desired frequency.
void aclk_configure(UINT32 frequency, UINT32 src, UINT32 freqSel);

/// Disable an ACLK reference channel.
/// \param src THe ACLK channel we are disabling.
void aclk_disableClock(UINT32 src);

/* @}  */
#endif
