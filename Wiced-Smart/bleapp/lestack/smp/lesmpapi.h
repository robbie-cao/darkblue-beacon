#ifndef _LESMPAPI_H_
#define _LESMPAPI_H_
/*
******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-----------------------------------------------------------------------------
*
*           Copyright (c) 1999, 2011 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
******************************************************************************

******************************************************************************
*    File Name: lesmpapi.h
*
*    Abstract: This is the header file for LESMPAPI.
*              It declares the APIs for the LE SMP module.
*
*    $History:$
*
******************************************************************************
*/
// This is for WIN32 platform.
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "lesmp.h" //


// this is a generic API function.
typedef void *(*LESMPAPI_API)(void *);


typedef struct
{
    LESMPAPI_API reserved;      // 0x0,
    LESMPAPI_API PairingReq;    // 0x1
    LESMPAPI_API PairingRsp;    // 0x2
    LESMPAPI_API PairingConf;   // 0x3
    LESMPAPI_API PairingRand;   // 0x4
    LESMPAPI_API PairingFailed; // 0x5
    LESMPAPI_API EncInfo;       // 0x6
    LESMPAPI_API MasterId;      // 0x7
    LESMPAPI_API IdInfo;        // 0x8
    LESMPAPI_API IdAddrInfo;    // 0x9
    LESMPAPI_API SigningInfo;   // 0xa
    LESMPAPI_API SecurityReq;   // 0xb
} LESMPAPI_HANDLER;


/////////////////////////////////////////////////////////////////////////
// SMP msg related functions.
int    lesmpapi_pairingReq( LESMP_PAIRING_REQ *smpPkt);
#ifdef SMP_INITIATOR
int lesmpapi_pairingRsp(LESMP_PAIRING_RSP *smpPkt);
#else
void * lesmpapi_pairingRsp( void *);
#endif
int    lesmpapi_pairingConf( LESMP_PAIRING_CONF_AND_RAND *smpPkt);
int    lesmpapi_pairingRand( LESMP_PAIRING_CONF_AND_RAND *smpPkt);
void * lesmpapi_pairingFailed( void *);
void * lesmpapi_EncInfo( void *);
void * lesmpapi_MasterId( void *);
void * lesmpapi_IdInfo( void *);
void * lesmpapi_IdAddrInfo( void *);
void * lesmpapi_SigningInfo( void *);
void * lesmpapi_SecurityReq( void *);




/////////////////////////////////////////////////////////////////////////
//      



extern LESMPAPI_API *lesmpapi_msgHandlerPtr;

#define LESMPAPI_PAIRING_REQ(a) ( lesmpapi_msgHandlerPtr[LESMP_CODE_PAIRING_REQ]((a) ) )
#ifdef SMP_INITIATOR
#define LESMPAPI_PAIRING_RSP(a) ( lesmpapi_msgHandlerPtr[LESMP_CODE_PAIRING_RSP]((a) ) )
#endif
#define LESMPAPI_PAIRING_CONF(a) ( lesmpapi_msgHandlerPtr[LESMP_CODE_PAIRING_CONF]((a) ) )

#define LESMPAPI_PAIRING_RAND(a) ( lesmpapi_msgHandlerPtr[LESMP_CODE_PAIRING_RAND]((a) ) )

#define LESMPAPI_PAIRING_FAILED(a) ( lesmpapi_msgHandlerPtr[LESMP_CODE_PAIRING_FAILED]((a) ) )



// This is for WIN32 platform.
#ifdef __cplusplus
}
#endif
#endif // end of #ifndef _LESMPAPI_H_
