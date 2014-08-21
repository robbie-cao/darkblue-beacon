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
* This file provides definitions and macros for generating GATT database
*
*/
#ifndef _LEATTDB_H_
#define _LEATTDB_H_

#include "types.h"
#include "leatt.h"



// The purpose of these 2 macros is to abstract the multi-byte access.
// Some CPU architecture has difficulty to directly access un-aligned
// muti-byte entities. We need to extract 1 byte at a time for ARM7
// based CPU + ARM toolset. For CM3, we don't need to do the byte 
// access. ARM7 + Green Hills Toolset also do not need the byte access.
#define LEATTDB_GET_UINT16( a )   ( * ((UINT16 *)(a)) )
#define LEATTDB_SET_UINT16( a,b ) ( * ((UINT16 *)(a)) = (b) )

// 
#define LEATTDB_LAST_HANDLE   0xffff

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//              public data type definition.
#ifdef _WIN32
#include <pshpack1.h>
#endif
// GHS syntax.
#pragma pack(1)

///////////////////////////////////////////////////////////////////
//  The permission bits in the database.
// This flag is choosen to match Vol 3, Part F, 3.3.1.1
#define LEGATTDB_PERM_NONE                             (0x00)
#define LEGATTDB_PERM_VARIABLE_LENGTH                  (0x1<<0)
#define LEGATTDB_PERM_READABLE                         (0x1<<1)
#define LEGATTDB_PERM_WRITE_CMD                        (0x1<<2)
#define LEGATTDB_PERM_WRITE_REQ                        (0x1<<3)
#define LEGATTDB_PERM_AUTH_READABLE                    (0x1<<4)
#define LEGATTDB_PERM_RELIABLE_WRITE                   (0x1<<5)
#define LEGATTDB_PERM_AUTH_WRITABLE                    (0x1<<6)

#define LEGATTDB_PERM_WRITABLE  (LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ| LEGATTDB_PERM_AUTH_WRITABLE)


// All the permission bits.
#define LEGATTDB_PERM_MASK                             (0x7f)


#define LEGATTDB_PERM_SERVCIE_UUID_128                 (0x1<<7)

///////////////////////////////////////////////////////////////////
//  GATT Characteristic Properties
#define LEGATTDB_CHAR_PROP_BROADCAST                  (0x1<<0)
#define LEGATTDB_CHAR_PROP_READ                       (0x1<<1)
#define LEGATTDB_CHAR_PROP_WRITE_NO_RESPONSE          (0x1<<2)
#define LEGATTDB_CHAR_PROP_WRITE                      (0x1<<3)
#define LEGATTDB_CHAR_PROP_NOTIFY                     (0x1<<4)
#define LEGATTDB_CHAR_PROP_INDICATE                   (0x1<<5)
#define LEGATTDB_CHAR_PROP_AUTHD_WRITES               (0x1<<6)
#define LEGATTDB_CHAR_PROP_EXTENDED                   (0x1<<7)

typedef PACKED struct
{
    UINT16 handle;  // This is the handle of the entry.
    UINT8  perm;    // This is the permission.
    UINT8  len;     // This is the length of the entry. It excludes the header.
} LEGATTDB_ENTRY_HDR;

typedef PACKED struct
{
    UINT16 handle;  // This is the handle of the entry.
    UINT8  perm;    // This is the permission.
    UINT8  len;     // This is the length of the entry. It excludes the header.
    UINT8  maxLen;  // This is the max space for writable attribute. This byte is not
                    // counted in the length. It is used for attribute truncation.
} LEGATTDB_WRITABLE_ENTRY_HDR;

typedef PACKED struct
{
    UINT16 handle;  // This is the handle of the entry.
    UINT8  perm;    // This is the permission.
    UINT16 len;     // This is the length of the entry. It excludes the header.
} LEGATTDB_ENTRY_HDR16;

typedef PACKED struct
{
    UINT16 handle;  // This is the handle of the entry.
    UINT8  perm;    // This is the permission.
    UINT16 len;     // This is the length of the entry. It excludes the header.
    UINT16 maxLen;  // This is the max space for writable attribute. This byte is not
                    // counted in the length. It is used for attribute truncation.
} LEGATTDB_WRITABLE_ENTRY_HDR16;


#ifdef _WIN32
#include <poppack.h>
#endif
// GHS syntax.
#pragma pack()


// This call back takes one parameter.
typedef INT32 (*LEGATTDB_WRITE_CB)(void * );




// This is bit [2].
#define LEGATTDB_PERM_FLAG_WRITE_CMD    LEGATTDB_PERM_WRITE_CMD  
// This is bit [6].
#define LEGATTDB_PERM_FLAG_AUTH         LEGATTDB_PERM_AUTH_WRITABLE 
// Internal  means internal access, permission check is waived.
#define LEGATTDB_PERM_FLAG_INTERNAL     (0x1ul << 31)


//////////////////////////////////////////////////////////////////////////////
//                      public interface declaration
//////////////////////////////////////////////////////////////////////////////

/**
* \brief GATT DB Initi
* \ingroup blegatt
*
* \details This function should be called to initialize the variable.
*
*/
void legattdb_init(void);

// This function will set DB with different value
void legattdb_SetDb(UINT8 *ptr, UINT16 size);

// this function will search the LE GATT database to find the
INT32 legattdb_readBlob( 
    UINT16 handle, 
    INT32 offset,
    LEATT_INTERNAL_PDU *pdu
    );


INT32 legattdb_readMultipleReq( 
        UINT8 *attrHandleList, 
        INT32    numAttrHandles,
        LEATT_INTERNAL_PDU *pdu);


// 
INT32 legattdb_findInfo(
    UINT16 startHandle,     // input param. start handle.
    UINT16 endHandle,       // input param, end handle
    LEATT_INTERNAL_PDU *outPdu
    );

// this function will search the LE GATT database using 16 bit UUID.
INT32 legattdb_readByAttrGroupTypeUUID16(
    UINT16 startHandle,     // input param. start handle.
    UINT16 endHandle,       // input param. end handle.
    UINT16 uuid,            // 
    LEATT_INTERNAL_PDU *outPdu
    );

void legattdb_readByGroupTypeUUID128(
    UINT16 startHandle,     // input param. start handle.
    UINT16 endHandle,       // input param. end handle.
    UINT8 *uuid,            // 
    LEATT_INTERNAL_PDU *outPdu
    );

// this function will search the LE GATT database using 16 bit UUID.
INT32 _legattdb_findAttrGroupTypeUUID16(
    UINT16 startHandle,     // input param. start handle.
    UINT16 endHandle,       // input param, end handle
    UINT16 uuid,            // input param, 16 bit uuid to search for.
    INT32 len,              // input param, size of the output buffer.
    UINT8 *buffer,          // output param, the result will be here.
    INT32 *outLen           // output param, the length of attributes retrieved.
    );


INT32 legattdb_findByTypeValue( 
    UINT16 startHandle, //
    UINT16 endHandle,   //
    UINT16 uuid,       // attribute uuid.
    UINT8 *attrValue,   // attribute value
    INT32 attrValueLen,   // length of attribute value.
    LEATT_INTERNAL_PDU *outPdu
    );

INT32 legattdb_findByType(
    UINT16 startHandle,
    UINT16 endHandle,
    UINT16 attrType,  // input parameter. attribute type uuid.
    LEATT_INTERNAL_PDU *outPdu
    );

INT32 legattdb_readByType(
    UINT16 startHandle,
    UINT16 endHandle,
    INT32    uuidLen, // size of uuid.
    UINT8 *uuid, //  pointer to uuid.
    LEATT_INTERNAL_PDU *outPdu
    );

INT32 legattdb_readHandle( 
    UINT16 handle,      // handle to read.
    LEATT_INTERNAL_PDU *pdu,
    UINT32 permFlag     //
);


INT32 legattdb_writeHandle( 
    UINT16 handle,      // handle to write.    
    UINT16 offset,      // offset to write.
    UINT8 *attrValue,   // attribute value to write.
    INT32 attrValLen,     // length of attribute value.
    UINT32 permFlag     // This is permission flag.
);


INT32 legattdb_prepareWriteReq( LEATT_PDU_PREPARE_WRITE_REQ_HDR *hdr, 
        INT32 attrValLen);

INT32 legattdb_executeWriteReq(UINT8 flag);


INT32 legattdb_writeCmd( UINT16 handle, UINT8 *attrVale, INT32 attrLen);


// This function will dump the current database.
void legattdb_dumpDb(void);

// if 1, at least one notification is turned on
// if 0, all notifications are turned off
UINT8 legattdb_checkNotificationDb(void);

//////////////////////////////////////////////////////////////////////////////
//  Here are the functions that supports the database access.
//
/**
* \brief Get Next database entry
* \ingroup blegattaccess
*
* \details This function will return a pointer to next entry.
* 
* \param p Pointer to the current entry
*
* \return Pointer to the next entry, or NULL if the entry is the last in the database.
*/
// 
LEGATTDB_ENTRY_HDR * legattdb_nextEntry( LEGATTDB_ENTRY_HDR *p);


/**
* \brief Get entry handle
* \ingroup blegattaccess
*
* \details This function returns the attribute handle of this entry.
* 
* \param p Pointer to the current entry.
*
* \return Handle of the entry.
*/
UINT16 legattdb_getHandle(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Get entry handle
* \ingroup blegattaccess
*
* \details This will return a attribute UUID in the entry. If the attribute
* uuid is not 2 bytes the function will return invalid uuid 0x00.
* 
* \param p Pointer to the current entry.
*
* \return Attribute UUID of the entry.
*/
UINT16 legattdb_getAttrUUID16(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Copy UUID of an entry to a buffer
* \ingroup blegattaccess
*
* \details This function will copy UUID of the entry to a buf 
* 
* \param p Pointer to the current entry.
*
* \return Length of the UUID which can be 2 or 16 bytes.
*/
INT32    legattdb_getAttrUUID(LEGATTDB_ENTRY_HDR *h, UINT8 *buf);

/**
* \brief Get value UUID
* \ingroup blegattaccess
*
* \details This is a convenient function. It returns the attribute value in the 
* form of UUID16. Caller need to be sure this is a UUID16. Or else it...
* 
* \param p Pointer to the current entry.
*
* \return Value UUID of the entry.
*/
UINT16 legattdb_getAttrValueUUID16(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Get Attribute Value
* \ingroup blegattaccess
*
* \details This function will return a pointer to the attribute value.
* 
* \param p Pointer to the current entry.
*
* \return Pointer to the attribute value.
*/
UINT8 *legattdb_getAttrValue(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Get Attribute Value Length
* \ingroup blegattaccess
*
* \details This function returns length of the attribute value.
* 
* \param p Pointer to the current entry.
*
* \return Length of the attrbute value.
*/
INT32    legattdb_getAttrValueLen(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Set Attribute Value Length
* \ingroup blegattaccess
*
* \details This function change the length of current attriute length.
* 
* \param p Pointer to the current entry.
* \param len New length.
*
*/
void   legattdb_setAttrValueLen(LEGATTDB_ENTRY_HDR *p, INT32 len);

/**
* \brief Get Maxmimut Writable Attribute Value Length
* \ingroup blegattaccess
*
* \details This function return the maximum writable attribute length. If the attribute
* is not writable, it will return 0.
* 
* \param p Pointer to the current entry.
*
* \return Length that can be written to the current entrie's value, or 0 if not writable.
*
*/
INT32    legattdb_getMaxWritableAttrValueLen(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Update Value of the Attribute
* \ingroup blegattaccess
*
* \details This function will update the writable attribute.
* 
* \param p Pointer to the current entry.
* \param offset Offset at which data need to be written.
* \param Pointer to the data to write.
* \param len Length of the data to write.
* \param pwermFlag Attribute permissions.
*
* \return Length that can be written to the current entrie's value, or 0 if not writable.
*
*/
INT32 legattdb_writeAttrValue(
   LEGATTDB_ENTRY_HDR *p, UINT16 offset, UINT8 *val, INT32 len, UINT32 permFlag);

/**
* \brief Get Attribute Permissions
* \ingroup blegattaccess
*
* \details This function will return permission byte of the attribute.
* 
* \param p Pointer to the entry.
*
* \return Permission byte.
*
*/
INT32    legattdb_getPerm(LEGATTDB_ENTRY_HDR *p);

/**
* \brief Get Pointer to Attribute
* \ingroup blegattaccess
*
* \details This function will return a pointer to the actual attribute skipping over the header.
* 
* \param p Pointer to the entry.
*
* \return Pointer to the attribute.
*
*/
UINT8 *legattdb_getAttrPtr( LEGATTDB_ENTRY_HDR *p);

/**
* \brief Compare UUIDs
* \ingroup blegattaccess
*
* \details This function will compare the UUID of _1 and _2 . It return TRUE if they are the same. 
* if the size of the two uuid are not the same, they will be converted to 128 bit uuid for comparison.
*
* \param uuidLen_1 Length of the first UUID to compare.
* \param uuidPtr_1 Pointer to the first UUID to compare.
* \param uuidLen_2 Length of the second UUID to compare.
* \param uuidPtr_2 Pointer to the second UUID to compare.
*
* \return TRUE if UUIDs are the same, FALSE otherwise.
*
*/
INT32 legattdb_uuidIsTheSame( 
        INT32 uuidLen_1, UINT8 *uuidPtr_1, INT32 uuidLen_2, UINT8 *uuidPtr_2);

UINT32 legattdb_checkPermission( INT32 permToCheck,UINT32 perm, LEGATTDB_ENTRY_HDR *p);
void legattdb_connDown();
void legattdb_connUp();

/**
* \brief Register write callback
* \ingroup blegattaccess
*
* \details This function register a callback when data is written
*
* \param cb Pointer to the application callback.
*
* \return TRUE if callback was set successully, FALSE if not enough space.
*
*/
INT32  legattdb_regWriteHandleCb(LEGATTDB_WRITE_CB cb);

/**
* \brief Register write callback
* \ingroup blegattaccess
*
* \details This function de-register a callback set by \c legattdb_regWriteHandleCb
*
* \param cb Pointer to the application callback.
*
*/
void legattdb_deRegWriteHandleCb(LEGATTDB_WRITE_CB cb);


INT32 legattdb_dispatchWriteCb( LEGATTDB_ENTRY_HDR *p);

UINT32 legattdb_checkEncAuthRequirement( );

/**
* \brief Finds Characteristic Descriptor
* \ingroup blegattaccess
*
* \details This function finds descriptor with specified UUID which belongs to the specific characteristic
*
* \param cb Pointer to the application callback.
*
*/
UINT16 legattdb_findCharacteristicDescriptor(UINT16 char_handle, UINT16 descriptor_uuid);

#ifdef __cplusplus
}
#endif
#endif // end of #ifndef _LEATTDB_H_
