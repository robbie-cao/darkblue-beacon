/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------------------
*
*           Copyright (c) 2003, 2004 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************************

********************************************************************************
*    File Name: cfa.h
*
*    Abstract:  Core Firmware API Definitions
*
*    Functions:
*            --
*
*    $History:$
*
*******************************************************************************/


#ifndef _CFA_H_
#define _CFA_H_


#include "types.h"
#include "gpio.h"

// DO NOT INCLUDE ANY OTHER HEADERS.  THIS HEADER NEEDS TO BE SUITABLE FOR
// EXTERNAL RELEASE!!!

#ifdef __cplusplus
extern "C" {
#endif

//******************************************************************************
// 10   Public Interfaces
//******************************************************************************

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.1     Initialization
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.1.1       Types
//------------------------------------------------------------------------------
typedef void (*CFA_INIT_FP)(void);
// The CFA_INIT_FP type is a function pointer for an initialization function.

typedef void (*CFA_CSA_EV_CB)(UINT32 eventBits);
typedef CFA_CSA_EV_CB CFA_HCITHR_EV_CB;
// The CFA_HCITHR_EV_CB type is a function pointer for an callback function
// called whenever a wait is satisfied on any CSA-specific events in the csa
// thread.

typedef void (*CFA_IDLE_FP)(void);
// The CFA_IDLE_FP type is a function pointer for an idle thread polling
// function.

typedef void (*CFA_EARLY_WAKE_NOTIFICATION_HANDLER_FP)(void);
// The CFA_EARLY_WAKE_NOTIFICATION_HANDLER_FP is a function pointer for
// early wake up notification.

typedef UINT32 (*CFA_GET_TIME_TO_SLEEP_FP)(void);
// The CFA_GET_TIME_TO_SLEEP_FP type is a function pointer for a function which
// will get called to check how long the system can sleep for.

typedef UINT32 (*CFA_FLASH_DRV_FP)( UINT32 action,
                                    UINT32 addr,
                                    UINT32 len,
                                    UINT16 *p_buf );
typedef CFA_FLASH_DRV_FP CFA_FLASH_WRITE_FP, CFA_FLASH_ERASE_FP;
#define CFA_FLASH_DRV_GET_SIZE      0
#define CFA_FLASH_DRV_WRITE         1
#define CFA_FLASH_DRV_ERASE         2
// The CFA_FLASH_*_FP types are function pointers to flash erase and write
// functions.  The erase function accepts CFA_FLASH_DRV_GET_SIZE or
// CFA_FLASH_DRV_ERASE.  The write function accepts CFA_FLASH_DRV_GET_SIZE or
// CFA_FLASH_DRV_WRITE.  In either case, the CFA_FLASH_DRV_GET_SIZE operation
// returns the size of the function.

typedef void (*CFA_ROM_ASSIST_DFU_FP)(void);
// The CFA_ROM_ASSIST_DFU_FP type is a function pointer for a ROM reentry vector
// when flash-based firmware is entering DFU mode.  The target function must be
// ARM mode.



typedef BOOL32 (*IS_FW_VALID_FP)(void);

typedef enum
{
    CFA_TRAN_UART,
    CFA_TRAN_USB,
    CFA_TRAN_SPI,
    CFA_TRAN_SLIMBUS,
    CFA_TRAN_SDIO,
    CFA_TRAN_I2C,
    CFA_TRAN_SPIFFY,    

    // this should be the last.
    CFA_TRAN_UNDEFINED
} CFA_TRAN_TRANSPORT_TYPE;

typedef void (*CFA_TRANS_SELECT_CB)(CFA_TRAN_TRANSPORT_TYPE);


//------------------------------------------------------------------------------
// 10.1.2       Installing Init Functions and Initializing Protected Data
//------------------------------------------------------------------------------
void cfa_InitProtectedData( CFA_INIT_FP preDSInitFunct,
                            CFA_INIT_FP postVSInitFunct,
                            CFA_INIT_FP postFWInitFunct,
                            UINT8 configAndFirmwareStatus,
                            UINT32 configSSOffset,
                            UINT32 configVSOffset,
                            UINT32 configVSLength,
                            CFA_FLASH_ERASE_FP flashEraseFunct,
                            CFA_FLASH_WRITE_FP flashWriteFunct,
                            CFA_ROM_ASSIST_DFU_FP romAssistDFUVector );
// This function is used to install init functions which will be called at
// various times during the firmware initialization process, and set the
// location of configuration data.  It must be invoked by a flash boot sector if
// one is used and the firmware early boot code which would otherwise initialize
// the corresponding firmware variables to NULL and 0 is bypassed.  This
// function may only be called by a flash boot sector.  The following defines
// pertain to these ConfigAndFirmwareStatus parameter:

//------------------------------------------------------------------------------
#define CFA_FIRMWARE_LOCATION_MASK          0x03
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define CFA_FIRMWARE_LOCATION_NONE          0x00
#define CFA_FIRMWARE_LOCATION_ROM           0x01
#define CFA_FIRMWARE_LOCATION_FLASH         0x02
#define CFA_FIRMWARE_LOCATION_EPM           0x03
//------------------------------------------------------------------------------
#define CFA_ROM_DIRECT_TO_DFU_FLAG          0x08
//------------------------------------------------------------------------------
#ifdef SERIAL_FLASH
#define CFA_CONFIG_LOCATION_MASK            0x70
#define CFA_CONFIG_LOCATION_SERIAL_FLASH    0x40
#else
#define CFA_CONFIG_LOCATION_MASK            0x30
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define CFA_CONFIG_LOCATION_NONE            0x00
#define CFA_CONFIG_LOCATION_EEPROM          0x10
#define CFA_CONFIG_LOCATION_FLASH           0x20
#define CFA_CONFIG_LOCATION_RAM_BUF         0x30
//------------------------------------------------------------------------------

// The romAssistDFUVector function pointer is a vector to an ARM mode reentry
// vector in a flash boot sector which facilitates entering the ROM for DFU
// purposes.

//------------------------------------------------------------------------------
void cfa_InstallPostVSInit(CFA_INIT_FP postVSInitFunct);
// This function will install an initialization function to be called by
// firmware once VS write capability initialization has taken place.  Reading
// from the VS is allowed prior to this function call, but writing to the VS is
// only allowed after this function has been called if one is installed, or
// would have been called if one is not (see Table 2 in the CFA SDD)

//------------------------------------------------------------------------------
void cfa_InstallPostFWInit(CFA_INIT_FP postFWInitFunct);
// This function will install an initialization function to be called by
// firmware once all the firmware threads have started running.

//------------------------------------------------------------------------------
void cfa_tran_InstallTranSelCb (CFA_TRANS_SELECT_CB tranSelcb);
//This function will install a calback to report transport selection

//------------------------------------------------------------------------------
// 10.1.3       CSA-Specific Events in the HCI Thread
//------------------------------------------------------------------------------
void cfa_InstallHCIThreadCSAEventHandler(CFA_CSA_EV_CB callback);
// This function installs a callback to be triggered whenever any CSA-specific
// events are set in the HCI thread.  Event bits 31:16 are reserved for
// CSA-specific events in the HCI thread.  Whenever the HCI thread event loop's
// call to osapi_waitEvent returns a value eventBits for which (eventBits &
// 0xFFFF0000) != 0, the callback function will be called, passing to the
// callback function an argument of (eventBits & 0xFFFF0000).

//------------------------------------------------------------------------------
void cfa_SetHCIThreadCSAEvent(UINT32 eventBits);
// This function sets CSA-specific event bits to trigger the event handler
// callback in the HCI thread.  Event bits 15:0 are reserved and must be zero.

//------------------------------------------------------------------------------
void cfa_SetHCIThreadCSAMouseKeyboardUSBSentEvent(UINT32 eventBit);
// This function sets a CSA-specific event bit to be triggered for the event
// handler callback in the HCI thread whenever a mouse or keyboard USB event is
// sent.  Event bits 15:0 are reserved and must be zero.  This provides a hint
// as to when the mouse or keyboard USB event queues may have space available.
// In the event that a mouse or keyboard event needs to be sent to the host but
// cfa_tran_GetKeyBdEventQueueCount or cfa_tran_GetMouseEventQueueCount
// indicates that the queues are full, this provides a means to trigger the CSA,
// both to wake it up for the purpose of sending the event, and to avoid having
// to poll for queue capacity to be available.

//------------------------------------------------------------------------------
void cfa_InstallEarlyWakeNotificationHandler(CFA_EARLY_WAKE_NOTIFICATION_HANDLER_FP callback);
// THis function sets the PMU's callback for Early wake.

//------------------------------------------------------------------------------
void cfa_setMinTimeToSleepForEnablingEarlyWake(UINT32 value);
// This function sets up the minimum time to sleep for early wake to be effective

//------------------------------------------------------------------------------
// 10.1.4       CSA Polling Function in the Idle Thread
//------------------------------------------------------------------------------
void cfa_InstallIdleThreadCSAPoll(CFA_IDLE_FP callback);
// This function installs a callback to be polled on every loop of the idle
// thread.  Being in the idle thread, this function will only be called when
// there is no other work to be done by firmware.  In general, the idle thread
// will poll without a timeout, but this behavior is not guaranteed, and may
// poll with an indeterminate interval up to one second.

void cfa_InstallGetTimeToSleep(CFA_GET_TIME_TO_SLEEP_FP callback);
// This function installs a callback which will get called every time
// the system has an option to go to sleep. The callback must return a
// value in microseconds.  "0" means "don't sleep"; a large value
// would mean "app is OK with sleeping for a long time".  The system
// takes the minimum of the various sources; the callback can force
// the system to never sleeps, but it can't force the system to sleep
// if other things are going on. Small values (e.g. 1000 us) will be
// interpreted as "don't sleep". When no callback is installed, the
// system behaves as if the callback always returned PMU_MAX_TIME_TO_SLEEP 
// i.e. the system will assume that CSA is OK with sleeping.

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.2     Configuration Data VS
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.2.1       Types
//------------------------------------------------------------------------------
typedef BOOL32 (*CFA_VS_READ_CB)(   UINT8 vsID,
                                    UINT8 itemLength,
                                    UINT8* payload );
// The CFA_VS_READ_CB type is a function pointer for a callback from the
// cfa_ConfigVSReadCB function.

//------------------------------------------------------------------------------
// 10.2.2       Configuration Data VS Access Functions
//------------------------------------------------------------------------------
UINT8 cfa_ConfigVSRead( UINT8 vsID, UINT8 itemLength, UINT8 *payload );
// Reads an item from the configuration data volatile section.  Returns the
// number of bytes actually read, or 0 if the item was not present.  The maximum
// valid vsID is 0x7D, with 0x7E and 0x7F being reserved.

//------------------------------------------------------------------------------
UINT8 cfa_ConfigVSReadCB(   CFA_VS_READ_CB callback,
                            UINT8 maxItemLength,
                            BYTE* payloadBuffer );
// Reads all valid items with a length less than or equal to maxItemLength from
// the configuration data volatile section.  For each valid item, the callback
// function is invoked indicting the item's ID tag, valid payload length, and
// payload.  For any given invocation of the callback, if the callback function
// returns TRUE, reading the VS is terminated and the item length is returned
// from cfa_ConfigVSReadCB, with the payloadBuffer still containing the item's
// payload.  If the callback function returned FALSE to every item,
// cfa_ConfigVSReadCB returns 0.

//------------------------------------------------------------------------------
UINT8 cfa_ConfigVSWrite( UINT8 vsID, UINT8 itemLength, UINT8 *payload );
// Writes an item to the configuration data volatile section.  This function may
// not be called prior to when the post-VS init function is called, or would
// have been called if that init function is not installed (see Table 2).
// Returns the number of bytes actually written.  If the number of bytes written
// is less than what was requested, the volatile section is full and needs to be
// compacted.  Compacting the volatile section is performed in the idle thread,
// once all connections have exited sniff.  In the event that the volatile
// section needs to be compacted, the CSA should attempt to write the VS item
// again after a timeout period of a few seconds.  If memory is available,
// cfa_ConfigVSWrite can buffer up to four writes in RAM in the event that
// compaction is necessary.  In that case, cfa_ConfigVSWrite will return an
// indication that the number of bytes written was equal to itemLength.  The
// maximum valid vsID is 0x7B, with 0x7C, 0x7D, 0x7E and 0x7F being reserved.

//------------------------------------------------------------------------------
BOOL32 cfa_ConfigVSDelete(UINT8 vsID);
// Deletes an item in the configuration data volatile section.  Returns TRUE on
// success, FALSE on failure.  A FALSE indication generally indicates that the
// item was not present.  Typically, an item will be overwritten using
// cfa_ConfigVSWrite rather than deleted, and deleting an item is not necessary
// prior to overwriting it: the old item is implicitly deleted.

//------------------------------------------------------------------------------
UINT16 cfa_ConfigVSReadLongItem(    UINT8 controlID,
                                    UINT8 numReservedIDs,
                                    UINT16 itemLength,
                                    UINT8* payload );
// Reads a long item from the configuration data volatile section.  The item
// is actually stored in multiple vsIDs immediately after controlID, which must
// be reserved for the purpose.  The maximum length is (numReservedIDs-1)*255.
// For example if controlID is 5 and numReservedIDs is 3, vsIDs 5, 6 and 7 must
// be reserved for this long item and the maximum itemLength is 510.  In the
// event of success, returns the number of bytes read, or 0 in the event of
// failure or if the controlID or a required fragment is absent.

//------------------------------------------------------------------------------
UINT16 cfa_ConfigVSWriteLongItem(   UINT8 controlID,
                                    UINT8 numReservedIDs,
                                    UINT16 itemLength,
                                    UINT8* payload );
// Writes a long item to the configuration data volatile section.  The item
// is actually stored in multiple vsIDs immediately after controlID, which must
// be reserved for the purpose.  The maximum length is (numReservedIDs-1)*255.
// For example if controlID is 5 and numReservedIDs is 3, vsIDs 5, 6 and 7 must
// be reserved for this long item and the maximum itemLength is 510.  In the
// event of success, returns the number of bytes written, or 0 in the event of
// failure.



#ifdef CONFIG_RAMBUF
UINT8 cfa_ConfigRambufRead( UINT8 vsID, UINT8 itemLength, UINT8* payload );
UINT8 cfa_ConfigRambufWrite(UINT8 vsID, UINT8 itemLength, UINT8 *payload);
BOOL32 cfa_ConfigRambufSDelete(UINT8 vsID);
#endif


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.3     Kernel Abstraction Layer (KAL)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.3.1       Types
//------------------------------------------------------------------------------
typedef struct {long tx_private[8];} CFA_EVENT_GROUP;
typedef struct {long tx_private[5];} CFA_TIMER;
typedef struct {long tx_private[14];} CFA_QUEUE;
typedef void (*CFA_TIMER_CB)(INT32 callbackArg);

//------------------------------------------------------------------------------
// 10.3.2       Event Functions
//------------------------------------------------------------------------------
void cfa_kal_CreateEventGroup(CFA_EVENT_GROUP* eventGroupPtr);
// This interface creates a Event Group. It takes a pointer to a Event Group
// data structure as parameter. The pointer should point to a piece of memory
// that is allocated for the event group data structure.

//------------------------------------------------------------------------------
void cfa_kal_SetEvent( CFA_EVENT_GROUP* eventGroupPtr,
                       UINT32 eventBitMask );
// This interface set the event flag to the Event Group. The first parameter is
// the pointer to a Event Group data structure. The second parameter is the
// event bit flag that will be set.

//------------------------------------------------------------------------------
UINT32 cfa_kal_WaitEvent( CFA_EVENT_GROUP* eventGroupPtr,
                          UINT32 eventBitMask,
                          UINT32 clearEventFlag,
                          UINT32 waitForever );
// This interface waits for one or more event flags to be set. The first
// parameter is a pointer to the event group. The second parameter is the event
// flags to wait for. The third parameter determines if the event flags will be
// cleared or not, and whether the wait should return on any bits in
// eventBitMask being set or wait for all bits in eventBitMask to be set:
#define CFA_EVENT_OR        0
#define CFA_EVENT_OR_CLEAR  1
#define CFA_EVENT_AND       2
#define CFA_EVENT_AND_CLEAR 3
// Typically, CFA_EVENT_OR_CLEAR is used, causing cfa_kal_WaitEvent to return
// when any of the event bits in eventBitMask are set, and clearing in the event
// group all bits which are indicated as being set in the return value.  Note
// that more than one bit may be set simultaneously in the return value.
// The fourth parameter determines if the thread should wait forever or return
// immediately if no bits are set:
#define CFA_EVENT_WAIT_FOREVER 0xFFFFFFFF
#define CFA_EVENT_NO_WAIT      0

//------------------------------------------------------------------------------
// 10.3.3       Timer Functions
//------------------------------------------------------------------------------
void cfa_kal_CreateTimer( CFA_TIMER* timer,
                          CFA_TIMER_CB callback,
                          INT32 callbackArg );
// Create a software timer. The newly create timer is in de-activated state.
// cfa_kal_ActivateTimer() is required to start the timer.  Unless the work to
// be done by the callback can be completed in a few clock cycles, the callback
// function should not take any action other than setting an event for
// processing in a thread of an appropriate priority to handle the event.

//------------------------------------------------------------------------------
void cfa_kal_CreatePeriodicTimer( CFA_TIMER* timer,
                                  CFA_TIMER_CB callback,
                                  INT32 callbackArg,
                                  UINT32 timerIntervalTicks );
// Create a software timer. The newly create timer is in de-activated state.
// cfa_kal_ActivateTimer() is required to start the timer.  Unless the work to
// be done by the callback can be completed in a few clock cycles, the callback
// function should not take any action other than setting an event for
// processing in a thread of an appropriate priority to handle the event.

//------------------------------------------------------------------------------
void cfa_kal_ChangeTimer( CFA_TIMER* timer, INT32 callbackArg );
// Change the callbackArg of a software timer.

//------------------------------------------------------------------------------
void cfa_kal_ActivateTimer( CFA_TIMER* timer, UINT32 ticksUntilExpiration );
// Activate a software timer.

//------------------------------------------------------------------------------
void cfa_kal_DeactivateTimer(CFA_TIMER* timer);
// Deactivate a software timer.

BOOL32 cfa_kal_IsTimerActive(CFA_TIMER* timer);
// check wether timer running

//------------------------------------------------------------------------------
// 10.3.4       Queue Functions
//------------------------------------------------------------------------------
void cfa_kal_CreateQueue( CFA_QUEUE* queue, void* dataArea, UINT32 queueSize );
// This interface creates a queue. The first parameter is a pointer to the Queue
// Control data structure. The second parameter is the queue storage area. The
// queue item can only be a pointer, the queue storage area is sizeof(void *)
// multiple by queueSize. The third parameter is queueSize. This is the maximum
// number of queue items the queue can store.

//------------------------------------------------------------------------------
void cfa_kal_SendQueueItem( CFA_QUEUE* queueQcbPtr, void* queueItemPtr );
// This interface put an item on the queue. The first parameter is a Queue
// Control data structure. The second parameter is the pointer to the item that
// will be put on the queue. The Queue system is not making a copy of the queue
// item. The caller should not release the memory that is being put on the
// queue.

//------------------------------------------------------------------------------
void* cfa_kal_GetQueueItem(CFA_QUEUE* queueQcbPtr);
// This interface returns a void pointer to a item removed from the queue. If
// the queue is empty, the return pointer is NULL. The caller of this function
// need to release the memory after it is done with the data structure.  This
// function takes a Queue Control data structure as parameter.

//------------------------------------------------------------------------------
BOOL32 cfa_kal_IsQueueEmpty(CFA_QUEUE* queueQcbPtr);
// This interface returns TRUE if the queue is empty. Otherwise it returns
// FALSE.  This function is faster in determining if the queue is empty or not.
// This function takes a Queue Control data structure as parameter.

UINT32 cfa_kal_GetNumQueuedMsgs(CFA_QUEUE* queueQcbPtr);
// This interface returns the number of queud messages in a specified queue.

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.4       Memory Manager (MM)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void* cfa_mm_Alloc(UINT32 length);
// Allocates memory from the heap.

//------------------------------------------------------------------------------
void cfa_mm_Free(void *memoryBlock);
// Returns memory to the heap.

//------------------------------------------------------------------------------
void* cfa_mm_Sbrk(UINT32 length);
// Moves up the top of used memory by length, and returns a pointer to the 
// resulting space

UINT32 cfa_mm_MemFreeBytes(void);
// Returns the numebr of free bytes of RAM left.

//------------------------------------------------------------------------------
UINT32 cfa_GetCSAStackSizeLowWaterMark(void);
// Debugging function for retrieving the current low-water-mark of the HCI
// thread's stack

UINT32 cfa_GetCSARemainingStackBytes(void);
// Debugging function for retrieving the currently remaining bytes of the HCI
// thread's stack


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.5       Host Controller Interface (HCI)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.5.1       Types
//------------------------------------------------------------------------------
#pragma pack(1)
typedef PACKED struct
{
    UINT16 opcode;
    UINT8 length;
    /* parameter payload follows */
} HCI_CMD_HDR;
#pragma pack()

#pragma pack(1)
typedef PACKED struct
{
    UINT8 eventCode;
    UINT8 length;
    /* parameter payload follows */
} HCI_EVT_HDR;
#pragma pack()

#pragma pack(1)
typedef PACKED struct
{
    UINT16 connectionHandleEtc;
    UINT16 length;
    /* data payload follows */
} HCI_ACL_HDR;
#pragma pack()


#define BD_ADDR_LEN     6

#pragma pack(1)
typedef UINT8 CFA_BD_ADDR[BD_ADDR_LEN];
#pragma pack()

#pragma pack(1)
typedef PACKED struct
{
    HCI_EVT_HDR     header;                // event header
    UINT8           numResponses;
    CFA_BD_ADDR     bdAddr;
    UINT8           pageScanRepetitionMode;
    UINT8           pageScanPeriodMode;
    UINT8           pageScanMode;
    UINT8           classOfDevice[3];
    UINT16          clockOffset;

} HCI_EVT_INQUIRY_RESULT;
#pragma pack()

#pragma pack(1)
typedef PACKED struct
{
    HCI_EVT_HDR     header;                // event header
    UINT8           numResponses;
    CFA_BD_ADDR     bdAddr;
    UINT8           pageScanRepetitionMode;
    UINT8           pageScanPeriodMode;
    UINT8           classOfDevice[3];
    UINT16          clockOffset;
    INT8            rssi;

} HCI_EVT_INQUIRY_RESULT_WITH_RSSI;
#pragma pack()

#define EIR_MAX_PAYLOAD_LEN 240

#pragma pack(1)
typedef PACKED struct
{
    HCI_EVT_HDR     header;                // event header
    UINT8           numResponses;
    CFA_BD_ADDR     bdAddr;
    UINT8           pageScanRepetitionMode;
    UINT8           pageScanPeriodMode;
    UINT8           classOfDevice[3];
    UINT16          clockOffset;
    INT8            rssi;
    UINT8           ird[EIR_MAX_PAYLOAD_LEN];

} HCI_EVT_EXTENDED_INQUIRY_RESULT;
#pragma pack()

#pragma pack(1)
// connectionDescriptorArray parameter
typedef PACKED struct
{
    UINT16           connHandle;          // connection handle
    UINT16           numCompletedPackets; // the number of HCI Data Packets completed since previous time
} HCI_PARAM_CONN_DESCRIPTOR;

typedef PACKED struct
{
    HCI_EVT_HDR             header;     // event header
    UINT8                   numHandles; // number of connection handles

    // variable size array of HCI_PARAM_CONNECTION_DESCRIPTOR (at least one entry)
    HCI_PARAM_CONN_DESCRIPTOR  descriptor [1];  // array of connection parameters
} HCI_EVT_NUM_COMPLETED_PACKETS;
#pragma pack()

typedef enum
{
    CFA_FILTER_PROCESS_NORMALLY = 0,
    CFA_FILTER_SUPPRESS_AND_FREE = 1,
    CFA_FILTER_SUPPRESS_AND_PASS_TO_CSA_THREAD = 2,
    CFA_FILTER_SUPPRESS_AND_PASS_TO_HCI_THREAD =
            CFA_FILTER_SUPPRESS_AND_PASS_TO_CSA_THREAD,
    CFA_FILTER_SUPPRESS_AND_IGNORE,
} CFA_FILTER_RSP;

typedef CFA_FILTER_RSP (*CFA_HCICMD_HANDLER_CB)(const HCI_CMD_HDR* command);
typedef CFA_FILTER_RSP (*CFA_HCIEVT_HANDLER_CB)(const HCI_EVT_HDR* event);
typedef CFA_FILTER_RSP (*CFA_HCIACL_HANDLER_CB)(const HCI_ACL_HDR* acl);
//! Applications can define the handler which will be called in cfa context
typedef void (*CFA_DEFER_HANDLER_CB) (UINT32 context);

// The handler callback function types are functions used to handle either HCI
// commands received from the host on the Bluetooth device's HCI command
// endpoint, ACL data received by the baseband and about to be sent to the host,
// or events about to be sent to the host by firmware on the Bluetooth device's
// HCI event endpoint.  Data will be routed to a function of this type if a
// corresponding filter function returned a value of
// CFA_FILTER_SUPPRESS_AND_PASS_TO_HCI_THREAD.

typedef CFA_FILTER_RSP (*CFA_HCICMD_FILTER_CB)(
                                            const HCI_CMD_HDR* command,
                                            CFA_HCICMD_HANDLER_CB* handler );
typedef CFA_FILTER_RSP (*CFA_HCIEVT_FILTER_CB)(
                                            const HCI_EVT_HDR* event,
                                            CFA_HCIEVT_HANDLER_CB* handler );

#if defined(BT_FW_AUDIO_CORE)
typedef CFA_FILTER_RSP (*CFA_HCIACL_FILTER_CB)(
                                            const HCI_ACL_HDR* acl,
                                            UINT16  len,
                                            CFA_HCIACL_HANDLER_CB* handler );
#else
typedef CFA_FILTER_RSP (*CFA_HCIACL_FILTER_CB)(
                                            const HCI_ACL_HDR* acl,
                                            CFA_HCIACL_HANDLER_CB* handler );
#endif
// The filter callback function types are functions used to filter either HCI
// commands received from the host on the Bluetooth device's HCI command
// endpoint, ACL data received by the baseband and about to be sent to the host,
// or events about to be sent to the host by firmware on the Bluetooth device's
// HCI event endpoint, referred to aggregately as CF-CSA input.  An installed
// filter function should return CFA_FILTER_PROCESS_NORMALLY if the calling
// firmware function should process the CF-CSA input normally,
// CFA_FILTER_SUPPRESS_AND_FREE if the calling function should take no action
// other than freeing the buffer, or CFA_FILTER_SUPPRESS_AND_PASS_TO_HCI_THREAD
// if the core firmware should not process the CF-CSA input, but should pass the
// CF-CSA input to the HCI thread for a CSA handler function to process the
// CF-CSA input in the HCI thread context.  The filter callback should do no
// actual processing of data, only filtering or routing to the HCI thread based
// on state information of the CSA which is read or written atomically and
// therefore needs no data protection.  To this end, the following clock cycle
// limitations are enforced:
//
// CFA_HCICMD_FILTER_CB:    250 clock cycles
// CFA_HCIEVT_FILTER_CB:    250 clock cycles
// CFA_HCIACL_FILTER_CB:    100 clock cycles
//
// CSA authors are encouraged to write single-threaded code for execution in the
// HCI thread, with the only synchronization necessary being simple decision
// making in the filter functions based on state information which can be read
// atomically.  In the event that the filter callback returns
// CFA_FILTER_SUPPRESS_AND_PASS_TO_HCI_THREAD, *handler must be set to the
// address of a handler function.  In that event, the CSA must also have
// instantiated an HCI deferral queue (see section 10.5.2).

typedef enum
{
    CFA_HANDLER_TYPE_CMD = 0x00000000,
    CFA_HANDLER_TYPE_EVT = 0x40000000,
    CFA_HANDLER_TYPE_ACLTX = (int)0x80000000,
    CFA_HANDLER_TYPE_ACLRX = (int)0xC0000000,
    // Note: This message type is used to defer the call.
    CFA_HANDLER_TYPE_DEFER = 0x20000000,
} CFA_HANDLER_TYPE;

typedef struct
{
    UINT32 bufferAndType;
    #define CFA_HCI_DEFERRAL_ENTRY_BUFFER_MASK 0x1FFFFFFF
    #define CFA_HCI_DEFERRAL_ENTRY_TYPE_MASK 0xE0000000
    union
    {
        CFA_HCICMD_HANDLER_CB   cmdCallback;
        CFA_HCIEVT_HANDLER_CB   evtCallback;
        CFA_HCIACL_HANDLER_CB   aclCallback;
        CFA_DEFER_HANDLER_CB    deferCallback;
        void*                   abstractCallback;
    } u;
}
CFA_HCI_DEFERRAL_ENTRY;

//------------------------------------------------------------------------------
// 10.5.2       Installing an HCI Thread Deferral Queue
//------------------------------------------------------------------------------
void cfa_hci_InstallHCIThreadDeferralQueue( CFA_QUEUE* queue,
                                            CFA_HCI_DEFERRAL_ENTRY* dataArea,
                                            UINT32 queueSizeEntries );
// Installs an HCI Thread Deferral Queue to push HCI commands, events, and
// ACL data, both Tx and Rx, to the HCI thread for more intensive processing
// than what is allowed in the callbacks.

//------------------------------------------------------------------------------
// 10.5.3       HCI Command, Event, and ACL Data Filters
//------------------------------------------------------------------------------
void cfa_hci_InstallHCICmdFilter(CFA_HCICMD_FILTER_CB filterFunct);
// Provides the CSA with an opportunity to filter the HCI command.  See section
// 10.5.1 for a discussion of possible actions and the corresponding return
// values from within the filter function itself.

//------------------------------------------------------------------------------
void cfa_hci_InstallHCIEventFilter(CFA_HCIEVT_FILTER_CB filterFunct);
// Provides the CSA with an opportunity to filter the HCI event.  See section
// 10.5.1 for a discussion of possible actions and the corresponding return
// values from within the filter function itself.

//------------------------------------------------------------------------------
void cfa_hci_InstallACLRxFilter(CFA_HCIACL_FILTER_CB filterFunct);
// Provides the CSA with an opportunity to filter the received ACL data, with
// the ACL data packet having been received from over the air and destined for
// delivery to the host if the filter function returns
// CFA_FILTER_PROCESS_NORMALLY.  See section 10.5.1 for a discussion of possible
// actions and the corresponding return values from within the filter function
// itself.

//------------------------------------------------------------------------------
void cfa_hci_InstallACLTxFilter(CFA_HCIACL_FILTER_CB filterFunct);
// Provides the CSA with an opportunity to filter the received ACL data, with
// the ACL data packet having been received from the the host and destined for
// transmission over the air if the filter function returns
// CFA_FILTER_PROCESS_NORMALLY.  See section 10.5.1 for a discussion of possible
// actions and the corresponding return values from within the filter function
// itself.

//------------------------------------------------------------------------------
// 10.5.3       HCI Command, Event, and ACL Data Generation
//------------------------------------------------------------------------------
void cfa_hci_ExecuteHCICmd(const HCI_CMD_HDR* command);
// Enqueues an HCI command for processing by the HCI thread.  Although the
// command is processed by the same code and in the same thread which processes
// HCI commands received from a transport, an installed HCI command filter will
// not be invoked for programmatically enqueued HCI commands.  If the command
// buffer passed to this function was dynamically allocated, the caller is
// responsible for releasing it, and is free to do so after this function
// returns.
//
void cfa_hci_ExecuteHCICmd_No_Alloc(const HCI_CMD_HDR* command);
// Enqueues an HCI command for processing by the HCI thread.  Although the
// command is processed by the same code and in the same thread which processes
// HCI commands received from a transport, an installed HCI command filter will
// not be invoked for programmatically enqueued HCI commands.  If the command
// buffer passed to this function was dynamically allocated, the caller is
// responsible for releasing it, and is free to do so after this function
// returns.
// The only difference between the other similarly named function, 
// "cfa_hci_ExecuteHCICmd" is that this function rely on the caller to allocate
// memory.

//------------------------------------------------------------------------------
void cfa_hci_SendHCIEvent(const HCI_EVT_HDR* event);
// Enqueues an HCI event to be sent to the host over the transport.  Although
// the event is sent to the host by the same mechanism which would handle
// sending events generated by the core firmware, an installed HCI event filter
// will not be invoked for events enqueued by a CSA.  If the event buffer passed
// to this function was dynamically allocated, the caller is responsible for
// releasing it, and is free to do so after this function returns.

//------------------------------------------------------------------------------
void cfa_hci_SendHCIEventNoAlloc(const HCI_EVT_HDR* event);
// Enqueues an HCI event to be sent to the host over the transport.
// Although the event is sent to the host by the same mechanism which
// would handle sending events generated by the core firmware, an
// installed HCI event filter will not be invoked for events enqueued
// by a CSA.  The event buffer passed to this function should have been
// dynamically allocated, the caller should not release it,
// and also should not modify the contents.


//------------------------------------------------------------------------------
HCI_ACL_HDR* cfa_hci_AllocateACL( UINT16 payloadSize, BOOL32 forTx );
// Allocates a block of memory for ACL Tx or Rx use.  Tx is for data to be
// transmitted over the air.  Rx is for data to be sent to the host.  The
// payloadSize parameter does not include the size of an ACL header.  The
// returned buffer will be of sufficient size of store both the four byte ACL
// header and the requested payload size.  This function may only be called from
// the HCI thread.

//------------------------------------------------------------------------------
HCI_ACL_HDR* cfa_hci_AllocateBLEACL( UINT16 payloadSize, BOOL32 forTx );
// Allocates a block of memory for BLE Tx or Rx use.  Tx is for data to be
// transmitted over the air.  Rx is for data to be sent to the host.  The
// payloadSize parameter does not include the size of an ACL header.  The
// returned buffer will be of sufficient size of store both the four byte ACL
// header and the requested payload size.  This function may only be called from
// the HCI thread.
//

// Frees a block of previously a allocated ACL buffer
void cfa_mm_FreeACLBuffer( void *ptr);

HCI_CMD_HDR* cfa_hci_AllocateHCICmdBuffer(const int payloadSize );
// Allocates a block of memory for HCI Cmd. This piece of memory should be passed
// to "cfa_hci_ExecuteHCICmd_No_Alloc".


//------------------------------------------------------------------------------
void cfa_hci_SendACLTx(HCI_ACL_HDR* acl);
// Enqueues an ACL data packet to be sent over the air.  Although the ACL data
// is enqueued by the same mechanism which would handle enqueuing ACL data
// received from the transport, an installed ACL Tx filter will not be invoked
// for ACL data enqueued by a CSA.  The ACL data block must have been allocated
// using cfa_AllocateACL with the forTx parameter having been TRUE.  Once this
// function has been called, the caller no longer owns the acl data buffer block
// and may not use or release it.

//------------------------------------------------------------------------------
void cfa_hci_SendACLRx(HCI_ACL_HDR* acl);
// Enqueues an ACL data packet to be sent to the host over the transport.
// Although the ACL data is enqueued by the same mechanism which would handle
// enqueuing ACL data received from the air, an installed ACL Rx Filter will not
// be invoked for ACL data enqueued by a CSA.  The ACL data block must have been
// allocated using cfa_AllocateACL with the forTx parameter having been FALSE.
// Once this function has been called, the caller no longer owns the acl data
// buffer block and may not use or release it.

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.6     USB
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.6.1       Types
//------------------------------------------------------------------------------
typedef enum
{
    CFA_UII_BT_DEVICE_DESC = 0,         // BT device descriptor
    CFA_UII_BT_CONFIG_DESC = 1,         // BT configuration descriptor
    CFA_UII_KB_DEVICE_DESC = 2,         // KB device descriptor
    CFA_UII_KB_CONFIG_DESC = 3,         // KB configuration descriptor
    CFA_UII_KB_REPORT_DESC = 4,         // KB report descriptor
    CFA_UII_MS_DEVICE_DESC = 5,         // MS configuration descriptor
    CFA_UII_MS_CONFIG_DESC = 6,         // MS configuration descriptor
    CFA_UII_MS_REPORT_DESC = 7,         // MS report descriptor
    CFA_UII_LANG_ID_STRING = 8,         // Language ID for all ports
    CFA_UII_MANUF_ID_STRING = 9,        // Manufacturer ID for all ports
    CFA_UII_PROD_ID_STRING = 10,        // Product ID for all ports
    CFA_UII_SER_NUM_ID_STRING = 11,     // Serial Number for all ports
    CFA_UII_HUB_DEVICE_DESC = 12,       // Hub Device descriptor
    CFA_UII_HUB_CONFIG_DESC = 13,       // Hub Configuration descriptor
    CFA_UII_HUB_DESC = 14,              // Hub descriptor
    CFA_UII_HUB_MANUF_ID_STRING = 15,   // Hub manufacturer ID string
    CFA_UII_HUB_PROD_ID_STRING = 16,    // Hub product ID string
    CFA_UII_DFU_DEVICE_DESC = 17,       // DFU device descriptor
    CFA_UII_DFU_CONFIG_DESC = 18,       // DFU configuration descriptor
    CFA_UII_DFU_MANUF_ID_STRING = 19,   // DFU manufacturer ID string
    CFA_UII_DFU_PROD_ID_STRING = 20,     // DFU product ID string
#if defined(USB_GENERIC_HID)
    CFA_UII_GENERIC_HID_DEVICE_DESC = 21,   // Generic HID device descriptor
    CFA_UII_GENERIC_HID_CONFIG_DESC = 22,   // Generic HID configuration descriptor
    CFA_UII_GENERIC_HID_REPORT_DESC = 23,   // Generic HID report descriptor
#endif
    CFA_UII_NUM_ENTRIES,
} CFA_USB_INFO_ID;

typedef enum
{
    CFA_UHFI_KB_REPORT_FUNC = 0,
    CFA_UHFI_KB_PROTOCOL_FUNC = 1,
    CFA_UHFI_KB_IDLE_RATE_FUNC = 2,
    CFA_UHFI_MS_REPORT_FUNC = 3,
    CFA_UHFI_MS_PROTOCOL_FUNC = 4,
    CFA_UHFI_MS_IDLE_RATE_FUNC = 5,
} CFA_USB_HID_FUNC_ID;


typedef enum
{
    CFA_USB_PORT_BT = 0,
    CFA_USB_PORT_KB = 1,
    CFA_USB_PORT_MS = 2,
    CFA_USB_PORT_4  = 3,
    CFA_USB_PORT_HUB = 4,
}CFA_USB_PORT_ID;

#define CFA_USB_CLASS_HID_GET_REPORT            0x01
#define CFA_USB_CLASS_HID_GET_IDLE              0x02
#define CFA_USB_CLASS_HID_GET_PROTOCOL          0x03
#define CFA_USB_CLASS_HID_SET_REPORT            0x09
#define CFA_USB_CLASS_HID_SET_IDLE              0x0A
#define CFA_USB_CLASS_HID_SET_PROTOCOL          0x0B


typedef BOOL32 (*CFA_USB_GETHID_CB)(    CFA_USB_HID_FUNC_ID id,
                                        UINT8 *cmdPtr,
                                        UINT8 **dataPtr,
                                        UINT32 *len );

typedef BOOL32 (*CFA_USB_SETHID_CB)(    CFA_USB_HID_FUNC_ID id,
                                        UINT8 *cmdPtr,
                                        UINT8 *dataPtr,
                                        UINT32 len );

typedef BOOL32 (*CFA_USB_GETSTRINGDESC_CB)(CFA_USB_PORT_ID portID,
                                         UINT8 *cmdPtr,
                                         UINT8 **dataPtr);

typedef void (*CFA_USB_SUSPEND_CB)(CFA_USB_PORT_ID portID);
typedef void (*CFA_USB_RESUME_CB)(CFA_USB_PORT_ID portID);
typedef void (*CFA_USB_PORT_RESET_CB)(CFA_USB_PORT_ID portID);

typedef void (*CFA_USB_ENABLE_UHE_CB)(BOOL32 enable, BOOL32 permanent);

typedef void (*CFA_USB_ENUM_DONE_CB)(CFA_USB_PORT_ID portID);
typedef void (*CFA_USB_SOF_CB)(void);

#if defined(USB_GENERIC_HID)
#define CFA_USB_PORT1_EP0_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT1_EP1_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT1_EP2_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT1_EP3_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT1_EP4_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT2_EP0_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT2_EP1_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT3_EP0_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)
#define CFA_USB_PORT3_EP1_TX_QUEUE_SIZE     ((sizeof(void *)) * 32)

#define CFA_USB_PORT_1  CFA_USB_PORT_BT
#define CFA_USB_PORT_2  CFA_USB_PORT_KB
#define CFA_USB_PORT_3  CFA_USB_PORT_MS

typedef enum
{
    CFA_USB_HID_MODE = 0,
    CFA_USB_HCI_MODE = 1,
    CFA_USB_DFU_MODE = 2,
    CFA_USB_OTHER_MODE = 3,
} CFA_USB_MODE;

typedef enum
{
    CFA_USB_EP0 = 0,
    CFA_USB_EP1 = 1,
    CFA_USB_EP2 = 2,
    CFA_USB_EP3 = 3,
    CFA_USB_EP4 = 4,
    CFA_USB_EP_NUM
}CFA_USB_END_POINT_ID;

typedef void (*CFA_USB_RX_CB) (CFA_USB_PORT_ID portID, CFA_USB_END_POINT_ID endPointID, UINT8 *bufPtr, int len);
typedef void (*CFA_USB_HID_CLASS_COMMAND_CB) (CFA_USB_PORT_ID portID, UINT8 *pCmdBuf, UINT8 *bufPtr, int len);
typedef void (*CFA_USB_TX_DONE_CB) (void* cbdata, BOOL32 status);
typedef struct
{
    // use 4 bytes to get the structure aligned.
    UINT32 len;
    void*  dataPtr;
    CFA_USB_TX_DONE_CB txDoneFncPtr;
    void *cbdata;
} CFA_USB_TX_QUE_MSG;

void cfa_usb_createTxQueues(void);
void cfa_usb_InstallRxCb(CFA_USB_RX_CB cb);
void cfa_usb_InstallHidClassCommandCb(CFA_USB_HID_CLASS_COMMAND_CB cb);
void cfa_usb_InstallSofCb(CFA_USB_SOF_CB cb, int interval);
void cfa_usb_SwitchMode(CFA_USB_MODE mode);
UINT8 cfa_usb_GetRuntimeMode(void);
void cfa_usb_SendPacket(CFA_USB_PORT_ID port,CFA_USB_END_POINT_ID endPoint,UINT8 *bufPtr,UINT32 len,CFA_USB_TX_DONE_CB cb,void* cbdata);
void cfa_usb_AttachToBus(void);
void cfa_usb_DetachFromBus(void);
void cfa_usb_SendAck(CFA_USB_PORT_ID port);
void cfa_usb_SendStall(CFA_USB_PORT_ID port);

#endif

//------------------------------------------------------------------------------
// 10.6.2   Interface Functions
//------------------------------------------------------------------------------
void cfa_usb_SetInformation( CFA_USB_INFO_ID id, const UINT8* dataPtr, UINT8 len );
// This routine allows an outside module to override the information that
// defines how the USB device looks to the local Host.  It is expected that the
// memory pointed to by dataPtr will remain in context throughout system
// operation.  In most cases, the USB driver will not make its own local copy of
// the data provided.
//
// All strings must be in 16-bit Unicode format with the LSB first in the pair.
//
// Report descriptors must consist of a 16-bit length, followed by the actual
// descriptor returned in response to report descriptor requests over USB.

//------------------------------------------------------------------------------
void cfa_usb_SetHidCallBack_Set_Func(CFA_USB_SETHID_CB callback);
void cfa_usb_SetHidCallBack_Get_Func(CFA_USB_GETHID_CB callback);
// This routine registers callback routines that process commands from the host
// that perform set or get functions.  The direction of the command is indicated
// by the getFlag variable.  If no callback value is set the default handler is
// executed.
//
// The memory buffer passed in to the CFA must not be depended on to remain the
// in the possesion of the CFA.  The usb driver will free this memory.

//------------------------------------------------------------------------------
void cfa_usb_SetSuspendCallBackFunc(CFA_USB_SUSPEND_CB callback);
void cfa_usb_SetResumeCallBackFunc(CFA_USB_RESUME_CB callback);
// To have the USB driver call a routine when the host suspends/resumes a port,
// call this routine and pass in a pointer to a function to handle the event.
// It takes one parameter indicating which port received the suspend/resume
// event.

//------------------------------------------------------------------------------
void cfa_usb_SetPortResetCallBackFunc(CFA_USB_PORT_RESET_CB callback);
// To have the USB driver call a routine when the host resets a port, call
// this routine and pass in a pointer to a function to handle the event.  It
// takes one parameter indicating which port received the reset event.

//------------------------------------------------------------------------------
void cfa_usb_SetStringDescGetCallBackFunc(CFA_USB_GETSTRINGDESC_CB callback);
// To have the USB driver call a routine when the host asks for
// a string descriptor.  The application will provide the pointer
// of the string and the length.  If the application decides that
// the request is an error, it sets the return value to false
// which causes the usb driver to stall the response.

//------------------------------------------------------------------------------
BOOL32 cfa_usb_IsEnumerated (CFA_USB_PORT_ID portID);
// Allows caller to find out if a port is enumerated.

//------------------------------------------------------------------------------
BOOL32 cfa_tran_IsActive (CFA_USB_PORT_ID portID);
//Allows caller to find if underlying transport is active


//------------------------------------------------------------------------------
void cfa_usb_PortControl( CFA_USB_PORT_ID portID, BOOL32 enableFlag );
// Allows caller to attach and deattach ports 1 thru 3.  Port 4 is controllable
// only at boot time through the config record, as is the hub.

//------------------------------------------------------------------------------
void cfa_usb_EnableHub(void);
// Allows caller to enable the hub, but this may only be done from a preDS init
// function or CFA init function called during dynamic section processing.

//------------------------------------------------------------------------------
void cfa_usb_InstallEnableUHE(CFA_USB_ENABLE_UHE_CB enableUHE);
// This function is called to initialize the func ptr to enable/disable UHE
// from the core.

//------------------------------------------------------------------------------
void cfa_usb_InstallEnumerationDone(CFA_USB_ENUM_DONE_CB enumDoneCallback);
// This function is called to initialize the func ptr to callback func
// when a port is enumerated

//------------------------------------------------------------------------------
void cfa_usb_disableKeyboardZlp(void);
void cfa_usb_disableMouseZlp(void);
void cfa_usb_enableKeyboardZlp(void);
void cfa_usb_enableMouseZlp(void);
// Individually enable and disable the core's insertion of ZLP (zero length packet) 
// on endpoint-1 (EP_1) of the the mouse and keyboard ports

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.7 Transport
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
void cfa_tran_SendKeyBdEventToHost( const UINT8 *dataPtr, UINT8 len );
// Sends an RTOS buffer of event data to the Host on the Keyboard port of a
// specified length.  If the event buffer passed to this function was
// dynamically allocated, the caller is responsible for releasing it, and is
// free to do so after this function returns.

void cfa_tran_SendKeyBdClearEventToHost( UINT8 rep_id, UINT8 len );
// Sends an RTOS buffer of event data to the Host on the Mouse port of a
// specified length with zero data for given report ID.

//------------------------------------------------------------------------------
void cfa_tran_GetKeyBdEventQueueCount( UINT8* current, UINT8* max );
// Obtains the number of events currently in the keyboard event queue, and the
// maximum number of events which can be held by the queue.

//------------------------------------------------------------------------------
void cfa_tran_FlushKeyBdEventQueue(void);
// Flushes all pending keyboard events from the USB keyboard event queue.

//------------------------------------------------------------------------------
void cfa_tran_SendMouseEventToHost( const UINT8 *dataPtr, UINT8 len );
// Sends an RTOS buffer of event data to the Host on the Mouse port of a
// specified length.  If the event buffer passed to this function was
// dynamically allocated, the caller is responsible for releasing it, and is
// free to do so after this function returns.

void cfa_tran_SendMouseClearEventToHost( UINT8 rep_id, UINT8 len );
// Sends an RTOS buffer of event data to the Host on the Mouse port of a
// specified length with zero data for given report ID.

void cfa_tran_SendCtrlToHost( BOOLEAN mse_or_kbd, const UINT8 *dataPtr, UINT8 len );

#ifdef UHE_DEV_RECOVERY
void cfa_set_ps_suspend_inq_flag( BOOL flg );
#endif

#if !(defined(BB_2045B0) || defined(BB_2045B1) || defined(BB_2045B2))
BOOL32 cfa_usb_GetBtRemoteWakeStatus(void);
// Retuns the remote wake status for the bt port
BOOL32 cfa_usb_GetKeyboardRemoteWakeStatus(void);
// Retuns the remote wake status for the keyboard port
BOOL32 cfa_usb_GetMouseRemoteWakeStatus(void);
// Retuns the remote wake status for the mouse port
//
#endif

//------------------------------------------------------------------------------
void cfa_tran_GetMouseEventQueueCount( UINT8* current, UINT8* max );
// Obtains the number of events currently in the mouse event queue, and the
// maximum number of events which can be held by the queue.

//------------------------------------------------------------------------------
void cfa_tran_FlushMouseEventQueue(void);
// Flushes all pending mouse events from the USB mouse event queue.

//------------------------------------------------------------------------------
void cfa_tran_ResumeHost(CFA_USB_PORT_ID portID);
// Sends the remote wake signal to the host via the active transport.  It takes
// no parameters.

//------------------------------------------------------------------------------
CFA_TRAN_TRANSPORT_TYPE cfa_tran_GetActiveTransportType(void);
// Returns the transport type, or CFA_TRAN_UNDEFINED

#if defined(UHE_ENABLE) || defined(MPAF_ENABLE)
typedef BOOL32 (*CFA_IS_EMBEDDED_HOST_ENABLED_CB)(void);
extern CFA_IS_EMBEDDED_HOST_ENABLED_CB cfa_IsEmbeddedHostEnabledCb;
void cfa_InstallIsEmbeddedHostEnabledCb(CFA_IS_EMBEDDED_HOST_ENABLED_CB callback);
#endif

#ifdef UHE_ENABLE
void cfa_lm_TurnRadio(BOOL32);
#ifdef IT_MANAGER
void cfa_app_stack_shutdown_complete(void);
void cfa_hid_set_secure_mode_received(void);
#endif
#endif //UHE_ENABLE



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.8 Debug UART
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
void cfa_debug_InitUART(void);
// initialize the debug uart (115200, no h/w flow control).

#ifdef BLEAPP_SUPPORT
void cfa_debug_InitUARTBaud(UINT32 baud, UINT8 initprint);
// initialize the debug uart (baud, no h/w flow control).
#endif

//------------------------------------------------------------------------------
BOOL32 cfa_debug_WriteDWORD(UINT32 dword);
// Sends one DWORD (32 bits) over the debug UART if there is FIFO space
// available.  Returns FALSE if the debug UART FIFO is full.

//------------------------------------------------------------------------------
BOOL32 cfa_debug_ReadChar(UINT8* ch);
// Reads one byte from the debug UART if any are available.  Returns FALSE if no
// bytes are available.

//------------------------------------------------------------------------------
void cfa_debug_PrintString(char* string);
// Prints a sting to the debug UART.  Success is guaranteed, but the function
// will poll until completion so the operation could take a significant amount
// of time to complete.

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.9 GPIO Control
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.9.1       Types
//------------------------------------------------------------------------------
typedef enum
{
    CFA_GPIO_PIN_0 = GPIO_PIN_0,
    CFA_GPIO_PIN_1 = GPIO_PIN_1,
    CFA_GPIO_PIN_2 = GPIO_PIN_2,
    CFA_GPIO_PIN_3 = GPIO_PIN_3,
    CFA_GPIO_PIN_4 = GPIO_PIN_4,
    CFA_GPIO_PIN_5 = GPIO_PIN_5,
    CFA_GPIO_PIN_6 = GPIO_PIN_6,
    CFA_GPIO_PIN_7 = GPIO_PIN_7
} CFA_GPIO_PIN_ID;

//------------------------------------------------------------------------------
// 10.9.2       Interface Functions
//------------------------------------------------------------------------------
void cfa_gpio_SetDirection( CFA_GPIO_PIN_ID pin, BOOL32 output );
// Sets whether a given GPIO is an input or an output.

//------------------------------------------------------------------------------
void cfa_gpio_SetOutputValue( CFA_GPIO_PIN_ID pin, BOOL32 high );
// If the specified GPIO is an output, sets its output value to high or low.

//------------------------------------------------------------------------------
UINT8 cfa_gpio_GetInputOutputValues(void);
// Gets the value of all GPIO's.  Each bit in the returned byte represents a
// GPIO pin.  The pin/bit correlation is defined by CFA_GPIO_PIN_ID.  A one bit
// indicates that the GPIO pin is high.

#ifdef CONFIG_IN_NVRAM

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// 10.10 I2C Interface
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//------------------------------------------------------------------------------
// 10.10.1       Types
//------------------------------------------------------------------------------

#define CFA_BSC_CADDR_WCTL_ADR_SLAVE0   0xA0
#define CFA_BSC_CTL_DV_CLK_BY_4         (0x01 << 7)

#define CFA_BSC_CTL_SCL_400KHZ          (0x00 << 4)
#define CFA_BSC_CTL_SCL_100KHZ          (0x01 << 4)
#define CFA_BSC_CTL_SCL_800KHZ          (0x02 << 4)
#define CFA_BSC_CTL_SCL_1000KHZ         (0x03 << 4)
#define CFA_BSC_CTL_SCL_MASK            (0x03 << 4)

#define CFA_BSC_CTL_INT_ENBL            0x40
#define CFA_BSC_CNTL_DISABLE            0x00

// Operation
#define CFA_BSC_OP_READ     0
#define CFA_BSC_OP_WRITE    1

// Max transaction size
#define CFA_BSC_MAX_TRANSACTION_SIZE  8

typedef enum
{
    CFA_BSC_STATUS_INCOMPLETE = 0,
    CFA_BSC_STATUS_SUCCESS = 1,
    CFA_BSC_STATUS_NO_ACK = 3
}CFA_BSC_STATUS;

//------------------------------------------------------------------------------
// 10.9.2       Interface Functions
//------------------------------------------------------------------------------

void cfa_bsc_Configure(UINT8 newConfig);
// To configure the I2C serial interface. The configuration values include:
//          - Clock Rate
//          - Clock divider
//          - Delay disable
//          - Deglitch disable


CFA_BSC_STATUS cfa_bsc_OpExtended(UINT8* buf,
                         UINT16 bufCount,
                         UINT8* addr,
                         UINT8 addrCount,
                         UINT8 slaveAdr,
                         UINT8 operation);
//      This function performs blocking read/writes, both addressed and unaddressed
//      The amount of data read/written can exceed 8. The address is assumed to be in
//      big endian format.
//  Input:
//      - Pointer to data buffer. For reads, this is where data will be read into
//        For writes, data will be written from here.
//      - Number of bytes to read/write from the buffer.
//      - Pointer to address. Only valid if address count is non-zero; if address count is
//        zero, it must be set to 0. This is the address that is sent to the device
//        before the operation. In the case of a read, the address is written followed
//        by a read. For a write, the address is written, followed by the data.
//        The address is assumed to be in big endian format. This is pertinent only
//        if the operation requires multiple hardware operations to complete; otherwise
//        it is of no significance. The maximum number of bytes that can be read
//        in a single hardware transaction is 8 and the maximum that can be written
//        is 8 - address size. Note that the address will be modified
//      - Number of address bytes. If this is zero, no address is written before the
//        operation. If non-zero, the address is written before the read/write. In
//        case of a write, the size of the address plus the size of the data must be <= 8
//      - Slave address
//      - Operation (read/write)
#endif

#ifdef UHE_PPK

typedef struct
{
    UINT8*  Pk;
    UINT8*  RNH;
    UINT8*  Bd_addr;
} PPK_PIN_REQ_t;

typedef BOOL32 (*CFA_LM_PPK_PUBLIC_REQ_CB)(void);
typedef BOOL32 (*CFA_LM_PPK_PIN_REQ_CB)(void);

void cfa_lm_ppk_SetPublicCb(CFA_LM_PPK_PUBLIC_REQ_CB callback);
//To register callback function for lm
// Input/Output:
//   callback - The callback function to call
//
// Return: None


void cfa_lm_ppk_PublicRequest(void);
// To request Public key and random number generation
// Input/Output: None
// Return: None


UINT8* cfa_lm_ppk_PublicGet(void);
//To retrieve Pk and RNT from lm
// Input/Output: None
//
// Return: 
//   Pk - pointer of Public Key

UINT8* cfa_lm_ppk_RNTGet(void);
//To retrieve Pk and RNT from lm
// Input/Output: None
//
// Return: 
//   RNT - pointer of Random number
//
// RNT is deleted after calling cfa_lm_ppk_PinRequest()

void cfa_lm_ppk_SetPinCb(CFA_LM_PPK_PIN_REQ_CB callback);
//To register callback function for lm
// Input/Output:
//   callback - The callback function to call
//
// Return: None


void cfa_lm_ppk_PinRequest(UINT8 *Pk, UINT8* RNH, UINT8* Bd_addr);
// To request PIN Code generation
// Input/Output:
//   Pk - Pointer of the pulic key received from hid
//   RNH - Pointer of the random number received from hid
//   Bd_addr - Pointer of reversed form of Bd Addr
//
// Return: None
// Pk, RNH, Bd_addr are passed as pointer, 
// these pointer indicated location should be protected until callback function is called. 


UINT8* cfa_lm_ppk_PinGet(void);
//To retrieve Pin from lm
// Input/Output: None
//
// Return: 
//   Pin - pointer of Pin

#endif

UINT8* cfa_bdaddr_from_conhandle(UINT32 conhandle);
// To retireve bd_addr from connection handle
// Input/Output:
//   conhandle - Connection Handle
//
// Return:
//   bd_addr - pointer of UINT8 bd_addr[6];

#ifdef HIDD_LITE_STACK
UINT8* cfa_get_local_bdaddr(void);
void cfa_get_local_acl_buf_size(UINT16* aclbuf, UINT16* aclNum, UINT16* scoBuf, UINT16* scoNum);
void cfa_get_local_version_info(UINT8* hciVer, UINT16* hciRev, UINT8* lmpVer, UINT16* lmpSubver, UINT16* manuf);
UINT8* cfa_get_local_features(void);
#endif

#ifdef TBFC
BOOL32 cfa_lm_tbfc_startTbfcScan(BOOL32 onForEver);
BOOL32 cfa_lm_tbfc_stopTbfcScan(void);
#endif

enum
{
    UHE_APP = 0x00,
    NINTENDO_APP = 0x01,
    THREE_D_TV_APP = 0x02,
    THREE_D_PC_APP = 0x03,
    THREE_D_IR2BT_APP = 0x04,
    PNP_HEADSET_APP = 0x5
};

enum
{
    MODE_IN_USE_MONITOR_DEBUGGING = 0x00,
    MODE_IN_USE_FULL_CONTROL      = 0x01,
    MODE_IN_USE_HOSTLESS          = 0x02,
};

//------------------------------------------------------------------------------
//        Pet Watch Dog
//------------------------------------------------------------------------------
// This function will pet the watch dog.
void cfa_kickWatchdog(void);

#ifdef MERCURY
void cfa_mercuryStart(void (*callback)(UINT32));
void cfa_mercuryQueuePacket(void *mercury_data, UINT32 pkt_type);
void cfa_mercuryFlushPacket(void);
void cfa_mercuryStop(void);
void cfa_mercuryClearID(void);
#endif

#ifdef ITON
void cfa_itonStart(void (*callback)(UINT32 reason, UINT32 para), 
    UINT16 max_retry_in_msec, UINT16 max_idle_in_sec, UINT16 max_bind_retry_in_sec, UINT16 tx_power_level,
    UINT16 data_length, UINT16 kb_ms_mode);
void cfa_itonQueuePacket(void *iton_data);
void cfa_itonFlushPacket(void);
void cfa_itonStop(void);
void cfa_itonClearID(void);
#endif

#ifdef RAPOO
void cfa_rapooStart(void (*callback)(UINT32 reason, UINT32 para), 
    UINT16 max_retry_in_msec, UINT16 max_idle_in_sec, UINT16 max_bind_retry_in_sec, UINT16 tx_power_level, UINT8* id, 
    BOOL32 test_dongle);
void cfa_rapooQueuePacket(void *rapoo_data);
void cfa_rapooFlushPacket(void);
void cfa_rapooStop(void);
void cfa_rapooBind(void);
void cfa_rapooModulationTestMode(void);
void cfa_rapooDataTestMode(void);
void cfa_rapooTestModeFreq(UINT8 freq);
enum
{
    RAPOO_NONE_TEST,
    RAPOO_MODULATION_TEST,
    RAPOO_DATA_TEST
};
#endif

#if defined(MERCURY) || defined(ITON)
enum
{
    /// Volatile section for Dongle ID
    /// This will be removed and redefined under spar once spar related code is implemented!!
#ifdef MERCURY
    VS_MERCURY_DONGLE_ID,
#endif
#ifdef ITON
    VS_ITON_DONGLE_ID
#endif
};
#endif

#if defined(MERCURY) || defined(ITON) || defined(RAPOO)
enum
{
#ifdef MERCURY
    MERCURY_START,
    MERCURY_QUEUEPKT,
    MERCURY_FLUSHPKT,
    MERCURY_STOP,
    MERCURY_CLEARID,
#endif
#ifdef ITON
    ITON_START,
    ITON_QUEUEPKT,
    ITON_FLUSHPKT,
    ITON_STOP,
    ITON_CLEARID,
#endif
#ifdef RAPOO
    RAPOO_START,
    RAPOO_QUEUEPKT,
    RAPOO_FLUSHPKT,
    RAPOO_STOP,
    RAPOO_BIND,
    RAPOO_MODULATION_TEST_MODE,
    RAPOO_DATA_TEST_MODE,
#endif
};
#endif

#ifdef __cplusplus
}
#endif

#endif // _CFA_H_
