/*******************************************************************************
* ------------------------------------------------------------------------------
*
* Copyright (c), 2005, 2007, 2010, 2011 BROADCOM Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: devlpm.cpp
*
* Abstract: 
*   This file implements the device low power management functions.
*******************************************************************************/

#include "miadriver.h"
#include "csa.h"
#include "cfa.h"
#include "pmu.h"
#include "bleappconfig.h"
#include "devicelpm.h"
#include "osapi.h"
#include "bleappevent.h"
#include "bleprofile.h"


DeviceLpmState devLpmState = 
{
    NULL, // fn
    NULL, // context
    NULL, // devLpmState.firstReg
    NULL, // devLpmState.oldCsaGetTimeToSleep
};

/// Configuration record of this LPM
DeviceLpmConfig devLpmConfig = 
{
    // Use sleep in disconnect
    DEV_LPM_DISC_LOW_POWER_MODES_SLEEP,

    // Never wakeup
    HID_OFF_WAKEUP_TIME_NONE,

    // Use 128 KHz internal MIA clock as reference
    HID_OFF_TIMED_WAKE_CLK_SRC_128KHZ,
}; 

/*******************************************************************************
 * Methods
 *******************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// Initializes the Device Lpm
/// Also registers with CSA for its own callbacks. Has to be called first before using
/// any of this LPM's services
////////////////////////////////////////////////////////////////////////////
void devlpm_init(void)
{
    // HID App will be the entry point for deciding HID OFF
    cfa_InstallIdleThreadCSAPoll(/*CFA_IDLE_FP*/ devlpm_checkForDiscLowPowerCondition);
    
    // HID App will be the entry point for deciding how long to sleep
    devLpmState.oldCsaGetTimeToSleep = csa_getTimeToSleep;
    cfa_InstallGetTimeToSleep(/*CFA_GET_TIME_TO_SLEEP_FP*/ devlpm_timeToSleep);
    cfa_InstallEarlyWakeNotificationHandler(/* CFA_EARLY_WAKE_NOTIFICATION_HANDLER_FP */ devlpm_handleEarlyWakeNotification);
}

////////////////////////////////////////////////////////////////////////////////
/// Adds the given object to the list of objects that need to be queried for sleep and hid-off
/// \param callback The callback that needs to be invoked to check for sleep/hid-off OK.
///           The callback should decide very quickly and return within a few 10s of uS.
/// \param context Any arbitrary context that the callback function may need later. Will be passed
///           in as is when the callback is invoked.
/// \return TRUE if successfully added, else FALSE.
////////////////////////////////////////////////////////////////////////////////
BOOLEAN devlpm_registerForLowPowerQueries(DeviceLpmQueriableMethodCallback callback, UINT32 context)
{
    DeviceLpmQueriableRegistration* reg = (DeviceLpmQueriableRegistration*) cfa_mm_Sbrk(sizeof(DeviceLpmQueriableRegistration));
    if(reg)
    {
        reg->pFunc = callback;
        reg->context = context;
        reg->next = devLpmState.firstReg;
        devLpmState.firstReg = reg;
        return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// Allows an application to register for an warly wake notification.
/// \param fn Pointer to function that will handle the early notification
/// \param data An opaque data that is handed to callback.
////////////////////////////////////////////////////////////////////////////////
void devlpm_registerForEarlyWakeNotification(INT32 (*fn)(void*), void* data)
{
    devLpmState.fn = fn;
    devLpmState.context = data;
}

/////////////////////////////////////////////////////////////////////////////////
///  Don't allow sleep if all registered objects are idled and the
/// disconnect low power mode is set to something other than SLEEP.
/// Otherwise ask all registered objects how long they can sleep for, and
/// ask the CSA thread how long it will allow us to sleep.
///  Return the minimum sleep time between all objects and CSA thread
/// 
/// \return time to sleep according to application
/// 
/////////////////////////////////////////////////////////////////////////////////
UINT32 devlpm_timeToSleep(void)
{
    UINT32 csaTimeToSleep          = PMU_MAX_TIME_TO_SLEEP;
    UINT32 minTimeToSleep = PMU_MAX_TIME_TO_SLEEP;
    DeviceLpmQueriableRegistration* reg = devLpmState.firstReg;
    
    // Check if a non SLEEP disconnected low power mode has been chosen and all registered object
    // are idle, go hid-off instead of sleep. Else, query for sleep
    if ((devLpmConfig.disconnectedLowPowerMode != DEV_LPM_DISC_LOW_POWER_MODES_SLEEP) && 
        devlpm_registeredObjectsApproveHidOff())
    {
        // Don't sleep
        return 0;
    }

    // Ask all registered objects and the CSA thread how long
    // they can sleep for.
    
    if (devLpmState.oldCsaGetTimeToSleep != NULL)
    {
        // How long will CSA let us sleep?
        csaTimeToSleep = devLpmState.oldCsaGetTimeToSleep();
    }
    
    // Check with the list of objects by iterating through all
    for(reg = devLpmState.firstReg; reg != NULL; reg = reg->next)
    {
        UINT32 tts = PMU_MAX_TIME_TO_SLEEP;
        
        if(reg->pFunc && ((tts = reg->pFunc(LOW_POWER_MODE_POLL_TYPE_SLEEP, reg->context)) == 0))
        {
            // Some object said don't sleep
            return 0;
        }
        
        if(tts < minTimeToSleep)
            minTimeToSleep = tts;
    }
    
    // Return the minimum sleep time allowed
    return MIN(csaTimeToSleep, minTimeToSleep);
}


////////////////////////////////////////////////////////////////////////////////
/// Checks whether objects queriably by this LPM are OK to go hid-off
/// \return TRUE if all objects are idle, FALSE if at least one is not
////////////////////////////////////////////////////////////////////////////////
BOOLEAN devlpm_registeredObjectsApproveHidOff(void)
{
    BOOLEAN clientsAreIdle = TRUE;
    DeviceLpmQueriableRegistration* reg = devLpmState.firstReg;
    
    for(reg = devLpmState.firstReg; reg != NULL; reg = reg->next)
    { 
        if(reg->pFunc && !(reg->pFunc(LOW_POWER_MODE_POLL_TYPE_POWER_OFF, reg->context)))
        {
            // Some object said don't sleep
            clientsAreIdle = FALSE;
            break;
        }
    }    
    
    return clientsAreIdle;
}

/////////////////////////////////////////////////////////////////////////////////
/// This function will ask the registered objects if they can go
/// into a disconnected low power mode.  If they can, the configured
/// disconnected low power mode is used.
///
/// !WARNING! This will be called from a non-CSA (ie. app) thread and
///           with interrupts disabled.
///
/////////////////////////////////////////////////////////////////////////////////
void devlpm_checkForDiscLowPowerCondition(void)
{
    // Are we configured for something other than sleep? 
    if(devLpmConfig.disconnectedLowPowerMode != DEV_LPM_DISC_LOW_POWER_MODES_SLEEP)
    {
        // Maybe go disconnected low power mode as long as the application is ok
        // with it, no transport has activity which could lead to
        // a HID connection or is tearing down a current HID connection
        //  This is done the first time without interrupts locked to save from lock
        if (devlpm_registeredObjectsApproveHidOff())
        {
            osapi_INIT_LOCK_CONTEXT;
            
            osapi_LOCK_CONTEXT;
            
            // Yes we are checking for a second time, because we didn't check
            // with interrupts locked the first time.  
            if (devlpm_registeredObjectsApproveHidOff())
            {
                devlpm_enterLowPowerMode();
            }
            osapi_UNLOCK_CONTEXT;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Puts the device in the configured low power mode.
/// NOTE: This has to be invoked within an OS context/interrupt lock.
////////////////////////////////////////////////////////////////////////////////
void devlpm_enterLowPowerMode(void)
{
    // Check if the configuration allows going into HID-off
    if (devLpmConfig.disconnectedLowPowerMode == DEV_LPM_DISC_LOW_POWER_MODES_HID_OFF)
    {
        // Give app chance to reprogram GPIO Interrupt sources or whatever
        // it needs to do before going HID Off
        bleprofile_enteringDiscLowPowerMode(DEV_LPM_DISC_LOW_POWER_MODES_HID_OFF);
        
        // We can go hid-off
        mia_enterHidOff(devLpmConfig.wakeFromHidoffInMs, devLpmConfig.wakeFromHidoffRefClk);
        
        // If we get here HID-OFF was aborted
        bleprofile_abortingDiscLowPowerMode(DEV_LPM_DISC_LOW_POWER_MODES_HID_OFF);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Enables wake up from the given sources
/// \param sources The DEV_LPM_WAKE_SOURCE_* sources for wake
////////////////////////////////////////////////////////////////////////////////
void devlpm_enableWakeFrom(UINT16 sources)
{
    // Consider only the HID wake source bits
    sources &= DEV_LPM_WAKE_SOURCE_MASK;

    // Or in the given sources to the PMU wake bits
    pmu_setWakeupInterruptSource2(pmu_getWakeupInterruptSource2() | sources);
}

////////////////////////////////////////////////////////////////////////////////
/// Disables wake up from the given sources
/// \param sources The DEV_LPM_WAKE_SOURCE_* wake sources that need to be disabled
////////////////////////////////////////////////////////////////////////////////
void devlpm_disableWakeFrom(UINT16 sources)
{
    // Consider only the HID wake source bits
    sources &= DEV_LPM_WAKE_SOURCE_MASK;

    // Clear the given bits in the PMU wake bits
    pmu_setWakeupInterruptSource2(pmu_getWakeupInterruptSource2() & ~sources);
}

////////////////////////////////////////////////////////////////////////////////
/// Notifies the app instance when the system is woken up earlier than expected.
////////////////////////////////////////////////////////////////////////////////
void devlpm_handleEarlyWakeNotification(void)
{
    // If a callback was registered, serialize it.
    if(devLpmState.fn)
    {
        bleappevt_serialize(devLpmState.fn, devLpmState.context); 
    }
}
