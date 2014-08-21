#ifndef _BLEFIND_H_
#define _BLEFIND_H_
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
* BLE Find Me client profile, service, application
*
* Refer to Bluetooth SIG Find Me Profile1.0 specifications for details.
*
* This file contains definitions and function declarations for the Find Me
* profile.
*
*/
#include "bleprofile.h"



//////////////////////////////////////////////////////////////////////////////
//                      public data type definition.
//////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)





#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()


//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////
void blefind_Init(void);
void blefind_FindmeButton(void);
void    blefind_FindmeSetHandle(UINT16 findme_hdl);
UINT16  blefind_FindmeGetHandle(void);
void blefind_FindmeReq(void);

#endif // end of #ifndef _BLEFIND_H_