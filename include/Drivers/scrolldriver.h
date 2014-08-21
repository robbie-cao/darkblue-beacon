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
* Define functions to access Scroll driver that uses the Quadrature hardware
*/
#ifndef __SCROLL_DRIVER_H__
#define __SCROLL_DRIVER_H__

#include "types.h"


/** \addtogroup  Scrolldriver
* \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines a scroll driver that uses the quadrature HW.
*/


/// Turn off scroll HW. This is used when entering
/// power-off mode (software or low battery).
void scroll_turnOff(void);

/// Turn on scroll HW. This is used when exiting
/// power-off mode (software or low battery).
void scroll_turnOn(void);

/// Register a thread context interrupt handler.
/// \param userfn Pointer to the callback function to be invoked.
/// \param userdata Pointer to a context that is passed into callback when the interrupt occures.
void scroll_registerForInterrupt(void (*userfn)(void*), void* userdata);

/// Gets the current count of the scroll wheel.
/// \return scroll count.
INT16 scroll_getCount(void);
    
// Initialize the Scroll driver based on the configuration.
// Will also initialize the HW if the last reset was because of a
// power up condition
void scroll_init(void);

/* @}  */
#endif


