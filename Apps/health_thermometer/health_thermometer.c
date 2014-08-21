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
* BLE Health Thermometer profile, service, application 
*
* Refer to Bluetooth SIG Health Thermometer Profile 1.0, Health Thermometer 
* Service 1.0
*
* This is an application which improves functionality which exits in the ROM code.
* See bleapp\app\blether for complete implementation of the sample BLE health
* thermometer.
*
*/
#include "blether.h"
#include "ble_uuid.h"

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
static void health_thermometer_create(void);
static void health_thermometer_fine_timeout(UINT32 finecount);
static void health_thermometer_app_fine_timer_callback(UINT32 arg);

typedef struct
{
    BLETHER_HOSTINFO    blether_hostinfo;       //NVRAM save area

    BLETHER_THER_DATA   blether_ther_data;

    UINT32              blether_apptimer_count;
    UINT32              blether_appfinetimer_count;
    UINT16              blether_con_handle;
    BD_ADDR             blether_remote_addr;

    UINT16              blether_ther_hdl;
    UINT16              blether_ther_client_hdl;
    UINT16              blether_tt_hdl;
    UINT16              blether_it_hdl;
    UINT16              blether_it_client_hdl;
    UINT16              blether_mi_hdl;
    UINT16              blether_mi_client_hdl;
    UINT16              blether_mi_range_hdl;

    UINT16              blether_mi;
    UINT8               blether_mi_indication_sent;

    UINT8               blether_bat_enable;
    UINT8               blether_indication_enable;
    UINT8               blether_indication_defer;
    UINT8               blether_indication_sent;
    UINT8               blether_measurement_done;
} tTherAppState;


extern tTherAppState *therAppState;
extern void          blether_appTimerCb(UINT32 arg);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

const  UINT8 health_thermometer_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service Changed, handle 0x03 characteristic value
    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
        0x00, 0x00, 0x00, 0x00,

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),
    
    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','T','h','e','r','m','o','m','e','t','e','r',0x00,  

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_THERMOMETER),

    // Handle 0x28: Health thermometer service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_HEALTH_THERMOMETER),

    // Handle 0x29: characteristic temperature measurement, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16 (0x0029, 0x002a, UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT, 
                           LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 5),  
        0x01,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD |LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x2c: characteristic temperature type, handle 0x2d characteristic value
    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_TEMPERATURE_TYPE, 
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        THER_BODY,                  // value body (general)

    // Handle 0x2e: characteristic intermediate measurement, handle 0x2f characteristic value
    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE, 
                           LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 5),  
        0x01,0x00,0x00,0x00,0x00,

    // Handle 0x30: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0030, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x31: characteristic measurement interval, handle 0x32 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0031, 0x0032, UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL, 
                           LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE,
                           LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_AUTH_WRITABLE, 2),
        0x04,0x00,              //4 seconds

    // Handle 0x33: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0033, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x33: Valid Range descriptor
    CHAR_DESCRIPTOR_UUID16 (0x0034, UUID_DESCRIPTOR_VALID_RANGE, LEGATTDB_PERM_READABLE, 4),
        0x01,0x00,0xFF,0xFF,

    // Handle 0x3d: Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x3e: characteristic Manufacturer Name, handle 0x3f characteristic value
    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x40: characteristic Model Number, handle 0x41 characteristic value
    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const BLE_PROFILE_CFG health_thermometer_cfg =
{
    /*.fine_timer_interval            =*/ 500, // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "BLE Thermometer", //[LOCAL_NAME_LEN_MAX]
    /*.cod                            =*/ BIT16_TO_8(APPEARANCE_GENERIC_THERMOMETER), 0,  // [COD_LEN]
    /*.ver                            =*/ "1.00",            //[VERSION_LEN]
    /*.encr_required                  =*/ 0,    // if 1, encryption is needed before sending indication/notification
    /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
    /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x04, // dbm
    /*.con_idle_timeout               =*/ 5,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x002a, 0x002d, 0x002f, 0x0032, 0x00}, // GATT HANDLE number
    /*.serv                           =*/ {UUID_SERVICE_HEALTH_THERMOMETER, UUID_SERVICE_HEALTH_THERMOMETER,
                                           UUID_SERVICE_HEALTH_THERMOMETER, UUID_SERVICE_HEALTH_THERMOMETER, 0x00}, // GATT service UUID
    /*.cha                            =*/ {UUID_CHARACTERISTIC_TEMPERATURE_MEASUREMENT, UUID_CHARACTERISTIC_TEMPERATURE_TYPE,
                                           UUID_CHARACTERISTIC_INTERMEDIATE_TEMPERATURE, UUID_CHARACTERISTIC_MEASUREMENT_INTERVAL, 0x00}, // GATT characteristic UUID
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

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)health_thermometer_db_data, sizeof(health_thermometer_db_data), (void *)&health_thermometer_cfg,
        (void *)&blether_puart_cfg, (void *)&blether_gpio_cfg, health_thermometer_create);
}

//
// this function overwrites the one that is in the ROM to replace fine timer processing
//
void health_thermometer_create(void)
{
    blether_Create();

    bleprofile_KillTimer();
    bleprofile_regTimerCb(health_thermometer_app_fine_timer_callback, blether_appTimerCb);
    bleprofile_StartTimer();
}


void health_thermometer_fine_timeout(UINT32 finecount)
{
	char ther_char[READ_UART_LEN+1];

    ther_char[0] = 0;

	//Reading
	bleprofile_ReadUART(ther_char);
#if 1
	if (bleprofile_p_cfg->test_enable)
	{
		//This is making faking data
		//For test only
		blether_FakeUART(ther_char, finecount);
	}
#endif

	if (ther_char[0] == 'D' && ther_char[1] == 'D') //download start
	{
		blecm_setFilterEnable(0);
		ble_trace0("CSA_filter disabled");
	}
	else if (ther_char[0] == 'A' && ther_char[1] == 'A') //download start
	{
		blecm_setFilterEnable(1);
		ble_trace0("CSA_filter enabled");

		blether_connDown();
	}
	else  //ther main reading
	{
		blether_handleUART(ther_char);
	}

    // button control
    bleprofile_ReadButton();
}

void health_thermometer_app_fine_timer_callback(UINT32 arg)
{
    (therAppState->blether_appfinetimer_count)++;

    health_thermometer_fine_timeout(therAppState->blether_appfinetimer_count);
}


