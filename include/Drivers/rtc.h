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
* Define functions to access Real-Time Clock (RTC) peripheral
*/
#ifndef __RTC_H__
#define __RTC_H__


#include "types.h"


/**  \addtogroup RTC
 *  \ingroup HardwareDrivers
*/

/*! @{ */
/**
* Defines an Rtc driver. 
*  
* 2073x support 48 bits RTC timer from either the internal 128 KHz
* RC oscillator or an external 32 KHz oscillator.
*  
* Usage: 
*  
* (1) use rtc_setRTCTime() to set the current calendar time.
*  
* (2) user rtc_getRTCTime() to retrieve the current calendar.
* time. 
*  
* (3) rtc_ctime(), will convert the RTC_time to ASCII user
* friendly string. 
*  
*/

/// RTC HW control bits.
enum
{
    LHL_CTL_32K_OSC_POWER_UP = 0x04,
    LHL_CTL_32K_OSC_POWER_DN = 0x00,
    LHL_CTL_32K_OSC_POWER_MASK = 0x04
};

/// LHL_CTL register definition. Internal.
typedef union
{
    UINT32          lhl_adc_rtc_ctl_reg;

    struct 
    {
        /// bit 0:11  reserved
        UINT32                  reserved1              : 12;                 

        /// bit 12 -   Real time clock terminal count status enable
        UINT32                  rtcTerminalCntStatusEn  : 1;  

        /// bit 13 - RTC reset counter
        UINT32                  rtcResetCounter         : 1;

        /// Bit 14 - RTC timer function enable
        UINT32                  rtcTimerFuncEn          : 1;

        /// Bit 15 - RTC counter enable
        UINT32                  rtcCounterEn            : 1;

        /// reserved 31:3
        UINT32                  reserved2               : 16;               
    }bitmap;
} tRTC_LHL_ADC_RTC_CTL_REG;


/// RTC enable type.
typedef enum 
{
    // enable RTC and power up 32kHz crystal oscillator
    LHL_CTL_RTC_ENABLE              = 1,

    // disable RTC and power down the 32kHz crystal oscillator
    LHL_CTL_RTC_DISABLE             = 0,

}tRTC_LHL_CTL_RTC_ENABLE_MODE;

/// Reference clock for the RTC.
enum
{
    RTC_REF_CLOCK_SRC_32KHZ  = 32,
    RTC_REF_CLOCK_SRC_128KHZ = 128
};



/// Real time clock read from hardware (48 bits).
typedef union 
{
    UINT64          rtc64;

    struct    
    {
        UINT16      rtc16[4];
    }reg16map;

    struct 
    {
        UINT32      rtc32[2];
    }reg32map;

} tRTC_REAL_TIME_CLOCK;


/// Time converation reference timebase 2010/1/1.
typedef enum 
{
    BASE_LINE_REF_YEAR      =   2010,
    BASE_LINE_REF_MONTH     =   1,
    BASE_LINE_REF_DATE      =   1,
} tRTC_REFERENCE_TIME_BASE_LINE;


/// Time structure.
typedef struct
{
    /// seconds (0 - 59), not support leap seconds
    UINT16  second;

    /// minutes (0 - 59),
    UINT16  minute;

    /// hours (0 - 23)
    UINT16  hour;

    /// day of the month (1 - 31)
    UINT16  day;    
    
    /// month (0 - 11, 0=January)   
    UINT16  month;

    /// year 
    /// should larger then 2010
    UINT16  year;

} RtcTime; 

/// Internal runtime-state of RTC driver.
typedef struct
{
    UINT32                       userSetRtcClockInSeconds;
    tRTC_REAL_TIME_CLOCK         userSetRtcHWTimeStamp;
} RtcState;


/// Initialize the RTC block.
void rtc_init(void);


/// Reads current RTC value from hardware clock.
/// \param rtcClock Pointer to allocated RTC time structure into which the current time is to be copied into.
void rtc_getRTCRawClock(tRTC_REAL_TIME_CLOCK *rtcClock);


/// Get current time in RTC_time format.
/// \param timebuf Pointer to allocated time structure into which the current time is to be copied into.
void rtc_getRTCTime(RtcTime *timebuf);


/// Set the current time.
/// \param newTime Pointer to time structure from which the current time is to be copied into the HW clock.
BOOL32 rtc_setRTCTime(RtcTime  *newTime);

/// Convert the time object pointed by timer to a
/// c string containing a human-readable version of
/// the corresponding local time and data.
/// The returned string has the following format:
/// Mmm dd hh:mm:ss yyyy
/// where
///      Mmm - the month in letters
///      dd  - the day of the month
///      hh:mm:ss -  time
///      yyyy - year
/// \param timer Pointer to the time structure that is to be converted to a C-String.
/// \param outbuf Pointer to a character buffer into which the converted string is stored.
char *rtc_ctime(RtcTime *timer, char *outbuf);

/// Convert the 32 bit seconds to time.
/// \param second The number of seconds since timebase.
/// \param rtctime Pointer to an allocated time structure into which the converted time is returned.
void    rtc_sec2RtcTime(UINT32 second, RtcTime *rtctime);

/// Convert time to 32 bit seconds since timebase.
/// \param rtctime Pointer to the structure that contains the time.
/// \param second Number of seconds since timebase.
void    rtc_RtcTime2Sec(RtcTime *rtctime, UINT32 *second);


/* @}  */
#endif

