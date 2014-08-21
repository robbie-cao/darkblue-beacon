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
* BLE SPI Master Sample
*
* The application initializes the second SPI interface to communicate with the
* a peer device as a SPI master.
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
* Features demonstrated
*  - SPI in master mode
* To demonstrate the app, work through the following steps.
* 1. Connect Slave device to SPI interface and FCO
*    (Use P2 for CS, P24 for CLK, P4 for MOSI, P25 for MISO, and P14 for FCO)
* 2. Plug the WICED eval board into your computer
* 3. Build and download the application (to the WICED board)
* 4. Application transmits and receives data to SPI bus and prints it out to the debug output.
*/
#include "bleprofile.h"
#include "bleapp.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "../spi_comm_slave/spi_comm_slave.h"
#include "spiffydriver.h"
#include "bleappconfig.h"
#include "devicelpm.h"

/******************************************************
 *                      Constants
 ******************************************************/

// Comment out following definition to have Client run transmissions
//#define SPI_MASTER_TRANSMIT_ON_TIMEOUT

// Comment out following definition to send to the Client only data received from SPI
//#define SPI_MASTER_HOST_LOOPBACK

// Use 1M speed
#define SPEED                           1000000

// CS is active low
#define CS_ASSERT                       0
#define CS_DEASSERT                     1

// use GPIO P14 for output flow control
#define SPIFFY2_OUTPUT_FLOW_CTRL_PIN    14
#define SPIFFY2_OUTPUT_FLOW_CTRL_PORT   0

// use GPIO P2 for input flow control
#define SPIFFY2_INPUT_FLOW_CTRL_PIN     2
#define SPIFFY2_INPUT_FLOW_CTRL_PORT    0

// input flow control also defined as CS
#define CS_PORT                         SPIFFY2_INPUT_FLOW_CTRL_PORT
#define CS_PIN                          SPIFFY2_INPUT_FLOW_CTRL_PIN

// Max transaction size
#define SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION 15
#define SPI_TRANSACTION_BUFFER_SIZE                 16

#define DEBUG_PORT                                  1
#define DEBUG_PIN                                   12

#define SPI_COMM_CHECK_BYTE                         0xA0

#define SPI_TRANSFER_STATE_IDLE                     0
#define SPI_TRANSFER_STATE_MASTER                   1
#define SPI_TRANSFER_STATE_SLAVE                    2
#define SPI_TRANSFER_STATE_ABORT                    3

#define SPI_TRANSFER_SUBSTATE_NONE                  0

#define SPI_TRANSFER_SUBSTATE_START                 1
#define SPI_TRANSFER_SUBSTATE_END                   2


/******************************************************
 *               Function Prototypes
 ******************************************************/

void application_gpio_interrupt_handler(void* parameter, UINT8 u8);
static void   spiffy2_master_initialize(void);

static void application_spiffy2_init_in_master_mode(void);
static UINT8 ringBufferAdd(UINT8* buffer, UINT8 length);
static UINT8 application_spiffy2_send_bytes(void);
static void spi_comm_master_create(void);
static void spi_comm_master_timeout(UINT32 arg);
static void spi_comm_master_fine_timeout(UINT32 arg);
static void spi_comm_master_connection_down(void);
static void spi_comm_master_advertisement_stopped(void);
static int  spi_comm_master_write_handler(LEGATTDB_ENTRY_HDR *p);
UINT8       spiProcessingCheck(void);
UINT32      device_lpm_queriable(LowPowerModePollType type, UINT32 context);



extern void   bleapputils_delayUs(UINT32 delay);
/******************************************************
 *               Variables Definitions
 ******************************************************/
UINT32  timer_count         = 0;
UINT8   count               = 0;
UINT8   spiTransferState    = SPI_TRANSFER_STATE_IDLE;
UINT8   spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;


UINT32  spiTxhwfifoHead = 0;
UINT32  spiTxhwfifoTail = 0;
UINT32  spiTxnumEmptyEntries = SPI_TRANSACTION_BUFFER_SIZE;

// The bytes we want to transmit as a slave.
UINT8 spi_master_bytes_to_tx[SPI_TRANSACTION_BUFFER_SIZE][SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION];

/*
 * This is the GATT database for the SPI Master sample.  It is
 * currently empty with only required GAP and GATT services.
 */
const UINT8 spi_comm_master_gatt_database[]=
{
    // Standard GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Standard GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME,
           LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
       'S','P','I',' ','m','a','s','t','e','r',0x00,0x00,0x00,0x00,0x00,0x00,

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

const BLE_PROFILE_CFG spi_comm_master_cfg =
{
    /*.fine_timer_interval            =*/ 20, // ms
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
    /*.local_name                     =*/ "SPI Master", // [LOCAL_NAME_LEN_MAX];
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
const BLE_PROFILE_PUART_CFG spi_comm_master_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ PUARTDISABLE | GPIO_PIN_UART_TX,
    /*.rxpin      =*/ PUARTDISABLE | GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG spi_comm_master_gpio_cfg =
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

UINT16  spi_comm_master_data_client_configuration    = 0;
UINT16  spi_comm_master_control_client_configuration = 0;
UINT32  spi_comm_master_timer_count                  = 0;
UINT32  spi_comm_master_timer_timeout                = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg((UINT8 *)spi_comm_master_gatt_database,
                   sizeof(spi_comm_master_gatt_database),
                   (void *)&spi_comm_master_cfg,
                   (void *)&spi_comm_master_puart_cfg,
                   (void *)&spi_comm_master_gpio_cfg,
                   spi_comm_master_create);
}

// Create spi comm master
void spi_comm_master_create(void)
{
    ble_trace0("\rspi_comm_master_create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, spi_comm_master_connection_down);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, spi_comm_master_advertisement_stopped);

    // register to process client writes
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)spi_comm_master_write_handler);

    // Initialization for SPI2 interface
    application_spiffy2_init_in_master_mode();

    bleprofile_regTimerCb(spi_comm_master_fine_timeout, spi_comm_master_timeout);
    bleprofile_StartTimer();

	bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);

    // disable sleep
    devlpm_init();
    devlpm_registerForLowPowerQueries(device_lpm_queriable, 0);
}

// This function will be called when connection goes down
void spi_comm_master_connection_down(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rConnection down ADV start");
}

void spi_comm_master_advertisement_stopped(void)
{
    bleprofile_Discoverable(LOW_UNDIRECTED_DISCOVERABLE, NULL);
    ble_trace0("\rRestart ADV");
}

void spi_comm_master_timeout(UINT32 arg)
{

}

void spi_comm_master_fine_timeout(UINT32 arg)
{
    UINT8 result;
#ifdef SPI_MASTER_TRANSMIT_ON_TIMEOUT
    UINT8 i;
    UINT8 buffer[SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION-1];

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
            ble_trace1("master transmit: %d\n", result);

    }
#else
    if (spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE)
    {
        result = application_spiffy2_send_bytes();
        if (result)
            ble_trace1("master transmit: %d\n", result);
    }
#endif
    spi_comm_master_timer_count++;

    spiProcessingCheck();

}

// sends data to the slave
void masterToSlaveSend(void)
{

    UINT8 length = 0;
    UINT8 result[1];

    if ( (spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE) &&
         (spiTransferState == SPI_TRANSFER_STATE_MASTER) )
    {
        // ble_trace0("Starting M->S transaction\n");

        // CS is asserted, so this is a M->S transaction. We can transmit data to slave.
        //ble_trace1("Transmitted Length: 0x%02X\nBytes:\n", spi_master_bytes_to_tx[spiTxhwfifoTail][0]);
        //ble_tracen(&spi_master_bytes_to_tx[spiTxhwfifoTail][1], spi_master_bytes_to_tx[spiTxhwfifoTail][0]);
        length = spi_master_bytes_to_tx[spiTxhwfifoTail][0]+1;

        spi_master_bytes_to_tx[spiTxhwfifoTail][0] |= SPI_COMM_CHECK_BYTE;

        spiffyd_txData(SPIFFYD_2, length, &spi_master_bytes_to_tx[spiTxhwfifoTail][0]);

        // update ring buffer variables
        spiTxnumEmptyEntries++;
        spiTxhwfifoTail++;
        if (spiTxhwfifoTail == SPI_TRANSACTION_BUFFER_SIZE)
        {
            spiTxhwfifoTail = 0;
        }

        // deassert CSB  to end the command
        gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

        if (spi_comm_master_control_client_configuration & CCC_NOTIFICATION)
        {
            //result[0] = spiTxnumEmptyEntries;
            result[0] = 1;
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE, result, 1);
        }

        spi_comm_master_timer_timeout = spi_comm_master_timer_count + 2;
        spiTransferSubState = SPI_TRANSFER_SUBSTATE_END;
    }
}

// Start Slave to Master transaction
void slaveToMasterStart(void)
{
    // pull CSB low to start the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect
    spiTransferState = SPI_TRANSFER_STATE_SLAVE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_START;
    
    spi_comm_master_timer_timeout = spi_comm_master_timer_count + 2;

    // ble_trace0("Starting S->M transaction\n");
}

// Start Master to Slave transaction
void masterToSlaveStart(void)
{
    // Assert CS to indicate that we want to transmit something to the slave.
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_ASSERT);     // Assert chipselect

    spiTransferState = SPI_TRANSFER_STATE_MASTER;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_START;

    spi_comm_master_timer_timeout = spi_comm_master_timer_count + 2;

}

// Abort Master transcation
void masterToAbort(void)
{
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);     // Deassert chipselect
    spiTransferState = SPI_TRANSFER_STATE_ABORT;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;
    spi_comm_master_timer_timeout = spi_comm_master_timer_count + 1;
}

// End Master transcation
void masterToIdle(void)
{
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);     // Deassert chipselect
    spiTransferState = SPI_TRANSFER_STATE_IDLE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;
}

// Abort Slave transcation
void slaveToAbort(void)
{
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);     // Deassert chipselect
    spiTransferState = SPI_TRANSFER_STATE_ABORT;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;
    spi_comm_master_timer_timeout = spi_comm_master_timer_count + 1;
}

// End Slave transcation
void slaveToIdle(void)
{
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);     // Deassert chipselect
    spiTransferState = SPI_TRANSFER_STATE_IDLE;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_NONE;
}

// Perform Slave to Master transaction
void slaveToMasterReceive(void)
{
    // try to receive 15 bytes at a time at most
    UINT8 buffer[SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION];

    UINT8 length = 0;

    memset(buffer, 0x00, sizeof(buffer));

    // Get the first byte and check the length
    spiffyd_rxData(SPIFFYD_2, 1, &length);

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
                spiffyd_rxData(SPIFFYD_2, length, &buffer[1]);
            }

            // ble_tracen(&buffer[1], length);

        }
    }
    else
    {
        ble_trace1("Bad Received Length: 0x%0x Bytes\n", length);
    }

    // deassert CSB  to end the command
    gpio_setPinOutput(CS_PORT, CS_PIN, CS_DEASSERT);   // Deassert chipselect

    // if bytes were received, send notification up to the client
    if ((length > 0) && (spi_comm_master_data_client_configuration != 0))
    {
        if (spi_comm_master_data_client_configuration & CCC_NOTIFICATION)
        {
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE, &buffer[1], length);
        }
        else
        {
            ble_trace0("\rfailed to send notification");
        }
    }

    spi_comm_master_timer_timeout = spi_comm_master_timer_count + 2;
    spiTransferSubState = SPI_TRANSFER_SUBSTATE_END;
}

// Based on states and FC, perform next step in the transaction
UINT8 spiProcessingCheck(void)
{

    UINT8 result;
    UINT8 retVal = 0;


    if (spiTransferState == SPI_TRANSFER_STATE_SLAVE)
    {
        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_START)
        {
            // Check FCO is low indicating request for S->M transaction 
            if (!gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
            {
                slaveToMasterReceive();
                retVal = TRUE;
            }
            else if (spi_comm_master_timer_count >= spi_comm_master_timer_timeout)
            {
                slaveToAbort();
            }
        }

        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_END)
        {
            // If slave requests again, that is ok as well
            if (gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
            {
                slaveToMasterStart();
                retVal = TRUE;
            }
            // No indication so just need to timeout
            else if (spi_comm_master_timer_count >= spi_comm_master_timer_timeout)
            {
                slaveToIdle();
            }
        }
    }

    if (spiTransferState == SPI_TRANSFER_STATE_MASTER)
    {
        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_START)
        {
            // Check FCO is high indicating ack for M->S transaction 
            if (gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
            {
                masterToSlaveSend();
                retVal = TRUE;
            }
            else if (spi_comm_master_timer_count >= spi_comm_master_timer_timeout)
            {
                // assume race condition and slave was trying to send  
                slaveToMasterReceive();
            }
        }

        if (spiTransferSubState == SPI_TRANSFER_SUBSTATE_END)
        {
            // Check for end of M->S transaction.
            if (!gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
            {
                masterToIdle();
                retVal = TRUE;
            }
            else if (spi_comm_master_timer_count >= spi_comm_master_timer_timeout)
            {
                masterToAbort();
            }
        }
    }


    if (spiTransferState == SPI_TRANSFER_STATE_ABORT)
    {
        if (spi_comm_master_timer_count >= spi_comm_master_timer_timeout)
        {
            masterToIdle();
        }
    }

    if (spiTransferState == SPI_TRANSFER_STATE_IDLE)
    {
        if (gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
        {
            slaveToMasterStart();

            // If slave asserted FCO while we are processing the data, set retVal so we can process again
            retVal = TRUE;
        }
    }

    return(retVal);
}

// Thread context interrupt handler.
void application_gpio_interrupt_handler(void* parameter, UINT8 u8)
{
    UINT8 result = 1;

    while (result)
    {
        result = spiProcessingCheck();
    }

    while ((spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE) && (spiTransferState == SPI_TRANSFER_STATE_IDLE))
    if ((spiTxnumEmptyEntries != SPI_TRANSACTION_BUFFER_SIZE) && (spiTransferState == SPI_TRANSFER_STATE_IDLE))
    {
        result = application_spiffy2_send_bytes();
        if (result)
        {
            ble_trace1("master transmit: %d\n", result);
        }
    }

    // Pending interrupt on this GPIO will automatically be cleared by the driver.
}

// initialize spiffy2 as SPI master
void application_spiffy2_init_in_master_mode(void)
{
    // We will use an interrupt for the first rx byte and then start a state machine from then on.
    UINT16 interrupt_handler_mask[3] = {0, 0, 0};

    // Use SPIFFY2 interface as master
    spi2PortConfig.masterOrSlave = MASTER2_CONFIG;

    // pull for MISO for master, MOSI/CLOCK/CS if slave mode
    spi2PortConfig.pinPullConfig = INPUT_PIN_PULL_UP;

    // Use P24 for CLK, P4 for MOSI and P25 for MISO
    spi2PortConfig.spiGpioConfig = MASTER2_P24_CLK_P04_MOSI_P25_MISO;

    // Initialize SPIFFY2 instance
    spiffyd_init(SPIFFYD_2);

    // Configure the SPIFFY2 HW block
    spiffyd_configure(SPIFFYD_2, SPEED, SPI_MSB_FIRST, SPI_SS_ACTIVE_LOW, SPI_MODE_3);

    gpio_configurePin(CS_PORT, CS_PIN, GPIO_OUTPUT_ENABLE | GPIO_INPUT_DISABLE, CS_DEASSERT);

    // configure GPIO used for output flow control (FCO) (input for the app)
    gpio_configurePin(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN,
        GPIO_INPUT_ENABLE|GPIO_PULL_DOWN|GPIO_EN_INT_BOTH_EDGE,GPIO_PIN_OUTPUT_LOW);

    interrupt_handler_mask[SPIFFY2_OUTPUT_FLOW_CTRL_PORT] |= (1 << SPIFFY2_OUTPUT_FLOW_CTRL_PIN);

    // Now register the interrupt handler.
    gpio_registerForInterrupt(interrupt_handler_mask, application_gpio_interrupt_handler, NULL);

    // Clear out any spurious interrupt status.
    gpio_clearPinInterruptStatus(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN);

}

// Adds buffer to the ring buffer
UINT8 ringBufferAdd(UINT8* buffer, UINT8 length)
{
    if (spiTxnumEmptyEntries == 0)
    {
        return(1);
    }

    // Copy over the data to a ring buffer so we can send it when the master is ready.
    spi_master_bytes_to_tx[spiTxhwfifoHead][0] = length;
    memcpy(&spi_master_bytes_to_tx[spiTxhwfifoHead][1], buffer, length);

    // update ring buffer variables
    spiTxnumEmptyEntries--;
    spiTxhwfifoHead++;
    if (spiTxhwfifoHead == SPI_TRANSACTION_BUFFER_SIZE)
    {
        spiTxhwfifoHead = 0;
    }

    return(0);
}
// Sends some bytes to the SPI slave. Bytes will be sent asynchronously.
UINT8 application_spiffy2_send_bytes(void)
{
    // check to see if we are not already doing a SPI transfer
    if (spiTransferState != SPI_TRANSFER_STATE_IDLE)
    {
        spiProcessingCheck();
        ble_trace1("spiTransferState %d\n", spiTransferState);
        return 1;
    }

    // read the input of CS to make sure it is not asserted so we are not already in another transaction
    if (gpio_getPinOutput(SPIFFY2_INPUT_FLOW_CTRL_PORT, SPIFFY2_INPUT_FLOW_CTRL_PIN) == CS_ASSERT)
    {
        spiProcessingCheck();
        return 2;
    }

    // If FCO is already high, either we are either transmitting or in the start of a receive already
    if (gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
    {
        spiProcessingCheck();
        return 3;
    }

    masterToSlaveStart();

    // If FCO is already high, may be race condition so abort
    if (gpio_getPinInput(SPIFFY2_OUTPUT_FLOW_CTRL_PORT, SPIFFY2_OUTPUT_FLOW_CTRL_PIN))
    {
        masterToAbort();
        return 4;
    }
    return 0;
}

//
// Process write request or command from the Bluetooth client
//
int spi_comm_master_write_handler(LEGATTDB_ENTRY_HDR *p)
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
        spi_comm_master_data_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("data descriptor %04x\n", spi_comm_master_data_client_configuration);

        // to make upgrade faster request host to reduce connection interval to minimum
        lel2cap_sendConnParamUpdateReq(6, 6, 0, 700);
    }
    else if ((len == 2) && (handle == HANDLE_SPI_COMM_CONTROL_CLIENT_CONFIGURATION_DESCRIPTOR))
    {
        spi_comm_master_control_client_configuration = attrPtr[0] + (attrPtr[1] << 8);
        ble_trace1("control descriptor %04x\n", spi_comm_master_control_client_configuration);
    }
//    else if ((len <= SPIFFY2_MAX_NUMBER_OF_BYTES_PER_TRANSACTION) && (handle == HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE))
    else if (handle == HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE)
    {
        // this is a request from the client app to send data over SPI
#ifndef SPI_MASTER_HOST_LOOPBACK

//        ble_trace1("master transmit empty entries: %d\n", spiTxnumEmptyEntries);
        if (spiTxnumEmptyEntries > 0)
        {
            ringBufferAdd(attrPtr, len);
            result = application_spiffy2_send_bytes();
            if (result)
                ble_trace1("master transmit: %d\n", result);
        }
        else
        {
            ble_trace0("master buffer dropped\n");

            if (spi_comm_master_control_client_configuration & CCC_NOTIFICATION)
            {
                //notificationValue[0] = spiTxnumEmptyEntries;
                bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_CONTROL_VALUE, notificationValue, 1);
            }
        }
#else
        if (spi_comm_master_data_client_configuration & CCC_NOTIFICATION)
        {
            bleprofile_sendNotification(HANDLE_SPI_COMM_CHARACTERISTIC_DATA_VALUE, attrPtr, len);
        }

        if (spi_comm_master_control_client_configuration & CCC_NOTIFICATION)
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

