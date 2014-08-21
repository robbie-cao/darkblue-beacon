/*******************************************************************************
* ------------------------------------------------------------------------------
*
* Copyright (c), 2005, 2007, 2010, 2011 BROADCOM Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: devicelpm.h
*
* Abstract: 
*   This file declares the device level lpm.
*******************************************************************************/

#ifndef __DEV_LPM_H_
#define __DEV_LPM_H_

#include "types.h"
#include "cfa.h"

/*******************************************************************************
* Types and Defines
*******************************************************************************/

/**  \addtogroup application Application */
/*! @{ */
/**
\brief Device level low power management.

This provides a concrete implementation for hid-off and sleep routines for the
application.

The key features of the DeviceLpm  include:

- Hid-off dispatch
- Time to sleep dispatch
- Actual hid-off implementation

*/
/// The type of low power mode that HidLpm is currently contemplating
typedef enum LowPowerModePollType
{
    /// HidLpm is probing the registered method if device can sleep
    LOW_POWER_MODE_POLL_TYPE_SLEEP,

    /// HidLpm is probing the registered method if the device can go hid-off
    LOW_POWER_MODE_POLL_TYPE_POWER_OFF
}LowPowerModePollType;

enum
{
	/// Sleep
    DEV_LPM_DISC_LOW_POWER_MODES_SLEEP,

    /// Deep sleep.
    DEV_LPM_DISC_LOW_POWER_MODES_HID_OFF   
};

/// This typedef is what is expected when registering a function as
/// a callback. This callback function is expected to respond with time to
/// sleep or if it is ok to enter hid off or not. A 0 value indicates
/// do not sleep or do not go hid-off and a non-zero value for sleep is the sleep time
/// or ok to go hid-off
typedef UINT32 (*DeviceLpmQueriableMethodCallback)(LowPowerModePollType type, UINT32 context);

/// The structure to hold a notification registration
typedef struct DeviceLpmQueriableRegistration
{
    /// The notifiable function
    DeviceLpmQueriableMethodCallback pFunc;

    /// The notifiable function's context
    UINT32 context;
    
    /// Next in the list
    struct DeviceLpmQueriableRegistration* next;
} DeviceLpmQueriableRegistration;

enum
{
    /// Wake from GPIO
    DEV_LPM_WAKE_SOURCE_GPIO    = (1 << 0x08),

    /// Wake from LHL is the same as waking from GPIO
    DEV_LPM_WAKE_SOURCE_LHL     = (1 << 0x08),

    /// Wake from keyscan
    DEV_LPM_WAKE_SOURCE_KEYSCAN = (1 << 0x09),

    /// Wake from wuadrature sensor
    DEV_LPM_WAKE_SOURCE_QUAD    = (1 << 0x0A),

    /// All wake HID sources
    DEV_LPM_WAKE_SOURCE_MASK    = (DEV_LPM_WAKE_SOURCE_GPIO | DEV_LPM_WAKE_SOURCE_KEYSCAN | DEV_LPM_WAKE_SOURCE_QUAD)
};

typedef struct
{
    /// The function to invoke in CSA context
    int (*fn)(void*);

    /// The opaque context data to be passed back in CSA context
    void *context;
    
    /// All of this LPMs registrants in the linked list
    DeviceLpmQueriableRegistration* firstReg;

    /// CSA's version of timetosleep if it exists
    CFA_GET_TIME_TO_SLEEP_FP oldCsaGetTimeToSleep;
} DeviceLpmState; 

/// Initialize device Low Power Manager. Internal, invoked before the application is created.
void devlpm_init(void);

/// Add the given object to the list of objects that need to be 
/// queried for time to sleep and low power queries. An application can participate in sleep  and
/// deep sleep decisions if required by registering a callback as in the sample below.
/// NOTE: This callback is always called in a different (lower priority) thread context.
/// So the call may get preempted by the application thread any time, use caution when accessing
/// memory shared with the application thread. Use bleapputils_cpuIntDisable/Enable if appropriate.
/// \verbatim
/// UINT32 application_time_to_sleep_callback(LowPowerModePollType type, UINT32 context)
/// {
/// 	switch(type)
///		{
///			case LOW_POWER_MODE_POLL_TYPE_SLEEP:
///			// System wants to sleep. Return how long the system can sleep in microseconds.
///			// Sleep times under ~3.75mS may be ignored. Guaranteed to sleep for not more
///			// than the time returned by this function. May sleep for less than this if
///			// other subsystems in the FW return a time less than this return value.
///			// To disable sleep, return 0; to leave the decision to some other subsystem
///			// (typically when advertising/connected, the BT sub-system will ensure wake
///			// at the right time), return ~0 (this is the default if the app does not register
///			// for this callback).
///				return ~0;		// Let some other sub-system decide sleep time (default).
///			case LOW_POWER_MODE_POLL_TYPE_POWER_OFF:
///			// System wants to enter deep sleep. Return 0 to disable deep sleep or any non-zero
///			// value to indicate OK to enter deep sleep. Returning 0 will guarantee that deep
///			// sleep is not entered. Device may not enter deep sleep even when the application
///			// allows deep sleep because some other subsystem disabled it.
///				return 1;		// Let some other sub-system decide deep sleep time (default).
///			default:
///				return ~0;
///		}
/// }
///	void application_create(void)
///	{
///		// .... All other application initialization.
///		devlpm_registerForLowPowerQueries(application_time_to_sleep_callback, 0);
///	}
/// \endverbatim
/// \param callback Pointer to function that will be called for time-to-sleep decision.
/// \param context Any application context that needs to be passed back to the callback.
/// \return TRUE when successfully registered; else FALSE.
BOOLEAN devlpm_registerForLowPowerQueries(DeviceLpmQueriableMethodCallback callback, UINT32 context);

/// Register for an early wake notification. Typically, when connected, the device will
/// wake just in time to service the connection event and send out any data in its buffers.
/// In cases where the application has to service an external interrupt between connection
/// events (which may happen often), and have the data ready in time for the TX, the app
/// may want to register a callback for any non-scheduled wake event so it can take action
/// as quickly as possible.
/// \param fn Pointer to a function that will serialized to the application thread. See bleappevent.h
/// \param data An application context that will be passed back when invoking the callback.
void devlpm_registerForEarlyWakeNotification(INT32 (*fn)(void*), void* data);

/// Enters the configured low power mode (deep sleep). When deep sleep is enabled, this function
/// will never return.
/// NOTE: This has to be invoked inside an OS context lock.
void devlpm_enterLowPowerMode(void);

/// Check for disconnected low power mode. Internal, not for application use.
void devlpm_checkForDiscLowPowerCondition(void);

/// Check if deep sleep is allowed. Internal, not for application use.
BOOLEAN devlpm_registeredObjectsApproveHidOff(void);

/// Check for sleep. Internal, not for application use.
UINT32 devlpm_timeToSleep(void);

/// Early wake dispatch function. Internal, not for application use.
void devlpm_handleEarlyWakeNotification(void);

/// Enable early wake sources. There are three sources - Keyscan, Quadrature sensor
/// and GPIOs.
/// \param sources Bitwise OR of the sources to enable. Valid sources are:
///		DEV_LPM_WAKE_SOURCE_GPIO, DEV_LPM_WAKE_SOURCE_KEYSCAN and DEV_LPM_WAKE_SOURCE_QUAD,
void devlpm_enableWakeFrom(UINT16 sources);

/// Disable early wake sources. There are three sources - Keyscan, Quadrature sensor
/// and GPIOs.
/// \param sources Bitwise OR of the sources to disable. Valid sources are:
///		DEV_LPM_WAKE_SOURCE_GPIO, DEV_LPM_WAKE_SOURCE_KEYSCAN and DEV_LPM_WAKE_SOURCE_QUAD,
void devlpm_disableWakeFrom(UINT16 sources);

/// @}

#endif
