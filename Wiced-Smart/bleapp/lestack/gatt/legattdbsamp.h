#ifndef _LEGATTDBSAMP_H_
#define _LEGATTDBSAMP_H_
/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*                                                                        
*           Copyright (c) 2011 Broadcom Corp.
*                      ALL RIGHTS RESERVED                              
*                                                                       
********************************************************************

 ********************************************************************
 *    File Name: legattDbSamp.h
 *
 *    Abstract: This file exports a default database for test purpose.
 *
 *    Functions:
 *            --     
 *
 *    $History:$ 
 *
 ********************************************************************
*/
//
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"



extern const UINT8 legattDbSamp_data[];
extern const UINT16 legattDbSamp_size;

void legattDbSamp_init(UINT8**startPtr, UINT8 **endPtr);

//
#ifdef __cplusplus
}
#endif
#endif // end of #ifndef _LEGATTDBSAMP_H_