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
 * Abstract:  Provides support for characteristics of up to 256 octets in length
 *
 */

#include "types.h"
#include "cfa.h"

#ifndef _LONG_CHARACTERISTIC_SUPPORT_H_
#define _LONG_CHARACTERISTIC_SUPPORT_H_

/// Allows the app to read a characteristic (larger than 23 bytes) from its GATT DB.
/// When using buffer after reading, the len field will have the actual length read.
/// Note that the allocated buffer needs to be 2 + max size of
/// the characteristic you want to read. For example, if the characteristic you want to
/// read is max 50 bytes long and its handle is 0x1234:
/// #define MY_CHARACTERISTIC_SIZE_MAX     50
/// // Allocate a buffer big enough for my characterestic = MY_CHARACTERISTIC_SIZE_MAX + size of header.
/// UINT16 size_to_allocate = sizeof(UINT8) + sizeof(UINT8) + MY_CHARACTERISTIC_SIZE_MAX;
/// BLEPROFILE_DB_PDU* my_characteristic = cfa_mm_alloc(size_to_allocate);
/// // Assuming allocation succeeded here. You need to check for a NULL return value.
/// if(bleprofile_ReadHandleData(0x1234, my_characteristic, size_to_allocate) == 0)
/// {
///     // Successful.
///     UINT8 characterestic_length = my_characteristic->len;
///     UINT8* characteristic_value_ptr = my_characteristic->pdu;
/// }
/// // Done with buffer, so free it.
/// cfa_mm_Free(my characterestic);
/// \param hdl The handle of the characteristic to read.
/// \param buffer Pointer to an allocated buffer into which to read into.
/// \param pdu_alloc_length Length of the allocated buffer = sizeof(UINT8) + sizeof(UINT8) + MY_CHARACTERISTIC_SIZE_MAX
/// \return 0 for success, else failure code.
int bleprofile_ReadHandleData(UINT16 hdl, BLEPROFILE_DB_PDU* buffer, UINT16 pdu_alloc_length);

/// Allows the app to write a characteristic (larger than 23 bytes) to its GATT DB.
/// If my_characteristic is 100 bytes long:
/// BLEPROFILE_DB_PDU* my_characterestic = cfa_mm_Alloc(sizeof(UINT8) + sizeof(UINT8) + 100);
/// //// Fill up the characteristic pdu value however you see fit. For example:
/// my_characterestic->pdu[99] = 0x42;
/// if(bleprofile_WriteHandleData(0x1234, my_characteresti, sizeof(UINT8) + sizeof(UINT8) + 100))
/// {
///     // All good....
/// }
/// // Now free the buffer
/// cfa_mm_Free(my_characteresti);
/// \param hdl The handle of the characteristic to write to.
/// \param buffer Pointer to the characteristic value.
/// \param length Length of the characteristic to write
/// \return 0 for success, else failure code.
int bleprofile_WriteHandleData(UINT16 hdl, BLEPROFILE_DB_PDU* buffer, UINT16 length);

/// Set the maximum number of prepare write requests this GATT server can take before needing to execute them.
/// The default is 5 (i.e. you can have at most 5 prepare write requests before the execute, the 6th
/// will return an insufficient resources errror). If the memory pool manager limits are hit before
/// this, prepare writes will still fail.
/// \param queue_size Max number of prepare write requests before needing to execute the writes.
void bleprofile_SetMaxQueuedWriteRequests(UINT16 queue_size);


#endif  // _LONG_CHARACTERISTIC_SUPPORT_H_
