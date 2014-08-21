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
* BLE A4WP Power Receiving Unit (PRU)
*
* The BLE PRU application is designed to start A4WP ATT service and advertise
* with the A4WP specific data palyoad. When the application is started, 
* configures the device with PRU service and enables advertising with limited
* discoverable flag. The ADV payload will have the primary service handle for
* PTU to avoid the service discovery to achieve the 500ms registration
* period. As part of the registration procedure, the characteristic values
* exchanged are as per BSS 1.2.
* After the connection setup, the PRU continues to respond with the dynamic
* parameter values read from the PMU unit.
* PRU application is the sample application to implement the A4WP BLE protocol
* and it does not handle all the charging scenarios.
*
* Features demonstrated
*  - Registration with LE stack for various events
*  - Configure A4WP service and start advertisement with A4WP service data.
*  - Connection with PTU device
*  - Sample interfaces with Broadcom PMU (59350) for receiving the power from PTU coil
*
* To demonstrate the app, work through the following steps.
* 1. Build and download the application
* 2. Power On PRU to advertise
* 3. PRU gets connected with PTU and PTU is expected to read the dynamic parameters.
* 4. Monitor the communication between PTU and PRU through debug or OTA traces.
*/

#include "bleprofile.h"
#include "a4wp_power_receiver.h"
#include "spar_utils.h"

/******************************************************
 *               Constants
 ******************************************************/
#define BLEPRU_HDL_PRIMARY_SERVICE_A4WP_CHARGING              0x100
#define BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL   0x102
#define BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC    0x104
#define BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT     0x106
#define BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC     0x107
#define BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC    0x109
#define BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC   0x10b

/******************************************************
 *               Function Prototypes
 ******************************************************/
static void            blepru_Timeout(UINT32 count);
static void            blepru_FineTimeout(UINT32 finecount);
static int             blepru_writeCb(LEGATTDB_ENTRY_HDR *p);
static void            blepru_DBInit(void);
static void            blepru_ADVdata(void);
static void            blepru_advStop(void);
static void            blepru_appTimerCb(UINT32 arg);
static void            blepru_appFineTimerCb(UINT32 arg);
static void            blepru_connUp(void);
static void            blepru_connDown(void);
static void            blepru_Create(void);
#ifdef BLEPRU_PMU_INF_ENABLE
static A4WP_HAL_STATUS blepru_hal_notify(A4WP_HAL_ALERT_t alert);
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/

/*
 * This is the GATT database for the BLE PRU application.
 */
const UINT8 blepru_gatt_database[] =
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service change, handle 0x03 characteristic value.
    CHARACTERISTIC_UUID16 (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED,
                           LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
        0x00, 0x00, 0x00, 0x00, // value

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value.
    // Any 16 byte string can be used to identify the PRU. Just need to
    // replace the "Pressure sensor" string below.
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'W','P','T',' ','P','R','U', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value.
    // Current value is set to 0x0000
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00, 0x00,

    // Handle 0x2e: characteristic preferred connection param, handle 0x2f characteristic value.
    // 0x0050 (100ms preferred min connection interval)
    // 0x00A0 (200ms preferred max connection interval)
    // 0x0000 (0 preferred slave latency)
    // 0x03E8 (10000ms preferred supervision timeout)
    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),

        0x50, 0x00, 0xA0, 0x00, 0x00, 0x00, 0xE8, 0x03,

    // Handle 0x100: A4WP service
    PRIMARY_SERVICE_UUID16 (0x0100, UUID_SERVCLASS_A4WP),

#ifdef PRU_128BIT_UUID
    // Handle 0x101: characteristic PRU control, handle 0x102 characteristic value.
    // PRU control value - 5 octets, values are updated later at run time
    CHARACTERISTIC_UUID128_WRITABLE (0x0101, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL,
                                     GATT_UUID_A4WP_CHARGING_PRU_CONTROL_128,
                                     LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 5),
        0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x103: characteristic PTU static parameter, handle 0x104 characteristic value.
    // PTU static parameter value - 17 octets, values are updated later at run time
    CHARACTERISTIC_UUID128_WRITABLE (0x0103, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC,
                                     GATT_UUID_A4WP_CHARGING_PTU_STATIC_128,
                                     LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 17),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x105: characteristic PRU alert, handle 0x106 characteristic value.
    // PTU static parameter value - 17 octets, values are updated later at run time
    CHARACTERISTIC_UUID128 (0x0105, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT,
                            GATT_UUID_A4WP_CHARGING_PRU_ALERT_128,
                            LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                            LEGATTDB_PERM_READABLE, 1),
        0x00,

    // Handle 0x107: characteristic descriptor for PRU alert.
    // value 2 octets, value updated at run time.
    CHAR_DESCRIPTOR_UUID16_WRITABLE(BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x108: characteristic PRU static parameter, handle 0x109 characteristic value.
    // PRU static parameter value - 20 octets, values are updated later at run time
    CHARACTERISTIC_UUID128 (0x0108, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC,
                            GATT_UUID_A4WP_CHARGING_PRU_STATIC_128,
                            LEGATTDB_CHAR_PROP_READ,
                            LEGATTDB_PERM_READABLE, 20),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x10a: characteristic PRU dynamic parameter, handle 0x10b characteristic value.
    // PRU dynamic parameter value - 20 octets, values are updated later at run time
    CHARACTERISTIC_UUID128 (0x010a, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC,
                            GATT_UUID_A4WP_CHARGING_PRU_DYNAMIC_128,
                            LEGATTDB_CHAR_PROP_READ,
                            LEGATTDB_PERM_READABLE, 20),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#else
    // 16 bit UUID for characteristics for testing purpose

    // Handle 0x101: characteristic PRU control, handle 0x102 characteristic value.
    // PRU control value - 5 octets, values are updated later at run time
    CHARACTERISTIC_UUID16_WRITABLE (0x0101, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL,
                                    GATT_UUID_A4WP_CHARGING_PRU_CONTROL,
                                    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 5),
        0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x103: characteristic PTU static parameter, handle 0x104 characteristic value.
    // PTU static parameter value - 17 octets, values are updated later at run time
    CHARACTERISTIC_UUID16_WRITABLE (0x0103, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC,
                                    GATT_UUID_A4WP_CHARGING_PTU_STATIC,
                                    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE | LEGATTDB_CHAR_PROP_WRITE,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 17),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x105: characteristic PRU alert, handle 0x106 characteristic value.
    // PTU static parameter value - 17 octets, values are updated later at run time
    CHARACTERISTIC_UUID16 (0x0105, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT,
                           GATT_UUID_A4WP_CHARGING_PRU_ALERT,
                           LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                           LEGATTDB_PERM_READABLE, 1),
        0x00,

    // Handle 0x107: characteristic descriptor for PRU alert.
    // value 2 octets, value updated at run time.
    CHAR_DESCRIPTOR_UUID16_WRITABLE(BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x108: characteristic PRU static parameter, handle 0x109 characteristic value.
    // PRU static parameter value - 20 octets, values are updated later at run time
    CHARACTERISTIC_UUID16 (0x0108, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC, GATT_UUID_A4WP_CHARGING_PRU_STATIC,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE, 20),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // Handle 0x10a: characteristic PRU dynamic parameter, handle 0x10b characteristic value.
    // PRU dynamic parameter value - 20 octets, values are updated later at run time
    CHARACTERISTIC_UUID16 (0x010a, BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC, GATT_UUID_A4WP_CHARGING_PRU_DYNAMIC,
                           LEGATTDB_CHAR_PROP_READ,
                           LEGATTDB_PERM_READABLE, 20),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif
};

const BLE_PROFILE_CFG blepru_cfg =
{
    /*.fine_timer_interval            =*/ 12,   // ms
    /*.default_adv                    =*/ HIGH_UNDIRECTED_DISCOVERABLE,
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "WPT PRU", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(0x0000),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0, // (SECURITY_ENABLED | SECURITY_REQUEST),
    /*.disc_required                  =*/ 1,    // actions on disconnection
    /*.test_enable                    =*/ 0,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0x4,  // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ { 0x00, 0x00, 0x00, 0x00, 0x00 }, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ { 0x00, 0x00, 0x00, 0x00, 0x00 },
    /*.cha                            =*/ { 0x00, 0x00, 0x00, 0x00, 0x00 },
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 1,    // if 1 grouptype read can be used
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
    /*.buz_on_ms                      =*/ 0  ,  // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,    // seconds
    /*.button_client_timeout          =*/ 1,    // seconds
    /*.button_discover_timeout        =*/ 3,    // seconds
    /*.button_filter_timeout          =*/ 0,    // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG blepru_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ BLEPRU_GPIO_PIN_UART_TX,
    /*.rxpin      =*/ BLEPRU_GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG blepru_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        BLEPRU_GPIO_PIN_WP,             // This need to be used to enable/disable NVRAM write protect
        BLEPRU_GPIO_PIN_PMU_INT,        // GPIO connected with PMU for interrupts
        BLEPRU_GPIO_PIN_BOOT_COMPLETE,  // GPIO to indicate boot up complete, used to enable the I2C lines to PMU
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // other GPIOs are not used
    },
    /*.gpio_flag =*/
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        GPIO_INPUT | GPIO_INIT_HIGH | GPIO_BUTTON1 | GPIO_INT,
        GPIO_OUTPUT | GPIO_INIT_HIGH,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

// timer counters
static UINT32 blepru_appfinetimer_count     = 0;
static UINT32 blepru_apptimer_count         = 0;

// LE connection handle
static UINT16 blepru_connhandle             = 0;

// to remember the generation of alert
static UINT8  blepru_alert_triggered        = FALSE;

// default PRU static parameter
static BLEPRU_PRU_STATIC blepru_pru_static  =
{
    0x00,   // Optional fields validity
    0x00,   // A4WP Supported Revision
    0x00,   // rfu 
    0x03,   // Category of PRU
    0x08,   // Capabilites of PRU,
    0x01,   // HW rev
    0x01,   // SW rev
    0x32,   // Prect_max
    0x157C, // Vrect_min, 5500mV
    0x3C23, // Vrect_high, 15395mV
    0x2710, // Vrect_set,  10000mV
    0x0000, // Delta R1
    0x00,   // RFU (4 octets)
    0x00,
    0x00,
    0x00
};

// default PRU dynamic parameter
static BLEPRU_PRU_DYNAMIC blepru_pru_dynamic =
{
    0xFC,
    0x2710,
    0x01F4,
    0x1388,
    0x03E8,
    0x3c,
    0x1388,
    0x2710,
    0x3A98,
    0x00,
    { 0x0, 0x0, 0x0 }
};

// Following variables are in ROM
extern UINT8  bleapp_trace_enable;


/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)blepru_gatt_database,
                   sizeof(blepru_gatt_database),
                   (void *)&blepru_cfg,
                   (void *)&blepru_puart_cfg,
                   (void *)&blepru_gpio_cfg,
                   blepru_Create);

    bleapp_trace_enable = 1;
}

// This is entry function for blepru application.
void blepru_Create(void)
{
    // Mux the lines to switch from Serial flash mode to I2C to communicate with PMU chip.
    a4wp_hal_i2c_mux_configure();

    ble_trace0("blepru_Create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    // this would generate the adv payload and scan rsp payload.
    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // Load the ATT handles and update the default values to DB.
    blepru_DBInit();

    // Set the A4WP ADV payload
    blepru_ADVdata();
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blepru_advStop);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blepru_connUp);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blepru_connDown);

    // write DB cb registration
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blepru_writeCb);

#ifdef BLEPRU_PMU_INF_ENABLE
    // Initialize the PMU and enable the PMU A1 work around for Iout measurement.
    a4wp_hal_init(blepru_hal_notify, GPIO_BUTTON1, TRUE);
#endif

    // register for timer callback and start the timer.
    bleprofile_regTimerCb(blepru_appFineTimerCb, blepru_appTimerCb);
    bleprofile_StartTimer();

    // make the PRU to discoverable
    blepru_connDown();
}

#ifdef BLEPRU_PMU_INF_ENABLE
// Callback function handler to process the alerts from a4wp hal layer
A4WP_HAL_STATUS blepru_hal_notify(A4WP_HAL_ALERT_t alert)
{
    // update Alert
    BLEPROFILE_DB_PDU db_pdu;

    ble_trace0("========================================Alert");

    // Update the alert field in dynamic parameters
    blepru_pru_dynamic.pru_alert = (UINT8)(alert & 0xFF);

    memcpy(db_pdu.pdu, &blepru_pru_dynamic, sizeof(BLEPRU_PRU_DYNAMIC));
    db_pdu.len = sizeof(BLEPRU_PRU_DYNAMIC);
    bleprofile_WriteHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC, &db_pdu);

    // update the alert characteristic in DB
    memcpy(db_pdu.pdu, &(blepru_pru_dynamic.pru_alert), 1);
    db_pdu.len = 1;
    bleprofile_WriteHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT, &db_pdu);

    // when valid alert present, set global to trigger alert in timeout
    if (alert)
    {
        blepru_alert_triggered = TRUE;
    }
    else
    {
        blepru_alert_triggered = FALSE;
    }

    return A4WP_HAL_STATUS_SUCCESS;
}
#endif

// application timeout handling (second's)
void blepru_Timeout(UINT32 count)
{
    if (blepru_connhandle)
    {
        if (blepru_alert_triggered)
        {
            BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;

            // check client characteristic configuration descriptor
            bleprofile_ReadHandle(BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC, &db_cl_pdu);

            if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
            {
                bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT, &db_pdu);

                // send notification only when notification is enabled by PTU
                // and alert is triggered locally.
                bleprofile_sendNotification(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT,
                                            (UINT8 *)db_pdu.pdu, db_pdu.len);
            }
        }
    }
}

// fine timeout handling (tick = 12.5ms)
void blepru_FineTimeout(UINT32 finecount)
{
#ifdef BLEPRU_PMU_INF_ENABLE
    // read the dynamic parameters every 100ms and update the ATT db.
    if (finecount % 8 == 0)
    {
        BLEPROFILE_DB_PDU db_pdu;

        blepru_pru_dynamic.v_rect       = (UINT16)a4wp_hal_reg_read(A4WP_REG_VRECT);
        blepru_pru_dynamic.i_rect       = (UINT16)a4wp_hal_reg_read(A4WP_REG_IRECT);
        blepru_pru_dynamic.v_out        = (UINT16)a4wp_hal_reg_read(A4WP_REG_VOUT);
        blepru_pru_dynamic.i_out        = (UINT16)a4wp_hal_reg_read(A4WP_REG_IOUT);
        blepru_pru_dynamic.temperature  = a4wp_hal_reg_read(A4WP_REG_TEMP);

        memcpy(db_pdu.pdu, &blepru_pru_dynamic, sizeof(BLEPRU_PRU_DYNAMIC));
        db_pdu.len = sizeof(BLEPRU_PRU_DYNAMIC);
        bleprofile_WriteHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC, &db_pdu);
    }
#endif
}

// this function loads the a4wp characteristic handles from the DB and updates
// the default PRU static and dynamic parameter values in DB.
void blepru_DBInit(void)
{
    BLEPROFILE_DB_PDU   db_pdu;

    ble_trace1("blepru_primary_hdl:%04x\n", BLEPRU_HDL_PRIMARY_SERVICE_A4WP_CHARGING);

    bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL, &db_pdu);
    ble_trace1("blepru_control_hdl:%04x\n", BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC, &db_pdu);
    ble_trace1("blepru_ptu_static_hdl:%04x\n", BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT, &db_pdu);
    ble_trace1("blepru_alert_hdl:%04x\n", BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_ALERT);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    bleprofile_ReadHandle(BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC, &db_pdu);
    ble_trace1("blepru_alert_ccc_hdl:%04x\n", BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC, &db_pdu);
    ble_trace1("blepru_static_hdl:%04x\n", BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    bleprofile_ReadHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC, &db_pdu);
    ble_trace1("blepru_dynamic_hdl:%04x\n", BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);

    // update PRU static and dynamic characteristic values
    memcpy(db_pdu.pdu, &blepru_pru_static, sizeof(BLEPRU_PRU_STATIC));
    db_pdu.len = sizeof(BLEPRU_PRU_STATIC);
    bleprofile_WriteHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_STATIC, &db_pdu);

    memcpy(db_pdu.pdu, &blepru_pru_dynamic, sizeof(BLEPRU_PRU_DYNAMIC));
    db_pdu.len = sizeof(BLEPRU_PRU_DYNAMIC);
    bleprofile_WriteHandle(BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_DYNAMIC, &db_pdu);
}

// function to build and update the A4WP ADV payload with controller.
void blepru_ADVdata(void)
{
    BLE_ADV_FIELD       adv_field[2];
    BLEPRU_ADV_PAYLOAD  adv_payload;

    // flags
    adv_field[0].len                        = FLAGS_LEN + 1;
    adv_field[0].val                        = ADV_FLAGS;
    adv_field[0].data[0]                    = LE_LIMITED_DISCOVERABLE;

    // a4wp spec. 6.5.1, 6.5.1.1
    adv_field[1].len                        = sizeof(BLEPRU_ADV_PAYLOAD) + 1;
    adv_field[1].val                        = ADV_SERVICE_DATA;
    adv_payload.wpt_service_uuid            = UUID_SERVCLASS_A4WP;

    adv_payload.gatt_primary_service_handle = BLEPRU_HDL_PRIMARY_SERVICE_A4WP_CHARGING;
    adv_payload.pru_rssi_parameters         = 0xFF;
    adv_payload.adv_flag                    = BLEPRU_CAT3;

    memcpy(&(adv_field[1].data), (UINT8 *)&adv_payload,
           sizeof(BLEPRU_ADV_PAYLOAD));

    bleprofile_GenerateADVData(adv_field, 2);
}

// LE connection complete callback
void blepru_connUp(void)
{
    blepru_connhandle = emconinfo_getConnHandle();
    ble_trace1("blepru conn up: %02x",blepru_connhandle );
}

// LE connection down cb
void blepru_connDown(void)
{
    ble_trace0("connDown:");
    ble_trace1("DiscReason:%02x", emconinfo_getDiscReason());

    blepru_connhandle = 0;

    // Mandatory discovery mode
    if (bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, NULL);
    }
}

// restart ADV after timeout
void blepru_advStop(void)
{
    ble_trace0("Restart ADV");

    blepru_connDown();
}

// Second timer callback
void blepru_appTimerCb(UINT32 arg)
{
    switch (arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
        {
            blepru_apptimer_count ++;
            blepru_Timeout(blepru_apptimer_count);
            break;
        }
    }
}

// fine timer callback
void blepru_appFineTimerCb(UINT32 arg)
{
    blepru_appfinetimer_count ++;
    blepru_FineTimeout(blepru_appfinetimer_count);
}

// ATT characteristic write callback, called when PTU writes characteristic value.
int blepru_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16                    handle    = legattdb_getHandle(p);
    int                       len       = legattdb_getAttrValueLen(p);
    UINT8                     *attrPtr  = legattdb_getAttrValue(p);
    BLEPRU_PRU_CTRL_CHAR_VAL  *p_ctrl_data;

    ble_trace2("PRU Write handle =%04x, length = %04x", handle, len);
    ble_tracen ((char *) attrPtr, len );

    if (handle == BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PRU_CONTROL)
    {
        ble_trace0("blepru_control_hdl");
        p_ctrl_data = (BLEPRU_PRU_CTRL_CHAR_VAL *)attrPtr;

#ifdef BLEPRU_PMU_INF_ENABLE
        if (p_ctrl_data->enables.s.output)
        {
            // Enable charging
            // Enable the VBUCK to start charging
            a4wp_hal_reg_write(A4WP_REG_CTRL, A4WP_CTRL_CHARGING_EN | A4WP_CTRL_CHARGING_MASK);
        }
        else
        {
            //Disable charging
            a4wp_hal_reg_write(A4WP_REG_CTRL, A4WP_CTRL_CHARGING_EN);
        }
#endif
    }
    else if (handle == BLEPRU_HDL_CHARACTERISTIC_A4WP_CHARGING_PTU_STATIC)
    {
        ble_trace0("blepru_ptu_static_hdl");
    }
    else if (handle == BLEPRU_HDL_DESCRIPTOR_A4WP_CHARGING_PRU_ALERT_CCC)
    {
        ble_trace0("blepru_alert_ccc_hdl");
    }

    return 0;
}

