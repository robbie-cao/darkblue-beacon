#ifndef _AES_CMAC_H_
#define _AES_CMAC_H_
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
*    File Name: aes_cmac.h
*
*    Abstract: This is header file export one function. This function is 
*               a public domain source code. We don't own the right to it.
*
*    $History:$
*
******************************************************************************
*/


#ifdef __cplusplus
extern "C" {
#endif

// This function takes input in Big endian format. Its output is also in 
// big endian format. "length" is in unit of bytes.
void AES_CMAC( unsigned char *key, unsigned char *input, int length,
                  unsigned char *mac );


#ifdef __cplusplus
}
#endif

#endif // end of #ifndef _AES_CMAC_H_
