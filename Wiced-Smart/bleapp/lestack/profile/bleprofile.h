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
* GATT profiles
*
*/
#ifndef _BLEPROFILE_H_
#define _BLEPROFILE_H_

#include "bleappcfa.h"
#include "blecm.h"
#include "bleapp.h"
#include "blebat.h"
#include "lesmp.h"
#include "legattdb.h"
#include "leatt.h"
#include "emconinfo.h"
#include "ble_uuid.h"
#include "stacknvram.h"
#include "error.h"
#include "bt_rtos.h"

/*****************************************************************************/
/** @defgroup bleprofile       GATT Profiles
 *
 *  WICED Smart GATT Profiles functions
 */
/*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

#define BD_ADDR_LEN        6                   // Device address length 
#define ADV_LEN_MAX        31
#define FLAGS_LEN          1
#define COD_LEN            3
#define APPEARANCE_LEN     2
#define UUID_LEN           2
#define LOCAL_NAME_LEN_MAX 17
#define TX_POWER_LEN       1

#define VERSION_LEN        6

#define TIMESTAMP_LEN      7

#define READ_UART_LEN      15

#define SECURITY_ENABLED   0x01
#define SECURITY_REQUEST   0x02

#define BLEPROFILE_GENERIC_APP_TIMER 0

//GATT related definition
#define HANDLE_NUM_MAX 5

//GPIO flag bit (this is different than definition that used in gpio driver source code (GPIO_INPUT_ENABLE, etc)
#define GPIO_NUM_MAX  16
#define GPIO_INPUT          0x0000
#define GPIO_OUTPUT         0x0001
#define GPIO_INIT_LOW       0x0000
#define GPIO_INIT_HIGH      0x0002
#define GPIO_POLL           0x0000
#define GPIO_INT            0x0004   // Interrupt
#define GPIO_BOTHEDGE_INT   0x0008   // Both Edge

#define GPIO_WP             0x0010   //EEPROM write protect
#define GPIO_BAT            0x0020

#define GPIO_BUTTON         0x0100
#define GPIO_BUTTON1        0x0100
#define GPIO_BUTTON2        0x0200
#define GPIO_BUTTON3        0x0400

#define GPIO_LED            0x1000
#define GPIO_BUZ            0x2000


#ifdef BLE_PWM
#if defined(BCM20732)
#define PWM_START_GPIO  26
#define PWM_END_GPIO    (PWM_START_GPIO + MAX_PWMS)
#else
#define PWM_START_GPIO  0
#define PWM_END_GPIO    0
#endif
#else
#define PWM_START_GPIO  0
#define PWM_END_GPIO    0
#endif

// Define macros helpful for GATT DB definitions

#define BIT16_TO_8( val ) \
    (UINT8)(  (val)        & 0xff),/* LSB */ \
    (UINT8)(( (val) >> 8 ) & 0xff) /* MSB */

#define BIT32_TO_8( val ) \
    (UINT8)(  (val)         & 0xff),/* LSB */ \
    (UINT8)(( (val) >> 8  ) & 0xff),          \
    (UINT8)(  (val) >> 16   & 0xff),          \
    (UINT8)(( (val) >> 24 ) & 0xff)  /* MSB */

#define ATTRIBUTE16(  handle, permission, datalen, uuid ) \
    BIT16_TO_8(handle), \
    (UINT8)(permission), \
    (UINT8)(datalen + 2), \
    BIT16_TO_8(uuid)

#define ATTRIBUTE128( handle, permission, len, uuid, data ) \
    Not implemented

#define PRIMARY_SERVICE_UUID16(handle, service)  \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    4, \
    BIT16_TO_8((UUID_ATTRIBUTE_PRIMARY_SERVICE)), \
    BIT16_TO_8((service))

#define PRIMARY_SERVICE_UUID128(handle, service)  \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    18, \
    BIT16_TO_8(UUID_ATTRIBUTE_PRIMARY_SERVICE), \
    service

#define SECONDARY_SERVICE_UUID16(handle, service)  \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    4, \
    BIT16_TO_8((UUID_ATTRIBUTE_SECONDARY_SERVICE)), \
    BIT16_TO_8((service))

#define SECONDARY_SERVICE_UUID128(handle, service)  \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    18, \
    BIT16_TO_8(UUID_ATTRIBUTE_SECONDARY_SERVICE), \
    service

#define INCLUDE_SERVICE_UUID16(handle, service_handle, end_group_handle, service)  \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    8, \
    BIT16_TO_8(UUID_ATTRIBUTE_INCLUDE), \
    BIT16_TO_8(service_handle), \
    BIT16_TO_8(end_group_handle), \
    BIT16_TO_8(service)

#define CHARACTERISTIC_UUID16(handle, handle_value, uuid, properties, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    0x07, \
    BIT16_TO_8(UUID_ATTRIBUTE_CHARACTERISTIC), \
    (UINT8)(properties), \
    BIT16_TO_8((UINT16)(handle_value)), \
    BIT16_TO_8(uuid), \
    BIT16_TO_8((UINT16)(handle_value)), \
    (UINT8)(permission), \
    (UINT8)(value_len+2), \
    BIT16_TO_8(uuid)

#define CHARACTERISTIC_UUID128(handle, handle_value, uuid, properties, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    21, \
    BIT16_TO_8(UUID_ATTRIBUTE_CHARACTERISTIC), \
    (UINT8)(properties), \
    BIT16_TO_8((UINT16)(handle_value)), \
    uuid, \
    BIT16_TO_8((UINT16)(handle_value)), \
    (UINT8)(permission | LEGATTDB_PERM_SERVCIE_UUID_128), \
    (UINT8)(value_len+16), \
    uuid

#define CHARACTERISTIC_UUID16_WRITABLE(handle, handle_value, uuid, properties, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    0x07, \
    BIT16_TO_8(UUID_ATTRIBUTE_CHARACTERISTIC), \
    (UINT8)(properties), \
    BIT16_TO_8((UINT16)(handle_value)), \
    BIT16_TO_8(uuid), \
    BIT16_TO_8((UINT16)(handle_value)), \
    (UINT8)(permission), \
    (UINT8)(value_len+2), \
    (UINT8)(value_len), \
    BIT16_TO_8(uuid)

#define CHARACTERISTIC_UUID128_WRITABLE(handle, handle_value, uuid, properties, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    LEGATTDB_PERM_READABLE, \
    21, \
    BIT16_TO_8(UUID_ATTRIBUTE_CHARACTERISTIC), \
    (UINT8)(properties), \
    BIT16_TO_8((UINT16)(handle_value)), \
    uuid, \
    BIT16_TO_8((UINT16)(handle_value)), \
    (UINT8)(permission | LEGATTDB_PERM_SERVCIE_UUID_128), \
    (UINT8)(value_len+16), \
    (UINT8)(value_len), \
    uuid

#define CHAR_DESCRIPTOR_UUID16_WRITABLE(handle, uuid, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    (UINT8)(permission), \
    (UINT8)(value_len+2), \
    (UINT8)(value_len), \
    BIT16_TO_8(uuid)

#define CHAR_DESCRIPTOR_UUID16(handle, uuid, permission, value_len) \
    BIT16_TO_8((UINT16)(handle)), \
    (UINT8)(permission), \
    (UINT8)(value_len+2), \
    BIT16_TO_8(uuid)


// ADV flag values
enum ble_adv_flag_value
{
    ADV_FLAGS                           = 0x01,
    ADV_SERVICE_UUID16_MORE             = 0x02,
    ADV_SERVICE_UUID16_COMP             = 0x03,
    ADV_SERVICE_UUID32_MORE             = 0x04,
    ADV_SERVICE_UUID32_COMP             = 0x05,
    ADV_SERVICE_UUID128_MORE            = 0x06,
    ADV_SERVICE_UUID128_COMP            = 0x07,
    ADV_LOCAL_NAME_SHORT                = 0x08,
    ADV_LOCAL_NAME_COMP                 = 0x09,
    ADV_TX_POWER_LEVEL                  = 0x0A,
    ADV_CLASS_OF_DEVICE                 = 0x0D,
    ADV_SIMPLE_PAIRING_HASH_C           = 0x0E,
    ADV_SIMPLE_PAIRING_RANDOMIZER_R     = 0x0F,
    ADV_TK_VALUE                        = 0x10,
    ADV_OOB_FLAGS                       = 0x11,
    ADV_SLAVE_CONNECTION_INTERVAL_RANGE = 0x12,
    ADV_SERVICE_UUID16                  = 0x14,
    ADV_SERVICE_UUID128                 = 0x15,
    ADV_SERVICE_DATA                    = 0x16,
    ADV_SERVICE_TARGET_PUBLIC_ADR       = 0x17,
    ADV_SERVICE_TARGET_RANDOM_ADR       = 0x18,
    ADV_APPEARANCE                      = 0x19,
    ADV_ADVERTISING_INTERVAL            = 0x1A,
    ADV_3D_INFORMATION_DATA             = 0x3D,
    ADV_MANUFACTURER_DATA               = 0xFF,
};

// ADV and Scan Responce data fields bit mask 
#define ADV_DATA_MASK_FLAGS             0x01
#define ADV_DATA_MASK_SERVICE_UUID      0x02
#define ADV_DATA_MASK_APPEARANCE        0x04
#define ADV_DATA_MASK_NAME              0x08

#define SCAN_RSP_MASK_TX_POWER          0x01

#define LE_LIMITED_DISCOVERABLE         0x01 // LE Limited Discoverable Mode
#define LE_GENERAL_DISCOVERABLE         0x02 // LE General Discoverable Mode
#define BR_EDR_NOT_SUPPORTED            0x04 // BR/EDR Not Supported
#define SIMUL_LE_BR_EDR_CONTROLLER      0x08 // Simultaneous LE and BR/EDR to Same Device Capable (Controller)
#define SIMUL_LE_BR_EDR_HOST            0x10 // Simultaneous LE and BR/EDR to Same Device Capable (Host)

// ADV flag values
enum ble_appearance_value
{
    APPEARANCE_GENERIC_PHONE                    = 64,
    APPEARANCE_GENERIC_COMPUTER                 = 128, 
    APPEARANCE_GENERIC_WATCH                    = 192, 
    APPEARANCE_WATCH_SPORTS                     = 193, 
    APPEARANCE_GENERIC_CLOCK                    = 256, 
    APPEARANCE_GENERIC_DISPLAY                  = 320, 
    APPEARANCE_GENERIC_REMOTE_CONTROL           = 384, 
    APPEARANCE_GENERIC_EYE_GLASSES              = 448, 
    APPEARANCE_GENERIC_TAG                      = 512, 
    APPEARANCE_GENERIC_KEYRING                  = 576, 
    APPEARANCE_GENERIC_MEDIA_PLAYER             = 640, 
    APPEARANCE_GENERIC_BARCODE_SCANNER          = 704, 
    APPEARANCE_GENERIC_THERMOMETER              = 768, 
    APPEARANCE_THERMOMETER_EAR                  = 769, 
    APPEARANCE_GENERIC_HEART_RATE_SENSOR        = 832, 
    APPEARANCE_HEART_RATE_BELT                  = 833, 
    APPEARANCE_GENERIC_BLOOD_PRESSURE           = 896, 
    APPEARANCE_BLOOD_PRESSURE_ARM               = 897, 
    APPEARANCE_BLOOD_PRESSURE_WRIST             = 898, 
    APPEARANCE_GENERIC_HID_DEVICE               = 960, 
    APPEARANCE_HID_KEYBOARD                     = 961, 
    APPEARANCE_HID_MOUSE                        = 962, 
    APPEARANCE_HID_JOYSTICK                     = 963, 
    APPEARANCE_HID_GAMEPAD                      = 964, 
    APPEARANCE_HID_DIGITIZER_TABLET             = 965, 
    APPEARANCE_HID_CARD_READER                  = 966, 
    APPEARANCE_HID_DIGITAL_PEN                  = 967, 
    APPEARANCE_HID_BARCODE_SCANNER              = 968, 
    APPEARANCE_GENERIC_GLUCOSE_METER            = 1024,
    APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR   = 1088,
    APPEARANCE_RUNNING_WALKING_SENSOR_IN_SHOE   = 1089,
    APPEARANCE_RUNNING_WALKING_SENSOR_ON_SHOE   = 1090,
    APPEARANCE_RUNNING_WALKING_SENSOR_ON_HIP    = 1091,
    APPEARANCE_GENERIC_CYCLING                  = 1152,
    APPEARANCE_CYCLING_COMPUTER                 = 1153,
    APPEARANCE_CYCLING_SPEED_SENSOR             = 1154,
    APPEARANCE_CYCLING_CADENCE_SENSOR           = 1155,
    APPEARANCE_CYCLING_POWER_SENSOR             = 1156,
    APPEARANCE_CYCLING_SPEED_AND_CADENCE_SENSOR = 1157,
    APPEARANCE_LOCATION_AND_NAVIGATION_POD      = 5188,
};

enum bleprofile_puart_rx_int
{
    PUART_RX_INT_ENABLE                 = 0x01,
    PUART_RX_FIFO_INT_ENABLE            = 0x02,
    PUART_RX_CTS_ENABLE                 = 0x04,
};

typedef UINT8 BD_ADDR[BD_ADDR_LEN];         /* Device address */
typedef UINT8 *BD_ADDR_PTR;                 /* Pointer to Device Address */

typedef PACKED struct
{
    UINT8 len; //length of field
    UINT8 val; //value of field
    UINT8 data[ADV_LEN_MAX-2]; // This is the data and the biggest one is 31-2 = 29
} BLE_ADV_FIELD;

/**
* \brief BLE Profile Configuration
* \ingroup bleprofile
*
* \details BLE_PROFILE_CFG is used for setting application timer interval, ADV related
* parameters, local device information, connection/encryption related parameters, GATT
* DB handle, alert/led/buzzer related parameters, button related parameters.
*/
typedef PACKED struct
{
    UINT16 fine_timer_interval;         // ms
    UINT8  default_adv;                 // default adv
    UINT8  button_adv_toggle;	        // pairing button make adv toggle (if 1) or always on (if 0)
    UINT16 high_undirect_adv_interval;	// slots
    UINT16 low_undirect_adv_interval;	// slots
    UINT16 high_undirect_adv_duration;	// seconds
    UINT16 low_undirect_adv_duration;	// seconds
    UINT16 high_direct_adv_interval;    // seconds
    UINT16 low_direct_adv_interval;     // seconds
    UINT16 high_direct_adv_duration;    // seconds
    UINT16 low_direct_adv_duration;     // seconds
    char   local_name[LOCAL_NAME_LEN_MAX];
    char cod[COD_LEN];                  // First two bytes are used as appearance
    char   ver[VERSION_LEN];
    UINT8  encr_required;               // if 1, encryption is needed before sending indication/notification
    UINT8  disc_required;               // if 1, disconnection after confirmation
    UINT8  test_enable;
    UINT8  tx_power_level;              // dbm
    UINT8  con_idle_timeout;            // second
    UINT8  powersave_timeout;           // second
    UINT16 hdl[HANDLE_NUM_MAX];         // GATT HANDLE number
    UINT16 serv[HANDLE_NUM_MAX];        // GATT service UUID
    UINT16 cha[HANDLE_NUM_MAX];         // GATT characteristic UUID
    UINT8  findme_locator_enable;       // if 1 Find me locator is enable
    UINT8  findme_alert_level;          // alert level of find me
    UINT8  client_grouptype_enable;     // if 1 grouptype read can be used
    UINT8  linkloss_button_enable;      // if 1 linkloss button is enable
    UINT8  pathloss_check_interval;     // second
    UINT8  alert_interval;              // interval of alert
    UINT8  high_alert_num;              // number of alert for each interval
    UINT8  mild_alert_num;              // number of alert for each interval
    UINT8  status_led_enable;           // if 1 status LED is enable
    UINT8  status_led_interval;         // second
    UINT8  status_led_con_blink;        // blink num of connection
    UINT8  status_led_dir_adv_blink;    // blink num of dir adv
    UINT8  status_led_un_adv_blink;     // blink num of undir adv
    UINT16 led_on_ms;                   // led blink on duration in ms
    UINT16 led_off_ms;                  // led blink off duration in ms
    UINT16 buz_on_ms;                   // buzzer on duration in ms
    UINT8  button_power_timeout;        // seconds
    UINT8  button_client_timeout;       // seconds
    UINT8  button_discover_timeout;     // seconds
    UINT8  button_filter_timeout;       // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    UINT8  button_uart_timeout;         // seconds
#endif
} BLE_PROFILE_CFG;

/**
* \brief UART Configuration
* \ingroup bleprofile
*
* \details BLE_PROFILE_PUART_CFG defines baudrate, txpin, and rxpin of Periperal UART.
*/
typedef PACKED struct
{
    UINT32 baudrate; 
    UINT8  txpin;       // GPIO TX pin number
    UINT8  rxpin;       // GPIO RX pin number
} BLE_PROFILE_PUART_CFG;

/**
* \brief GPIO Configuration
* \ingroup bleprofile
*
* \details BLE_PROFILE_GPIO_CFG defines up to GPIO_NUM_MAX (currently 16) GPIO settings.
* Gpio_pin denotes GPIO pin number and gpio_flag defines GPIO setting (Input/Output,
* Interrupt, polarity) and the purpose (LED, Buzzer, Button, EEPROM Write Protect, etc).
* When GPIO purpose flag is set, profile common library functions can be usable. Without
* using GPIO purpose flag, GPIOs still can be usable with GPIO driver.
*/
typedef PACKED struct
{
    INT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
} BLE_PROFILE_GPIO_CFG;

//GATT_PDU
typedef PACKED struct
{
  UINT8 len;
  UINT8 header;
  UINT8 pdu[LEATT_ATT_MTU-1];
} BLEPROFILE_DB_PDU;


//Timestamp
typedef UINT8 TIMESTAMP[TIMESTAMP_LEN];

// Timer control 
typedef PACKED struct
{
    UINT8 count;
    UINT8 interval;
    UINT8 blinknum;
    UINT8 duration;
    UINT8 timeout;
}  BLE_PROFILE_TIMER_CONTROL;

//host information for NVRAM
typedef PACKED struct
//typedef struct
{
    // BD address of the bonded host
    BD_ADDR  bdAddr;
    UINT16 serv;
    UINT16 cha;
    UINT16 cli_cha_desc;	
}  BLEPROFILE_HOSTINFO;


// pwm control
typedef PACKED struct
{
    UINT8 freq;
    UINT16 init_value;
    UINT16 toggle_val;
}  BLE_PROFILE_PWM_CFG;

typedef UINT16 SFLOAT;
typedef UINT32 FLOAT32;

//Discoverable mode
enum ble_discover_mode
{
    NO_DISCOVERABLE              = 0,
    LOW_DIRECTED_DISCOVERABLE    = 1,
    HIGH_DIRECTED_DISCOVERABLE   = 2,
    LOW_UNDIRECTED_DISCOVERABLE  = 3,
    HIGH_UNDIRECTED_DISCOVERABLE = 4,
    MANDATORY_DISCOVERABLE       = 0xFF, //without NVRAM checking and start high undirected adv
}; 

//Find me client state
enum ble_findme_state
{
    FINDME_IDLE = 0,
    FINDME_WAIT_READ_PRIMARY_SERVICE_DEFER = 1,
    FINDME_WAIT_READ_PRIMARY_SERVICE       = 2,
    FINDME_WAIT_READ_CHARACTERISTIC_DEFER  = 3,
    FINDME_WAIT_READ_CHARACTERISTIC        = 4,
    FINDME_WAIT_WRITE_ALERTLEVEL_DEFER     = 5,
};

//time client state
enum ble_timeclient_state
{
    TIMECLIENT_IDLE = 0,
    TIMECLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER = 1,
    TIMECLIENT_WAIT_READ_PRIMARY_SERVICE       = 2,
    TIMECLIENT_WAIT_READ_CHARACTERISTIC_DEFER  = 3,
    TIMECLIENT_WAIT_READ_CHARACTERISTIC        = 4,
    TIMECLIENT_WAIT_READ_DESC_DEFER            = 5,
    TIMECLIENT_WAIT_READ_DESC                  = 6,
    TIMECLIENT_WAIT_READ_CURRENTTIME_DEFER     = 7,
    TIMECLIENT_WAIT_READ_CURRENTTIME           = 8,
}; 

//client state
enum ble_client_state
{
    CLIENT_IDLE = 0,
    CLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER = 1,
    CLIENT_WAIT_READ_PRIMARY_SERVICE       = 2,
    CLIENT_WAIT_READ_CHARACTERISTIC_DEFER  = 3,
    CLIENT_WAIT_READ_CHARACTERISTIC        = 4,
    CLIENT_WAIT_READ_DESC_DEFER            = 5,
    CLIENT_WAIT_READ_DESC                  = 6,
    CLIENT_WAIT_ACTION_DEFER               = 7,
    CLIENT_WAIT_ACTION                     = 8,
    CLIENT_WAIT_READ                       = 9,
    CLIENT_WAIT_WRITE                      = 10,
};

//client action
enum ble_client_action
{
    CLIENT_READREQ  = 0,
    CLIENT_WRITECMD = 1,
    CLIENT_WRITEREQ = 2,
};

//Client Characteristic Configuration mode
enum ble_ccc_mode
{
    CCC_NONE         = 0x00,
    CCC_NOTIFICATION = 0x01,
    CCC_INDICATION   = 0x02,
    CCC_RESERVED     = 0x04,
};

//Server Characteristic Configuration mode
enum ble_scc_mode
{
    SCC_NONE = 0x00,
    SCC_BROADCAST = 0x01,
};

enum ble_disc_setting
{
    DISC_NONE           = 0x00,
    DISC_AFTER_CONFIRM  = 0x01,
    DISC_ATT_TIMEOUT    = 0x02,
    DISABLE_ATT_TIMEOUT = 0x04,
};

//Button Function list
enum ble_button_function
{
    BUTTON_PRESS    = 0,
    BUTTON_RELEASE  = 1,
    BUTTON_POWER    = 2,
    BUTTON_CLIENT   = 3,
    BUTTON_DISCOVER = 4,
    BUTTON_FILTER   = 5,
#ifdef BLE_UART_LOOPBACK_TRACE
    BUTTON_UART     = 6,
#endif
    TIMEOUT_POWER   = 10, // This is not related to button, but notify when powersave is started
};

enum ble_puart_disable
{
    PUARTENABLE  = 0x00,
    PUARTDISABLE = 0x80,
};

enum ble_pwmbuz_freq
{
    PWMBUZ_125         = 0,
    PWMBUZ_250         = 1,
    PWMBUZ_500         = 2,
    PWMBUZ_1000        = 3,
    PWMBUZ_2000        = 4,
    PWMBUZ_4000        = 5,
    PWMBUZ_8000        = 6,
    PWMBUZ_FREQ_MAX    = 7,
    PWMBUZ_FREQ_MANUAL = 0xFF,
};

typedef UINT32 (*BLEPROFILE_QUERY_PARAM_CB)(UINT32, UINT32);
typedef UINT8 (*BLEPROFILE_DOUBLE_PARAM_CB)(UINT8 *, UINT8 *);
typedef UINT32 (*BLEPROFILE_SINGLE_PARAM_CB)(UINT32);
typedef UINT8 (*BLEPROFILE_NO_PARAM_CB)(void);

#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()

//////////////////////////////////////////////////////////////////////////////
//                      extern global variable
//////////////////////////////////////////////////////////////////////////////
extern BLE_PROFILE_CFG          bleprofile_cfg;
extern BLE_PROFILE_PUART_CFG    bleprofile_puart_cfg;
extern BLE_PROFILE_GPIO_CFG     bleprofile_gpio_cfg;
extern BLE_PROFILE_PWM_CFG      bleprofile_pwm_cfg;

extern UINT8                    *bleprofile_p_db;
extern UINT16                   bleprofile_db_size;
extern BLE_PROFILE_PUART_CFG    *bleprofile_puart_p_cfg;
extern BLE_PROFILE_GPIO_CFG     *bleprofile_gpio_p_cfg;
extern BLE_PROFILE_CFG          *bleprofile_p_cfg;
extern BD_ADDR                  bleprofile_remote_addr;
extern UINT8                    bleapp_trace_enable;

extern BLEPROFILE_DOUBLE_PARAM_CB bleprofile_handleUARTCb;

extern UINT8 bleprofile_PUART_RxInt_Enable;
extern UINT8 bleprofile_puart_tx_waterlevel;

//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////

/**
* \brief Generate Advertisement data
* \ingroup bleprofile
*
* \details This function generates ADV data to be used in the LE advertisement packets. When \c p_adv 
* is NULL, default fields (Flags, Local Name, Class of Device, Service UUID) are included. If \c p_adv 
* and \c number are not zeroes, ADV data is set based on the user defined values.
* 
* If application does not call this function default values will be used.  
*
* \param p_adv Pointer to the first advertisement field or NULL if defaults need to be used.
* \param number Number of fields.
*
*/
void bleprofile_GenerateADVData(BLE_ADV_FIELD *p_adv, UINT8 number);

/**
* \brief Generate Scan Response data
* \ingroup bleprofile
*
* \details This function generates data to be used in the LE Scan Response packets. When \c p_adv 
* is NULL, default data (Tx Power) is included. If \c p_adv and \c number are none zeroes, ADV data is
* set based on the user defined values.
* 
* If application does not call this function default values will be used.  
*
* \param p_adv Pointer to the first advertisement field or NULL if defaults need to be used.
* \param number Number of fields.
*
*/
void bleprofile_GenerateScanRspData(BLE_ADV_FIELD *p_adv, UINT8 number);

void bleprofile_DBRegister(UINT8 *p_db, UINT16 db_size);
void bleprofile_DBInit(UINT8**startPtr, UINT8 **endPtr);

/**
* \brief Register Configuration
* \ingroup bleprofile
*
* \details Application can call this function to overwrite default configuration.
*
* \param p_cfg Pointer to the BLE_PROFILE_CFG configuration structure.
*
*/
void bleprofile_CFGRegister(BLE_PROFILE_CFG *p_cfg);

/**
* \brief Register Peripheral UART Configuration
* \ingroup bleprofile
*
* \details This function registers peripheral UART configuration. bleapp_Init 
* function calls this function using arguments that passed by bleapp_set_cfg function.
*
* \param p_uart_cfg Pointer to the BLE_PROFILE_PUART_CFG config structure
*
*/
void bleprofile_PUARTRegister(BLE_PROFILE_PUART_CFG *p_puart_cfg);

/**
* \brief Register GPIO Configuration
* \ingroup bleprofile
*
* \details This function registers GPIO configuration. bleapp_Init 
* function calls this function using arguments that passed by bleapp_set_cfg function.
*
* \param p_uart_cfg Pointer to the BLE_PROFILE_GPIO_CFG config structure
*
*/
void bleprofile_GPIORegister(BLE_PROFILE_GPIO_CFG *p_gpio_cfg);

/**
* \brief Initialize BLE profile
* \ingroup bleprofile
*
* \details This function is the main profile common library initialization.  Application must call 
* this function during blexxxx_Create. The function registers profile with BLE stack 
* initializes power saving, enables interrupts and generates default advertisement and scan response
* data.
* 
* \param p_cfg Pointer to the BLE_PROFILE_CFG configuration structure.
*
*/
void bleprofile_Init(BLE_PROFILE_CFG *p_cfg);
void bleprofile_GPIOWPInit(BLE_PROFILE_GPIO_CFG *p_gpio_cfg);

/**
* \brief Initialize GPIO
* \ingroup bleprofile
*
* \details This function initializes GPIO pins defined in \c p_cfg
* 
* \param p_cfg Pointer to the BLE_PROFILE_GPIO_CFG GPIO configuration structure.
*
*/
void bleprofile_GPIOInit(BLE_PROFILE_GPIO_CFG *p_gpio_cfg);

/**
* \brief Read NVRAM
* \ingroup bleprofile
*
* \details This function reads bytes from the NVRAM(EEPROM).  Application should use
* the same \c vsID that was used during @bleprofile_WriteNVRAM call.
* 
* \param vsID ID number of NVRAM.  Valid values are 0x10 to 0x6F.
* \param itemLength Number of bytes to read.
* \param payload Address of the \c itemLength bytes where the bytes from NVRAM are copied to.
*
* \return
*        \li Number of bytes read from the NVRAM.
*        \li 0 if attempt to read failed.
*
*/
UINT8 bleprofile_ReadNVRAM(UINT8 vsID, UINT8 itemLength, UINT8* payload);

/**
* \brief Write NVRAM
* \ingroup bleprofile
*
* \details This function writes bytes to the NVRAM(EEPROM).  Application can specify
* any \c vsID between 0x10 and 0x6F.  Stack allocates memory and copies contents of the
* payload provided.  The same payload can be retrieved later on using the same
* @ vsID by calling \c bleprofile_ReadNVRAM.
* 
* \param vsID ID number of NVRAM.  Valid values are 0x10 to 0x6F.
* \param itemLength Number of bytes to write.  Valid values are 1 to 255.
* \param payload Address of the \c itemLength bytes to be stored in the NVRAM.
*
* \return
*        \li Number of bytes written to the NVRAM.
*        \li 0 if attempt to write failed.
*
*/
UINT8 bleprofile_WriteNVRAM(UINT8 vsID, UINT8 itemLength, UINT8 *payload);

/**
* \brief Delete NVRAM
* \ingroup bleprofile
*
* \details This function releases the NVRAM previously allocated by the \c bleprofile_WriteNVRAM call.  
* 
* \param vsID ID number of NVRAM.  Valid values are 0x10 to 0x6F.
*
* \return
*        \li 1 Delete succeeded.
*        \li 0 delete failed.
*
*/
BOOL32 bleprofile_DeleteNVRAM(UINT8 vsID);

/**
* \brief Sets NVRAM Write Protection
* \ingroup bleprofile
*
* \details This function enables or disables NVRAM (EEPROM) write protection.  
* 
* \param wp_enable If 0, write protect is off (EEPROM is writable). If 1, write protect is on
* (EEPROM is not writable).
*
*/
void bleprofile_WPEnable(UINT8 wp_enable);

/**
* \brief Check NVRAM
* \ingroup bleprofile
*
* \details Application can call this function to verify NVRAM.  Application traces will display result
* of the verification.  Please note that the contents of the NVRAM will be erased, so it is not 
* recommended to use this function other than during the development or module verification.
* 
*/
void bleprofile_NVRAMCheck(void);

/**
* \brief Read UART
* \ingroup bleprofile
*
* \details Application can call this function to read the content of the UART received buffer.  Application
* needs to provide at least \c READ_UART_LEN byte buffer.
* 
* \param data Pointer to the buffer to read UART to.
*
*/
void bleprofile_ReadUART(char *data);

/**
* \brief Read local GATT database by handle
* \ingroup bleprofile
*
* \details This function provides a way for the application to read local GATT database using 
* specific handle.  GATT database can modified internally or when peer writes data 
* over the air.
* 
* \param handle Handle in the GATT database.
* \param p_pdu Pointer to the \c BLEPROFILE_DB_PDU to retrieve the data to
*
* \return
*        \li \c 0 if the function succeeds.
*        \li \c error_code returned from the read operation. 
*
*/
INT32 bleprofile_ReadHandle(UINT16 hdl, BLEPROFILE_DB_PDU *p_pdu);

/**
* \brief Write data to GATT database by handle
* \ingroup bleprofile
*
* \details This function provides a way for the application to write data to GATT 
* database using specific handle.  
* 
* \param handle Handle in the GATT database.
* \param p_pdu Pointer to the \c BLEPROFILE_DB_PDU to write 
*
* \return
*        \li \c 0 if the function succeeds.
*        \li \c error_code returned from the write operation. 
*
*/
INT32 bleprofile_WriteHandle(UINT16 hdl, BLEPROFILE_DB_PDU *p_pdu);

/**
* \brief Send Write Request
* \ingroup bleprofile
*
* \details Client side application can call this function to send a write request to the server. 
* The \c handle can be GATT handle of the characteristic value, or a descriptor.  Before calling 
* this function application can register to receive notification with a result of write 
* operation using \c leatt_regWriteRspCb function.
* 
* \param attrHandle Handle of the attribute in the GATT database.
* \param data Pointer to the data chunk to be sent
* \param len Length of the data chunk to send
*
*/
void bleprofile_sendWriteReq(UINT16 attrHandle, UINT8 *attr, INT32 len);

/**
* \brief Send Write Command
* \ingroup bleprofile
*
* \details Client side application can call this function to send a write command to the server. 
* The \c handle can be GATT handle of the characteristic value, or a descriptor.
* 
* \param attrHandle Handle of the attribute in the GATT database.
* \param data Pointer to the data chunk to be sent
* \param len Length of the data chunk to send
*
*/
void bleprofile_sendWriteCmd(UINT16 attrHandle, UINT8 *data, INT32 len);

/**
* \brief Send Read Request
* \ingroup bleprofile
*
* \details Client side application can call this function to send a Read Request to the server. 
* Before calling this function application will typically register to receive notification
* with a read results using \c leatt_regReadRspCb function.
* 
* \param handle Handle of the attribute in the GATT database.
* \param data Pointer to the data chunk to be sent.
* \param len Length of the data chunk to send.
*
*/
void bleprofile_sendReadReq(UINT16 Handle);

/**
* \brief Send Read by Type Request
* \ingroup bleprofile
*
* \details Client side application can call this function to send a Read by Type Request to 
* the server. Before calling this function application will typically register to receive 
* notification with a read results using \c leatt_regReadByTypeRspCb function.  Application
* must specified range of the handles and UUID of the item to search for. This function
* usually is called by the client during GATT discovery.
* 
* \param start_handle Start handle in the GATT database.
* \param end_handle End handle in the GATT database.
* \param uuid 16 bit UUID to search for
*
*/
void bleprofile_sendReadByTypeReq(UINT16 startHandle, UINT16 endHandle, UINT16 uuid16);

/**
* \brief Send Read by Group Type Request
* \ingroup bleprofile
*
* \details Client side application can call this function to send a Read by Group Type Request 
* to the server. Before calling this function application will typically register to receive 
* notification with a read results using \c leatt_regReadByGroupTypeRspCb function.  Application
* must specified range of the handles and UUID of the item to search for. This function
* usually is called by the client during GATT discovery.
* 
* \param start_handle Start handle in the GATT database.
* \param end_handle End handle in the GATT database.
* \param uuid 16 bit UUID to search for
*
*/
void bleprofile_sendReadByGroupTypeReq(UINT16 startHandle, UINT16 endHandle, UINT16 uuid16 );

/**
* \brief Send Handle Value Confirmation
* \ingroup bleprofile
*
* \details Client side application should call this function when it receives indication 
* from the server to acknowledge the receipt. 
* 
*/
void bleprofile_sendHandleValueConf(void);

/**
* \brief Send Indication
* \ingroup bleprofile
*
* \details Server side application can call this function to send an Indication packet to the 
* client.  To receive indication the client needs to register with the server by writing 
* to Client Characteristic Configuration descriptor.  The \c handle is the GATT handle of 
* the characteristic value.  When client receives indication it sends back confirmation
* packet, and application receives it through the \c cb callback.  Application should not 
* send next indication until previous one has been confirmed.
* 
* \param handle Handle of the attribute in the GATT database.
* \param data Pointer to the data chunk to be sent.
* \param len Length of the data chunk to send.
* \param cb Callback function to be called when peer acks the indication.
*
*/
void bleprofile_sendIndication(UINT16 attrHandle, UINT8 *attr, INT32 len, LEATT_NO_PARAM_CB cb);

/**
* \brief Send Notification
* \ingroup bleprofile
*
* \details Server side application can call this function to send a Notification packet to 
* the client.  To receive a Notification the client needs to register with the server by 
* writing to Client Characteristic Configuration descriptor.  The \c handle is the GATT 
* handle of the characteristic value.  
* 
* \param handle Handle of the attribute in the GATT database.
* \param data Pointer to the data chunk to be sent.
* \param len Length of the data chunk to send.
*
*/
void bleprofile_sendNotification(UINT16 attrHandle, UINT8 *attr, INT32 len);

/**
* \brief Turn LED On
* \ingroup bleprofile
*
* \details This function turns on LED that is defined in BLE_PROFILE_GPIO_CFG.
*
*/
void bleprofile_LEDOn(void);

/**
* \brief Turn LED Off
* \ingroup bleprofile
*
* \details This function turns on LED that is defined in BLE_PROFILE_GPIO_CFG.
*
*/
void bleprofile_LEDOff(void);

/**
* \brief Perform LED blinking
* \ingroup bleprofile
*
* \details This function blinks LED that is defined in BLE_PROFILE_GPIO_CFG.
*
* \param on_ms LED on time (multiple of 12.5ms, MIN: 25ms, MAX: 1sec).
* \param off_ms LED off time (multiple of 12.5ms, MIN: 25ms, MAX: 1sec).
* \param num : number of times to blink.
*
*/
void bleprofile_LEDBlink(UINT16 on_ms, UINT16 off_ms, UINT8 num);

/**
* \brief Turn Buzzer On
* \ingroup bleprofile
*
* \details This function turns on Buzzer that is defined in BLE_PROFILE_GPIO_CFG.
* When PWM supporting GPIO (26-29) is used for buzzer, PWM is used to generate
* constant frequency.  When other GPIO is used for buzzer, FW will control beeping 
* sound and frequency may be changed.
*
*/
void bleprofile_BUZOn(void);

/**
* \brief Turn Buzzer Off
* \ingroup bleprofile
*
* \details This function turns off Buzzer that is defined in BLE_PROFILE_GPIO_CFG.
*
*/
void bleprofile_BUZOff(void);

/**
* \brief Play a Buzzer beep
* \ingroup bleprofile
*
* \details This function plays a beep tone over Buzzer that is defined in 
* BLE_PROFILE_GPIO_CFG.
* When PWM supporting GPIO (26-29) is used for buzzer, PWM is used to generate
* constant frequency.  When other GPIO is used for buzzer, FW will control beeping 
* sound and frequency may be changed.
*
* \param duration_ms : beeping duration (multiple of 12.5ms, MIN: 25ms, MAX: 1sec)
*
*/
void bleprofile_BUZBeep(UINT16 duration_ms);

/**
* \brief Change Buzzer beeping frequency
* \ingroup bleprofile
*
* \details This function changes frequency of beeping sound when PWM enabled 
* GPIO is used.  GPIO is configured in BLE_PROFILE_GPIO_CFG.
*
* \param preset Preset number. Following values can be usable.
*             \li HSP_SESSION_RFCOMM_CONNECTION_STATE_CHANGED - The RFCOMM connection state has changed. The application should examine the value of the bt_hsp_session::state member to determine weather the RFCOMM session has connected or disconnected. The RFCOMM session has connected if the HSP_SESSION_STATE_RFCOMM_CONNECTED bit is set in the bt_hsp_session::state member.
*             \li PWMBUZ_125 = 0  // 125hz
*             \li PWMBUZ_250 = 1  // 250hz
*             \li PWMBUZ_500 = 2  // 500hz
*             \li PWMBUZ_1000 = 3 // 1khz
*             \li PWMBUZ_2000 = 4 // 2khz
*             \li PWMBUZ_4000 = 5 // 4khz
*             \li PWMBUZ_8000 = 6 // 8khz
*             \li PWMBUZ_FREQ_MANUAL = 0xFF 
* \param init_value Initial value of PWM. only meaningful when PWMBUZ_FREQ_MANUAL
* \param toggle_val Toggle value of PWM. only meaningful when preset PWMBUZ_FREQ_MANUAL
*
*/
void bleprofile_PWMBUZFreq(UINT8 preset, UINT16 init_value, UINT16 toggle_val);

/**
* \brief Turn Battery monitoring on
* \ingroup bleprofile
*
* \details This function turns on battery monitor (ADC) GPIO.
*
*/
void bleprofile_BatOn(void);

/**
* \brief Turn Battery monitoring off
* \ingroup bleprofile
*
* \details This function turns off battery monitor (ADC) GPIO by using input disable mode.
*
*/
void bleprofile_BatOff(void);

/**
* \brief Turn UART on
* \ingroup bleprofile
*
* \details This function turns on UART pins.
*
*/
void bleprofile_UARTOn(void);

/**
* \brief Turn UART off
* \ingroup bleprofile
*
* \details This function turns off UART pins.
*
*/
void bleprofile_UARTOff(void);

/**
* \brief Turn on Peripheral UART TX 
* \ingroup bleprofile
*
* \details This function turns on Peripheral UART TX pin.
*
*/
void bleprofile_PUARTTxOn(void);

/**
* \brief Turn off Peripheral UART TX 
* \ingroup bleprofile
*
* \details This function turns off Peripheral UART TX pins.
*
*/
void bleprofile_PUARTTxOff(void);


/**
* \brief Turn on Peripheral UART RX 
* \ingroup bleprofile
*
* \details This function turns on Peripheral UART RX pin.
*
*/
void bleprofile_PUARTRxOn(void);

/**
* \brief Turn off Peripheral UART RX 
* \ingroup bleprofile
*
* \details This function turns off Peripheral UART RX pins.
*
*/
void bleprofile_PUARTRxOff(void);

/**
* \brief Read button 
* \ingroup bleprofile
*
* \details This function reads the state of the default (first) button.
*
* \param button_state 
*             \li -1 if button is not configured
*             \li 0 if button is not pressed
*             \li 1 if button is pressed
*
*/
INT8 bleprofile_ReadBut(void);

/**
* \brief Read button 1
* \ingroup bleprofile
*
* \details This function reads the state of the button 1.
*
* \param button_state 
*             \li -1 if button is not configured
*             \li 0 if button is not pressed
*             \li 1 if button is pressed
*
*/
INT8 bleprofile_ReadBut1(void);

/**
* \brief Read button 2
* \ingroup bleprofile
*
* \details This function reads the state of the button 2.
*
* \param button_state 
*             \li -1 if button is not configured
*             \li 0 if button is not pressed
*             \li 1 if button is pressed
*
*/
INT8 bleprofile_ReadBut2(void);

/**
* \brief Read button 3
* \ingroup bleprofile
*
* \details This function reads the state of the button 3.
*
* \param button_state 
*             \li -1 if button is not configured
*             \li 0 if button is not pressed
*             \li 1 if button is pressed
*
*/
INT8 bleprofile_ReadBut3(void);

/**
* \brief Convert UINT16 to SFLOAT
* \ingroup bleprofile
*
* \details A helper function to convert UINT16 into SFLOAT (2 byte float type used in some 
* medical profiles).
*
* \param uin 16 bit unsigned interger
*
* \return sfl SFLOAT value converted from \c uin.
*
*/
SFLOAT bleprofile_UINT16toSFLOAT(UINT16 uin);

/**
* \brief Convert SFLOAT to UINT16
* \ingroup bleprofile
*
* \details A helper function to convert SFLOAT (2 byte float type used in some medical 
* profiles into UINT16.
*
* \param sfl SFLOAT value
*
* \return uin 16 bit unsigned integer value converted from \c sfl.
*
*/
UINT16 bleprofile_SFLOATtoUINT16(SFLOAT sfl);

/**
* \brief Creates SFLOAT value
* \ingroup bleprofile
*
* \details A helper function to create an SFLOAT value (2 byte float type used in some 
* medical profiles) from sign, integer, and below decimal point.  For example -1.5 can 
* be inputted as minus = 1, uin = 1, bdp 5.
*
* \param minus When 0, result should be a positive number, if 1 the negative number.
* \param uin 8 bit unsigned interger above the decimal point.
* \param bdp 8 bit unsigned interger below the decimal point.
*
* \return sfl SFLOAT value converted from \c minus, \c uin and \c bdp.
*
*/
SFLOAT bleprofile_UINT8_UINT8toSFLOAT(UINT8 minus, UINT8 uin, UINT8 bdp);

/**
* \brief Convert UINT32 to FLOAT32
* \ingroup bleprofile
*
* \details A helper function to convert 32 bit unsigned integer value into FLOAT32 a 
* 4 byte float type used in some medical profiles.
*
* \param uin 32 bit unsigned interger
*
* \return sfl FLOAT32 value converted from \c uin.
*
*/
FLOAT32 bleprofile_UINT32toFLOAT32(UINT32 uin);

/**
* \brief Convert FLOAT32 to UINT32
* \ingroup bleprofile
*
* \details A helper function to convert FLOAT32 a 4 byte float type used in some medical 
* profiles into UINT32.
*
* \param sfl FLOAT32 value
*
* \return uin 32 bit unsigned integer value converted from \c sfl.
*
*/
UINT32 bleprofile_FLOAT32toUINT32(FLOAT32 fl);

/**
* \brief Creates FLOAT32 value
* \ingroup bleprofile
*
* \details A helper function to create a FLOAT32 value a 4 byte float type used in some 
* medical profiles from sign, integer, and below decimal point.  For example -1.5 can 
* be inputted as minus = 1, uin = 1, bdp 5.
*
* \param minus When 0, result should be a positive number, if 1 the negative number.
* \param uin 16 bit unsigned interger above the decimal point.
* \param bdp 16 bit unsigned interger below the decimal point.
*
* \return sfl FLOAT32 value converted from \c minus, \c uin and \c bdp.
*
*/
FLOAT32 bleprofile_UINT16_UINT16toFLOAT32(UINT8 minus, UINT16 uin, UINT16 bdp);

void bleprofile_FLOAT32Test(void);

/**
* \brief Get Discoverable State
* \ingroup bleprofile
*
* \details returns current discoverable state
*
* \return discoverable_state
*       \li NO_DISCOVERABLE              = 0,
*       \li LOW_DIRECTED_DISCOVERABLE    = 1,
*       \li HIGH_DIRECTED_DISCOVERABLE   = 2,
*       \li LOW_UNDIRECTED_DISCOVERABLE  = 3,
*       \li HIGH_UNDIRECTED_DISCOVERABLE = 4,
*
*/
UINT8 bleprofile_GetDiscoverable(void);

/**
* \brief Set Discoverable 
* \ingroup bleprofile
*
* \details This function sets ADV state
*
* \param state
*       \li NO_DISCOVERABLE              = 0,
*       \li LOW_DIRECTED_DISCOVERABLE    = 1,
*       \li HIGH_DIRECTED_DISCOVERABLE   = 2,
*       \li LOW_UNDIRECTED_DISCOVERABLE  = 3,
*       \li HIGH_UNDIRECTED_DISCOVERABLE = 4,
* \param bda Bluetooth address of the peer for directed advertisements, not use for
*           undirected advertisements
*
*/
void bleprofile_Discoverable(UINT8 Discoverable, BD_ADDR paired_addr);

void bleprofile_enteringDiscLowPowerMode(UINT32 lowPowerMode);
void bleprofile_abortingDiscLowPowerMode(UINT32 lowPowerMode);
void bleprofile_PrepareHidOff(void);

/**
* \brief Register callback to receive UART data
* \ingroup bleprofile
*
* \details Application can register a callback function to be notified when data is received over UART.
*
* \param data Pointer to the function to be called when data in the UART is available.
*
*/
void bleprofile_regHandleUARTCb(BLEPROFILE_DOUBLE_PARAM_CB cb);

/**
* \brief Register receive GPIO Interrupt 
* \ingroup bleprofile
*
* \details Application can register a callback function to be notified on the interrupt button.
* Whenever buttons (1-3) are pressed, registered callback function will be called.
* 
* \param callback Pointer to the application defined callback function that can handle interrupt.
*
*/
void bleprofile_regIntCb(BLEPROFILE_SINGLE_PARAM_CB callback);

/**
* \brief Register receive polling button callback 
* \ingroup bleprofile
*
* \details Application can register a callback function to be notified on the polling button push.
* Whenever the polling button is pressed for timeout values that defined in BLE_PROFILE_CFG,
* registered callback function will be called for additional user level action.
* 
* \param callback Pointer to the application defined callback function that can handle polling.
*
*/
void bleprofile_regButtonFunctionCb(BLEPROFILE_SINGLE_PARAM_CB cb);

/**
* \brief Callback function for polling button functionality
* \ingroup bleprofile
*
* \details This function can handle powersave, discover, client, filter timeout related 
* functions that are defined in button_power_timeout, button_client_timeout, 
* button_discover_timeout, button_filter_timeout of BLE_PROFILE_CFG config item. When 
* button is pressed for timeout (sec) of powersave, discover, client, filter function, 
* such function (powersaving, undirected ADV for pairing, client function (find me, 
* time, etc), toggle HCI mode) will be initiated.  
*
* This function will call user defined callback function that is registered by
* bleprofile_regButtonFunctionCb function.
*
*/
void bleprofile_ReadButton(void);

/**
* \brief Register receive timer callbacks
* \ingroup bleprofile
*
* \details This function registers fine timer and normal timer callback function.
* Normal timer expires every second and fine resolution timer is called every 12.5ms,
* configurable in BLE_PROFILE_CFG. 
* 
* \param fine_cb Pointer to the callback function for fine resolution timer timeout.
* \param normal_cb Pointer to the callback function for normal timer timeout.
*
*/
void bleprofile_regTimerCb(BLEAPP_TIMER_CB fine_cb, BLEAPP_TIMER_CB normal_cb);

/**
* \brief Start timers
* \ingroup bleprofile
*
* \details This function starts fine and normal timers.  Timers are
* configurable in BLE_PROFILE_CFG. 
* 
*/
void bleprofile_StartTimer(void);

/**
* \brief Stop timers
* \ingroup bleprofile
*
* \details This function stops fine and normal timers.  
* 
*/
void bleprofile_KillTimer(void);


/**
* \brief Register application event handler
* \ingroup bleprofile
*
* \details Register a callback to receive certain event.  Although registering for the
* notifications is optional and default processing is assumed, in most cases
* application will want to be notified and process the events.
* 
* \param idx Event for the registration.  Following events are defined.
*        \li BLECM_APP_EVT_START_UP, registeres a callback to be called on startup.
*        \li BLECM_APP_EVT_LINK_UP, radio link up event.
*        \li BLECM_APP_EVT_LINK_DOWN, raio link down event.
*        \li BLECM_APP_EVT_ADV_TIMEOUT, readio exiting the advertisement state based on timeout
*        \li BLECM_APP_EVT_ENTERING_HIDOFF, device is preparing to enter hid-ff
*        \li BLECM_APP_EVT_ABORTING_HIDOFF, device had to abort entering hid-off
* \param func Pointer to the function to be called when event occurs
*/
void bleprofile_regAppEvtHandler(BLECM_APP_EVT_ENUM idx, BLECM_NO_PARAM_FUNC func);

/**
* \brief Init Powersave
* \ingroup bleprofile
*
* \details bleprofile_Init function calls this function during profile initialization when powersave_timeout
* of BLE_PROFILE_CFG is set.  
* 
*/
void bleprofile_InitPowersave(void);

/**
* \brief Poll Powersave
* \ingroup bleprofile
*
* \details This function is called to check idle timeout powersaving condition is met or not. This function is
* usually called by timer timeout function.
* 
* \return powersave Returns 1 if powersave condition is met. 0 otherwise.
*/
UINT8 bleprofile_pollPowersave(void);

/**
* \brief Start Powersave
* \ingroup bleprofile
*
* \details This function starts powersafe mode
* 
*/
void bleprofile_StartPowersave(void);

/**
* \brief Stop Powersave
* \ingroup bleprofile
*
* \details This function is called during wake up from Sleep powersave mode. Wake from HID off mode
* does not call this function.
*
*/
void bleprofile_StopPowersave(void);

/**
* \brief Prepare Powersave
* \ingroup bleprofile
*
* \details This function initiates idle mode to prepare powersave mode.
*
*/
void bleprofile_PreparePowersave(void);

/**
* \brief Setup Scan Response
* \ingroup bleprofile
*
* \details This function sets up the target address in the scan response
* payload if there are bonded keys.
*
*/
void bleprofile_setupTargetAdrInScanRsp(void);

/**
* \brief Application Timer Callback
* \ingroup bleprofile
*
* \details This function is called when ADV, Connection related events happen. ADV related event can
* change ADV setting (eg. High undirected ADV -> Low undirected ADV). Connection Idle timeout
* event can disconnect the BLE connection.
*
* \param arg Type of events 
*   \li BLEAPP_APP_TIMER_ADV
*   \li BLEAPP_APP_TIMER_DIRECT_ADV
*   \li BLEAPP_APP_TIMER_CONN_IDLE
*
*/
void bleprofile_appTimerCb( UINT32 arg);

/**
* \brief Configure Idle timer depending on the notification state
* \ingroup bleprofile
*
* \details This function turns idle timer off if there is a characteristic with the client
* configuration descriptor set for notifications or indications.  If there are no characteristics
* currently set to send indication or notification, idle timer is stopped.
*
*/
void bleprofile_setidletimer_withNotification(void);

/**
* \brief Start Connection Idle Timer 
* \ingroup bleprofile
*
* \details Starts connection idle timer.
* \param Timeout value in seconds.
* \param Callback function to be called when timer expires.
* 
*/
void bleprofile_StartConnIdleTimer(UINT8 timeout, BLEAPP_TIMER_CB cb);

/**
* \brief Stop Connection Idle Timer 
* \ingroup bleprofile
*
* \details Stops connection idle timer 
* 
*/
void bleprofile_StopConnIdleTimer(void);

/**
* \brief Send Connection Parameters Update Request 
* \ingroup bleprofile
*
* \details The Connection Parameter Update Request allows the application to
* request a set of new connection parameters.  During connection establishement 
* and initial data exchange master typically sets parameters requiring very fast 
* polling which is typically not needed during normal connection.  
* 
* \param minInterval Defines minimum value for the connection event interval in 1.25ms
* shall be less or equal to maximum
* \param maxInterval Defines maximum value for the connection event interval in 1.25ms.
* \param slaveLatency Defines the slave latency parameter (as number of LL connection events).
* \param timeout Defines connection timeout parameter.  Timeout * 10ms is Connection 
* supervision timeout
* 
*/
void bleprofile_SendConnParamUpdateReq(UINT16 minInterval, UINT16 maxInterval, 
                                       UINT16 slaveLatency, UINT16 timeout);


void bleprofile_Sleep(UINT8 hund_us);

int bleprofile_PUARTTx(char *data, UINT8 len);
int bleprofile_PUARTTxMaxWait(char *data, UINT8 len, UINT8 ms);
int bleprofile_PUARTTxEchoMaxWait(char *data, UINT8 len, UINT8 tx_ms, UINT8 rx_ms, UINT8 flush);
int bleprofile_PUARTRx(char *data, UINT8 len);
int bleprofile_PUARTRxMaxFail(char *data, UINT8 len, UINT8 max);
int bleprofile_PUARTRxMaxWait(char *data, UINT8 len, UINT8 ms);

void bleprofile_PUARTInitTxWaterlevel(UINT8 puart_tx_waterlevel);
void bleprofile_PUARTSetTxWaterlevel(UINT8 puart_tx_waterlevel);
void bleprofile_PUART_EnableRxInt(UINT8 rxpin, void (*userfn)(void*));
void bleprofile_PUART_EnableRxFIFOInt(void (*userfn)(void*));
void bleprofile_PUARTSetRxWaterlevel(UINT8 puart_rx_waterlevel);
void bleprofile_PUARTRegisterRxHandler(void (*RxHandler)(void));
void bleprofile_PUARTRegisterCSARxHandler(void (*userfn)(void*));
void bleprofile_PUARTEnableAFFInt(void);
void bleprofile_PUART_deassertCts(void);
void bleprofile_PUART_assertCts(void);
void bleprofile_PUART_ConfigRTS(BOOL enabled, UINT8 rtsPortPin, UINT8 assertvalue);
void bleprofile_PUART_EnableRxFIFOCTSInt(UINT8 ctsPortPin, UINT8 waterLevel, UINT8 assertvalue, UINT8 disableINT);
INT32 bleprofile_PUARTRxCTS(char *data, UINT8 len);
INT32 bleprofile_PUARTRxMaxWaitCTS(char *data, UINT8 len, UINT16 us);
void bleprofile_PUART_RxIntCb(void *ptr, UINT8 portPin);

#endif
