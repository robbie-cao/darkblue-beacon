/*******************************************************************************
*
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2010 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: bt_clock_based_timer.h
*
* Abstract:
* Bluetooth clock based periodic timer that can enqueue the callback in the application
* thread context. This is a more accurate timer than the software timers and is unaffected by sleep
* and wake cycles. Any jitter, if present is generally due to higher priority Bluetooth
* tasks/interrupts. The resolution of the timer is 1.25mS (one Bluetooth frame) and it is
* recommended that the period of the timer be kept at 5mS or more.  Shorter timeouts will adversely
* affect Bluetooth performance.
*
*******************************************************************************/
#include "types.h"

/// Initialize the BT clock based HE timer module. Has to be done once at initialization
/// before using the modile. Initializing a second time will invoke undefined behavoior.
void bt_clock_based_periodic_timer_Init(void);

/// Enable/start the periodic timer.
/// \param clientCallback The application callback function that is serialized to the
///        application thread. The function is required to return BLE_APP_EVENT_NO_ACTION if
///        the clientContext was not allocated using cfa_mm_Alloc() or some memory that
///        should not be freed. If the system should free clientContext using cfa_mm_Free,
///        return BLE_APP_EVENT_FREE_BUFFER.
/// \param clientContext Any context that is to be passed back to clientCallback when invoked.
/// \param defaultPeriod The default period in Bluetooth slots. 1 BT slot = 626uS and this has to
///        be an even number of BT slots. So, for 100ms, use 100000/625; for 12.5ms, use 12500/625.
void bt_clock_based_periodic_timer_Enable(int (*clientCallback)(void*), void* clientContext, UINT16 defaultPeriod);

/// Disable the periodic timer. Not necessary to disable before restarting.
void bt_clock_based_periodic_timer_Disable(void);
