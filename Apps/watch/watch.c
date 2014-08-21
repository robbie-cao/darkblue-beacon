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
* BLE Watch Reference application
*
* The watch reference application combines multiple services and clients
* commonly used in the BLE watches including Apple's vendor specific ANCS,
* HID over GATT, Proximity, Find Me, and Time.  Device works in the
* peripheral mode accepting connection from central typically a phone.
* The GATT database for the device includes definitions for all services
* including HID descriptor used to send AVRC play, pause and other commands.
*
* After pairing application performs GATT discovery of the connected
* device.  This module figures out what services are available and then
* each particular modules performs discovery of the characteristics of
* each particular service.  Similarly for each notification/indication
* message is passed to each particular module.
*
* Features demonstrated
*  - Executing multiple clients/services in the same application
*  - GATT database and Device configuration initialization
*  - Registration with LE stack for various events
*  - NVRAM read/write operation
*  - Processing control and data from the client
*  - Processing indications and notifications from the servers
*  - Sending data to the client
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Start tracing (see quick start guide)
* 3. Pair with a client (iOS device)
* 4. Send SMS/incoming call to the phone and verify traces
* 5. Change Time on the phone and see notification in traces
* 6. Play music on the phone.  Push button on the tag to toggle play/stop
*
*/

#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "bleprox.h"
#include "watch_db.h"
#include "ancs_client.h"
#include "spar_utils.h"

/******************************************************
 *                      Constants
 ******************************************************/
#define HID_CODE_PLAY		0xcd
#define HID_CODE_PAUSE		0xcd
#define HID_CODE_FORWARD	0xb5
#define HID_CODE_BACKWARD	0xb6

/******************************************************
 *                     Structures
 ******************************************************/

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void watch_create(void);
static void watch_connection_up(void);
static void watch_connection_down(void);
static void watch_advertisement_stopped(void);
static void watch_smp_bond_result(LESMP_PARING_RESULT result);
static void watch_encryption_changed(HCI_EVT_HDR *evt);
static int  watch_write_handler(LEGATTDB_ENTRY_HDR *p);
static void watch_client_notification_handler(int len, int handle, UINT8 *data);
static void watch_interrupt_handler(UINT8 value);
static void watch_timeout(UINT32 count);
static void watch_fine_timeout(UINT32 finecount);
static void watch_client_process_rsp(int len, int attr_len, UINT8 *data);
static void watch_init_next_client(void);

/******************************************************
 *               Variables Definitions
 ******************************************************/

const BLE_PROFILE_CFG gatt_db_cfg =
{
    /*.fine_timer_interval            =*/ 100, // ms
    /*.default_adv                    =*/ HIGH_UNDIRECTED_DISCOVERABLE,
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 5,    // seconds
    /*.low_direct_adv_interval        =*/ 10,   // seconds
    /*.high_direct_adv_duration       =*/ 30,   // seconds
    /*.low_direct_adv_duration        =*/ 300,  // seconds
    /*.local_name                     =*/ "My Watch", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_WATCH), 0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ (SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {
    	HDLC_LINK_LOSS_ALERT_LEVEL_VALUE,  HDLC_IMMEDIATE_ALERT_ALERT_LEVEL_VALUE, HDLC_TX_POWER_TX_POWER_LEVEL_VALUE, HDLC_BATTERY_SERVICE_BATTERY_LEVEL_VALUE, 0x00}, // [HANDLE_NUM_MAX] GATT HANDLE number
    /*.serv                           =*/ {
    	UUID_SERVICE_LINK_LOSS, 		   UUID_SERVICE_IMMEDIATE_ALERT,           UUID_SERVICE_TX_POWER,              UUID_SERVICE_BATTERY,                     0x00}, // GATT service UUID
    /*.cha                            =*/ {
    	UUID_CHARACTERISTIC_ALERT_LEVEL,   UUID_CHARACTERISTIC_ALERT_LEVEL,        UUID_CHARACTERISTIC_TX_POWER_LEVEL, UUID_CHARACTERISTIC_BATTERY_LEVEL,        0x00}, // GATT characteristic UUID
        /*.findme_locator_enable      =*/ 3,    // if 1 Find me locator is enable if 3 Find me alert will be toggled (Alert/No Alert)
        /*.findme_alert_level         =*/ HIGH_ALERT, // alert level of find me
        /*.client_grouptype_enable    =*/ 1,    // if 1 groupbytype read can be used
        /*.linkloss_button_enable     =*/ 1,    // if 1 linkloss button is enable
        /*.pathloss_check_interval    =*/ 1,    // second
        /*.alert_interval             =*/ 1,    // interval of alert
        /*.high_alert_num             =*/ 5,    // number of alert for each interval
        /*.mild_alert_num             =*/ 2,    // number of alert for each interval
        /*.status_led_enable          =*/ 1,    // if 1 status LED is enable
        /*.status_led_interval        =*/ 5,    // second
        /*.status_led_con_blink       =*/ 1,    // blink num of connection
        /*.status_led_dir_adv_blink   =*/ 2,    // blink num of dir adv
        /*.status_led_un_adv_blink    =*/ 3,    // blink num of undir adv
        /*.led_on_ms                  =*/ 5,    // led blink on duration in ms
        /*.led_off_ms                 =*/ 250,  // led blink off duration in ms
        /*.buz_on_ms                  =*/ 100,  // buzzer on duration in ms
        /*.button_power_timeout       =*/ 1,    // seconds
        /*.button_client_timeout      =*/ 3,    // seconds
        /*.button_discover_timeout    =*/ 5,    // seconds
        /*.button_filter_timeout      =*/ 10,   // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};


// NVRAM save area
HOSTINFO watch_hostinfo;

UINT16 	watch_connection_handle			= 0;	// HCI handle of connection, not zero when connected
BD_ADDR watch_remote_addr        		= {0, 0, 0, 0, 0, 0};	//address of currently connected client
BOOL    watch_hid_report_client_configuration;
BOOL	watch_hid_empty_report_pending 	= 0;
UINT8 	gatt_db_indication_sent    		= 0;	// indication sent, waiting for ack

int     watch_gatt_client_initialized   = FALSE;
int     watch_time_client_initialized   = FALSE;
int     watch_findme_client_initialized = FALSE;
int     watch_ancs_client_initialized	= FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)gatt_database,
                   gatt_database_len,
                   (void *)&gatt_db_cfg,
                   (void *)&bleprox_puart_cfg,
                   (void *)&bleprox_gpio_cfg,
                   watch_create);
}

// Create device
void watch_create(void)
{
    BLEPROFILE_DB_PDU db_pdu;

    bleprox_Create();

    ble_trace0("watch_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // Read NVRAM and check if it is initialized and if not then initialize it and write initialized NVRAM
//    bleprofile_ReadNVRAM(VS_WATCH_HOST_INFO, sizeof(watch_hostinfo), (UINT8 *)&watch_hostinfo);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, watch_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, watch_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, watch_advertisement_stopped);

    // register to process peripheral advertisements, notifications and indications
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) watch_client_notification_handler);

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(watch_encryption_changed);

    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB)watch_smp_bond_result);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)watch_write_handler);

    // register interrupt handler
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB)watch_interrupt_handler);

    //registers timer.  Bleprox already registered its callbacks replace with local.
    bleprofile_KillTimer();
    bleprofile_regTimerCb(watch_fine_timeout, watch_timeout);
    bleprofile_StartTimer();

    blebat_Init();

    db_pdu.len = 1;
    db_pdu.pdu[0] = 99;

	bleprofile_WriteHandle(HDLC_BATTERY_SERVICE_BATTERY_LEVEL_VALUE, &db_pdu);

	BLE_ADV_FIELD adv[4];

    // flags
    adv[0].len     = 1 + 1;
    adv[0].val     = ADV_FLAGS;
    adv[0].data[0] = LE_LIMITED_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

    adv[1].len     = 6 + 1;
    adv[1].val     = ADV_SERVICE_UUID16_MORE;
    adv[1].data[0] = UUID_SERVICE_LINK_LOSS & 0xff;
    adv[1].data[1] = (UUID_SERVICE_LINK_LOSS >> 8) & 0xff;
    adv[1].data[2] = UUID_SERVICE_IMMEDIATE_ALERT & 0xff;
    adv[1].data[3] = (UUID_SERVICE_IMMEDIATE_ALERT >> 8) & 0xff;
    adv[1].data[0] = UUID_SERVICE_HID & 0xff;
    adv[1].data[1] = (UUID_SERVICE_HID >> 8) & 0xff;

    adv[2].len     = 2 + 1;
    adv[2].val     = ADV_APPEARANCE;
    adv[2].data[0] = APPEARANCE_GENERIC_WATCH & 0xff;
    adv[2].data[1] = (APPEARANCE_GENERIC_WATCH >> 8) & 0xff;

    // name
    adv[3].len      = strlen(bleprofile_p_cfg->local_name) + 1;
    adv[3].val      = ADV_LOCAL_NAME_COMP;
    memcpy(adv[3].data, bleprofile_p_cfg->local_name, adv[3].len - 1);

    bleprofile_GenerateADVData(adv, 4);

	bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, watch_remote_addr);
}

// This function will be called on every connection establishment
void watch_connection_up(void)
{
	watch_connection_handle = (UINT16)emconinfo_getConnHandle();
	gatt_db_indication_sent = 0;

    // save address of the connected device and print it out.
    memcpy(watch_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(watch_remote_addr));

    ble_trace5("watch_connection_up: %08x%04x type %d bonded %d handle %d",
                (watch_remote_addr[5] << 24) + (watch_remote_addr[4] << 16) +
                (watch_remote_addr[3] << 8) + watch_remote_addr[2],
                (watch_remote_addr[1] << 8) + watch_remote_addr[0],
                emconninfo_getPeerAddrType(), emconninfo_deviceBonded(), watch_connection_handle);

    bleprox_connUp();

    ancs_client_connection_up();

	if (emconninfo_deviceBonded())
	{
		ble_trace0("device bonded");
	}
	else
    {
        ble_trace0("Watch: sending security request");
    	lesmp_pinfo->pairingParam.AuthReq  |= LESMP_AUTH_FLAG_BONDING;
        lesmp_sendSecurityRequest();
    }
}

// This function will be called when connection goes down
void watch_connection_down(void)
{
    ble_trace4("watch_connection_down:%08x%04x bonded:%d handle:%d\n",
                (watch_remote_addr[5] << 24) + (watch_remote_addr[4] << 16) +
                (watch_remote_addr[3] << 8) + watch_remote_addr[2],
                (watch_remote_addr[1] << 8) + watch_remote_addr[0],
                emconninfo_deviceBonded(), watch_connection_handle);

	//remember peer address for advertising
	memset (watch_remote_addr, 0, 6);
	watch_connection_handle = 0;

	bleprox_connDown();

    ancs_client_connection_down();
}

void watch_advertisement_stopped(void)
{
    ble_trace0("ADV stop!!!!");
}

// when timer is running this will tick every 1 second
void watch_timeout(UINT32 arg)
{
	bleprox_appTimerCb(arg);

	switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
        {
        }
        break;
    }
}

// when timer is running this will tick every p_cfg->fine_timer_interval (12msec - 1sec)
void watch_fine_timeout(UINT32 arg)
{
	UINT8 report[4] = {0x00, 0x00, 0x00, 0x00};

    // this produces too much traces right now
//	bleprox_appFineTimerCb(arg);

	if (watch_hid_empty_report_pending != 0)
	{
		if (--watch_hid_empty_report_pending == 0)
		{
			bleprofile_sendNotification(HDLC_HID_REPORT2_VALUE, report, 4);
		}
	}
}


//
// Process SMP bonding result.  If we successfully paired with the
// central device, save its BDADDR in the NVRAM and initialize
// associated data
//
void watch_smp_bond_result(LESMP_PARING_RESULT  result)
{
    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        // saving bd_addr in nvram
        UINT8 bytes_written;

        // save address of the connected device and print it out.
        memcpy(watch_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(watch_remote_addr));

        ble_trace5("Watch: bonded: %08x%04x type %d bonded %d handle %d",
                    (watch_remote_addr[5] << 24) + (watch_remote_addr[4] << 16) +
                    (watch_remote_addr[3] << 8) + watch_remote_addr[2],
                    (watch_remote_addr[1] << 8) + watch_remote_addr[0],
                    emconninfo_getPeerAddrType(), emconninfo_deviceBonded(), watch_connection_handle);

        BT_MEMSET (&watch_hostinfo, 0, sizeof (HOSTINFO));
        BT_MEMCPY (watch_hostinfo.bdaddr, watch_remote_addr, 6);

        //now write hostinfo into NVRAM
        bytes_written = bleprofile_WriteNVRAM(VS_WATCH_HOST_INFO, sizeof(watch_hostinfo), (UINT8 *)&watch_hostinfo);
        ble_trace1("NVRAM write:%04x", bytes_written);

        // perform primary service search
        leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) watch_client_process_rsp);

        bleprox_smpBondResult(result);

        ancs_client_smp_bond_result(result);

        bleprofile_sendReadByGroupTypeReq(1, 0xffff, UUID_ATTRIBUTE_PRIMARY_SERVICE);
        ble_trace0("\rService Search");
    }
    else
    {
        ble_trace1("Watch: bond result %02x\n", result);
    }
}

// process responses from the server during the GATT discovery
void watch_client_process_rsp(int len, int attr_len, UINT8 *data)
{
    int    i;
    UINT16 uuid, s_handle, e_handle;

    ble_trace2("\rClient Rsp len:%d, attr_len:%d", len, attr_len);

    if (len)
    {
        // always use group type
        // search UUID
        // S_HANDLE(2) E_HANDLE(2) UUID(2-16) 
        for (i = 0; i < len; i += attr_len)
        {
            s_handle = data[i]     + (data[i + 1] << 8);
            e_handle = data[i + 2] + (data[i + 3] << 8);

            // services on the server can be based on 2 or 16 bytes UUIDs
            if (attr_len == 6)
            {
                uuid = data[i + 4] + (data[i + 5] << 8);
                ble_trace3("\rs:%04x e:%04x uuid:%04x", s_handle, e_handle, uuid);
                if (uuid == UUID_SERVICE_GATT)
                {
                    // remember GATT service start and end handles
                	watch_hostinfo.gatt_s_handle = s_handle;
                	watch_hostinfo.gatt_e_handle = e_handle;
                }
                else if (uuid == UUID_SERVICE_CURRENT_TIME)
                {
                	watch_hostinfo.current_time_s_handle = s_handle;
                	watch_hostinfo.current_time_e_handle = e_handle;
                }
                else if (uuid == UUID_SERVICE_IMMEDIATE_ALERT)
                {
                	watch_hostinfo.findme_s_handle = s_handle;
                	watch_hostinfo.findme_e_handle = e_handle;
                }
            }
            else if (attr_len == 20)
            {
                ble_trace2("\r%04x e:%04x uuid", s_handle, e_handle);
                ble_tracen((char *)&data[i + 4], 16);
                if (BT_MEMCMP(&data[i + 4], ANCS_SERVICE, 16) == 0)
                {
                	watch_hostinfo.ancs_s_handle = s_handle;
                	watch_hostinfo.ancs_e_handle = e_handle;
                }
            }
        }
        // continue reading
        // start reading from the last handle + 1
        bleprofile_sendReadByGroupTypeReq(e_handle + 1, 0xffff, UUID_ATTRIBUTE_PRIMARY_SERVICE);
        ble_trace2("Service Search s_hdl:%04x, e_hdl:%04x", e_handle + 1, 0xffff);
    }
    else
    {
        watch_init_next_client();
    }
}

void watch_gatt_initialization_callback(int result)
{
	watch_gatt_client_initialized = TRUE;
    watch_init_next_client();
}

void watch_time_initialization_callback(int result)
{
    watch_time_client_initialized = TRUE;
    watch_init_next_client();
}

void watch_findme_initialization_callback(int result)
{
    watch_findme_client_initialized = TRUE;
    watch_init_next_client();
}

void watch_ancs_initialization_callback(int result)
{
    watch_ancs_client_initialized = TRUE;
    watch_init_next_client();
}

void watch_init_next_client(void)
{
    if (!watch_gatt_client_initialized && (watch_hostinfo.gatt_s_handle != 0) && (watch_hostinfo.gatt_e_handle != 0))
    {
        gatt_client_initialize(watch_hostinfo.gatt_s_handle, watch_hostinfo.gatt_e_handle, &watch_gatt_initialization_callback);
    }
    else if (!watch_time_client_initialized && (watch_hostinfo.current_time_s_handle != 0) && (watch_hostinfo.current_time_e_handle != 0))
    {
        time_client_initialize(watch_hostinfo.current_time_s_handle, watch_hostinfo.current_time_e_handle, &watch_time_initialization_callback);
    }
    else if (!watch_findme_client_initialized && (watch_hostinfo.findme_s_handle != 0) && (watch_hostinfo.findme_e_handle != 0))
    {
        findme_client_initialize(watch_hostinfo.findme_s_handle, watch_hostinfo.findme_e_handle, &watch_findme_initialization_callback);
    }
    else if (!watch_ancs_client_initialized && (watch_hostinfo.ancs_s_handle != 0) && (watch_hostinfo.ancs_e_handle != 0))
    {
        ancs_client_initialize(watch_hostinfo.ancs_s_handle, watch_hostinfo.ancs_e_handle, &watch_ancs_initialization_callback);
    }
    else
    {
        UINT8  writtenbyte;

        // We are done with initial settings, and need to stay connected.  It is a good
        // time to slow down the pace of master polls to save power.  Following request asks
        // host to setup polling every 100-500 msec, with link supervision timeout 7 seconds.
        bleprofile_SendConnParamUpdateReq(80, 400, 0, 700);

		writtenbyte = bleprofile_WriteNVRAM(VS_WATCH_HOST_INFO, sizeof(watch_hostinfo), (UINT8 *)&watch_hostinfo);
		ble_trace1("watch_write_handler: NVRAM write:%04x\n", writtenbyte);
    }
}

//
// Process notification from the stack that encryption has been set.  If connected
// client is registered for notification or indication, it is a good time to
// send it out
//
void watch_encryption_changed(HCI_EVT_HDR *evt)
{
    ble_trace2("Watch: encryption changed: on %08x%04x\n",
                (watch_remote_addr[5] << 24) + (watch_remote_addr[4] << 16) +
                (watch_remote_addr[3] << 8) + watch_remote_addr[2],
                (watch_remote_addr[1] << 8) + watch_remote_addr[0]);

    if (!emconninfo_deviceBonded())
    {
    	return;
    }

    // Connection has been encrypted meaning that we have correct/paired device
    // read the handles from the NVRAM
    bleprofile_ReadNVRAM(VS_WATCH_HOST_INFO, sizeof(watch_hostinfo), (UINT8 *)&watch_hostinfo);

    ancs_client_encryption_changed(evt);

    // We are done with initial settings, and need to stay connected.  It is a good
	// time to slow down the pace of master polls to save power.  Following request asks
	// host to setup polling every 100-500 msec, with link supervision timeout 7 seconds.
    // Send request to client to update connection parameters
    // lel2cap_sendConnParamUpdateReq(80, 400, 0, 700);
}

// Process write request or command from peer device
int watch_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT8  writtenbyte;
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    BOOL   save  	= FALSE;

    ble_trace2("watch_write_handler: len:%d handle %04x\n", len, handle);

    if ((len == 1) && (handle == HDLC_HID_HID_CONTROL_POINT_VALUE))
    {
        //call custom on_write function
        on_write_hid_control_point(len, attrPtr);
    }
    else if ((len == 2) && (handle == HDLD_HID_REPORT_CLIENT_CONFIGURATION))
    {
        watch_hid_report_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("write_handler: hid_report_client_configuration: %04x\n", watch_hid_report_client_configuration);
        save = TRUE;
    }
    // Save update to NVRAM if it has been changed.
    if (save)
    {
		writtenbyte = bleprofile_WriteNVRAM(VS_WATCH_HOST_INFO, sizeof(watch_hostinfo), (UINT8 *)&watch_hostinfo);
		ble_trace1("watch_write_handler: NVRAM write:%04x\n", writtenbyte);
    }
    return 0;
}

// process notifications from the client
void watch_client_notification_handler(int len, int handle, UINT8 *data)
{
    ble_trace2("\rWatch Notification handle:%02x len:%d", (UINT16)handle, len);
    if ((handle >= watch_hostinfo.gatt_s_handle) && (handle <= watch_hostinfo.gatt_e_handle))
    {
    	gatt_client_notification_handler(len, handle, data);
    }
    else if ((handle >= watch_hostinfo.current_time_s_handle) && (handle <= watch_hostinfo.current_time_e_handle))
    {
    	time_client_notification_handler(len, handle, data);
    }
    else
    {
    	ancs_client_notification_handler(len, handle, data);
    }
}

// Three Interrupt inputs (Buttons) can be handled here.
// If the following value == 1, Button is pressed. Different than initial value.
// If the following value == 0, Button is depressed. Same as initial value.
// Button1 : value&0x01
// Button2 : (value&0x02)>>1
// Button3 : (value&0x04)>>2
void watch_interrupt_handler(UINT8 value)
{
	UINT8 report[4] = {0x00, 0x00, 0x00, 0x00};
    ble_trace5("(INT)But1:%d But2:%d But3:%d handle:%d CCC:%d\n", value&0x01, (value& 0x02) >> 1, (value & 0x04) >> 2,
    		watch_connection_handle, watch_hid_report_client_configuration);

    if (watch_connection_handle != 0)
    {
        if (watch_hid_report_client_configuration & CCC_NOTIFICATION)
        {
            report[0] = HID_CODE_PLAY;
            bleprofile_sendNotification(HDLC_HID_REPORT2_VALUE, report, 4);

            // it is not good to send button up right away.  Do it on the next fine timeout.
        	watch_hid_empty_report_pending = 5;
        }
    }
}


// It will be called at the write handler and should return TRUE if any persistent value is changed
BOOL on_write_hid_control_point(int len, UINT8 *attrPtr)
{
	if (*attrPtr == 0)
	{
		ble_trace0("Suspend");
	}
	else if (*attrPtr == 1)
	{
		ble_trace0("Exit Suspend");
	}
	else if (*attrPtr == 1)
	{
		ble_trace1("Unknown:%d", *attrPtr);
	}
    return FALSE;
}

