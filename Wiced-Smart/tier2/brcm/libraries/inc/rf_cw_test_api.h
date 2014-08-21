/********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2007 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: rf_cw_test_api.h
*
*    Abstract: Provides additional API for RF conformance tests.
*
********************************************************************
*/
#ifndef _RF_CW_TEST_API_H_
#define _RF_CW_TEST_API_H_

#include "types.h"

////////////////////////////////////////////////////////////////////////////////
/// Enables TX CW test.
/// \param cwFreq   The desired Bluetooth carrier frequency (min: 2402MHz, max: 2490MHz).
/// \param txPower    The TX power to use.Will be rounded to closest value possible.
/// \return TRUE if successful; else FALSE.
////////////////////////////////////////////////////////////////////////////////
UINT8 blecm_EnableUnmodulatedTxCarrierFrequencyTest(UINT16 cwFreq, INT8 txPower);

////////////////////////////////////////////////////////////////////////////////
/// Disables unmodulated carrier frequenct TX test.
/// \return TRUE if successful; else FALSE.
////////////////////////////////////////////////////////////////////////////////
UINT8 blecm_DisableUnmodulatedTxCarrierFrequencyTest(void);

#endif
