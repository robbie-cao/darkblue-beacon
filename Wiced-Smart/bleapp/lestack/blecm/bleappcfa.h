#ifndef _BLEAPPCFA_H_
#define _BLEAPPCFA_H_
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
* File Name: bleappcfa.h
*
* Abstract: This file implements the glue with CFA, Core Firmware API layer.
*
* Functions:
*
*******************************************************************************/


#include "cfa.h" // Get the Core Firmware API interfaces.

#ifdef __cplusplus
extern "C" {
#endif


//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void bleappcfa_install(void);

void bleappcfa_startTimer(
            CFA_TIMER*, void *cb,UINT32 intervalTicks, INT32 arg);

void bleappcfa_stopTimer(CFA_TIMER *timer);



#ifdef __cplusplus
}
#endif


#endif // end of #ifndef _BLEAPPCFA_H_
