/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2013 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************
*/

/*
********************************************************************
*    File Name: brcmcryptoglue.h.h
*
*    Abstract: API to adapt crypto functions to BRCM's API.
********************************************************************
*/
/// Frees the given buffer.
void brcmcryptoglue_free(void* buffer);

/// Allocates a buffer from mem pool.
void* brcmcryptoglue_malloc(size_t size);

/// Sets up a custom deallocator.
void brcmcryptoglue_set_deallocator(void (*f)(void*));

/// Sets up a custom allocator.
void brcmcryptoglue_set_allocator(void* (*f)(size_t));