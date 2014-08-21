/********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2014 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: additional_gatt_apis.h
*
*    Abstract:  Prvide additional GATT APIs.
*
*    $History:
*           Created   07/22/14
*
*********************************************************************/

//===================================================================
//      Include
//===================================================================
#include <types.h>

//===================================================================
// Functions : bleprofile_sendReadByTypeReq_v2
//
// The function provides a second version bleprofile level API to send ATT ¡§Read By Type Request¡¨.
// The original one can only accept 16-bit UUID, but this one can accept 16-bit UUID or 128-bit UUID.
//===================================================================
void bleprofile_sendReadByTypeReq_v2(UINT16 startHandle, UINT16 endHandle, UINT8 uuid[], UINT8 uuid_len);

//===================================================================
// Functions : bleprofile_sendFindByTypeValueReq
//
// The function provides a bleprofile level API to send ATT ¡§Find By Type Value Request¡¨.
//===================================================================
void bleprofile_sendFindByTypeValueReq(UINT16 startHandle, UINT16 endHandle, UINT16 attrType, UINT8 attrValue[], UINT8 attrValueLength);

//===================================================================
// Functions : bleprofile_sendFindInfoReq
//
// The function provides a bleprofile level API to send ATT ¡§Find Information Request¡¨.
//===================================================================
void bleprofile_sendFindInfoReq(UINT16 startHandle, UINT16 endHandle);

//===================================================================
// Functions : leatt_regFindByTypeValueRspCb
//
// The function provides an API to register the callback function for ATT ¡§Find By Type Value Response¡¨.
//===================================================================
void leatt_regFindByTypeValueRspCb(LEATT_DOUBLE_PARAM_CB cb);

//===================================================================
// Functions : leatt_regFindByTypeValueRspCb
//
// The function provides an API to register the callback function for ATT ¡§Find Information Response¡¨.
//===================================================================
void leatt_regFindInfoRspCb(LEATT_TRIPLE_PARAM_CB cb);

//===================================================================
// Functions : leatt_regFindByTypeValueRspCb
//
// The function provides an API to register the callback function for ATT "Error Response¡¨.
//===================================================================
void leatt_regErrRspCb(LEATT_QUADRUPLE_PARAM_CB cb);
