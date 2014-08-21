/*******************************************************************************
*  THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
* *******************************************************************************
*
*        Copyright (c) 2010 Broadcom Corp. ALL RIGHTS RESERVED
*
* \file   bleapputils.h
* \brief  
*
*  Utility functions for BLE application/profile
*
*  \author Arvind Sridharan
*  \date   2011-05-16
*******************************************************************************/
#include "types.h"

/// Utilities function initialization. Internal, FW initializes before the app is created.
void bleapputils_init(void);

/// Reads from the EEPROM using miscBleAppConfig.bleapputils_eepromCustAreaOffset as the base.
/// The app can configure miscBleAppConfig so:
/// \verbatim
/// #include "bleappconfig.h"
/// void some_function(void)
/// {
///		char buffer[32];
/// 	// 0x4000 is the offset of the application specific area.
/// 	miscBleAppConfig.bleapputils_eepromCustAreaOffset = 0x4000;
///		// Allocate 1K for it.
///		miscBleAppConfig.bleapputils_eepromCustAreaLen = 0x400;
///		// Read 32 bytes from 0x00 offset from bleapputils_eepromCustAreaOffset, which is physical location 0x4000 of the EEPROM.
///		bleapputils_eepromRead(buffer, 0x00, sizeof(buffer));
///	}
///	\endverbatim
/// Note that the area of the EEPROM starting from bleapputils_eepromCustAreaOffset, of length
///	bleapputils_eepromCustAreaLen should not already be a part of the FW image.
///	This functionality is not available on serial flash.
/// \param buf A buffer at least len bytes long to read into.
/// \param offset Offset from miscBleAppConfig.bleapputils_eepromCustAreaOffset to read from.
/// \param len Number of bytes to read.
/// \return TRUE for success; else FALSE.
BOOLEAN bleapputils_eepromRead(void* buf, UINT32 offset, UINT32 len);

/// Writes to the EEPROM miscBleAppConfig.bleapputils_eepromCustAreaOffset as the base physicall address.
/// \param buf Pointer to a buffer to write from.
/// \param offset Offset from miscBleAppConfig.bleapputils_eepromCustAreaOffset to write to.
/// \param len Number of bytes to write.
/// \return TRUE for success; else FALSE.
BOOLEAN bleapputils_eepromWrite(void* buf, UINT32 offset, UINT32 len);

/// Disables interrupts and preemption. Use with caution; never lock out interrupts for more than ~100uS.
/// \return Returns an opaque current posture that is to be used when restoring interrupts.
UINT32 bleapputil_cpuIntDisable(void);

/// Enables/restores interrupt and preemption state.
/// \param _newPosture The value returned by bleapputil_cpuIntDisable previously.
void bleapputil_cpuIntEnable(UINT32 _newPosture);

/// This function returns the current native Bluetooth clock; this
/// counter is 28 bits, and ticks every 312.5 us and is adjusted for
/// drift through sleep, etc. To compute differences and times elapsed,
/// use bleapputils_diffNativeBtClks() or bleapputils_BtClksSince().
/// \return the counter value.
/// \return The current BT clock value.
UINT32 bleapputils_currentNativeBtClk(void);

/// This function computes the signed difference between two BT
/// clock instants. The general "garbage-in garbage-out" principle
/// applies, so clocks must be from the same piconet, must be valid when
/// they are taken, etc. BT clock is 28 bits only and cannot be relied upon when
/// timings in the hours are desired.
/// \param from is the "from" time.
/// \param to is the "to" time.
/// \return the signed difference (to - from); positive if "to" is after "from",
/// negative otherwise.
INT32 bleapputils_diffBtClks(UINT32 from, UINT32 to);

/// This function computes the time elapsed since "before", in BT
/// clocks. This functions handles rollovers. Clock resets will cause
/// a large value to be returned.
/// \param before is the previous counter value, as was returned by
/// hiddcfa_currentNativeBtClk().
/// \return the time elapsed, in BT clocks (312.5 us).
UINT32 bleapputils_BtClksSince(UINT32 before);

/// Change the current LPO source to use in sleep. Internal, not for application use.
void bleapputils_changeLPOSource(UINT8 pmu_clkSrc, BOOLEAN selSlaveLPOSrc, UINT16 driftRate);

/// Calibrates the LHL LPO and returns the value. Internal, not for application use.
UINT32 bleapputils_getLhlLpoCalibrationValue(UINT32 lpoCycles);

/// Busy-wait for the given number of uS. CAUTION: This may trip the watchdog if used for more than
/// a few milliseconds. Any busy wait of 12.5mS or longer are discouraged.
/// \param delayUs Number of microseconds to wait for.
void bleapputils_delayUs(UINT32 delayUs);

/// Busy-wait for hundreds of microseconds while petting the watchdog. CAUTION: This is guaranteed
/// to not trip the watchdog, but this will also starve the application thread.
/// \param hundredsOfUs Number of microseconds to wait in hundreds of Us.
void bleapputils_Sleep(UINT8 hundredsOfUs);

/// Driver for internal ROM tests. Internal, not for application use. Behavior undefined when
/// invoked by application.
/// \param lengthOfRom Length of the ROm image in bytes.
/// \param romChecksum The expected 32-bit checksum of the ROM.
/// \param romCrc The expected 32-bit CRC of the ROM.
/// \param romTestsToRun The mask of the tests to be run on the ROM.
/// \return The mask of the passing tests masked by romTestsToRun.
UINT32 bleapputils_runRomTests(UINT32 lengthOfRom, UINT32 romChecksum, UINT32 romCrc, UINT32 romTestsToRun);

/// Driver for running all the RAM tests. Internal, not for application use. Behavior undefined when
/// invoked by application.
/// \param startOfRam The start address of the RAM. This has to be 32-bit aligned.
/// \param endOfRam  The end address of the RAM. This has to be 32-bit aligned.
/// \param testsToRunMask Bits representing the RAM tests to be run.
/// \return Mask of passing tests set to 1 and failing tests to 0, masked by testsToRunMask
UINT32 bleapputils_runRamTests(UINT32* startOfRam, UINT32* endOfRam, UINT32 testsToRunMask);
