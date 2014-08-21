/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2014 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: blecm_set_static_bd_addr.h
*
*    Abstract: Set and commit a static random BD_ADDR according to
*              BT spec, Volume 3, Part C, section 10.8.1 Static address.
*              NOTE: This will set the BD_ADDR to a static random address
*                 if and only if the device address in the NV is
*                 0xFFFFFFFFFFFF. Once set, subsequent calls will always
*                 fail because the address in NV will no longer be all FF.
*
********************************************************************
*/
#ifndef _BLECM_SET_STATIC_BD_ADDR_H_
#define _BLECM_SET_STATIC_BD_ADDR_H_

#include "types.h"

/// Sets the bd_addr to a static random value and optionally commits to NV.
/// \param bd_addr OUT The static random bd_addr that was currently set. Buffer
///             must be long enough to hold the 6 byte BD_ADDR. Valid only
///             if commit succeeded. For the commit to succeed, device address
///             in NV must already be 0xFFFFFFFFFFFF.
/// \return TRUE on succes, FALSE on failure.
UINT8 blecm_set_static_random_bd_addr(UINT8* bd_addr);

#endif   // _BLECM_SET_STATIC_BD_ADDR_H_