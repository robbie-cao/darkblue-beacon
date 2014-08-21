/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 1999, 2009 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: dbfw.h
*
*    Abstract: 
*       This is based upon BCM2045 Debug Framework Software Design Document ver 0.6
*
*    $History:$
*
********************************************************************
*/

#ifndef __DBFW_APP_H__
#define __DBFW_APP_H__

#include "fid_app.h" 		// File ID
#include "types.h"

#define __AT__ __FUNCTION__ ":" TOSTRING(__LINE__)
#define __LOC__ __FILE__##" "##__AT__

#define DBFW_TRACE_FORMAT_VERSION       1 
#define DBFW_CORE_DUMP_FORMAT_VERSION   1

#define DBFW_MODULE_ID_MAX      (30)    //  0 - 30
#define DBFW_FILE_ID_MAX        (2047)  //  0 - 2047
#define DBFW_LINE_NUMBER_MAX    (16383) //  1 - 16383, 0: trace_ts is used

// Global Warning Flag
#define DBFW_WARNING_OS_QUEUE_ALLOC_FAIL        0x00000001
#define DBFW_WARNING_OS_QUEUE_FREE_FAIL         0x00000002
#define DBFW_WARNING_OS_QUEUE_SEND_FAIL         0x00000004
#define DBFW_WARNING_OS_QUEUE_SEND_FRONT_FAIL   0x00000008
#define DBFW_WARNING_OS_BLOCK_POOL_FREE_FAIL    0x00000010
#define DBFW_WARNING_OS_EXCEPTION               0x00000100

// Module ID
enum
{
    MODULE_ID_DBFW   = 0,
    MODULE_ID_BLEAPP = 0x0A,
    MODULE_ID_OTHERS =  DBFW_MODULE_ID_MAX,
};

// Levels of Trace
enum
{
    TRACE_DEBUG     = 0,
    TRACE_INFO      = 1,
    TRACE_WARNING   = 2,
    TRACE_CRITICAL  = 3,
};

typedef PACKED struct
{
    //UINT32  trace_level:2;      // TRACE_CRITICAL, TRACE_WARNING, TRACE_INFO or TRACE_DEBUG
    //UINT32  trace_module_id:5;  // which module this trace belongs to
	UINT32	unused:7;
    UINT32  file_id:11;         // file id
    UINT32  line_number:14;     // if line number is 0, trace_var is a timestamp
                                //    line number is 1, trace_var is us
								//	  line 2 - 20 are reserved for different usages of trace_var
    UINT32  trace_var;          // trace variable / timestamp in half BT slot unit
} TRACE_t;

extern void trace(UINT32 level_module_id, UINT32 fid, UINT32 line, UINT32 var);
extern int trace_dump(TRACE_t *buf, int size_in_byte, UINT8 *staus);


// Trace Variable Format
// UINT32 x 1
#define TVF_D(x)            (UINT32)(x)
// UINT16 x 2
#define TVF_WW(x,y)         (UINT32)((((UINT32)(x) << 16) & 0xFFFF0000) | ((UINT32)(y) & 0x0000FFFF))
// UINT8 x 2 
#define TVF_BB(x,y)         (UINT32)((((UINT32)(x) <<  8) & 0x0000FF00) | ((UINT32)(y) & 0x000000FF))
// UINT8 x 2 + UINT16
#define TVF_BBW(x,y,z)      TVF_WW(TVF_BB(x,y), z)
// UINT8 x 4
#define TVF_BBBB(x,y,z,w)   TVF_WW(TVF_BB(x, y), TVF_BB(z, w))

#ifndef TRACE_COMPILE_LEVEL
#define TRACE_COMPILE_LEVEL 3       // Default Trace Compile Level
#endif

#if (TRACE_COMPILE_LEVEL == 0)
#define TRACE_TRACE_DEBUG(module, msg, var) trace(TVF_WW(TRACE_DEBUG, module), FID, __LINE__,var)
#define TRACE_TS_TRACE_DEBUG(module, msg, var) trace_ts(TVF_WW(TRACE_DEBUG, module), FID, __LINE__,var)
#endif

#if (TRACE_COMPILE_LEVEL <= 1)
#define TRACE_TRACE_INFO(module, msg, var) trace(TVF_WW(TRACE_INFO, module), FID, __LINE__,var)
#define TRACE_TS_TRACE_INFO(module, msg, var) trace_ts(TVF_WW(TRACE_INFO, module), FID, __LINE__,var)
#endif

#if (TRACE_COMPILE_LEVEL <= 2)
#define TRACE_TRACE_WARNING(module, msg, var) trace(TVF_WW(TRACE_WARNING, module), FID, __LINE__,var)
#define TRACE_TS_TRACE_WARNING(module, msg, var) trace_ts(TVF_WW(TRACE_WARNING, module), FID, __LINE__,var)
#endif

#if (TRACE_COMPILE_LEVEL <= 3)
#define TRACE_TRACE_CRITICAL(module, msg, var) trace(TVF_WW(TRACE_CRITICAL, module), FID, __LINE__,var)
#define TRACE_TS_TRACE_CRITICAL(module, msg, var) trace_ts(TVF_WW(TRACE_CRITICAL, module), FID, __LINE__,var)
#endif

#if !defined(TRACE_TRACE_WARNING)
#define TRACE_TRACE_WARNING(module, msg, var)
#define TRACE_TS_TRACE_WARNING(module, msg, var)
#endif

#if !defined(TRACE_TRACE_INFO)
#define TRACE_TRACE_INFO(module, msg, var)
#define TRACE_TS_TRACE_INFO(module, msg, var)
#endif

#if !defined(TRACE_TRACE_DEBUG)
#define TRACE_TRACE_DEBUG(module, msg, var)
#define TRACE_TS_TRACE_DEBUG(module, msg, var)
#endif

#define TRACE(level, module, msg, var) TRACE_##level(module, msg, var)


#define TRACE_FORCE_DUMP()     dbfw_ForceDump()

#endif  // end of __DBFW_H__

