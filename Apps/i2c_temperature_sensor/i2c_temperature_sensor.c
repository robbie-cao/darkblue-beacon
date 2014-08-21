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
* I2C Temperature Sensor Sample
*
* This application provides the sample code interfacing the TI LM73
* temperature sensor (http://www.ti.com/product/lm73)
* over the I2C interface.  The application initializes the I2C interface
* to communicate with the sensor, initializes it an retrieves the
* temperature which is displayed on the debug UART.
*
* Features demonstrated
*  - Use of the I2C driver interface
*  - Reading data from LM73 temperature sensor
*  - Writing data to LM73 temperature sensor
*  - Handling ALERT interrupts from the temperature sensor.
*
* To demonstrate the app, work through the following steps.
* 1. Connect LM73 sensor to I2C interface
*   a. If ALERT is used, connect ALERT pin to P4
* 2. Plug the WICED eval board into your computer
* 3. Build and download the application (to the WICED board)
* 4. Application reads temperature information from LM73
*    every second and prints it out to the debug output.
* 5. If alerts are enabled, application reads and prints
*    temperature in the interrupt handler.
*/
#define CONFIG_IN_NVRAM 1
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "bleappconfig.h"
#include "cfa.h"

/******************************************************
 *                      Constants
 ******************************************************/
// Enable temperature threshold alerts from LM73. See datasheet.
#define LM73_ALERTS_ENABLED

// LM73 definitions. See LM73 datasheet for register names and addresses.
#define LM73_TEMPERATURE_DATA_REG_ADDR  0x00
#define LM73_CONFIGURATION_REG_ADDR     0x01
#define LM73_UPPER_LIMIT_REG_ADDR       0x02
#define LM73_LOWER_LIMIT_REG_ADDR       0x03
#define LM73_CONTROL_STATUS_REG_ADDR    0x04
#define LM73_IDENTIFICATION_REG_ADDR    0x07

// LM32 I2C slave addresses. See datasheet for the two flavors of LM73
// and the state of the address pin (_F = Float, _G = GND, _V = Vdd).
// Shift left once for the slave address to use with the I2C driver.
#define LM73_0_SLAVE_ADDR_F      (0x48 << 1)
#define LM73_0_SLAVE_ADDR_G      (0x49 << 1)
#define LM73_0_SLAVE_ADDR_V      (0x4A << 1)

#define LM73_1_SLAVE_ADDR_F      (0x4C << 1)
#define LM73_1_SLAVE_ADDR_G      (0x4D << 1)
#define LM73_1_SLAVE_ADDR_V      (0x4E << 1)

// The slave address of the LM73 in this sample app.
#define LM73_SLAVE_ADDR    LM73_0_SLAVE_ADDR_F

// Configuration register bit definitions from the datasheet.
#define LM73_CONFIG_FULL_PD_ENABLE      (1 << 7)
#define LM73_CONFIG_RESERVED_ALWAYS_1   (1 << 6)
#define LM73_CONFIG_ALERT_DISABLE       (1 << 5)
#define LM73_CONFIG_ALERT_POLARITY_HI   (1 << 4)
#define LM73_CONFIG_ALERT_RESET_ENABLE  (1 << 3)
#define LM73_CONFIG_ONESHOT_MODE_ENABLE (1 << 2)

// LM73 control/status register bit definitions from the datasheet.
#define LM73_CONTROL_TIMEOUT_DISABLE           (1 << 7)
#define LM73_CONTROL_TEMP_RESOLUTION_p25       (0 << 5)
#define LM73_CONTROL_TEMP_RESOLUTION_p125      (1 << 5)
#define LM73_CONTROL_TEMP_RESOLUTION_p0625     (2 << 5)
#define LM73_CONTROL_TEMP_RESOLUTION_p03125    (3 << 5)
#define LM73_CONTROL_ALERT_PIN_STATUS_SET      (1 << 3)
#define LM73_CONTROL_TEMP_HIGH_FLAG_SET        (1 << 2)
#define LM73_CONTROL_TEMP_LOW_FLAG_SET         (1 << 1)
#define LM73_CONTROL_DATA_AVAILABLE_FLAG       (1 << 0)

// LM73 Identification register bits.
#define LM73_ID_MANUFACTURER_MASK              (0xFF << 8)
#define LM73_ID_PRODUCT_MASK                   (0x0F << 4)
#define LM73_ID_DIE_REVISION_MASK              (0x0F << 0)

// The temperature resolution used in this sample app. Choose others if required.
#define LM73_CONTROL_TEMP_RESOLUTION                (LM73_CONTROL_TEMP_RESOLUTION_p03125)

// The sign bit is the MSB
#define LM73_TEMP_DATA_SIGN_MASK                    (1 << 15)

// The mask to be applied to get the fractional portion of the temp.
#define LM73_TEMP_DATA_TO_DEG_FRACTIONAL_MASK       (0x7F)

// Shifts to apply on the data read when converting the integer part to deg C.
#define LM73_TEMP_DATA_TO_DEG_C_INTEGER_SHIFT       (7)

// Shifts to apply on the data read when converting the fractional part to deg C.
#define LM73_TEMP_DATA_TO_DEG_C_FRACTIONAL_SHIFT    (2)

// Sample app uses this UPPER limit (26 deg C). See LM73 datasheet for temperature encoding.
#define LM73_UPPER_LIMIT_TEMPERATURE                (0xD00)

// Sample app uses this LOWER limit (24 deg C). See LM73 datasheet for temperature encoding.
#define LM73_LOWER_LIMIT_TEMPERATURE                (0xC00)

// The GPIO port number to which the ALERT pin is connected.
#define LM73_ALERT_INTERRUPT_PORT                   (0)

// The GPIO pin number to which the ALERT pin is connected
#define LM73_ALERT_INTERRUPT_PIN                    (4)

// Read operation to the lower level driver is 0.
#define I2C_SLAVE_OPERATION_READ                    (0)

// Write operation to the lower level driver is 1.
#define I2C_SLAVE_OPERATION_WRITE                   (1)

// Absolute macro if not available.
#ifndef ABS
#define ABS(a)                                      (((a) < 0) ? -(a) : (a))
#endif

/******************************************************
 *               Function Prototypes
 ******************************************************/
static void  temperature_sensor_create(void);
static void  temperature_sensor_timeout( UINT32 count );
static void  temperature_sensor_fine_timeout( UINT32 finecount );
static UINT8 temperature_sensor_read_16_bit_register(UINT8 register_address, INT16* temperature_data);
static UINT8 temperature_sensor_write_16_bit_register(UINT8 register_address, INT16 temperature_data);
static UINT8 temperature_sensor_read_8_bit_register(UINT8 register_address, UINT8* config_value);
static UINT8 temperature_sensor_write_8_bit_register(UINT8 register_address, UINT8 register_value);
static UINT8 temperature_sensor_initialize(void);
static INT32 temperature_sensor_gcd ( INT32 a, INT32 b );
static void  temperature_sensor_read_and_print_temperature_data(void);

#ifdef LM73_ALERTS_ENABLED
static void temperature_sensor_gpio_interrupt_handler(void* parameter, UINT8);
#endif

// Helper macros for better sounding names.
// Temperature value register access.
#define TEMPERATURE_SENSOR_GET_CURRENT_TEMPERATURE(x)       temperature_sensor_read_16_bit_register(LM73_TEMPERATURE_DATA_REG_ADDR, (x))

// Status register access
#define TEMPERATURE_SENSOR_GET_CURRENT_STATUS(x)            temperature_sensor_read_8_bit_register(LM73_CONTROL_STATUS_REG_ADDR, (x))
#define TEMPERATURE_SENSOR_SET_CURRENT_STATUS(x)            temperature_sensor_write_8_bit_register(LM73_CONTROL_STATUS_REG_ADDR, (x))

// Configuration register access
#define TEMPERATURE_SENSOR_GET_CURRENT_CONFIGURATION(x)     temperature_sensor_read_8_bit_register(LM73_CONFIGURATION_REG_ADDR, (x))
#define TEMPERATURE_SENSOR_SET_CURRENT_CONFIGURATION(x)     temperature_sensor_write_8_bit_register(LM73_CONFIGURATION_REG_ADDR, (x))

// Lower temperature threshold register access
#define TEMPERATURE_SENSOR_GET_LOWER_TEMPERATURE_LIMIT(x)   temperature_sensor_read_16_bit_register(LM73_LOWER_LIMIT_REG_ADDR, (x))
#define TEMPERATURE_SENSOR_SET_LOWER_TEMPERATURE_LIMIT(x)   temperature_sensor_write_16_bit_register(LM73_LOWER_LIMIT_REG_ADDR, (x))

// Upper temperature threshold register access
#define TEMPERATURE_SENSOR_GET_UPPER_TEMPERATURE_LIMIT(x)   temperature_sensor_read_16_bit_register(LM73_UPPER_LIMIT_REG_ADDR, (x))
#define TEMPERATURE_SENSOR_SET_UPPER_TEMPERATURE_LIMIT(x)   temperature_sensor_write_16_bit_register(LM73_UPPER_LIMIT_REG_ADDR, (x))

// Read ID register
#define TEMPERATURE_SENSOR_GET_DEVICE_IDENTIFICAION(x)      temperature_sensor_read_16_bit_register(LM73_IDENTIFICATION_REG_ADDR, (x))

/******************************************************
 *               Variables Definitions
 ******************************************************/

/*
 * This is the GATT database for the Temperature Sensor application.  It is
 * currently empty with only required GAP and GATT services.
 */
const UINT8 temperature_sensor_gatt_database[]=
{
    // Handle 0x01: GATT service
    // Service change characteristic is optional and is not present
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x14: GAP service
    // Device Name and Appearance are mandatory characteristics.  Peripheral
    // Privacy Flag only required if privacy feature is supported.  Reconnection
    // Address is optional and only when privacy feature is supported.
    // Peripheral Preferred Connection Parameters characteristic is optional
    // and not present.
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value.
    // Any 16 byte string can be used to identify the sensor.  Just need to
    // replace the "Pressure sensor" string below.
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
                                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'T','e','m','p',' ','s','e','n','s','o','r',0x00,0x00,0x00,0x00,0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value.
    // List of approved appearances is available at bluetooth.org.  Current
    // value is set to 0x200 - Generic Tag
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
                                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00,0x02,

};

/*
 * This is the application configuration.
 * */
const BLE_PROFILE_CFG temperature_sensor_cfg =
{
        /*.fine_timer_interval            =*/ 1000, // ms
        /*.default_adv                    =*/ NO_DISCOVERABLE, // current sensor is not discoverable
        /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
        /*.high_undirect_adv_interval     =*/ 32,   // slots
        /*.low_undirect_adv_interval      =*/ 1024, // slots
        /*.high_undirect_adv_duration     =*/ 30,   // seconds
        /*.low_undirect_adv_duration      =*/ 300,  // seconds
        /*.high_direct_adv_interval       =*/ 0,    // seconds
        /*.low_direct_adv_interval        =*/ 0,    // seconds
        /*.high_direct_adv_duration       =*/ 0,    // seconds
        /*.low_direct_adv_duration        =*/ 0,    // seconds
        /*.local_name                     =*/ "Temp Sensor",  // [LOCAL_NAME_LEN_MAX];
        /*.cod                            =*/ "\x00\x00\x00", // [COD_LEN];
        /*.ver                            =*/ "1.00",         // [VERSION_LEN];
        /*.encr_required                  =*/ 0,    // data encrypted and device sends security request on every connection
        /*.disc_required                  =*/ 0,    // if 1, disconnection after confirmation
        /*.test_enable                    =*/ 1,    // TEST MODE is enabled when 1
        /*.tx_power_level                 =*/ 0x04, // dbm
        /*.con_idle_timeout               =*/ 3,    // second  0-> no timeout
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
const BLE_PROFILE_PUART_CFG temperature_sensor_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ GPIO_PIN_UART_TX,
    /*.rxpin      =*/ GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG temperature_sensor_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        GPIO_PIN_WP,      // This need to be used to enable/disable NVRAM write protect
        GPIO_PIN_BUTTON,  // Button GPIO is configured to trigger either direction of interrupt
        GPIO_PIN_LED,     // LED GPIO, optional to provide visual effects
        GPIO_PIN_BATTERY, // Battery monitoring GPIO. When it is lower than particular level, it will give notification to the application
        GPIO_PIN_BUZZER,  // Buzzer GPIO, optional to provide audio effects
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // other GPIOs are not used
    },
    /*.gpio_flag =*/
    {
        GPIO_SETTINGS_WP,
        GPIO_SETTINGS_BUTTON,
        GPIO_SETTINGS_LED,
        GPIO_SETTINGS_BATTERY,
        GPIO_SETTINGS_BUZZER,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

// Remote address when connected.
BD_ADDR temperature_sensor_remote_addr        = {0, 0, 0, 0, 0, 0};
/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)temperature_sensor_gatt_database,
                   sizeof(temperature_sensor_gatt_database),
                   (void *)&temperature_sensor_cfg,
                   (void *)&temperature_sensor_puart_cfg,
                   (void *)&temperature_sensor_gpio_cfg,
                   temperature_sensor_create);
}

// Create temperature sensor
void temperature_sensor_create(void)
{
    extern UINT32 blecm_configFlag;

    blecm_configFlag |= BLECM_DBGUART_LOG;

    ble_trace0("temperature_sensor_create()\n");

    // dump the database to debug uart.
    legattdb_dumpDb();

    // Initialize LM73 temperature sensor connected to the I2C bus.
    if (!temperature_sensor_initialize())
    {
        ble_trace0("Could not initialize LM73 temperature sensor.\n");
    }

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    bleprofile_regTimerCb(temperature_sensor_fine_timeout, temperature_sensor_timeout);
    bleprofile_StartTimer();
}

// Utility function to calculate the GCD of two numbers.
INT32 temperature_sensor_gcd ( INT32 a, INT32 b )
{
    INT32 c;

    while (a != 0)
    {
        c = a;
        a = b % a;
        b = c;
    }

    return b;
}

// Reads 16 bits from the given register address in LM73.
// \param register_address The address to the register to read.
// \param data INOUT Pointer to a INT16 into which the data is to be read, valid only if return value is 1.
// \return 1 if successful, 0 on failure.
UINT8 temperature_sensor_read_16_bit_register(UINT8 register_address, INT16* data)
{
    CFA_BSC_STATUS read_status;
    UINT8 return_value = 0;

    UINT8 reg_read_bytes_to_write[1];
    UINT8 reg_bytes_to_read[2];

    reg_read_bytes_to_write[0] = register_address;

    // Do a combo write then read operation
    read_status = cfa_bsc_OpExtended(reg_bytes_to_read, sizeof(reg_bytes_to_read), reg_read_bytes_to_write,
                                                        sizeof(reg_read_bytes_to_write), LM73_SLAVE_ADDR,
                                                        I2C_SLAVE_OPERATION_READ);
    switch(read_status)
    {
        case CFA_BSC_STATUS_INCOMPLETE:
            // Transaction did not go through. ERROR. Handle this case.
            break;
        case CFA_BSC_STATUS_SUCCESS:
            // The read was successful.
            *data = (INT16)(reg_bytes_to_read[1] | reg_bytes_to_read[0] << 8);
            return_value = 1;
            break;
        case CFA_BSC_STATUS_NO_ACK:
            // No slave device with this address exists on the I2C bus. ERROR. Handle this.
        default:
            break;
    }

    return return_value;
}

// Writes the given value ino the given 16 bit register.
// \param register_address The address of the register to write to.
// \param data The data to write.
// \return 1 on success; else 0.
UINT8 temperature_sensor_write_16_bit_register(UINT8 register_address, INT16 data)
{
    CFA_BSC_STATUS read_status;
    UINT8 return_value = 0;

    UINT8 reg_bytes_to_write[3];
    reg_bytes_to_write[0] = register_address;
    reg_bytes_to_write[1] = (data >> 8) & 0xFF;
    reg_bytes_to_write[2] = data & 0xFF;

    // Invoke the lower level driver. Non-combo transaction, so set offset parameters to NULL/0.
    read_status = cfa_bsc_OpExtended(reg_bytes_to_write, sizeof(reg_bytes_to_write), NULL, 0, LM73_SLAVE_ADDR,
                                                I2C_SLAVE_OPERATION_WRITE);

    switch(read_status)
    {
        case CFA_BSC_STATUS_INCOMPLETE:
            // Transaction did not go through. ERROR. Handle this case.
            break;
        case CFA_BSC_STATUS_SUCCESS:
            // The read was successful.
            return_value = 1;
            break;
        case CFA_BSC_STATUS_NO_ACK:
            // No slave device with this address exists on the I2C bus. ERROR. Handle this.
        default:
            break;
    }

    return return_value;
}

// Reads given register by first writing the pointer register
// then reading the register value. Leaves the pointer register as is when leaving.
// \param register_address The register to read from.
// \param data INOUT Pointer to a buffer into which to read. Valid only if the return value is 1.
// \return 1 on success; else 0.
UINT8 temperature_sensor_read_8_bit_register(UINT8 register_address, UINT8* data)
{
    CFA_BSC_STATUS read_status;
    UINT8 return_value = 0;

    UINT8 reg_bytes_to_read[1];

    // Do a combo write then read operation
    read_status = cfa_bsc_OpExtended(reg_bytes_to_read, sizeof(reg_bytes_to_read), &register_address,
                                                        sizeof(register_address), LM73_SLAVE_ADDR,
                                                        I2C_SLAVE_OPERATION_READ);

    switch(read_status)
    {
        case CFA_BSC_STATUS_INCOMPLETE:
            // Transaction did not go through. ERROR. Handle this case.
            break;
        case CFA_BSC_STATUS_SUCCESS:
            // The read was successful.
            *data = reg_bytes_to_read[0];
            return_value = 1;
            break;
        case CFA_BSC_STATUS_NO_ACK:
            // No slave device with this address exists on the I2C bus. ERROR. Handle this.
        default:
            break;
    }

    return return_value;
}

// Writes the given value to the given register by first writing the pointer register
// then writing the value. Leaves the pointer register as is when leaving.
// \param register_address The address of the register to write to.
// \param data The data to write to the register.
// \return 1 on success; else 0.
UINT8 temperature_sensor_write_8_bit_register(UINT8 register_address, UINT8 data)
{
    CFA_BSC_STATUS read_status;
    UINT8 return_value = 0;
    UINT8 reg_data_bytes[2];

    reg_data_bytes[0]= register_address;
    reg_data_bytes[1] = data;

    // Invoke the lower level driver. Non-combo transaction, so set offset parameters to NULL/0.
    read_status = cfa_bsc_OpExtended(reg_data_bytes, sizeof(reg_data_bytes), NULL, 0, LM73_SLAVE_ADDR,
                                                I2C_SLAVE_OPERATION_WRITE);

    switch(read_status)
    {
        case CFA_BSC_STATUS_INCOMPLETE:
            // Transaction did not go through. ERROR. Handle this case.
            break;
        case CFA_BSC_STATUS_SUCCESS:
            // The read was successful.
            return_value = 1;
            break;
        case CFA_BSC_STATUS_NO_ACK:
            // No slave device with this address exists on the I2C bus. ERROR. Handle this.
        default:
            break;
    }

    return return_value;
}

#ifdef LM73_ALERTS_ENABLED
// Handles an interrupt (ALERT) from the LM73 if configured.
// \param parameter Unused.
void temperature_sensor_gpio_interrupt_handler(void* parameter, UINT8 arg)
{
    UINT8 sensor_status = 0, sensor_configuration;

    ble_trace0("Alert!\n");
    gpio_clearPinInterruptStatus(LM73_ALERT_INTERRUPT_PORT, LM73_ALERT_INTERRUPT_PIN);

    // Read the status register first.
    if (TEMPERATURE_SENSOR_GET_CURRENT_STATUS(&sensor_status))
    {
        // Check which threshold was exceeded.
        if (sensor_status & LM73_CONTROL_TEMP_HIGH_FLAG_SET)
        {
            ble_trace0("Temperature HIGH flag is set.\n");
        }
        else if (sensor_status & LM73_CONTROL_TEMP_LOW_FLAG_SET)
        {
            ble_trace0("Temperature LOW flag is set.\n");
        }

        // If temperature data is available, retrieve it and print it out.
        if (sensor_status & LM73_CONTROL_DATA_AVAILABLE_FLAG)
        {
            temperature_sensor_read_and_print_temperature_data();
        }
    }
}
#endif

// Reads and prints the current temperature data from LM73 sensor if available.
void temperature_sensor_read_and_print_temperature_data(void)
{
    INT16 temperature_data;
    INT16 gcd;

    if (TEMPERATURE_SENSOR_GET_CURRENT_TEMPERATURE(&temperature_data))
    {
        // Get the absolute value.
        UINT16 absolute_temperature = ABS(temperature_data);

        // Get the fractional portion of the temperature in C.
        UINT8 fractional_temperature = (absolute_temperature & LM73_TEMP_DATA_TO_DEG_FRACTIONAL_MASK) >>
                LM73_TEMP_DATA_TO_DEG_C_FRACTIONAL_SHIFT;

        // Get the integer portion of the temperature in C.
        UINT8 integer_temperature = absolute_temperature >> LM73_TEMP_DATA_TO_DEG_C_INTEGER_SHIFT;

        ble_trace1("temperature_sensor_read_temperature_data returned %d\n", temperature_data);

        // The highest resolution possible with LM73 is 0.03125 deg C == 1/32 deg C.
        gcd = temperature_sensor_gcd(fractional_temperature, 32);

        // Now print out temperature.
        ble_trace3("The current temperature is: %d & %d/%d degrees C ", integer_temperature, fractional_temperature/gcd, 32/gcd);

        // Was it reported as a negative temperature?
        if (temperature_data & LM73_TEMP_DATA_SIGN_MASK)
        {
            ble_trace0("below 0\n");
        }
    }
    else
    {
        ble_trace0("Could not read temperature data.\n");
    }
}

// One second timer expired. Read the sensor data.
void temperature_sensor_timeout(UINT32 arg)
{
    UINT8 sensor_status = 0;

    ble_trace0("\nStarting new temperature measurement.\n");

    // Read status register to see if data is available.
    if (TEMPERATURE_SENSOR_GET_CURRENT_STATUS(&sensor_status))
    {
        ble_trace1("Sensor read status returned %X\n", sensor_status);

        if (sensor_status & LM73_CONTROL_DATA_AVAILABLE_FLAG)
        {
            // Yes, so read and print the current temperature.
            temperature_sensor_read_and_print_temperature_data();
        }
        else
        {
            ble_trace0("Temperature data not available in sensor.\n");
        }
    }
    else
    {
        ble_trace0("Unable to read status register.\n");
    }
}

void temperature_sensor_fine_timeout(UINT32 arg)
{

}

// Initializes LM73 temperature sensor with defaults for this app.
UINT8 temperature_sensor_initialize(void)
{
    UINT16 interrupt_handler_mask[3] = {0, 0, 0};
    UINT16 id;
    UINT8 default_configuration = LM73_CONFIG_RESERVED_ALWAYS_1;

    ble_trace0("Reading identification register...");

    if (TEMPERATURE_SENSOR_GET_DEVICE_IDENTIFICAION(&id))
    {
        ble_trace3("Temperature sensor identified as Manuf: %d, Prod: %d, Rev: %d\n", (id & LM73_ID_MANUFACTURER_MASK) >> 8,
                (id & LM73_ID_PRODUCT_MASK) >> 4, id & LM73_ID_DIE_REVISION_MASK);
    }
    else
    {
        ble_trace0("Unable to read device ID.\n");
    }

    ble_trace0("Configuring sensor...\n");

#ifdef LM73_ALERTS_ENABLED
    // Set up the interrupt mask for the GPIO interrupt
    interrupt_handler_mask[LM73_ALERT_INTERRUPT_PORT] |= (1 << LM73_ALERT_INTERRUPT_PIN);

    // Now register the interrupt handler.
    gpio_registerForInterrupt(interrupt_handler_mask, temperature_sensor_gpio_interrupt_handler, NULL);

    // Now configure the pin. We will use both edges so that we know when the upper and lower
    // thresholds are crossed.
    gpio_configurePin(LM73_ALERT_INTERRUPT_PORT, LM73_ALERT_INTERRUPT_PIN,
            GPIO_EN_INT_BOTH_EDGE | GPIO_PULL_UP, GPIO_PIN_OUTPUT_LOW);

    ble_trace0("Configuring Upper and Lower threshold registers...\n");

    // Write the upper limit register value.
    if (!TEMPERATURE_SENSOR_SET_UPPER_TEMPERATURE_LIMIT(LM73_UPPER_LIMIT_TEMPERATURE))
    {
        ble_trace0("Could not set upper limit register.\n");
    }

    // Write the lower limit register value.
    if (!TEMPERATURE_SENSOR_SET_LOWER_TEMPERATURE_LIMIT(LM73_LOWER_LIMIT_TEMPERATURE))
    {
        ble_trace0("Could not set lower limit register.\n");
    }
#else
    default_configuration |= LM73_CONFIG_ALERT_DISABLE;
#endif

    // Configure the sensor, optionally disabling alerts if not required.
    if (!TEMPERATURE_SENSOR_SET_CURRENT_CONFIGURATION(default_configuration))
        return 0;

    ble_trace0("Setting up resolution\n");

    // Set up the resolution required. Also turn off timeout timers to save some power.
    if (!TEMPERATURE_SENSOR_SET_CURRENT_STATUS(LM73_CONTROL_TIMEOUT_DISABLE | LM73_CONTROL_TEMP_RESOLUTION))
        return 0;

    ble_trace0("Sensor init complete\n");
    return 1;
}
