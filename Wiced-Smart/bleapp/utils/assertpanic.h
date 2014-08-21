/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2007 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************
*/

/*
********************************************************************
*    File Name: assertpanic.h
*
*    Abstract: Fatal assert definition
*
* /!\ The software sometimes detects error conditions which make it
* impossible or very problematic to proceed in any meaningful way.
* An example would be a memory allocation failure. In such situations
* all but the most complex of programs abort. This file contains
* a fatal assert definition for use in such situations.
* 
********************************************************************
*/

#ifndef __ASSERT_PANIC_H__
#define __ASSERT_PANIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* We *DO* *NOT* *WANT* to pass __LINE__ in ASSERT_PANICs in ROM.
 *
 * When you pass __LINE__, and you do a "no-op" change like add a
 * comment to a source file, the __LINE__ for a given ASSERT_PANIC
 * changes. That causes a change to the taped-out image. But it's very
 * useful to be able to add labels, add things under #ifdef,
 * etc. without changing the actual bits when compiled under the right
 * options. So, we pass 0 for the line number. assert_fail() will then
 * use the LR instead. */

#ifdef FPGA_BD_2045

void assert_fail( char*   file,   INT32  line , UINT32 status, char* fatal);       
#ifdef ENABLE_ASSERT
#define	ASSERT_PANIC(exp, errCode, errStr)   if(!(exp)) assert_fail( __FILE__, __LINE__, (UINT32)(errCode), (errStr))
#else
#define	ASSERT_PANIC(exp, errCode, errStr)   if(!(exp)) assert_fail( NULL, 0, (UINT32)(errCode), NULL)
#endif

#else // ASIC

void assert_fail( char*   file,   INT32  line , UINT32 status);
#ifdef ENABLE_ASSERT
#define	ASSERT_PANIC(exp, errCode, errStr)   if(!(exp)) assert_fail( __FILE__, __LINE__, (UINT32)(errCode))
#else
#define	ASSERT_PANIC(exp, errCode, errStr)   if(!(exp)) assert_fail( NULL, 0, (UINT32)(errCode))
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif // __ASSERT_PANIC_H__
