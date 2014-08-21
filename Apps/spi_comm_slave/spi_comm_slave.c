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
* BLE SPI Slave Sample
*
* The application initializes the second SPI interface to communicate with the
* a peer device as a SPI slave.
* 4 standard SPI wires and a flow control GPIO (FCO) are used for communicating
* with the peer.  Following is the description of the protocol.
*
* A: Master to slave communication.
*    1. SPI master asserts CS.
*    2. SPI slave detects raising edge and asserts its Flow Control Out (FCO) to indicate it is ready to receive.
*    3. SPI master sends data to slave, at most 15 bytes at a time.
*    4. SPI master de-asserts CS.
*    5. SPI slave detects falling edge and reads data from HW.
*    6. SPI slave de-asserts FCO (implicit ack).
*  B: Slave to master communication.
*    1. SPI slave asserts FCO.
*    2. SPI master detects FCO from slave and asserts CS.
*    3. SPI slave de-asserts FCO to indicate that it is ready to transmit.
*    4. SPI master reads data from slave (at most 15 bytes at a time).
*    5. SPI master de-asserts CS.
*  C: Data framing:
*    1. The first byte of any transaction is the number of bytes in this transaction.
*    2. SPI data bytes follow the length bytes. So a transaction can at most have 14 bytes of data.
*        a. | Length (1byte) | Data (Length bytes) |
*    3. A Length byte of 0xFF indicates a don't care (useful for the M->S bytes received in a S->M communication).
*  D: Race conditions:
*    1. ISSUE: M->S and S->M race conditions must be handled.
*       a. Use a back-off algorithm?
*       b. Always let the master go first?
*  E: Optimizations:
*     1. We don't have to use a 1 byte length header for every transaction. The HW FIFO knows how many
*        bytes were received in a transaction (spiffyd_slaveGetRxFifoCount). If we use a more complex
*        state machine, we could also use the data received from the master during the S->M transaction.
*
* By default application sends packets to the SPI slave that it receives from a client over the
* Bluetooth LE connection.  And when a packet is received over the SPI interface, application sends
* it up to the client.  To test SPI interface without Bluetooth part, define
* SPI_MASTER_TRANSMIT_ON_TIMEOUT.  Application will send packets to the slave on timeout.
*
* Features demonstrated
*  - SPI in slave mode
*
* To demonstrate the app, work through the following steps.
* 1. Connect Master device to SPI interface and FCO
*    (Use P2 for CS, P3 for CLK, P0 for MOSI, P1 for MISO, and P14 for FCO)
* 2. Plug the WICED eval board into your computer
* 3. Build and download the application (to the WICED board)
* 4. Application reads data from SPI bus and prints it out to the debug output.
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "spi_comm_slave.h"
#include "spiffydriver.h"
#include "bleappconfig.h"
#include "devicelpm.h"


/******************************************************
 *                      Constants
 ******************************************************/

// Comment out following definition to have Client run transmissions
// #define SPI_SLAVE_TRANSMIT_ON_TIMEOUT

// Comment out following definition to send to the Client only data received from SPI
// #define SPI_SLAVE_HOST_LOOPBACK

// Use 1M speed
#define SPEED                                       1000000

// CS is active low
#define CS_ASSERT                                   0
#define CS_DEASSERT                                 1

// use GPIO P14 for output flow control
#define SPIFFY2_OUTPUT_FLOW_CTRL_PIN                14
#define SPIFFY2_OUTPUT_FLOW_CTRL_PORT               0

// use GPIO P2 for input flow control
#define SPIFFY2_INPUT_FLOW_CTRL_PIN                 2
#define SPIFFY2_INPUT_FLOW_CTRL_PORT                0

// Max transaction size
#define SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION 15
#define SPI_TRANSACTION_BUFFER_SIZE 16

#define DEBUG_PORT                                  1
#define DEBUG_PIN                                   12

#define SPI_COMM_CHECK_BYTE                         0xA0

#define SPI_TRANSFER_STATE_IDLE                     0
#define SPI_TRANSFER_STATE_MASTER                   1
#define SPI_TRANSFER_STATE_SLAVE                    2

#define SPI_TRANSFER_SUBSTATE_NONE                  0

#define SPI_TRANSFER_SUBSTATE_START                 1
#define SPI_TRANSFER_SUBSTATE_END                   2


/******************************************************
 *               Function Prototypes
 ******************************************************/

void        application_gpio_interrupt_handler(void* parameter, UINT8 u8);
int         application_spiffy2_send_bytes(void);
static UINT8 ringBufferAdd(UINT8* buffer, UINT8 length);
void        spi_comm_slave_create(void);
void        spi_comm_slave_timeout(UINT32 arg);
void        spi_comm_slave_fine_timeout(UINT32 arg);
static void application_spiffy2_init_in_slave_mode(void);
static void spi_comm_slave_connection_down(void);
static void spi_comm_slave_advertisement_stopped(void);
static int  spi_comm_slave_write_handler(LEGATTDB_ENTRY_HDR *p);
UINT8       spiProcessingCheck(void);
UINT32      device_lpm_queriable(LowPowerModePollType type, UINT32 context);




extern void bleapputils_delayUs(UINT32 delay);

/******************************************************
 *               Variables Definitions
 ******************************************************/
UINT32  timer_count          = 0;
UINT8   count                = 0;
UINT8   spiTransferState     = SPI_TRANSFER_STATE_IDLE;
UINT8   spiTransferSubState  = SPI_TRANSFER_SUBSTATE_NONE;

UINT32  spiTxhwfifoHead      = 0;
UINT32  spiTxhwfifoTail      = 0;
UINT32  spiTxnumEmptyEntries = SPI_TRANSACTION_BUFFER_SIZE;

// The bytes we want to transmit as a slave.
UINT8 spi_slave_bytes_to_tx[SPI_TRANSACTION_BUFFER_SIZE][SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION];

/*
 * This is the GATT database for the SPI Slave sample.  It is
 * currently empty with only required GAP and GATT services.
 */
const UINT8 spi_comm_slave_gatt_database[]=
{
    // Standard GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Standard GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'S','P','I',' ','s','l','a','v','e',0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE,
                           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_TAG),

    // vendor specific SPI communication service
    PRIMARY_SERVICE_UUID128(HANDLE_SPI_COMM_SERVICE_UUID, UUID_SPI_COMM_SERVICE),

    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_SPI_COMM_CHARACTERISTIC_DATA, HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE, 
        UUID_SPI_COMM_CHARACTERISTIC_DATA,
        LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
        LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_VARIABLE_LENGTH, 20),
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_SPI_COMM_DATA_CLIENT_CONFIGURATION_DESCRIPTOR,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL, HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE,
            UUID_SPI_COMM_CHARACTERISTIC_CONTROL_POINT,
        LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE,
        LEGATTDB_PERM_WRITE_CMD, 1),
    0x00,

    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_SPI_COMM_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,
};

const BLE_PROFILE_CFG spi_comm_slave_cfg =
{
    /*.fine_timer_interval            =*/ 20, // ms
    /*.default_adv                    =*/ 4,    // HIGH_UNDIRECTED_DISCOVERABLE
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 1024, // slots
    /*.high_undirect_adv_duration     =*/ 30,   // seconds
    /*.low_undirect_adv_duration      =*/ 3000, // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "SPI Slave", // [LOCAL_NAME_LEN_MAX];
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
const BLE_PROFILE_PUART_CFG spi_comm_slave_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG spi_comm_slave_gpio_cfg =
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

UINT16  spi_comm_slave_data_client_configuration    = 0;
UINT16  spi_comm_slave_control_client_configuration = 0;
UINT32  spi_comm_slave_timer_count                  = 0;
UINT32  spi_comm_slave_timer_timeout                = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)spi_comm_slave_gatt_database,
                   sizeof(spi_comm_slave_gatt_database),
                   (void *)&spi_comm_slave_cfg,
                   (void *)&spi_comm_slave_puart_cfg,
                   (void *)&spi_comm_slave_gpio_cfg,
                   spi_comm_slave_create);
}

// Create spi comm slave
void spi_comm_slave_create(void)
{
    ble_trace0("\rspi_comm_slave_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, spi_comm_slave_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, spi_comm_slave_advertisement_stopped);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)spi_comm_slave_write_handler);

    // Initialization for SPI2 interface
    application_spiffy2_init_in_slave_mode();

    bleprofile_regTimerCb(spi_comm_slave_fine_timeout, spi_comm_slave_timeout);
    bleprofile_StartTimer();

    bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);

    // disable sleep
    devlpm_init();
    devlpm_registerForLowPowerQueries(device_lpm_queriable, 0);
}

// This function will be called when connection goes down
void spi_comm_slave_connection_down(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rConnection down ADV start");
}

void spi_comm_slave_advertisement_stopped(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rRestart ADV");
}

void spi_comm_slave_timeout(UINT32 arg)
{

}

void spi_comm_slave_fine_timeout(UINT32 arg)
{
    UINT8 result;

#ifdef SPI_SLAVE_TRANSMIT_ON_TIMEOUT
    UINT8 i;
    UINT8 buffer[SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION - 1];


    for (i=0; i<SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION-1; i++)
    {
        buffer[i]=i;
    }
    timer_count++;

    if (timer_count == 10)
    {
        timer_count = 9;

        while (spiTxnumEmptyEntries > 0)
        {
            buffer[0] = count++;

            ringBufferAdd(buffer, SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION-1);
            // ble_trace1("adding buffer, num left: %d\n", spiTxnumEmptyEntries);

        }

        result = application_spiffy2_send_bytes();
        if (result)
            ble_trace1("slave transmit: %d\n", result);
    }
#else
    if (spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE)
    {
        result = application_spiffy2_send_bytes();
        if (result)
            ble_trace1("slave transmit: %d\n", result);
    }
#endif

    spi_comm_slave_timer_count++;

    spiProcessingCheck();
}

// receives data from the master
void masterToSlaveReceive(void)
{
    UINT8 length     = 0;
    UINT8 dump_count = 0;
    UINT8 junk;

    // try to receive 15 bytes at a time at most
    UINT8 buffer[SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION];

    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_LOW);

    memset(buffer, 0x00, sizeof(buffer));

    // CS is no longer asserted so some bytes will be available in the FIFO.
    // ble_trace1("Fifo Count at read: %02D\nBytes:\n", spiffyd_slaveGetRxFifoCount(SPIFFYD_2));

    // Get the first byte and check the length
    spiffyd_slaveRxData(SPIFFYD_2, 1, &length);
    // ble_trace1("Received Length: 0x%0x Bytes\n", length);

    if ((length & 0xF0) == SPI_COMM_CHECK_BYTE)
    {
        length = length & 0x0F;
        if (length < SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION)
        {
            // M->S transaction falling edge transition.
            // First byte is the bytes in this transaction.
            if (length > 0)
            {
                spiffyd_slaveRxData(SPIFFYD_2, length, &buffer[1]);
                // Do something with buffer. Here we just trace out the bytes to debug uart.
                // ble_tracen(&buffer[1], length);
            }

        }
    }


    // Clear out any extra data
    while (spiffyd_slaveGetRxFifoCount(SPIFFYD_2))
    {
        // Dump bytes
        spiffyd_slaveRxData(SPIFFYD_2, 1, &junk);
        dump_count++;
    }

    if (dump_count)
    {
        ble_trace1("Dumped %02D Bytes\n", dump_count);
#if (0)
        gpio_configurePin(DEBUG_PORT, DEBUG_PIN, GPIO_OUTPUT_ENABLE | GPIO_INPUT_DISABLE, 0);
        gpio_setPinOutput(DEBUG_PORT, DEBUG_PIN, 1);
        gpio_setPinOutput(DEBUG_PORT, DEBUG_PIN, 0);
#endif
    }

    spiTransferState = SPI_TRANSFER_STATE_IDLE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;

    // if bytes were received, send notification up to the client
    if ((length > 0) && (spi_comm_slave_data_client_configuration != 0))
    {
        if (spi_comm_slave_data_client_configuration & CCC_NOTIFICATION)
        {
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE, &buffer[1], length);
        }
        else
        {
            ble_trace0("\rfailed to send notification");
        }
    }
}

// Initiate Slave to Master transaction
void slaveToMasterStart(void)
{
    spiTransferState = SPI_TRANSFER_STATE_SLAVE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_START;

    // Set FCO high to indicate that we want to transmit something to the master.
    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_HIGH);

    spi_comm_slave_timer_timeout = spi_comm_slave_timer_count + 2;

}

// Initiate Master to Slave transaction
void masterToSlaveStart(void)
{
    // Raise FCO indicating we are ready for transmit from master.
    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_HIGH);
    // ble_trace0("Raising FCO for M->S transaction\n");
    // ble_trace1("Fifo Count at raise FCO: 0x%02D\nBytes:\n", spiffyd_slaveGetRxFifoCount(SPIFFYD_2));

    spiTransferState = SPI_TRANSFER_STATE_MASTER;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_START;
    spi_comm_slave_timer_timeout = spi_comm_slave_timer_count + 2;
}

// End Master transaction
void masterToIdle(void)
{
    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_LOW);
    spiTransferState = SPI_TRANSFER_STATE_IDLE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;
}

// End Slave transaction
void slaveToIdle(void)
{
    UINT8 junk;
    UINT8 result[1];
    UINT8 dump_count = 0;

    // Clear out data that got received when data was transmitted out
    while (spiffyd_slaveGetRxFifoCount(SPIFFYD_2))
    {
        // Dump bytes
        spiffyd_slaveRxData(SPIFFYD_2, 1, &junk);
        dump_count++;
    }

    // if (dump_count)
    //      ble_trace1("Dumped %02D Bytes\n", dump_count);

    // S->M Transfer done, lower FCO
    // if (dump_count)
    //     ble_trace0("Ending S->M transaction\n");

    if (spi_comm_slave_control_client_configuration & CCC_NOTIFICATION)
    {
        //result[0] = spiTxnumEmptyEntries;
        result[0] = 1;
        bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE, result, 1);
    }

    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_LOW);
    spiTransferState = SPI_TRANSFER_STATE_IDLE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;

}

// Perform Slave to Master transaction
void slaveToMasterSend(void)
{
    UINT8 txlength     = 0;

    //ble_trace1("Starting S->M transaction %d bytes\n", spi_slave_bytes_to_tx[0]);

    txlength = spi_slave_bytes_to_tx[spiTxhwfifoTail][0]+1;

    spi_slave_bytes_to_tx[spiTxhwfifoTail][0] |= SPI_COMM_CHECK_BYTE;

    // FCO is high, so this is a S->M transaction. Put the bytes in the FIFO and lower FCO.
    spiffyd_slaveTxData(SPIFFYD_2, txlength, spi_slave_bytes_to_tx[spiTxhwfifoTail]);

    //ble_trace1("Fifo Count after write: %02D Bytes:\n", spiffyd_slaveGetTxFifoCount(SPIFFYD_2));

    // ble_tracen(spi_slave_bytes_to_tx[spiTxhwfifoTail], txlength);

    // Lower FCO to indicate to master that data is ready to read.
    gpio_setPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN, GPIO_PIN_OUTPUT_LOW);

    // update ring buffer variables
    spiTxnumEmptyEntries++;
    spiTxhwfifoTail++;
    if (spiTxhwfifoTail == SPI_TRANSACTION_BUFFER_SIZE)
    {
        spiTxhwfifoTail = 0;
    }

    spiTransferSubState = SPI_TRANSFER_SUBSTATE_END;
    spi_comm_slave_timer_timeout = spi_comm_slave_timer_count + 2;

}

// Based on states and CS, perform next step in the transaction
UINT8 spiProcessingCheck(void)
{

    UINT8 retVal = 0;


    if (spiTransferState == SPI_TRANSFER_STATE_SLAVE)
    {
        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_START)
        {
            // Check CS is asserted indicating ack for S->M transaction 
            if (gpio_getPinInput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_ASSERT)
            {
                slaveToMasterSend();
                //retVal = TRUE;
            }
            else if (spi_comm_slave_timer_count >= spi_comm_slave_timer_timeout)
            {
                masterToIdle();
            }
        }

        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_END)
        {
            // Check for end of S->M transaction.
            if (gpio_getPinInput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_DEASSERT)
            {
                slaveToIdle();
                retVal = TRUE;
            }
            else if (spi_comm_slave_timer_count >= spi_comm_slave_timer_timeout)
            {
                slaveToIdle();
            }
        }
    }

    if (spiTransferState == SPI_TRANSFER_STATE_MASTER)
    {
        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_START)
        {
            // Check for end of M->S transaction.
            if (gpio_getPinInput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_DEASSERT)
            {
                masterToSlaveReceive();
                //retVal = TRUE;
            }
            else if (spi_comm_slave_timer_count >= spi_comm_slave_timer_timeout)
            {
                masterToIdle();
            }
        }
    }

    if (spiTransferState == SPI_TRANSFER_STATE_IDLE)
    {
        if (gpio_getPinInput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_ASSERT)
        {
            masterToSlaveStart();
            retVal = TRUE;
        }
    }

    return(retVal);
}


// Thread context interrupt handler.
void application_gpio_interrupt_handler(void* parameter, UINT8 u8)
{
    UINT8 result = 1;

    // keep processisng to increase throughput
    while (result)
    {
        result = spiProcessingCheck();
    }

    // Initiate slave transmit of data if we have something to send and is idle
    while ((spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE) && (spiTransferState == SPI_TRANSFER_STATE_IDLE))
    {
        result = application_spiffy2_send_bytes();
        if (result)
        {
            ble_trace1("slave transmit: %d\n", result);
        }
    }

    // Pending interrupt on this GPIO will automatically be cleared by the driver.
}

// initialize spiffy2 as SPI slave
void application_spiffy2_init_in_slave_mode(void)
{
    // We will use an interrupt for the first rx byte and then start a state machine from then on.
    UINT16 interrupt_handler_mask[3] = {0, 0, 0};
    UINT16 cs_pin_config = 0 ;

    // Use SPIFFY2 interface as slave
    spi2PortConfig.masterOrSlave = SLAVE2_CONFIG;

    // pull for MISO for master, MOSI/CLOCK/CS if slave mode
    spi2PortConfig.pinPullConfig = INPUT_PIN_PULL_DOWN;

    // Use P3 for CLK, P0 for MOSI and P1 for MISO
    spi2PortConfig.spiGpioConfig = SLAVE2_P02_CS_P03_CLK_P00_MOSI_P01_MISO;

    // Initialize SPIFFY2 instance
    spiffyd_init(SPIFFYD_2);

    // Configure the SPIFFY2 HW block
    spiffyd_configure(SPIFFYD_2, 0, SPI_MSB_FIRST, SPI_SS_ACTIVE_LOW, SPI_MODE_3);

    // enable SPIFFY2 slave receiver
    spiffyd_slaveEnableRx(SPIFFYD_2);

    // enable SPIFFY2 slave transmitter
    spiffyd_slaveEnableTx(SPIFFYD_2);

    interrupt_handler_mask[SPIFFY2_INPUT_FLOW_CTRL_PORT] |= (1 << SPIFFY2_INPUT_FLOW_CTRL_PIN);

    // Now register the interrupt handler.
    gpio_registerForInterrupt(interrupt_handler_mask, application_gpio_interrupt_handler, NULL);

    // Since we are using CS pin as flow control in and want to additionally enable interrupts, read and modify
    // the config because spiffyd_configure() would have already configured it for CS.
    cs_pin_config = gpio_getPinConfig(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN);

    // configure GPIO used for input flow control. Also enable interrupt on both edges.
    // Use the same pull as spi2PortConfig.pinPullConfig
    gpio_configurePin(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN,
        cs_pin_config | GPIO_INPUT_ENABLE | GPIO_PULL_DOWN | GPIO_EN_INT_BOTH_EDGE, GPIO_PIN_OUTPUT_LOW);

    // Clear out any spurious interrupt status.
    gpio_clearPinInterruptStatus(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN);

    // configure GPIO used for output flow control (FCO)
    gpio_configurePin(SPIFFY2_OUTPUT_FLOW_CTRL_PORT,
                      SPIFFY2_OUTPUT_FLOW_CTRL_PIN,
                      GPIO_OUTPUT_ENABLE, GPIO_PIN_OUTPUT_LOW);
}

// Adds transmit data to the ring buffer
UINT8 ringBufferAdd(UINT8* buffer, UINT8 length)
{
    if (spiTxnumEmptyEntries == 0)
    {
        return(1);
    }

    // Copy over the data to a ring buffer so we can send it when the master is ready.
    spi_slave_bytes_to_tx[spiTxhwfifoHead][0] = length;
    memcpy(&spi_slave_bytes_to_tx[spiTxhwfifoHead][1], buffer, length);

    // update ring buffer variables
    spiTxnumEmptyEntries--;
    spiTxhwfifoHead++;
    if (spiTxhwfifoHead == SPI_TRANSACTION_BUFFER_SIZE)
    {
        spiTxhwfifoHead = 0;
    }

    return(0);
}

// Sends some bytes to the SPI master. Bytes will be sent asynchronously.
int application_spiffy2_send_bytes(void)
{
    // check to see if we are not already doing a SPI transfer
    if (spiTransferState != SPI_TRANSFER_STATE_IDLE)
    {
        spiProcessingCheck();
        ble_trace2("spiTransferState %d, %d\n", spiTransferState, spiTransferSubState);
        return 1;
    }

    // If FCO is already high, either we are either receiving or in the start of a transmit already
    if (gpio_getPinOutput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
    {
        spiProcessingCheck();
        return 2;
    }

    // read the input of CS to make sure it is not asserted so we are not already in another transaction
    if (gpio_getPinInput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_ASSERT)
    {
        spiProcessingCheck();
        return 3;
    }

    slaveToMasterStart();

    return 0;
}

// Process write request or command from the Bluetooth client
//
int spi_comm_slave_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle   = legattdb_getHandle(p);
    int    len      = legattdb_getAttrValueLen(p);
    UINT8  *attrPtr = legattdb_getAttrValue(p);
    UINT8  result;
    UINT8  notificationValue[1] = {0x01};

    // By writing into Characteristic Client Configuration descriptor
    // peer can enable or disable notification or indication
    if ((len == 2) && (handle == HANDLE_SPI_COMM_DATA_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        spi_comm_slave_data_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("data descriptor %04x\n", spi_comm_slave_data_client_configuration);

        // to make upgrade faster request host to reduce connection interval to minimum
        lel2cap_sendConnParamUpdateReq(6, 6, 0, 700);
    }
    else if ((len == 2) && (handle == HANDLE_SPI_COMM_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        spi_comm_slave_control_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("control descriptor %04x\n", spi_comm_slave_control_client_configuration);
    }
    else if (handle == HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE)
    {
        // this is a request from the client app to send data over SPI
#ifndef SPI_SLAVE_HOST_LOOPBACK

        if (spiTxnumEmptyEntries > 0)
        {
            ringBufferAdd(attrPtr, len);
            result = application_spiffy2_send_bytes();
            if (result)
                ble_trace1("slave transmit: %d\n", result);
        }
        else
        {
            ble_trace0("slave buffer dropped\n");

            if (spi_comm_slave_control_client_configuration & CCC_NOTIFICATION)
            {
                //notificationValue[0] = spiTxnumEmptyEntries;
                bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE, notificationValue, 1);
            }
        }

#else
        if (spi_comm_slave_data_client_configuration & CCC_NOTIFICATION)
        {
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE, attrPtr, len);
        }
        if (spi_comm_slave_control_client_configuration & CCC_NOTIFICATION)
        {
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE, notificationValue, 1);
        }
#endif
    }
    else
    {
        ble_trace2("bad write_handler len:%d handle:%04x\n", len, handle);
    }
    return 0;
}

// Callback called by the FW when ready to sleep/deep-sleep. Disable both by returning 0.
UINT32 device_lpm_queriable(LowPowerModePollType type, UINT32 context)
{
    // Disable sleep.
    return 0;
}

