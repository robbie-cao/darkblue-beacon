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
* SPI Pressure Sensor Sample
*
* This application provides the sample code interfacing the MS5525DSO
* pressure sensor (http://www.meas-spec.com/product/pressure/MS5525DSO.aspx)
* over the SPI interface.  The application initializes the second SPI interface
* to communicate with the sensor to get calibration data then read the pressure
* and temperature which is displayed on the debug UART.  See application note
* http://www.amsys.de/sheets/amsys.de.an520_e.pdf on the calculation details.
*
* Features demonstrated
*  - Use of the SPI driver interface
*  - Reading data from MS5525DSO pressure sensor
*
* To demonstrate the app, work through the following steps.
* 1. Connect MS5525DSO pressure sensor to SPI interface
*    (Use P24 for CLK, P4 for MOSI and P25 for MISO)
* 2. Plug the WICED eval board into your computer
* 3. Build and download the application (to the WICED board)
* 4. Application reads pressure and temperature information from MS5525DSO
*    every second and prints it out to the debug output.
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"

#include "spiffydriver.h"
#include "bleappconfig.h"


/******************************************************
 *                      Constants
 ******************************************************/

// use P15 for CS
#define CS_PORT      0
#define CS_PIN       15

// Use 1M speed
#define SPEED        1000000

// CS is active low
#define CS_ASSERT    0
#define CS_DEASSERT  1

//MS5525 definitions
#define CMD_RESET    0x1E // ADC reset command
#define CMD_ADC_READ 0x00 // ADC read command
#define CMD_ADC_CONV 0x40 // ADC conversion command
#define CMD_ADC_D1   0x00 // ADC D1 conversion
#define CMD_ADC_D2   0x10 // ADC D2 conversion
#define CMD_ADC_256  0x00 // ADC OSR=256
#define CMD_ADC_512  0x02 // ADC OSR=512
#define CMD_ADC_1024 0x04 // ADC OSR=1024
#define CMD_ADC_2048 0x06 // ADC OSR=2056
#define CMD_ADC_4096 0x08 // ADC OSR=4096
#define CMD_PROM_RD  0xA0 // Prom read command (A0 to AE)

/******************************************************
 *               Function Prototypes
 ******************************************************/

static void   pressure_sensor_create(void);
static void   pressure_sensor_timeout( UINT32 count );
static void   pressure_sensor_fine_timeout( UINT32 finecount );
static void   spiffy2_master_initialize(void);
static void   cmd_reset(void);
static UINT32 cmd_adc(char cmd);
static UINT16 cmd_prom(char coef_num);
static UINT8  crc4(UINT16 n_prom[]);

extern void   bleapputils_delayUs(UINT32 delay);

/******************************************************
 *               Variables Definitions
 ******************************************************/

/*
 * This is the GATT database for the Pressure Sensor application.  It is
 * currently empty with only required GAP and GATT services.
 */
const UINT8 pressure_sensor_gatt_database[]=
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
       'P','r','e','s','s','u','r','e',' ','S','e','n','s','o','r',0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value.
    // List of approved appearances is available at bluetooth.org.  Current
    // value is set to 0x200 - Generic Tag
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
                                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x00,0x02,

};

const BLE_PROFILE_CFG pressure_sensor_cfg =
{
    /*.fine_timer_interval            =*/ 1000, // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 300,  // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "Pressure Sensor", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ "\x00\x00\x00", // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ (SECURITY_ENABLED | SECURITY_REQUEST),    // data encrypted and device sends security request on every connection
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
const BLE_PROFILE_PUART_CFG pressure_sensor_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG pressure_sensor_gpio_cfg =
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

UINT32  pressure_sensor_timer_count        = 0;
UINT16  pressure_sensor_Coef[8]; // calibration coefficients

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)pressure_sensor_gatt_database,
                   sizeof(pressure_sensor_gatt_database),
                   (void *)&pressure_sensor_cfg,
                   (void *)&pressure_sensor_puart_cfg,
                   (void *)&pressure_sensor_gpio_cfg,
                   pressure_sensor_create);
}

// Create pressure sensor
void pressure_sensor_create(void)
{
    extern UINT32 blecm_configFlag;

    INT8  i;
    UINT8 n_crc; // crc value of the prom


    blecm_configFlag |= BLECM_DBGUART_LOG;

    ble_trace0("pressure_sensor_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // Initialization for SPI2 interface
    spiffy2_master_initialize();

    // Initialization for MS5525
    cmd_reset(); // reset the module after powerup
    for (i = 0; i < 8; i++)
    {
        bleapputils_delayUs(1000);  // delay to slow down debug print only
        pressure_sensor_Coef[i]=cmd_prom(i);
        ble_trace2("C[%d]=%d\n", i, pressure_sensor_Coef[i]);

    } // read calibration coefficients

    n_crc=crc4(pressure_sensor_Coef);

    bleapputils_delayUs(5000);
    ble_trace1("crc check =%d\n", n_crc);

    bleprofile_regTimerCb(pressure_sensor_fine_timeout, pressure_sensor_timeout);
    bleprofile_StartTimer();
}

void pressure_sensor_timeout(UINT32 arg)
{
    UINT32 D1;  // ADC value of the pressure conversion
    UINT32 D2;  // ADC value of the temperature conversion

    INT64 P;    // compensated pressure value
    INT64 T;    // compensated temperature value
    INT64 dT;   // difference between actual and measured temperature
    INT64 OFF;  // offset at actual temperature
    INT64 SENS; // sensitivity at actual temperature

#if(1)
    ble_trace1("pressure_sensor ADC_256 : %d\n", pressure_sensor_timer_count);
    D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_256); // read uncompensated pressure
    D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_256); // read uncompensated temperature

#else
    // cycle through the different conversion times, noting that some take longer and need more time
    switch (pressure_sensor_timer_count & 0x07)
    {
        case 0:
            ble_trace1("pressure_sensor ADC_256 : %d\n", pressure_sensor_timer_count);
            D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_256); // read uncompensated pressure
            D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_256); // read uncompensated temperature
            break;
        case 1:
            ble_trace1("pressure_sensor ADC_512 : %d\n", pressure_sensor_timer_count);
            D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_512); // read uncompensated pressure
            D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_512); // read uncompensated temperature
            break;
        case 2:
            ble_trace1("pressure_sensor ADC_1024 : %d\n", pressure_sensor_timer_count);
            D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_1024); // read uncompensated pressure
            D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_1024); // read uncompensated temperature
            break;
        case 3:
            ble_trace1("pressure_sensor ADC_2048 : %d\n", pressure_sensor_timer_count);
            D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_2048); // read uncompensated pressure
            D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_2048); // read uncompensated temperature
            break;
        case 5:
            ble_trace1("pressure_sensor ADC_4096 : %d\n", pressure_sensor_timer_count);
            D1=cmd_adc(CMD_ADC_D1 + CMD_ADC_4096); // read uncompensated pressure
            D2=cmd_adc(CMD_ADC_D2 + CMD_ADC_4096); // read uncompensated temperature
            break;
        default:
            return;
            break;
    }
#endif
    // calculate 1st order pressure and temperature (MS5607 1st order algorithm)
    dT   = (INT64)D2 - ((INT64)pressure_sensor_Coef[5] << 8);
    OFF  = ((INT64)pressure_sensor_Coef[2] << 17) + (((INT64)dT * (INT64)pressure_sensor_Coef[4]) >> 6);
    SENS = ((INT64)pressure_sensor_Coef[1] << 16) + (((INT64)dT * (INT64)pressure_sensor_Coef[3]) >> 7);
    T    = ((dT * pressure_sensor_Coef[6]) >> 22) + 2000;
    P    = ( (((INT64)D1 * SENS) >> 21) - OFF) >> 15;


    ble_trace2("D2:%d, D1:%d\n", D2, D1);
    //ble_trace3("dT:%d, OFF:%d, SENS:%d\n", dT, OFF, SENS);
    ble_trace2("Temp:%d, Pressure:%d\n", T, P);


}

void pressure_sensor_fine_timeout(UINT32 arg)
{
}


// Initializes SPIFFY2 as a SPI master using P24 for SCLK,
// P4 for MOSI, P25 for MISO and P15 for CS.
void spiffy2_master_initialize(void)
{
    // Use SPIFFY2 interface as master
    spi2PortConfig.masterOrSlave = MASTER2_CONFIG;

    // pull for MISO for master, MOSI/CLOCK/CS if slave mode
    spi2PortConfig.pinPullConfig = INPUT_PIN_PULL_UP;

    // Use P24 for CLK, P4 for MOSI and P25 for MISO
    spi2PortConfig.spiGpioConfig = MASTER2_P24_CLK_P04_MOSI_P25_MISO;

    // Initialize SPIFFY2 instance
    spiffyd_init(SPIFFYD_2);

    // Define this to the Port/Pin you want to use for CS.
    // Port = P#/16 and PIN = P# % 16
    // Configure the CS pin and deassert it initially.
    // If enabling output, you only need to configure once. Use gpio_setPinOutput to toggle value being o/p
    gpio_configurePin(CS_PORT, CS_PIN, GPIO_OUTPUT_ENABLE | GPIO_INPUT_DISABLE, CS_DEASSERT);

    // Configure the SPIFFY2 HW block
    spiffyd_configure(SPIFFYD_2, SPEED, SPI_MSB_FIRST, SPI_SS_ACTIVE_LOW, SPI_MODE_3);
}


//********************************************************
//! @brief send reset sequence
//!
//! @return 0
//********************************************************
void cmd_reset(void)
{
    UINT8 byteToSend = CMD_RESET;

    // pull CSB low to start the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect
    bleapputils_delayUs(1000);

    spiffyd_txData(SPIFFYD_2, 1 ,&byteToSend); // send reset sequence
    bleapputils_delayUs(3000);  // wait for the reset sequence timing

    // pull CSB high to finish the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

}

//********************************************************
//! @brief preform adc conversion by sending conversion command,
//!        waiting for conversion time then reading the value
//!
//! @return 24 bit result
//********************************************************
UINT32 cmd_adc(char cmd)
{
    UINT8 byteReceived[3];
    UINT8 byteToSend = CMD_ADC_CONV + cmd;

    // pull CSB low to start the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect

    spiffyd_txData(SPIFFYD_2, 1 ,&byteToSend);         // send conversion command

    switch (cmd & 0x0f)                                // wait necessary conversion time
    {
        case CMD_ADC_256 : bleapputils_delayUs( 600 + 100); break;
        case CMD_ADC_512 : bleapputils_delayUs(1170 + 100); break;
        case CMD_ADC_1024: bleapputils_delayUs(2280 + 100); break;
        case CMD_ADC_2048: bleapputils_delayUs(4540 + 100); break;
        case CMD_ADC_4096: bleapputils_delayUs(9040 + 100); break;
    }

    // pull CSB high to finish the conversion
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

    // pull CSB low to start the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect

    byteToSend = CMD_ADC_READ;
    spiffyd_txData(SPIFFYD_2, 1 ,&byteToSend);         // send ADC read command

    spiffyd_rxData(SPIFFYD_2, 3, byteReceived);

    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

    return (byteReceived[0] << 16) | (byteReceived[1] << 8) | byteReceived[2];
}

//********************************************************
//! @brief Read calibration coefficients
//!
//! @return coefficient
//********************************************************
UINT16 cmd_prom(char coef_num)
{
    UINT8 byteReceived;
    UINT8 rC[2];
    UINT8 byteToSend = CMD_PROM_RD + coef_num * 2;


    // pull CSB low to start the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect

    spiffyd_txData(SPIFFYD_2, 1 ,&byteToSend);         // send PROM READ command

    spiffyd_rxData(SPIFFYD_2, 2, rC);

    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

    return (rC[0] << 8 | rC[1]);
}

//********************************************************
//! @brief calculate the CRC code for details look into CRC CODE NOTES
//!
//! @return crc code
//********************************************************
UINT8 crc4(UINT16 n_prom[])
{
    INT8   cnt;                           // simple counter
    UINT16 n_rem;                         // crc reminder
    UINT16 crc_read;                      // original value of the crc
    UINT8  n_bit;

    n_rem     = 0x00;
    crc_read  = n_prom[7];                // save read CRC

    n_prom[7] = (0xFF00 & (n_prom[7]));   // CRC byte is replaced by 0

    for (cnt = 0; cnt < 16; cnt++)        // operation is performed on bytes
    {
        // choose LSB or MSB
        if (cnt % 2 == 1)
        {
            n_rem ^= (unsigned short) ((n_prom[cnt >> 1]) & 0x00FF);
        }
        else
        {
            n_rem ^= (unsigned short) (n_prom[cnt >> 1] >> 8);
        }

        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & (0x8000))
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else
            {
                n_rem = (n_rem << 1);
            }
        }
    }

    n_rem     = (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
    n_prom[7] = crc_read;                 // restore the crc_read to its original place

    return (n_rem ^ 0x00);
}


