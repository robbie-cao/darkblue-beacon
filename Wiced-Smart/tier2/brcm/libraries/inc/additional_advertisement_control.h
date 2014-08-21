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
*    File Name: additional_advertisement_control.h
*
*    Abstract: Provides additional advertisement control to the application.
*
*   1.   The app can choose to skip an RF activity (ADV/connection event)
*        when a configured GPIO is asserted. Useful when a very basic
*        coex is required for a beacon-like application. We don't want
*        to use this when connected. If RF activity has already started
*        before the GPIO is asserted externally, then RF activity will
*        continue normally.
*
*   2.   Notifications before and after an advertisement event.
*
********************************************************************
*/
#ifndef _ADDITIONAL_ADVERTISEMENT_CONTROL_H_
#define _ADDITIONAL_ADVERTISEMENT_CONTROL_H_

#include "types.h"

enum
{
    /// Ready to send out an adv in the next few mS. App can change ADV data if required.
    /// Typically invoked about 2.5mS before te ADV. If there are other higher prriority
    /// tasks or other events in the app thread event queue, this will be delayed.
    /// Notification is best effort.
    ADV_NOTIFICATION_READY,

    /// Just completed transmitting an ADV packet.
    ADV_NOTIFICATION_DONE
};

/// Allows the app to configure a GPIO level as an input from another system (uC/uP)
/// to indicate to the 2073x to pause RF activity for the duration the GPIO is active.
/// This can be used as a rudimentary coex mechanism. Note that this may adversely affect
/// connection stability when used incorrectly. Useful for beacon-like applications
/// colocated with WLAN.
/// \param gpio The GPIO P# that is to be used.
/// \param active_level 1 for active high, 0 for active low.
void bleprofile_configureGpioForSkippingRf(UINT8 gpio, UINT8 active_level);

/// Allows the app to register a callback that will be invoked just before an ADV is packet is about
/// to be sent out and again, immediately after.
/// \param clientCallback Pointer to a function that will be invoked in the application
///        thread context with ADV_NOTIFICATION_READY for before ADV and
///        ADV_NOTIFICATION_DONE after ADV packet is complete.
/// \param advanceNoticeInMicroseconds Number of microseconds before the ADV the notification is
///        to be sent. Will be rounded down to the nearest 1.25mS. Has to be an even multiple of 625uS.
void bleprofile_notifyAdvPacketTransmissions(void (*clientCallback)(UINT8), UINT32 advanceNoticeInMicroseconds);

#endif   // _ADDITIONAL_ADVERTISEMENT_CONTROL_H_
