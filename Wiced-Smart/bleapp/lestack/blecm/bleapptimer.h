#ifndef _BLEAPPTIMER_H_
#define _BLEAPPTIMER_H_
/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP 
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: bleapp_timer.h
*
* Abstract: This is the header file for a bleapp timer. The source of the time
*           is cominig from cfa timer.
*
* Functions:
*
*******************************************************************************/


#include "cfa.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
typedef void (*BLEAPP_TIMER_CB)(UINT32 arg);
typedef PACKED struct
{
    BLEAPP_TIMER_CB cb;    // this is a call back 
    INT16           curTicks;      // 1 second source tick will give a max of 4 hrs.
    INT16           originalTicks; // 1 second source tick will give a max of 4 hrs.
    UINT32          arg;           // client argument to pass back.
} BLEAPPTIMER_BLK;



#define BLEAPPTIMER_INVALID_TIMER_ID                             (-1)



#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////

void bleapptimer_init(void);

// negative number means out of timer.
int bleapptimer_allocateTimer(void);

// this will start Basic stuffs.
void bleapptimer_startTimer(INT32 id, BLEAPP_TIMER_CB cb,INT16 timeOut, UINT16 arg);

// This function will return the Id of the timer. Negative number means out
// of resource.
int bleapptimer_startAppTimer(BLEAPP_TIMER_CB cb,INT16 timeOut, UINT16 arg);

void bleapptimer_refreshAppTimer( INT32 id );

// This interface stops the upper level timers.
void bleapptimer_stopAppTimer( INT32 id );

// This function will start a fine resolution timer. It should be used with care.
// The timeout is in ms unit.
void bleapptimer_startFineTimer( BLEAPP_TIMER_CB cb, INT16 timeOut);
void bleapptimer_stopFineTimer( void );

// This timer is for alert
void bleapptimer_startLEDTimer( BLEAPP_TIMER_CB cb, INT16 timeOut );
void bleapptimer_stopLEDTimer( void );
void bleapptimer_startBUZTimer( BLEAPP_TIMER_CB cb, INT16 timeOut );
void bleapptimer_stopBUZTimer( void );

#ifdef __cplusplus
}
#endif



#endif // end of #ifndef _BLEAPPTIMER_H_
