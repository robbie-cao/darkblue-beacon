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
* Define functions to access I2C master peripheral
*/
#ifndef __I2CM_DRIVER_H__
#define __I2CM_DRIVER_H__

#include "types.h"



/**  \addtogroup I2C
 *  \ingroup HardwareDrivers
*/
/*! @{ */
/**
* Defines the BCM I2C master driver. The   BCM standard I2CM driver provides
* the status and control for the I2C HW.
*
*/

enum
{
    /// I2C speed is 100 KHz
    I2CM_SPEED_100KHZ = (0x01 << 4),

    /// I2C speed is 400 KHz
    I2CM_SPEED_400KHZ = (0x00 << 4),

    /// I2C speed is 800 KHz
    I2CM_SPEED_800KHZ = (0x02 << 4),

    /// I2C speed is 1 MHz
    I2CM_SPEEC_1000KHZ = (0x03 << 4),

    /// I2C speed is 2 MHz
    /// NOT AVAILABLE IN ALL PARTS
    /// Contact BRCM for more info
    /// I2C will default to 100 KHz if not supported
    I2CM_SPEEC_2000KHZ = (0x04 << 4)
};

/// Driver status.
enum
{
    /// The transaction was sucessful
    I2CM_SUCCESS,

    /// The attempted operation failed, possibly because
    /// of no ack from slave.
    I2CM_OP_FAILED,

    /// The I2C HW block is busy with another transaction.
    I2CM_BUSY
};

/// SCL speed is governed by a counter that counts the number
/// of cycles of the reference clock which is always 24 MHz.
/// So, for a speed of 2.4 MHz, the counter has to be set to
/// a value of 10 while for a speed of 100 KHz, the counter
/// has to be set to 240.
enum
{
    /// The minimum value of SCL counter for maximum
    /// SCL speed of 2.4 MHz. Speeds higher than this may
    /// not be possible to achieve without restrictions.
    I2CM_SCL_SPEED_MAX = 10,

    /// The maximum value of SCL counter for minimum
    /// SCL speed of ~94.1 KHz. Speeds lower than this
    /// are not possible.
    I2CM_SCL_SPEED_MIN = 255,
};

/// I2C master initialization routine.
void i2cm_init(void);

/// Allows the I2C speed to be selected. Default is 100 KHz.
/// \param divisor The divisor to use on the 24 MHz reference to get the
///     desired I2C SCL speed.
void i2cm_setSpeed(UINT8 divisor);

/// Internal helper function to set speed.
void i2cm_setTransactionSpeed(void);

/// Gets the current speed set up by the application.
/// \return The currently configured divisor.
UINT8 i2cm_getSpeed(void);

/// Writes data to the I2C slave. Max transaction length is 16 bytes.
/// longer transactions will be broken down into multiple 16 byte
/// transactions.
/// param data Pointer to a buffer that has the data to write.
/// \param length Length of the data to write to slave device.
/// \param slave The address of the slave device. Slave address is always control address << 1 (slave[7:1] are valid).
/// \return Status - success, failure or busy.
UINT8 i2cm_write(UINT8* data, UINT16 length, UINT8 slave);

/// Reads data from the I2C slave. Max transaction length is 16 bytes.
/// longer transactions will be broken down into multiple 16 byte
/// transactions.
/// param data Pointer to a buffer into which data is to be read.
/// \param length Length of the data to read from slave device.
/// \param slave The address of the slave device. Slave address is always control address << 1 (slave[7:1] are valid).
/// \return Status - success, failure or busy.
UINT8 i2cm_read(UINT8* data, UINT16 length, UINT8 slave);

/// Do a combination write, followed by a repeated start followed by a read transaction. If the second
/// transaction is longer than 16 bytes, firstTranBuf will be treated as an address and automatically be
/// incremented by the transaction size before the next chunk is read back in. This is useful when reading
/// large number of bytes from an EEPROM.
/// \param secondTranBuf Pointer to a buffer into which the read portion of the transaction is to read into.
/// \param secondTranCount Number of bytes to read in the second transaction.
/// \param firstTranBuf Pointer to a buffer from which the write portion of the transaction is to write from.
/// \param firstTranCount Number of bytes to write in the first transaction.
/// \param slaveAdr The address of the slave device. Slave address is always control address << 1 (slave[7:1] are valid).
/// \return Status - success, failure or busy.
UINT8 i2cm_comboRead(UINT8* secondTranBuf, UINT16 secondTranCount, UINT8* firstTranBuf, UINT8 firstTranCount, UINT8 slaveAdr);

/// Do a combination write, followed by a repeated start followed by a write transaction.  If the second
/// transaction is longer than 16 bytes, firstTranBuf will be treated as an address and automatically be
/// incremented by the transaction size before the next chunk is read back in.
/// \param secondTranBuf Pointer to a buffer from which the second write portion of the transaction is to write from.
/// \param secondTranCount Number of bytes to write in the second transaction.
/// \param firstTranBuf Pointer to a buffer from which the write portion of the transaction is to write from.
/// \param firstTranCount Number of bytes to write in the first transaction.
/// \param slaveAdr The address of the slave device. Slave address is always control address << 1 (slave[7:1] are valid).
/// \return Status - success, failure or busy.
UINT8 i2cm_comboWrite(UINT8* secondTranBuf, UINT16 secondTranCount, UINT8* firstTranBuf, UINT8 firstTranCount, UINT8 slaveAdr);

/* @} */


#endif
