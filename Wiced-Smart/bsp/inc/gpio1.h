#ifndef _GPIO1_H_
#define _GPIO1_H_
/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP 
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2009 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: gpio.h
*
* Abstract: Header file for the functions and variables exported from gpio.c
*
*******************************************************************************/
#include "2045map.h"

/********************* MACRO DEFINITIONS **********************************/
#define   gpio_set( gpioNum, value )                           GPIO_set( gpioNum,      value )
#define   gpio2_set( gpioNum, value )                          GPIO_set( (8+gpioNum),  value )
#define   gpioc_2075_set( gpioNum, value )                     GPIO_set( (24+gpioNum), value )
#define   gpiod_2075_set( gpioNum, value )                     GPIO_set( (32+gpioNum), value )
// This interface used by audio chips only
#define   GPIO_WriteData( gpioNum, value )                     GPIO_set( gpioNum, value )

#define   gpio_get( dataPtr)                                   GPIO_Get( 0, dataPtr )
#define   gpio2_get( dataPtr)                                  GPIO_Get( 1, dataPtr )
#define   gpioc_2075_get( dataPtr)                             GPIO_Get( 3, dataPtr )
#define   gpiod_2075_get( dataPtr)                             GPIO_Get( 4, dataPtr )

#define   gpio_set_direction( gpioNum, value )                 GPIO_WriteReg( gpioNum,      0, value )
#define   gpio2_set_direction( gpioNum, value )                GPIO_WriteReg( (8+gpioNum),  0, value )
#define   gpioc_2075_set_direction( gpioNum, value )           GPIO_WriteReg( (24+gpioNum), 0, value )
#define   gpiod_2075_set_direction( gpioNum, value )           GPIO_WriteReg( (32+gpioNum), 0, value )

#define   gpio_interrupt_sense( gpioNum, value )               GPIO_WriteReg( gpioNum,      1, value )
#define   gpio2_interrupt_sense( gpioNum, value )              GPIO_WriteReg( (8+gpioNum),  1, value )
#define   gpioc_2075_interrupt_sense( gpioNum, value )         GPIO_WriteReg( (24+gpioNum), 1, value )
#define   gpiod_2075_interrupt_sense( gpioNum, value )         GPIO_WriteReg( (32+gpioNum), 1, value )

#define   gpio_interrupt_both_edges( gpioNum, value )          GPIO_WriteReg( gpioNum,      2, value )
#define   gpio2_interrupt_both_edges( gpioNum, value )         GPIO_WriteReg( (8+gpioNum),  2, value )
#define   gpioc_2075_interrupt_both_edges( gpioNum, value )    GPIO_WriteReg( (24+gpioNum), 2, value )
#define   gpiod_2075_interrupt_both_edges( gpioNum, value )    GPIO_WriteReg( (32+gpioNum), 2, value )
                     
#define   gpio_interrupt_event( gpioNum, value )               GPIO_WriteReg( gpioNum,      3, value )
#define   gpio2_interrupt_event( gpioNum, value )              GPIO_WriteReg( (8+gpioNum),  3, value )
#define   gpioc_2075_interrupt_event( gpioNum, value )         GPIO_WriteReg( (24+gpioNum), 3, value )
#define   gpiod_2075_interrupt_event( gpioNum, value )         GPIO_WriteReg( (32+gpioNum), 3, value )

#define   gpio_interrupt_mask( gpioNum, value )                GPIO_WriteReg( gpioNum,      4, value )
#define   gpio2_interrupt_mask( gpioNum, value )               GPIO_WriteReg( (8+gpioNum),  4, value )
#define   gpioc_2075_interrupt_mask( gpioNum, value )          GPIO_WriteReg( (24+gpioNum), 4, value )
#define   gpiod_2075_interrupt_mask( gpioNum, value )          GPIO_WriteReg( (32+gpioNum), 4, value )

#define   gpio_interrupt_raw_status( dataPtr )                 GPIO_ReadReg( 0, 5, dataPtr )
#define   gpio2_interrupt_raw_status( dataPtr )                GPIO_ReadReg( 1, 5, dataPtr )
#define   gpioc_2075_interrupt_raw_status( dataPtr )           GPIO_ReadReg( 3, 5, dataPtr )
#define   gpiod_2075_interrupt_raw_status( dataPtr )           GPIO_ReadReg( 4, 5, dataPtr )

#define   gpio_interrupt_status( dataPtr )                     GPIO_ReadReg( 0, 6, dataPtr )
#define   gpio2_interrupt_status( dataPtr )                    GPIO_ReadReg( 1, 6, dataPtr )
#define   gpioc_2075_interrupt_status( dataPtr )               GPIO_ReadReg( 3, 6, dataPtr )
#define   gpiod_2075_interrupt_status( dataPtr )               GPIO_ReadReg( 4, 6, dataPtr )

#define   gpio_interrupt_clear( gpioNum )                      GPIO_WriteReg( gpioNum,      7, TRUE )
#define   gpio2_interrupt_clear( gpioNum )                     GPIO_WriteReg( (8+gpioNum),  7, TRUE )
#define   gpioc_2075_interrupt_clear( gpioNum )                GPIO_WriteReg( (24+gpioNum), 7, TRUE )
#define   gpiod_2075_interrupt_clear( gpioNum )                GPIO_WriteReg( (32+gpioNum), 7, TRUE )

#define   gpio_register_interrupt_handler( gpioNum, callBack )       GPIO_register_interrupt_handler( gpioNum, callBack )
#define   gpio2_register_interrupt_handler( gpioNum, callBack )      GPIO_register_interrupt_handler( (8+gpioNum), callBack )
#define   gpioc_2075_register_interrupt_handler( gpioNum, callBack ) GPIO_register_interrupt_handler( (24+gpioNum), callBack )
#define   gpiod_2075_register_interrupt_handler( gpioNum, callBack ) GPIO_register_interrupt_handler( (32+gpioNum), callBack )

#define   gpio_enable_interrupt( gpioNum, enable )             GPIO_InterruptEnable( gpioNum, enable )
#define   gpio2_enable_interrupt( gpioNum, enable )            GPIO_InterruptEnable( (8+gpioNum), enable )
#define   gpioc_2075_enable_interrupt( gpioNum, enable )       GPIO_InterruptEnable( (24+gpioNum), enable )
#define   gpiod_2075_enable_interrupt( gpioNum, enable )       GPIO_InterruptEnable( (32+gpioNum), enable )

#define   gpio_interrupt_handler()                             GPIO_interrupt_handler( 0 )
#define   gpio2_interrupt_handler()                            GPIO_interrupt_handler( 1 )
#define   gpioc_2075_interrupt_handler()                  GPIO_interrupt_handler( 3 )
#define   gpiod_2075_interrupt_handler()                  GPIO_interrupt_handler( 4 )


#define   GPIO_debug_output_byte_012( val )                    GPIO_debug_output_012( val, 7 )
#define   GPIO_debug_output_word_012( val )                    GPIO_debug_output_012( val, 15 )
#define   GPIO_debug_output_dword_012( val )                   GPIO_debug_output_012( val, 31 )

// GPIO block numbers, used for calls to GPIO_GetDataPtr()
#define    GPIO_BLOCK_0        0
#define    GPIO_BLOCK_2        1
#define    GPIO_BLOCK_3        2
#define    GPIO_BLOCK_C        3
#define    GPIO_BLOCK_D        4

#define    GPIO_0              0
#define    GPIO_1              1
#define    GPIO_2              2
#define    GPIO_3              3
#define    GPIO_4              4
#define    GPIO_5              5
#define    GPIO_6              6
#define    GPIO_7              7
	
#define    GPIO_PIN_0          0
#define    GPIO_PIN_1          1
#define    GPIO_PIN_2          2
#define    GPIO_PIN_3          3
#define    GPIO_PIN_4          4
#define    GPIO_PIN_5          5
#define    GPIO_PIN_6          6
#define    GPIO_PIN_7          7

#define    GPIO2_PCM_OUT       2
#define    GPIO2_PCM_IN        3

#define GPIO_DIRECTION_INPUT                FALSE
#define GPIO_DIRECTION_OUTPUT               TRUE
#define GPIO_INTERRUPT_EDGE_SENSE           FALSE
#define GPIO_INTERRUPT_LEVEL_SENSE          TRUE
#define GPIO_INTERRUPT_BOTH_EDGES_TRIGGER   TRUE
#define GPIO_INTERRUPT_ONE_EDGE             FALSE
#define GPIO_INTERRUPT_LEVEL_SENSE_HIGH     TRUE
#define GPIO_INTERRUPT_LEVEL_SENSE_LOW      FALSE
#define GPIO_INTERRUPT_RISING_EDGES         TRUE
#define GPIO_INTERRUPT_FALLING_EDGES        FALSE
#define GPIO_INTERRUPT_PIN_ENABLE           TRUE
#define GPIO_INTERRUPT_PIN_DISABLE          FALSE

#ifdef AUXILIARY_GPIOS_SUPPORTED 
#define GPIO_AUX_BASE_ADDR           gpio2_adr_base
#define GPIO_AUX_DIRECTION_ADDR      (GPIO_AUX_BASE_ADDR + 0x400)
#define GPIO_AUX_INPUT_ADDR          (GPIO_AUX_BASE_ADDR + 0x3FC)
#endif


#if defined( gpio5data_adr )
    #define GPIO_NUMBER_OF_BLOCKS   5
#elif defined( gpio4data_adr )
    #define GPIO_NUMBER_OF_BLOCKS   4
#elif defined( gpio3data_adr )
    #define GPIO_NUMBER_OF_BLOCKS   3
#elif defined( gpio2data_adr )
    #define GPIO_NUMBER_OF_BLOCKS   2
#else
    #define GPIO_NUMBER_OF_BLOCKS   1
#endif

#define GPIO_NUMBER_OF_IOS   ( GPIO_NUMBER_OF_BLOCKS * 8 )

#define MIA_GPIO_ASSIGNMENT  0xAA // special value used for MIA

#if defined(BT_FW_AUDIO_CORE)
    #define ENHANCED_INTERRUPT_DRIVER
#endif

/********************* TYPE DEFINITIONS ***************************************/
typedef void GPIO_ISR_CALLBACK(void);
typedef UINT32 MIA_STATUS_TYPE(void);

typedef struct
{
    UINT32  DataDirection;
    UINT32  IntSense;
    UINT32  IntBothEdges;
    UINT32  IntEvent;
    UINT32  IntEnableMask;
    UINT32  RawIntStatus;
    UINT32  MaskedIntStatus;
    UINT32  IntClear;
    UINT32  ModeCtrlSel;
} GPIO_REG_INTERFACE;

/********************* GLOBAL VARIABLE DECLARATIONS ***************************/

/********************* FUNCTION DECLARATIONS **********************************/
void GPIO_Init( void );
void GPIO_WriteReg( UINT32 gpioNum, UINT32 regOffset, BOOL32 level);
void GPIO_ReadReg( UINT32 gpioBlockNum, UINT32 regOffset, UINT32 *dataPtr );
UINT32 *GPIO_GetDataPtr( UINT32 gpioBlockNum, UINT8 gpioBitMask );
UINT32 GPIO_ReadData( UINT32 gpioNum );
void SetMiaStatusFunctionPtr( MIA_STATUS_TYPE *funcPtr );
void GPIO_set( UINT32  gpioNum, BOOL32 val );
void GPIO_Get( UINT32 gpioBlockNum, UINT8 *pGpio );
GPIO_REG_INTERFACE *GetGpioRegPtr( UINT32 gpioNum, UINT32 *bitMaskPtr );
GPIO_REG_INTERFACE *GetGpioBlockRegPtr( UINT32 gpioBlockNum );
void GPIO_register_interrupt_handler(UINT32 gpioNum, GPIO_ISR_CALLBACK* GPIO_isr_callback);
void GPIO_InterruptEnable(UINT32 gpioNum, BOOL32 enable);
void GPIO_interrupt_handler( UINT32 gpioBlockNum );
void GPIO_debug_init_012(void);
#ifdef CPU_CM3
UINT64 GPIO_GetInterruptMask( UINT32 gpioBlockNum );
#else
UINT32 GPIO_GetInterruptMask( UINT32 gpioBlockNum );
#endif

void gpio_p2s(UINT8 cover_pin, UINT8 trace_pin, UINT32 value, UINT8 lens);
void gpio_debug_init_012(void);
void gpio_debug_output_byte_012(UINT8 val);
void gpio_debug_output_word_012(UINT16 val);
void gpio_debug_output_dword_012(UINT32 val);
#endif // _GPIO1_H_

