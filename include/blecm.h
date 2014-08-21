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
* This file provides definitions and function prototypes for the BLE
* Controller Manager (BLECM).
*
*/
#ifndef _BLECM_H_
#define _BLECM_H_

#include "cfa.h" // Get the Core Firmware API interfaces.
#include "bleapptimer.h" //
#include "dbguart.h" // debug uart.
#include "bleapp.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/
/** @defgroup blecm       Controller Manager
 *
 *  WICED Smart Controller Manager functions
 */
/*****************************************************************************/


//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////

typedef void(*BLECM_NO_PARAM_FUNC)(void);
typedef INT32(*BLECM_FUNC_WITH_PARAM)(void *); 



// Modules that can be activated.
#define BLECM_MODULE_SMP                                0x0
#define BLECM_MODULE_ATT                                0x1
#define BLECM_MODULE_MAX                                (BLECM_MODULE_ATT +1)


// This enum defined
typedef enum
{
    BLECM_APP_EVT_START_UP,     // 0x0
    BLECM_APP_EVT_LINK_UP,      // 0x1 this is the radio link up.
    BLECM_APP_EVT_LINK_DOWN,    // 0x2 this is the raio link down.
    BLECM_APP_EVT_ADV_TIMEOUT, //0x3 this is the adv stop based on timeout
    BLECM_APP_EVT_ENTERING_HIDOFF, // 0x4 this is to indicate that device is preparing to enter hid-ff
    BLECM_APP_EVT_ABORTING_HIDOFF, // 0x5 this is to indicate that the device had to abort entering hid-off
    BLECM_APP_EVT_MAX           // this is only used to indicate the size.
} BLECM_APP_EVT_ENUM;

#define BLECM_MAX_CFA_TIMERS                                0x4

#define BLECM_APP_NORMAL_TIMER_ID                           0x0
#define BLECM_APP_FINE_RESOLUTION_TIMER_ID                  0x1
#define BLECM_APP_LED_TIMER_ID                  0x2
#define BLECM_APP_BUZ_TIMER_ID                  0x3

#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

typedef PACKED struct
{
    UINT8 addr[6];
    UINT8 type;
}  BLECM_SELECT_ADDR;

typedef PACKED struct
{
    UINT16 con_handle;
    UINT16 db_size;
    void *p_db;
    void *dev_pinfo;
    void *smp_pinfo;
} BLECM_CON_MUX_INFO;

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()


//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
/**
* \brief BLECM Initialization
* \ingroup blecm
*
* \details This function performs BLECM variables initialization.  The function is
* called once during booting up.  Application typically does not need to call
* this function.
*
*/
void blecm_init(void);

/**
* \brief BLECM Start
* \ingroup blecm
*
* \details This function starts BLECM.  The function is called once during booting
* up.  Application typically does not need to call this function.
*
*/
void blecm_start(void);

/**
* \brief Enable/Disable CFA filter
* \ingroup blecm
*
* \details This function enables/disables CFA filter.  When CFA filter is set, all HCI
* commands, events and data are passed to the CFA/profile/application.  When CFA filter is
* not set, all HCI commands, events and data are passed to the transport (UART).
*
* \param enable If 0, CFA filter is not set. if 1, CFA filter is set.
*
*/
void blecm_setFilterEnable(UINT8 enable);

/**
* \brief Reading current CFA filter value
* \ingroup blecm
*
* \details This function returns current CFA filter state.  When CFA filter is set, all HCI
* commands, events and data are passed to the CFA/profile/application.  When CFA filter is
* not set, all HCI commands, events and data are passed to the transport (UART).
*
* \return enable If 0, CFA filter is not set. if 1, CFA filter is set.
*
*/
UINT8 blecm_getFilterEnable(void);


/**
* \brief Register Event Handler
* \ingroup blecm
*
* \details This function should be used before blecm_start is called. This function
* register the start up function for app.  Registered callback functions are called
* whenever related event happens.
*
* \param event_number BLECM_APP_EVT_ENUM
*   \li BLECM_APP_EVT_START_UP        - callback will be called during startup
*   \li BLECM_APP_EVT_LINK_UP         - callback will be called when the radio link comes up.
*   \li BLECM_APP_EVT_LINK_DOWN       - callback will be called radio link goes down.
*   \li BLECM_APP_EVT_ADV_TIMEOUT     - callback will be called when adv stop based on timeout
*   \li BLECM_APP_EVT_ENTERING_HIDOFF - callback will be called to indicate device is preparing to enter hid-ff
*   \li BLECM_APP_EVT_ABORTING_HIDOFF - callback will be called to indicate that the device had to abort entering hid-off
* \param function Callback function that is called whenever event happens
*
*/
void blecm_regAppEvtHandler(BLECM_APP_EVT_ENUM ,BLECM_NO_PARAM_FUNC func);

/**
* \brief Register for ACL Data receive
* \ingroup blecm
*
* \details This function will register a ACL Rx data handler.
*
* \param function Callback function that is called whenever Rx data packet is received
*
*/
void blecm_regAclRxHandler(void *);

/**
* \brief Register for Encryption Changed event
* \ingroup blecm
*
* \details This function will register a callback handler for
* Encryption changed Event.  Application can use this indication if it needs
* to send a packet over encrypted link.
*
* \param function Callback function that is called whenever encryption is changed.
*
*/
void blecm_regEncryptionChangedHandler(void *);

void blecm_activateModule( INT32 moduleIdx, BLECM_FUNC_WITH_PARAM handler);
void blecm_clearActivateModule( INT32 moduleIdx);


// This will start the timer source from lower layer.
void blecm_startTimerSource(INT32 id, BLECM_FUNC_WITH_PARAM appCb,
                            UINT32 ticks);

void blecm_stopTimerSource( INT32 id );

void blecm_refreshConnIdletimer(void);
void blecm_startConnIdleTimer(BLEAPP_TIMER_CB cb) ;
void blecm_stopConnIdleTimer(void);

#ifdef BLE_SLAVE_ONLY_ADDRESS_RESOLUTION
int  blecm_needToResolveRandomAddress( void );
void blecm_enableRandomAddressResolution( void );
void blecm_disableRandomAddressResolution( void );
void blecm_storeRPA(UINT8 *rpa, INT32 idx);
int  blecm_lookupAddress(INT32 type, UINT32 *rpa);
int  blecm_resolveRPA(INT32 type, UINT32 *rpa);
#endif


// There are for tracing. They can be turn off at run time.
void blecm_log(char * );
void blecm_log1(char *str,INT32 val );
void blecm_logBytes(char *str,UINT8 *data,INT32 len);
void blecm_log_byteArray(UINT8 *, INT32 len);
void blecm_log_6(char *, INT32, INT32, INT32, INT32,INT32,INT32);



//////////////////////////////////////////////////////////////////////////////
//              Here are the HCI Cmds.
void blecm_setAdvEnable(INT32 enableFlag);

void blecm_setAdvParam(INT32 interval, INT32 advType, INT32 advAdrType,
        INT32 advChanMap, INT32 advFilterPolicy ,INT32 initAdrType ,UINT8 *initAdr);

// This will issue HciReset.
void blecm_hciReset(void);
void blecm_hciReadBdAddr(void);



void blecm_getRand(void);
void blecm_hciEncrypt(UINT8 *key, UINT8 *text);
void blecm_hciLTKReply(UINT16 handle,UINT8 *ltk);

void blecm_hciLTKNegReply(UINT16 handle);

void blecm_disconnect(INT32 errCode );


void blecm_setAdvData(UINT8 *data, INT32 len);

void blecm_setScanRspData(UINT8 *data, INT32 len);

/**
* Sets the maximum TX power
*
* \param maxTxPowerDb The max TX power allowed in dB.
*/
void blecm_setTxPowerInConnection(INT8 maxTxPowerDb);

/**
* Ends all LE tests that were started with blecm_StartReveiverTest or blecm_StartTransmitterTest
*
* \param NONE
*/
void blecm_EndTest(void);

/**
* Starts the standard LE receiver test on the given channel
*
* \param channel The channel to use for the test. 0-39.
* \return <ReturnValue> True if test was started successfully, else could not start the test.
*/
UINT8 blecm_StartReveiverTest(UINT8 channel);

/**
* Starts the standard LE transmitter test with the given parameters
*
* \param channel  The channel to use for the test. 0-39.
* \param packetLength The Length test packet. 0-37
* \param packetType   The type of packet to use - The following are supported:
*				{0x00, "Pseudo-Random bit sequence 9"},
*				{0x01, "Pattern of alternating bits '11110000'"},
*				{0x02, "Pattern of alternating bits '10101010'"},
*				{0x03, "Pseudo-Random bit sequence 15 - Optional"},
*				{0x04, "Pattern of All '1' bits - Optional"},
*				{0x05, "Pattern of All '0' bits - Optional"},
*				{0x06, "Pattern of alternating bits '00001111' - Optional"},
*				{0x07, "Pattern of alternating bits '01010101' - Optional"}
* \return <ReturnValue> True if test was started successfully, else could not start the test.
*/
UINT8 blecm_StartTransmitterTest(UINT8 channel, UINT8 packetLength, UINT8 packetType);

void blecm_startAdv(
        INT32 advType,
        INT32 advInterval,
        INT32 advChannel,
        INT32 advAdrType,
        INT32 advFilterPolicy,
        INT32 initiatorAdrType,
        UINT8* initiatorAdr     // valid for Directed Adv.
        );


// This function will return the number of available Tx Buffers
INT32 blecm_getAvailableTxBuffers(void);
INT32 blecm_incAvailableTxBuffers(void);
INT32 blecm_decAvailableTxBuffers(void);
void blecm_setAvailableTxBuffers(INT32 val);


INT32  blecm_needToSelectAddress( void );
INT32 blecm_SelectAddress (BLECM_SELECT_ADDR *p_select_addr, UINT8 num);
INT32 blecm_SelectTargetAddress (BLECM_SELECT_ADDR *p_select_addr, UINT8 num);
void blecm_enableAddressSelection( void );
void blecm_disableAddressSelection( void );

/**
* \brief Configure LE scan parameters and start LE scan
* \ingroup blecm
*
* \details This function will start LE Scan with the given parameters. The
* Scan will start shortly after but not immediately after this function call returns. 
*
* \param LE_Scan_Type
*       \li HCIULP_PASSIVE_SCAN Passive Scanning. No SCAN_REQ packets shall be sent.
*       \li HCIULP_ACTIVE_SCAN Active scanning. SCAN_REQ packets may be sent.
* \param LE_Scan_Interval Time interval from when the Controller started its last LE scan 
* until it begins the subsequent LE scan.
* \param LE_Scan_Window The duration of the LE scan.
* \param Own_Address_Type
*       \li HCIULP_PUBLIC_ADDRESS
*       \li HCIULP_RANDOM_ADDRESS
* \param scanFilterPolicy
*       \li HCIULP_SCAN_FILTER_POLICY_WHITE_LIST_NOT_USED
*       \li HCIULP_SCAN_FILTER_POLICY_WHITE_LIST_USED
* \param Filter Duplicates
*       \li HCIULP_SCAN_DUPLICATE_FILTER_OFF Duplicate filtering disabled
*       \li HCIULP_SCAN_DUPLICATE_FILTER_ON Duplicate filtering enabled
*
*/
void blecm_startScan(
        INT32 scanType,
        INT32 scanInterval,
        INT32 scanWindow,
        INT32 scanAdrType,
        INT32 scanFilterPolicy,
        INT32 filterDuplicates
        );
void blecm_setScanParam(
        INT32 scanType,
        INT32 scanInterval,
        INT32 scanWindow,
        INT32 scanAdrType,
        INT32 scanFilterPolicy
        );
void blecm_scanOn(INT32 filterDuplicates);

/**
* \brief Configure controller for LE scan
* \ingroup blecm
*
* \details This function issues HCI LE Set Scan Enable Command.  This
* command may be used to start and configure, or stop LE scans.
*
* \param LE_Scan_Enable
*       \li HCIULP_SCAN_MODE_OFF LE scanninig disabled
*       \li HCIULP_SCAN_MODE_ON LE scanninig enabled
* \param Filter Duplicates
*       \li HCIULP_SCAN_DUPLICATE_FILTER_OFF Duplicate filtering disabled
*       \li HCIULP_SCAN_DUPLICATE_FILTER_ON Duplicate filtering enabled
*
*/
void blecm_setScanEnable(INT32 enableFlag, INT32 filterDuplicates );

/**
* \brief Create a Link Layer connection to a connectable advertiser.
* \ingroup blecm
*
* \details This function issues HCI LE Create Connection Command.  Typically
* this command will be used after client application receives advertisement
* from a peripheral device
*
* \param LE_Scan_Interval Time interval from when the Controller started its last LE scan 
* until it begins the subsequent LE scan.
* \param LE_Scan_Window The duration of the LE scan.
* \param Initiator_Filter_Policy
* \param scanFilterPolicy
*       \li HCIULP_INITIATOR_FILTER_POLICY_WHITE_LIST_NOT_USED
*       \li HCIULP_INITIATOR_FILTER_POLICY_WHITE_LIST_USED
* \param Peer_Address_Type
*       \li HCIULP_PUBLIC_ADDRESS
*       \li HCIULP_RANDOM_ADDRESS
* \param ownAddressType
*       \li HCIULP_PUBLIC_ADDRESS
*       \li HCIULP_RANDOM_ADDRESS
* \param PeerAddress Public Device Address or Random Device Address of the device to be connected
* \param connMinInterval Minimum value for the connection event interval
* \param connMaxInterval Maximum value for the connection event interval
* \param connLatency Slave latency for the connection in number of connection events
* \param supervisionTimeout Supervision timeout for the LE Link
* \param connMinEventLen Minimum length of connection needed for this LE connection
* \param connMaxEventLen Maximum length of connection needed for this LE connection
*
*/
void blecm_CreateConnection(
        INT32 scanInterval,
        INT32 scanWindow,
        INT32 initiatorFilterPolicy,
        INT32 peerAddressType,
        char *peerAddress,
        INT32 ownAddressType,
        INT32 connMinInterval,
        INT32 connMaxInterval,
        INT32 connLatency,
        INT32 supervisionTimeout,
        INT32 connMinEventLen,
        INT32 connMaxEventLen
        );
/**
* \brief Cancel attempt to create a Link Layer connection
* \ingroup blecm
*
* \details This function may be issued by the client application to
* send HCI LE Create Connection Cancel Command.  
*
*/
void blecm_CreateConnectionCancel(void);

/**
* \brief Connection update request
* \ingroup blecm
*
* \details This function issues HCI LE Connection Update Command.  This
* command may be used in the client application if it needs to update
* connection parameters when connection is already established and
* required parameters are different than ones requested by the peripheral.  
* When peripheral device requests some parameters using L2CAP message, stack
* automatically replies and sets controller appropriately and there is no
* need to use this function call.
*
* \param connHandle HCI handle of established connection
* \param connMinInterval Minimum value for the connection event interval
* \param connMaxInterval Maximum value for the connection event interval
* \param connLatency Slave latency for the connection in number of connection events
* \param supervisionTimeout Supervision timeout for the LE Link
* \param connMinEventLen Minimum length of connection needed for this LE connection
* \param connMaxEventLen Maximum length of connection needed for this LE connection
*
*/
void blecm_ConnectionUpdate(
        INT32 connHandle,
        INT32 connMinInterval,
        INT32 connMaxInterval,
        INT32 connLatency,
        INT32 supervisionTimeout,
        INT32 connMinEventLen,
        INT32 connMaxEventLen
        );

/**
* \brief Register to receive advertisement report
* \ingroup blecm
*
* \details Client application may use this function to receive notification when
* controller receives advertisement from the peripheral device.  
* 
* \param function Callback function that is called whenever advertisement is received.
* HCIULP_ADV_PACKET_REPORT_WDATA structure with advertisement data is passed as a 
* parameter to this function.
*/
void blecm_RegleAdvReportCb(BLECM_FUNC_WITH_PARAM cb);

#ifdef SMP_INITIATOR
void blecm_startEncryption(INT32 Connection_Handle, UINT8 *Random_Number,
                           INT32 Encrypted_Diversifier, UINT8 *Long_Term_Key);
#endif


INT32  blecm_needToConMux( void );
void   blecm_enableConMux( void );
void   blecm_disableConMux( void );
void   blecm_ConMuxInit(INT32 con_num);
void   blecm_AddConMux(INT32 index, UINT16 con_handle, UINT16 db_size, void *p_db, 
	                                     void *dev_pinfo, void *smp_pinfo);
void   blecm_DelConMux(int index);
INT32  blecm_FindFreeConMux(void);
INT32  blecm_FindConMux(UINT16 con_handle);
UINT16 blecm_GetDbSizeConMux(UINT16 con_handle);
void   *blecm_GetDbConMux(UINT16 con_handle);
void   *blecm_GetDevConMux(UINT16 con_handle);
void   *blecm_GetSmpConMux(UINT16 con_handle);
void   blecm_ResetPtrConMux(void);
void   blecm_SetPtrConMux(UINT16 con_handle);

INT32  blecm_needToGATTDB16( void );
void   blecm_enableGATTDB16( void );
void   blecm_disableGATTDB16( void );

void   blecm_initAppPoll(void);
void   blecm_appPollEnable(void (*clientCallback)(void*, UINT32),
                           UINT32 clientContext,
                           UINT16 defaultPeriod);
void   blecm_appPollDisable(void);

//////////////////////////////////////////////////////////////////////////////
// These MACROS controlls debugging print.
#define BLECM_DBGUART_LOG                                              0x1
#define BLECM_DBGUART_LOG_L2CAP                                        0x2
#define BLECM_DBGUART_LOG_SMP                                          0x4
#define BLECM_DBGUART_LOG_ATT                                          0x8
#define BLECM_DBGUART_LOG_AP                                           0x10

extern UINT32 blecm_configFlag ;
#define BLECM_DBGUART_LOG_ENABLED()  (blecm_configFlag & BLECM_DBGUART_LOG)
#define BLECM_DBGUART_LOG_L2CAP_ENABLED()  \
            (blecm_configFlag & BLECM_DBGUART_LOG_L2CAP)
#define BLECM_DBGUART_LOG_SMP_ENABLED()  \
            (blecm_configFlag & BLECM_DBGUART_LOG_SMP)
#define BLECM_DBGUART_LOG_ATT_ENABLED()  \
            (blecm_configFlag & BLECM_DBGUART_LOG_ATT)
#define BLECM_DBGUART_LOG_AP_ENABLED()  \
            (blecm_configFlag & BLECM_DBGUART_LOG_AP)

#ifdef __cplusplus
}
#endif
#endif // end of #ifndef _BLECM_H_
