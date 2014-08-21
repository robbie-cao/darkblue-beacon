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
* RTC Sample
*
* This application provides the sample code for interfacing with
* the on-chip RTC clock. It is highly recommended that an external 32K
* xtal be used when accurate time is to be maintained. The accuracy
* of the time maintained with the an external LPO is an order of magnitude
* better than the internal LPO.
*
*
* Features demonstrated
*  - Use of the on-chip RTC interface.
*  - Use of the external 32 KHz LPO for sleep and deep sleep.
*  - Entering deep sleep on demand.
*  - Using the external LPO (32KHz xtal) for timed wake from deep sleep.
*
* To demonstrate the app, work through the following steps.
* 0. Enable the external 32KHz oscillator on the TAG board (see Quick Start Guide -
*    make sure that R102 and R103 on BCM920737TAG_Q32 are populated
*    with a 0 Ohm resistor, else the the behavior is undefined).
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Application initializes and RTC and prints time every second.
* 3.a Application programs and enters deepsleep, optionally configuring for a timed wake.
*/

// Uncomment the #define RTC_SAMPLE_ENTER_DEEP_SLEEP_AND_ENABLE_TIMED_WAKE below to
// see how to enter deep sleep on demand and also program timed wake using the external
// 32KHz xtal.
#define RTC_SAMPLE_ENTER_DEEP_SLEEP_AND_ENABLE_TIMED_WAKE

#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "bleappconfig.h"
#include "cfa.h"
#include "rtc.h"
#include "bleapputils.h"
#include "bleapp.h"
#include "devicelpm.h"
#include "miadriver.h"

/******************************************************
 *                      Constants
 ******************************************************/

/******************************************************
 *               Function Prototypes
 ******************************************************/
static void rtc_sample_create(void);
static void rtc_sample_timeout(UINT32 arg);
static void rtc_sample_fine_timeout(UINT32 arg);

/******************************************************
 *               Variables Definitions
 ******************************************************/

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG rtc_sample_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ GPIO_PIN_UART_TX,
    /*.rxpin      =*/ GPIO_PIN_UART_RX,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg(NULL, 0, NULL, (void *)&rtc_sample_puart_cfg, NULL, rtc_sample_create);
}

// Create the RTC sample.
void rtc_sample_create(void)
{
    RtcTime current_time;
	char buffer[64];

    ble_trace0("rtc_sample_create()\n");

    if (!mia_isResetReasonPor())
    {
    	ble_trace0("Waking from deep sleep because the timer went off or a GPIO triggered while waiting for timer to expire.");
    }
    else
    {
    	ble_trace0("Not a timed wake.");
    }

    // Always clear interrupts on P39, which is the interrupt pin used by the wake-from-deep-sleep HW block.
    gpio_clearPinInterruptStatus(GPIO_PIN_P39 / GPIO_MAX_NUM_PINS_PER_PORT, GPIO_PIN_P39 % GPIO_MAX_NUM_PINS_PER_PORT);

    blecm_configFlag |= BLECM_DBGUART_LOG;

    bleprofile_Init(bleprofile_p_cfg);

	// If we need to use the external 32K, then configure the reference
	rtcConfig.oscillatorFrequencykHz = RTC_REF_CLOCK_SRC_32KHZ;

	// Since the 32K external LPO is connected tp P10, P11, P12, P26 and P27,
	// input and putput disable all 5 GPIOs.
	gpio_configurePin(0, 10, GPIO_INPUT_DISABLE, 0);
	gpio_configurePin(0, 11, GPIO_INPUT_DISABLE, 0);
	gpio_configurePin(0, 12, GPIO_INPUT_DISABLE, 0);
	gpio_configurePin(1, 10, GPIO_INPUT_DISABLE, 0);
	gpio_configurePin(1, 11, GPIO_INPUT_DISABLE, 0);

    // Initialize the RTC.
    rtc_init();

    memset(buffer, 0x00, sizeof(buffer));
	ble_trace0("Time base is:");

	// RtcTime of 0x00 is the start of RTC time.
    memset(&current_time, 0x00, sizeof(current_time));
	rtc_ctime(&current_time, buffer);
	ble_trace0(buffer);

	// Let year = 2014.
	current_time.year = 2014;
	// Let month = july = 6 (jan = 0)
	current_time.month = 6;
	// Let day = 1st.
	current_time.day = 15;
	// Let current time be 12:00:00 Noon.
	current_time.hour = 12;
	current_time.minute = 00;
	current_time.second = 0x00;

	// If this is a power-on reset, we need to set up the reference time.
	if (mia_isResetReasonPor())
	{
		// Now set the on-chip RTC.
		if(rtc_setRTCTime(&current_time))
		{
			memset(buffer, 0x00, sizeof(buffer));

			ble_trace0("Power-on reset, set current time to:");
			rtc_ctime(&current_time, buffer);
			ble_trace0(buffer);
		}
		else
		{
			ble_trace0("Unable to set time.");
		}
	}
	else
	{
		// Set up the original reference time instead of using 01/01/2010, 00:00:00 as the reference
		// because this is a wake from deep sleep. The HW clock keeps running in deep sleep so when
		// we wake up, the FW needs to know what was used as the original reference time.
		rtc_setReferenceTime(&current_time);
	}

    bleprofile_regTimerCb(rtc_sample_fine_timeout, rtc_sample_timeout);
    bleprofile_StartTimer();

    // Since we have an external 32 KHz LPO, switch to using this during sleep
    // because this will give us a more accurate sleep clock (lower drift than the
    // internal LPO, so the uncertainty window during a receive will be narrower).
    // In general, base sleep current will be lower too.
    // Switching to the external 32K with bleapputils_changeLPOSource without having
    // initialized the RTC, the high-z'ing bonded GPIOs and not having the 32KHz oscillator physically
    // connected to the chip will invoke undefined behavior.
    bleapputils_changeLPOSource(LPO_32KHZ_OSC, FALSE, 250);

    // Trace out number of bytes free.
    ble_trace1("Number of free bytes in RAM: %d",  cfa_mm_MemFreeBytes());
}

// One second timer expired. Read the time from RTC and print.
void rtc_sample_timeout(UINT32 arg)
{
	RtcTime current_time;
	UINT32 seconds_since_time_base;
	char buffer[64];
	tRTC_REAL_TIME_CLOCK raw_clock;

	memset(buffer, 0x00, sizeof(buffer));

	// Get and print current time.
	rtc_getRTCTime(&current_time);
	ble_trace0("Current date/time is:");
	rtc_ctime(&current_time, buffer);
	ble_trace0(buffer);

	// Get and print time since time base in seconds.
	rtc_RtcTime2Sec(&current_time, &seconds_since_time_base);
	ble_trace1("Its been %d seconds since bigbang.", seconds_since_time_base);

	// Get and print the raw 48 bit clock value.
	rtc_getRTCRawClock(&raw_clock);

	ble_trace2("Upper and lower 32 bit values: 0x%08X, 0x%08X\n", raw_clock.reg32map.rtc32[1], raw_clock.reg32map.rtc32[0]);

#ifdef RTC_SAMPLE_ENTER_DEEP_SLEEP_AND_ENABLE_TIMED_WAKE
	{
		static UINT32 num_timeouts_since_boot = 0;

		// Demo going into deep sleep with wake after 5s.
		if (num_timeouts_since_boot++ >= 10)
		{
			// If its been ~10s of ADV, configure timed wake and
			// enter deep sleep right now.
			ble_trace0("Entering deep sleep.");

			gpio_configurePin(0, 0, 0x100, 0);

			// Configure the low power manager to enter deep sleep.
			devLpmConfig.disconnectedLowPowerMode = DEV_LPM_DISC_LOW_POWER_MODES_HID_OFF;

			// Configure the wake time in mS.
			devLpmConfig.wakeFromHidoffInMs = 5000;

			// Configure the reference clock to use.

			// Use the external 32k.
			devLpmConfig.wakeFromHidoffRefClk = HID_OFF_TIMED_WAKE_CLK_SRC_32KHZ;

			gpio_configurePin(0, 0, 0x100, 0);

			// Enter deep-sleep now. Will not return.
			devlpm_enterLowPowerMode();
		}
	}
#endif
}

void rtc_sample_fine_timeout(UINT32 arg)
{

}

