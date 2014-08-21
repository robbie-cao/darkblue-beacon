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
 *    Function prototypes for ibeacon library
*
*/
#ifndef _IBEACON_H_
#define _IBEACON_H_

/// Start sending iBeacon advertisements with appropriate parameters
/// \param company_uuid 16 byte company UUID, identifies the company creating the iBeacon
/// \param major 2 byte Major ID identifying iBeacon location
/// \param major 2 byte Minor ID identifying the iBeacon withing this location
/// \param major 1 RSSI value measured by an iPhone at 1 meter distance from the iBeacon
void ibeacon_start(UINT8 *company_uuid, UINT16 major, UINT16 minor, UINT8 measured_power);

/// Stop sending iBeacons
void ibeacon_stop(void);

#endif   // _IBEACON_H
