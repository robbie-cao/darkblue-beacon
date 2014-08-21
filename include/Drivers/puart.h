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
* Define functions to access Peripheral UART block.
*/
#ifndef __PUART_DRIVER_H__
#define __PUART_DRIVER_H__

#include "types.h"

extern void (*puart_rxCb)(void *);
extern void (*puart_rxIntCb)(void);
extern void (*puart_txCb)(void);

#ifndef REG32
#define REG32(x)   *((volatile UINT32*)(x))
#endif

#define UART_REG            REG32
#define ROUND(x,y)          ((x+y/2)/y);
#define PUART_STRLEN(a)     strlen( (a) )

/// PUART configuration. Internal.
typedef struct 
{
    UINT32  baudrate;
    UINT16  hwFlowControlWaterMark;
    UINT32  minPktlength;
    UINT8   pUartFunction;
} puart_UartConfig;

/**  \addtogroup PeripheralUart
 *  \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines a Peripheral UART driver.
*/

/// PUART control registers. Internal.
#define P_UART_INT_STATUS_ADDR                          sr_ptu_status_adr5
#define P_UART_INT_ENABLE_ADDR                          sr_ptu_en_adr5
#define P_UART_RX_FIFO_ADDR                             dp_uart2_data_adr
#define P_UART_TX_FIFO_ADDR                             dp_uart2_data_adr
#define P_UART_WATER_MARK_RX_ADDR                       dc_ptu_uart2_rfl_adr
#define P_UART_WATER_MARK_TX_ADDR                       dc_ptu_uart2_tfl_adr
#define P_UART_FLOW_CONTROL_WATER_MARK_ADDR             dc_ptu_uart2_rfc_adr
#define P_UART_LINE_CONTROL_ADDR                        dc_ptu_uart2_lcr_adr
#define P_UART_LINE_STATUS_ADDR                         dc_ptu_uart2_lsr_adr
#define P_UART_MODEM_CONTROL_ADDR                       dc_ptu_uart2_mcr_adr
#define P_UART_HIGH_BAUDRATE_DIV_ADDR                   dc_ptu_uart2_dhbr_adr
#define P_UART_LOW_BAUDRATE_DIV_ADDR                    dc_ptu_uart2_dlbr_adr
#define P_UART_FIFO_CONTROL_ADDR                        dc_ptu_uart2_fcr_adr
#define P_UART_MSR_LINE_ADDR                            dc_ptu_uart2_msr_adr
#define P_UART_PTU_HC_SEL_ADDR                          dc_ptu_hc_sel_adr

/// PUART interrupt control bits. Internal.
enum
{
    P_UART_ISR_TX_FF_MASK                          = 0x01,
    P_UART_ISR_TX_FAE_MASK                         = 0x02,
    P_UART_ISR_RX_AFF_MASK                         = 0x04,
    P_UART_ISR_RX_FE_MASK                          = 0x08,
    P_UART_ISR_RX_RES_MASK                         = 0x10,
    P_UART_ISR_RX_PE_MASK                          = 0x20,
    P_UART_ISR_RX_BRK_MASK                         = 0x40,
    P_UART_ISR_RX_CTS_MASK                         = 0x80
};

/// PUART line control bits. Internal.
enum
{
	/// Stop bits mask. 0 = 1 stop bit (default); 1 = 2 stop bits.
    P_UART_LCR_STB_MASK                            = 0x0001,

    /// Parity enable mask. 0 = disable parity (default); 1 = enable parity.
    P_UART_LCR_PEN_MASK                            = 0x0002,

    /// Even parity mask. 0 = odd parity (default); 1 = even parity.
    P_UART_LCR_EPS_MASK                            = 0x0004,
    P_UART_LCR_RXEN_MASK                           = 0x0008,
    P_UART_LCR_LBC_MASK                            = 0x0010,
    P_UART_LCR_TXOEN_MASK                          = 0x0020,
    P_UART_LCR_RTSOEN_MASK                         = 0x0040,

    /// SLIP - unsupported.
    P_UART_LCR_SLIP_MASK                           = 0x0080,
    P_UART_LCR_SLIP_CRC_RX_ENABLE_MASK             = 0x0100,
    P_UART_LCR_SLIP_CRC_INVERT_MASK                = 0x0200,
    P_UART_LCR_SLIP_CRC_LSB_1ST_MASK               = 0x0400,
    P_UART_LCR_SLIP_CRC_TX_ENABLE_MASK             = 0x0800
};

/// PUART status bits. Internal.
enum
{
    P_UART_LSR_TX_FIFO_NOT_EMPTY                   = 0x0004, //'0':Data NOT available, '1':Data available
    P_UART_LSR_TX_IDLE                             = 0x0008,
    P_UART_LSR_END_DELIMETER_NOT_DONE_MASK         = 0x0010
};


/// PUART modem control bits. Internal.
enum
{
    P_UART_MCR_TXEN_MASK                           = 0x0001,
    P_UART_MCR_RTS_MASK                            = 0x0002,
    P_UART_MCR_XON_XOFF_MASK                       = 0x0004,
    P_UART_MCR_HIGH_RATE_MASK                      = 0x0008,

    /// Loopback mode. 0 = no loopback (default); 1 = enable loopback.
    /// When enabled, the UART_CTS_N pin is reflected back as UART_RTS_N,
    /// and UART_RXD is reflected back as UART_TXD. This is useful for
    /// verifying board connectivity.
    P_UART_MCR_LOOPBACK_MASK                       = 0x0010,
    P_UART_MCR_AUTORTS_MASK                        = 0x0020,
    P_UART_MCR_AUTOCTS_MASK                        = 0x0040,
    P_UART_MCR_BAUD_ADJ_MASK                       = 0x0080,
    P_UART_MCR_PKT_FLOW_RX_MASK                    = 0x0100,
    P_UART_MCR_SEND_XON_NOW                        = 0x0200
};

/// PUART modem status bits. Internal.
enum
{
    P_UART_MSR_CTS_MASK							   = 0x0001,
    P_UART_MSR_RX_MASK							   = 0x0004
};

/// PUART flow control bits. Internal.
enum
{
    P_UART_FCR_RX_FIFO_RESET_MASK                  = 0x001,
    P_UART_FCR_TX_FIFO_RESET_MASK                  = 0x002,
    P_UART_FCR_SLIP_RX_RESYNC_MASK                 = 0x004,
    P_UART_FCR_SLIP_START_END_FRAME_MASK           = 0x008
};


#define P_UART_ESC_ADDR                                 dc_ptu_uart2_esc_adr

/// PUART RX buffer depth. This is fixed to 16 bytes and cannot be changed.
#define P_UART_IN_BUFFER_LEN                            16

/// PUART HW RX FIFO depth. This is fixed to 16 bytes and cannot be changed.
#define P_UART_RX_FIFO_SIZE                             P_UART_IN_BUFFER_LEN

/// PUART TX buffer depth. This is fixed to 16 bytes and cannot be changed.
#define P_UART_OUT_BUFFER_LEN                           16

/// PUART HW TX FIFO depth. This is fixed to 16 bytes and cannot be changed.
#define P_UART_TX_FIFO_SIZE                             P_UART_OUT_BUFFER_LEN

/// Highest RX watermark is FIFO - 1.
#define P_UART_WATER_MARK_RX_LEVEL_HIGHEST              (P_UART_RX_FIFO_SIZE - 1)

/// Highest TX watermark is FIFO - 1.
#define P_UART_WATER_MARK_TX_LEVEL_HIGHEST              (P_UART_TX_FIFO_SIZE - 1)

/// Lowest RX watermark is 1 byte.
#define P_UART_WATER_MARK_RX_LEVEL_ONE_BYTE             1


/// Helper macros.
#define P_UART_INT_ENABLE                           UART_REG(P_UART_INT_ENABLE_ADDR)
#define P_UART_INT_STATUS                           UART_REG(P_UART_INT_STATUS_ADDR)
#define P_UART_INT_CLEAR(x)                         UART_REG(P_UART_INT_STATUS_ADDR)              = x
#define P_UART_INT_DISABLE                          UART_REG(P_UART_INT_DISABLE_ADDR)

#define P_UART_RX_FIFO()                            UART_REG(P_UART_RX_FIFO_ADDR)
#define P_UART_TX_FIFO(x)                           UART_REG(P_UART_TX_FIFO_ADDR)                 = x

#define P_UART_LINE_CONTROL(x)                      UART_REG(P_UART_LINE_CONTROL_ADDR)            = x
#define P_UART_LINE_CONTROL_GET()                   UART_REG(P_UART_LINE_CONTROL_ADDR)
#define P_UART_MODEM_CONTROL(x)                     UART_REG(P_UART_MODEM_CONTROL_ADDR)           = x
#define P_UART_MODEM_CONTROL_GET()                  UART_REG(P_UART_MODEM_CONTROL_ADDR)
#define P_UART_FIFO_CONTROL(x)                      UART_REG(P_UART_FIFO_CONTROL_ADDR)            = x

#define P_UART_MSR_STATUS()                         UART_REG(P_UART_MSR_LINE_ADDR)

#define P_UART_HIGH_BAUDRATE_DIV(x)                 UART_REG(P_UART_HIGH_BAUDRATE_DIV_ADDR)       = x
#define P_UART_LOW_BAUDRATE_DIV(x)                  UART_REG(P_UART_LOW_BAUDRATE_DIV_ADDR)        = x
#define P_UART_WATER_MARK_RX_LEVEL(x)               UART_REG(P_UART_WATER_MARK_RX_ADDR)           = x
#define P_UART_WATER_MARK_TX_LEVEL(x)               UART_REG(P_UART_WATER_MARK_TX_ADDR)           = x
#define P_UART_FLOW_CONTROL_WATER_MARK_LEVEL(x)     UART_REG(P_UART_FLOW_CONTROL_WATER_MARK_ADDR) = x

#define P_UART_ESC_CHAR                             UART_REG(P_UART_ESC_ADDR)

/// Some useful default constants used by teh driver.
enum
{
    P_UART_CLK                                 = 24000000,
    P_UART_SAMPLE_CLOCK                        = 16,
    DEFAULT_P_UART_BAUDRATE                    = 115200,
    DEFAULT_P_UART_HWFLOWCONTROLWATERMARK      = 13,     // Peer flow off waterline
    DEFAULT_P_UART_MIN_PKT_LENGTH              = 1
};

/// PUART driver RX states. Internal.
typedef enum P_UART_RX_STATE_TAG
{    
    P_UART_RX_IDLE,
    P_UART_RX_TYPE,
    P_UART_RX_PAYLOAD,
    P_UART_RX_READ_DONE,
    P_UART_RX_RESYNC,
    P_UART_DROP_PACKET,
} P_UART_RX_STATE;

/// PUART driver TX states. Internal.
typedef enum P_UART_TX_STATE_TAG
{
    P_UART_TX_IDLE,
    P_UART_TX_BUSY,
    P_UART_TX_WAIT_FIFO_EMPTY,
    P_UART_TX_DONE,
    P_UART_TX_WRONG_STATE       
} P_UART_TX_STATE;

/// HCI command header. Internal. NOTE: PUART does not support HCI commands.
typedef struct HCI_CMD_TAG
{
    UINT8   opcode;
    UINT16  length;
} P_UART_HCI_CMD;

/// PUART function control. Internal.
typedef enum P_UART_FUNCTION_TAG
{
    P_UART_HCI,             
    P_UART_HID,             
    P_UART_SENSOR,
} P_UART_FUNCTION;

typedef UINT8 P_UART_TRANSPORT_PACKET_TYPE; 

/// PUART transport packet type defines.
typedef struct P_UART_TRANSPORT_PACKET_TAG
{
    P_UART_RX_STATE                     RxState;
    P_UART_TX_STATE                     TxState;
    P_UART_FUNCTION                     peripheralUartFunction;

    // Rx Packet Types ******************************/
    P_UART_TRANSPORT_PACKET_TYPE        rxPktType;
    UINT8*                              rxPktPtr;
    UINT32                              rxPktLen;
    BOOL8                               rxPktSLIPHasCRC;
    UINT8                               rxPktSLIPSeqNo;
    // Tx Packet Types******************************/
    P_UART_TRANSPORT_PACKET_TYPE        txPktType;
    UINT8*                              txPktPtr;
    UINT32                              txPktLen;
    UINT8*                              txACLSegRootPtr;
    UINT8                               txPktSLIPSeqNo;
    // Resync types*********************************/
    UINT8                               resyncCurrentByte;
    UINT8                               resyncCurrentStream;
    BOOL8                               resynchSentHWErrorEvent;

} P_UART_TRANSPORT_PACKET;

/// PUART initialization function. This is to be invoked only once.
/// If the application uses a non-NULL BLE_PROFILE_PUART_CFG when invoking
/// bleapp_set_cfg, then this will be invoked by the framwork.
void puart_init(void);

/// ISR context interrupt handler. Internal. The application never needs to handle
/// PUART interrupts in the ISR context. Interrupts are always serialized to
/// application thread.
void puart_interruptHandler(void);

/// Calculates the baud rate. Internal, not for app use.
void puart_calculateBaudrate(UINT8* dhbr, UINT8* dlbr, UINT32 baudrate, UINT32 clk);

/// Enables CM3 interrupt from PUART HW block.
void puart_enableInterrupt(void);

/// Turn on flow control.
void puart_flowOn(void);

/// Turn off flow control.
void puart_flowOff(void);

/// Disable the HW TX FIFO.
void puart_disableTx(void);

/// Enable the HW TXFIFO.
void puart_enableTx(void);

/// Set baud rate.
/// \param dhbr Internal. Should be 0x00.
/// \param dlbr Internal. Should be 0x00.
/// \param baudrate The desired baudrate.
void puart_setBaudrate( UINT8 dhbr, UINT8 dlbr, UINT32 baudrate );

/// Performs a blocking read from the RX FIFO.
/// \param buffer Pointer to allocated memory into which data from the RX FIFO is read into.
///    If NULL, length bytes of data from the RX FIFO will be discarded.
/// \param length The number of bytes to read from the RX FIFO.
void puart_synchronousRead( UINT8* buffer, UINT32 length );

/// Performs a blocking write to the TX FIFO.
/// \param buffer Pointer to the buffer that has the data to be written to the TX FIFO. Cannot be NULL.
/// \param length The number of bytes to write. Cannot be larger than 16.
void puart_synchronousWrite( UINT8* buffer, UINT32 length );

/// Check if the RX FIFO is not empty.
/// \return TRUE if RX FIFO is not empty; else FALSE.
BOOL32 puart_rxFifoNotEmpty(void);

/// Thread context RX interrupt dispatch function. Internal.
int puart_rxHandler(void* unused);

/// Thread context TX interrupt dispatch function. Internal.
int puart_txHandler(void* unused);

/// Prints a string to PUART.
/// \param string NULL terminated string to write to PUART.
void puart_print(char * string);

/// Write a single byte to PUART.
/// \param byte The byte to write.
void puart_write(UINT8 byte);

/// Read a single byte from PUART.
/// \param readbyte Pointer to allocated byte into which to read.
/// \return TRUE if successfully read; else FALSE.
BOOL32 puart_read(UINT8* readbyte);

/// Allows the application to configure GPIOs for PUART. See datasheet for valid options.
/// \param rxdPortPin The GPIO PORT and pin combination for RXd. When P#, rxdPortPin = ((# / 16) << 5) | (# % 16).
/// \param txdPortPin The GPIO PORT and pin combination for TXd. When P#, txdPortPin = ((# / 16) << 5) | (# % 16).
/// \param ctsPortPin The GPIO PORT and pin combination for CTS. When P#, ctsPortPin = ((# / 16) << 5) | (# % 16).
/// \param rtsPortPin The GPIO PORT and pin combination for RTS. When P#, rtsPortPin = ((# / 16) << 5) | (# % 16).
/// \return TRUE when successful; else FALSE.
BOOL32 puart_selectUartPads(UINT8 rxdPortPin, UINT8 txdPortPin, UINT8 ctsPortPin, UINT8 rtsPortPin);

/// Check if the given RXd port-pin combination is valid for the SoC.
/// \param rxdPortPin The GPIO PORT and pin combination for RXd. When P#, rxdPortPin = ((# / 16) << 5) | (# % 16).
/// \return TRUE if valid; else FALSE.
BOOL32 puart_checkRxdPortPin(UINT8 rxdPortPin);

/// Check if the given TXd port-pin combination is valid for the SoC.
/// \param txdPortPin The GPIO PORT and pin combination for TXd. When P#, txdPortPin = ((# / 16) << 5) | (# % 16).
/// \return TRUE if valid; else FALSE.
BOOL32 puart_checkTxdPortPin(UINT8 txdPortPin);

/// Check if the given CTS port-pin combination is valid for the SoC.
/// \param ctsPortPin The GPIO PORT and pin combination for CTS. When P#, ctsPortPin = ((# / 16) << 5) | (# % 16).
/// \return TRUE if valid; else FALSE.
BOOL32 puart_checkCtsPortPin(UINT8 ctsPortPin);

/// Check if the given RTS port-pin combination is valid for the SoC.
/// \param rtsPortPin The GPIO PORT and pin combination for RTS. When P#, rtsPortPin = ((# / 16) << 5) | (# % 16).
/// \return TRUE if valid; else FALSE.
BOOL32 puart_checkRtsPortPin(UINT8 rtsPortPin);

/// Helper macro to check if the RX HW FIFO is not empty.
#define P_UART_RX_FIFO_NOT_EMPTY()      puart_rxFifoNotEmpty()

/// Helper macro to check if the TX HW FIFO is empty.
#define P_UART_TX_FIFO_IS_EMPTY()    ( ( REG32(P_UART_LINE_STATUS_ADDR) & ( P_UART_LSR_TX_FIFO_NOT_EMPTY | P_UART_LSR_TX_IDLE ) ) == P_UART_LSR_TX_IDLE )

/// @}
#endif



