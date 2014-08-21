/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2010 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: bleappfwu.h
*
********************************************************************
*/

#ifndef __BLEAPPFWU_H__
#define __BLEAPPFWU_H__

#include "cfa.h"


/**  \addtogroup utility
 *
*/
/*! @{ */

/// BleAppFwu implements firmware update.
/*! This provides a few methods to be called from the App to
    implement the Firmware Upgrade protocol.

    The protocol is very simple:
    - Host can Read EEPROM/ParallelFlash/SerialFlash any time by
        - Read ( SETUP_READ +  READ)

    - However Write and Erase (firmware upgrade) need to enabled first by sending an ENABLE_FWU .
    Once enabled, the host can:
    - Read (SETUP_READ + READ) memory, including EEPROM and Flash.
    - Write (WRITE), including EEPROM at 0xFFxxxxxx
    - Launch (make a call to a given address, with the usual "bit 0 == thumb"
    - Erase (memset 0xFF, on IRAM only right now)

    As soon ENABLE_FWU was enabled, a watchdog timer is started (default 10
    seconds). Every accepted FWU command restarts it; if it's
    let expire, we reset the system to get out of any issue (e.g. the
    host hangs or we lose the link).

    Based on these primitives, the host can perform almost any
    update. 

*/

enum
{
    DEFAULT_WATCHDOG_VALUE = 10
}; // seconds

enum
{
    DEFAULT_ID_ENABLE_FWU   = 0x70, /// "unlock" OTAFU, start the watchdog
    DEFAULT_ID_SETUP_READ   = 0x71, /// setup a read
    DEFAULT_ID_READ         = 0x72, /// get the data
    DEFAULT_ID_ERASE        = 0x73, /// erase a sector
    DEFAULT_ID_WRITE        = 0x74, /// write (preferrably after erase)
    DEFAULT_ID_LAUNCH       = 0x75, /// call a function (with arguments)
};

typedef struct BltFwuConfig_
{
    /// Report ID of "ENABLE OTAFU" set-report
    UINT8 idEnableOtafu;
    /// Report ID of "SETUP READ" set-report
    UINT8 idSetupRead;
    /// Report ID of "READ" get-report
    UINT8 idRead;
    /// Report ID of "ERASE" set-report
    UINT8 idErase;
    /// Report ID of "WRITE" set-report
    UINT8 idWrite;
    /// Report ID of "LAUNCH" set-report
    UINT8 idLaunch;
    /// The default channel ID to use for FWU
    UINT16 defaultFwuChannelId;    
} BltFwuConfig;

typedef struct BleFwuState_
{
    UINT8 * readAddr;
    UINT16 readLen;
    BOOL8 enabled;
    UINT16 watchdogValue;
    INT32 watchdogTimerId;
} BleFwuState;

#pragma pack(1)

/// Structure of the firmware upgrade header
/*! Almost all OTAFU-related reports have the following structure:
      - Address (4 bytes, little endian)
      - Length in bytes (2 bytes, little endian)
      - Data bytes (if present, their count must match "length")
      - Checksum (1 byte)

       The only exception is the ENABLE report, which doesn't have any
       payload at all (just the report ID).
       
       Note that "Length" might be greater than zero even if there are
       no data bytes, e.g. in an ERASE report.
*/
typedef PACKED struct FwReportHeader_
{
    UINT32 address;
    UINT16 len;     
}FwReportHeader;

typedef PACKED struct
{
    UINT8 commandId;
} FwCommandHeader;

#pragma pack()

enum 
{
    RESTART_WATCHDOG_TIMER = 0,

    // check watch dog timer before reset watchdog timer
    // if watchdog timer not enabled, leave as it is
    // this mainly for memory read support
    PET_WATCHDOG_ONLY,  
};

enum 
{
    INDIRECT_MEM_MAP_MASK   = 0xFF000000,

    /// indirect memory map for EEPROM Read/Write acess
    INDIRECT_MEM_MAP_EEPROM = 0xFF000000,

    /// indirect memory map for parallel flash
    INDIRECT_MEM_MAP_PF     = 0xFC000000, 

    /// indirect memory map for serial flash Read/Write/Erase access
    INDIRECT_MEM_MAP_SF     = 0xF8000000,
};

// The error codes returned by FWU - same as std HID handshake 
enum
{ 
    LEFWU_SUCCESS             = 0x00,
    LEFWU_ERR_NOT_READY       = 0x01,
    LEFWU_ERR_INVALID_CMD     = 0x02,
    LEFWU_ERR_UNSUPPORTED_REQ = 0x03,
    LEFWU_ERR_INVALID_PARAM   = 0x04,
    LEFWU_ERR_UNKNOWN         = 0x0E,
    LEFWU_ERR_FATAL           = 0x0F
};

void bleappfwu_init(void);
void bleappfwu_restartWatchdog(UINT32 checkWatchDog);
void bleappfwu_watchdogExpired(UINT32 unused);
UINT8 bleappfwu_checksum(UINT8 commandId, const UINT8* data, UINT32 len);
UINT32 bleappfwu_validate(UINT8 commandId, const UINT8* payload, UINT32 payloadSize);
UINT32 bleappfwu_readMem(UINT8* buf, UINT32 readFrom, int len);
UINT32 bleappfwu_writeMem(const UINT8* buf, UINT32 writeTo, UINT32 len);
UINT32 bleappfwu_eraseMem(UINT32 eraseAddr, UINT32 len);
UINT32  bleappfwu_processCommand(UINT8 commandId, const void *payload, UINT16 payloadSize);
UINT32  bleappfwu_getData(UINT8 commandId, UINT8 ** data, UINT32 * dataLen);
/* @}  */

#endif
