/********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2014 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: hw_timer.h
*
*    Abstract:  Prvide an accurate 1-shot timer (HW timer2) to app.
*
*    $History: 
*           Created   02/07/14
*
*********************************************************************/

//===================================================================
//      Include 
//===================================================================
#include <types.h>

/****************************************************************************
 * Use an accurate HW timer - timer2 as a 1-shot timer
 NOTE!!! NOT suggested for application use. It does not have any critical section protection.
               Use it only if your application must need an accurate timer (i.e. no way to work around).
               You need to code the callback func carefully to avoid any race condition/deadlock.
 ****************************************************************************/
typedef void (*HW_TIMER_EXPIRED_CALLBACK_FN)(void);


//===================================================================
// Functions : void hw_timer_register_timer_expired_callback
// 
// The function SHOULD be called in the Application to register the timeout callback function. 
// param: callback function
// param: userdata (can be class instance)
//===================================================================
void hw_timer_register_timer_expired_callback(HW_TIMER_EXPIRED_CALLBACK_FN callback);
//===================================================================
// Functions : hw_timer_start
// 
// The function SHOULD be called in the Application to start the 1-shot timer. Make sure the app already 
// registered the callback function.
// param: timer period in micro seconds.
//===================================================================
void hw_timer_start(UINT32 microseconds);
//===================================================================
// Functions : hw_timer_stop
// 
// The function should be called in the Application to abort the 1-shot timer, in case the app does not want 
// to wait for the timer to timeout
//===================================================================
void hw_timer_stop(void);

void hw_timer_int_handler(void);  