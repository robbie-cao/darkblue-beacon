/*******************************************************************************
*  THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
* *******************************************************************************
*
*        Copyright (c) 2011 Broadcom Corp. ALL RIGHTS RESERVED
*
* \file   bleappevent.h
* \brief  
*
*  This defines the event interface for the application.
*  An application can serialize callback functions to the app thread
*  from any other thread.
*
*  \author Arvind Sridharan
*  \date   2011-05-05
*******************************************************************************/
#ifndef _BLE_APP_EVENT_H_
#define _BLE_APP_EVENT_H_

#include "cfa.h"
#include "assertpanic.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/// Request the serialization framework to free the context buffer using cfa_mm_Free()
/// because it was allocated earlier using cfa_mm_Alloc()
#define BLE_APP_EVENT_FREE_BUFFER 42

/// No action needs to be taken by the framework to free the context buffer.
#define BLE_APP_EVENT_NO_ACTION   43

/// The serialization event flag. Internal.
#define BLE_APP_EVENT_CSA_SERIALIZE    (0x1ul << 31)

/// Serialization module initialization function. Invoked by the firmware before the
/// application is initialized. No need to be invoked separately.
void bleappevt_initSerialization(void);

/// Serialize a function call in the application thread context.
/// \param fn Pointer to a function that will be invoked in the application
///           thread context.
/// \param data Pointer to a context meaningful to the callback. This will be passed
///           back to the callback when fn is invoked.
BOOL32 bleappevt_serialize(INT32 (*fn)(void*), void* data);

/// Serialization event handler. Internal to the serialization framewrok.
void bleappevt_serializationHandler(void);




#ifdef __cplusplus
}
#endif
#endif
