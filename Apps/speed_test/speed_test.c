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
* BLE Speed Test Sample
*
* Application receives data over the air and sends ack for each n-th packet.
* Client can also configure service to loopback received data.
* During Notifcations test this application sends data as fast as possible.
*
* Features demonstrated
*  - receiving and sending data over the air
*
* To demonstrate the app, work through the following steps.
* 1. Connect from the client.
* 2. Configure Client Configuration Descriptor to allow service to send
*    notifications over Control characteristics
* 3. If loopback data is required, configure Client Configuration Descriptor
*    of the Data characteristics to allow send notifications
* 5. Send Start Test command over the Control characteristics indicating
*    how many frames to ack.  Or send a command to start Notifications test.
* 6. Start sending packets over Data characteristics.
* Write with no ack is used for the best performance.
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "speed_test.h"

/******************************************************
 *                      Constants
 ******************************************************/

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void speed_test_create(void);
static void speed_test_connection_up(void);
static void speed_test_connection_down(void);
static void speed_test_advertisement_stopped(void);
static int  speed_test_write_handler(LEGATTDB_ENTRY_HDR *p);
static void speed_test_fine_timeout(UINT32 arg);
static void speed_test_timeout(UINT32 count);

/******************************************************
 *               Variables Definitions
 ******************************************************/
/*
 * This is the GATT database for the Speed Test sample.  
 */
const UINT8 speed_test_gatt_database[]=
{
    // Standard GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Standard GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'S','p','e','e','d',' ','t','e','s','t',0x00,0x00,0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_TAG),

    // vendor specific Speed Test service
    PRIMARY_SERVICE_UUID128(HANDLE_SPEED_TEST_SERVICE_UUID, UUID_SPEED_TEST_SERVICE),

    // following characteristic is used to receive data.  It should use Write No Response property
    // to allow multiple tx within connection period
    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_SPEED_TEST_CHARACTERISTIC_DATA, HANDLE_SPEED_TEST_CHARACTERISTIC_DATA_VALUE, 
        UUID_SPEED_TEST_CHARACTERISTIC_DATA, LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
        LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_VARIABLE_LENGTH, 20),
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    // client can write 1 into the client configuration descriptor to receive data back
    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_SPEED_TEST_DATA_CLIENT_CONFIGURATION_DESCRIPTOR,
            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
            LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // following characteristic is used by the client to send commands and for this app to send
    // flow control notifications
    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL, HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL_VALUE,
            UUID_SPEED_TEST_CHARACTERISTIC_CONTROL_POINT,
        LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_WRITE,
        LEGATTDB_PERM_WRITE_REQ, 2),
    0x00,0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_SPEED_TEST_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR,
            UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
            LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,
};

const BLE_PROFILE_CFG speed_test_cfg =
{
    /*.fine_timer_interval            =*/ 12, // ms
    /*.default_adv                    =*/ HIGH_UNDIRECTED_DISCOVERABLE,
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "Speed Test", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ "\x00\x00\x00", // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0,    // data encrypted and device sends security request on every connection
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00}, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.cha                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 0,    // if 1 grouptype read can be used
    /*.linkloss_button_enable         =*/ 0,    // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 0,    // second
    /*.alert_interval                 =*/ 0,    // interval of alert
    /*.high_alert_num                 =*/ 0,    // number of alert for each interval
    /*.mild_alert_num                 =*/ 0,    // number of alert for each interval
    /*.status_led_enable              =*/ 0,    // if 1 status LED is enable
    /*.status_led_interval            =*/ 0,    // second
    /*.status_led_con_blink           =*/ 0,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 0,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 0,    // blink num of undir adv
    /*.led_on_ms                      =*/ 0,    // led blink on duration in ms
    /*.led_off_ms                     =*/ 0,    // led blink off duration in ms
    /*.buz_on_ms                      =*/ 0,    // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,    // seconds
    /*.button_client_timeout          =*/ 0,    // seconds
    /*.button_discover_timeout        =*/ 0,    // seconds
    /*.button_filter_timeout          =*/ 0,    // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG speed_test_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG speed_test_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // GPIOs are not used
    },
    /*.gpio_flag =*/
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

typedef struct PACKED t_APP_STATE
{
    UINT16  data_client_configuration;
    UINT16  control_client_configuration;
    UINT8   num_to_receive_before_ack;
    UINT8   received_unacked;

#define TEST_NONE			0
#define TEST_RECEIVE		1
#define TEST_NOTIFICATIONS	2
	UINT8	current_test;

//	HOSTINFO hostinfo;					// NVRAM save area

} tAPP_STATE;

tAPP_STATE speed_test;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)speed_test_gatt_database,
                   sizeof(speed_test_gatt_database),
                   (void *)&speed_test_cfg,
                   (void *)&speed_test_puart_cfg,
                   (void *)&speed_test_gpio_cfg,
                   speed_test_create);

//    bleapp_trace_enable = BLE_TRACE_UART_HCI;   // direct traces to HCI UART
}

// Create Speed Test application
void speed_test_create(void)
{
    ble_trace1("\rspeed_test_create() num bufs %d", blecm_getAvailableTxBuffers());
    ble_trace0(bleprofile_p_cfg->ver);

    memset (&speed_test, 0, sizeof (tAPP_STATE));

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, speed_test_connection_up);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, speed_test_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, speed_test_advertisement_stopped);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)speed_test_write_handler);

	bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
}

void speed_test_connection_up(void)
{
	// for the best performance set connection interval to minimum
	lel2cap_sendConnParamUpdateReq(6, 6, 0, 700);
}

// This function will be called when connection goes down
void speed_test_connection_down(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rConnection down ADV start");
}

void speed_test_advertisement_stopped(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rRestart ADV");
}

// fine timeout callback, send as many frames as we can
void speed_test_fine_timeout(UINT32 arg)
{
	while ((speed_test.current_test == TEST_NOTIFICATIONS) &&
	       ((speed_test.data_client_configuration & CCC_NOTIFICATION) != 0) &&
	       blecm_getAvailableTxBuffers())
	{
		extern INT32 ulp_getRand(void);
		int i;
		UINT8 buf[20];
		INT32 *p_buf = (INT32 *)buf;
		for (i = 0; i < 5; i++, p_buf++)
		{
			*p_buf = ulp_getRand();
		}
		bleprofile_sendNotification(HANDLE_SPEED_TEST_CHARACTERISTIC_DATA_VALUE, buf, 20);
	}
}

void speed_test_timeout(UINT32 arg)
{
}

//
// Process write request or command from the Bluetooth client
//
int speed_test_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    UINT8  result;

    if ((len == 2) && (handle == HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL_VALUE))
    {
    	// this is a command to start the test
    	if (attrPtr[0] == SPEED_TEST_START)
    	{
    		speed_test.num_to_receive_before_ack = attrPtr[1];
    		speed_test.received_unacked          = 0;

    		// host may change connection interval while we are not running the test. connection interval to minimum
    		if (emconninfo_getConnInterval() > 6)
    		{
        		lel2cap_sendConnParamUpdateReq(6, 6, 0, 700);
    		}

    		// if it is a request to start receive test, start timer,
    		if (speed_test.num_to_receive_before_ack == 0)
    		{
    			ble_trace0("starting notifications test");
    			speed_test.current_test = TEST_NOTIFICATIONS;
				bleprofile_regTimerCb(speed_test_fine_timeout, speed_test_timeout);
				bleprofile_StartTimer();
    		}
    		else
    		{
    			ble_trace0("starting receive test");
    			speed_test.current_test = TEST_RECEIVE;
				bleprofile_KillTimer();
    		}
    	}
    	else
    	{
			ble_trace0("test stopped");
			speed_test.current_test = TEST_NONE;
    	}
    }
    else if ((len == 2) && (handle == HANDLE_SPEED_TEST_DATA_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        speed_test.data_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("data descriptor %04x\n", speed_test.data_client_configuration);
    }
    else if ((len == 2) && (handle == HANDLE_SPEED_TEST_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        speed_test.control_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("control descriptor %04x\n", speed_test.control_client_configuration);
    }
    else if (handle == HANDLE_SPEED_TEST_CHARACTERISTIC_DATA_VALUE)
    {
        if (++speed_test.received_unacked == speed_test.num_to_receive_before_ack)
        {
			if (speed_test.control_client_configuration & CCC_NOTIFICATION)
			{
				UINT8 result[1] = {0x01};
				bleprofile_sendNotification(HANDLE_SPEED_TEST_CHARACTERISTIC_CONTROL_VALUE, result, 1);
			}
			speed_test.received_unacked = 0;
        }
        // check client want us to send back data
        if (speed_test.data_client_configuration & CCC_NOTIFICATION)
        {
			bleprofile_sendNotification(HANDLE_SPEED_TEST_CHARACTERISTIC_DATA_VALUE, attrPtr, len);
        }
    }
    else
    {
        ble_trace2("bad write_handler len:%d handle:%04x\n", len, handle);
    }
    return 0;
}
