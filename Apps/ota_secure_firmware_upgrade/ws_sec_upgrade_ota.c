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
* WICED Smart UART Upgrade 
*
* This file provides function required to support Over the Air WICED Smart Upgrade.
*
* To download host sends command to download with length of the patch to be
* transmitted.  After device acks that, host sends fixed chunks of data
* each of each has to be acked.  After all the bytes has been downloaded
* and acknowledged host sends verify command that includes CRC32 of the
* whole patch.  During the download device saves data directly to the EEPROM
* or serial flash.  At the verification stage device reads data back from the
* NVRAM and calculates checksum of the data stored there.  Result of the
* verification is sent back to the host.
*
*/
// #define BLE_TRACE_DISABLE 1

#include "bleprofile.h"
#include "bleapp.h"
#include "puart.h"
#include "gpiodriver.h"
#include "string.h"
#include "stdio.h"
#include "platform.h"
#include "bleapputils.h"
#include "bleappfwu.h"
#include "ws_sec_upgrade_ota.h"
#include "ws_upgrade.h"
#include "md.h"
#include "rsa.h"
#include "sha2.h"
#include "spar_utils.h"
#include "thread_and_mem_mgmt.h"

/******************************************************
 *                      Constants
 ******************************************************/

// device states during OTA FW upgrade
#define WS_UPGRADE_STATE_IDLE                   0
#define WS_UPGRADE_STATE_READY_FOR_DOWNLOAD     1
#define WS_UPGRADE_STATE_VERSION_TRANSFER       2
#define WS_UPGRADE_STATE_DATA_TRANSFER          3
#define WS_UPGRADE_STATE_SIGNATURE_TRANSFER     4
#define WS_UPGRADE_STATE_VERIFICATION           5
#define WS_UPGRADE_STATE_VERIFIED               6
#define WS_UPGRADE_STATE_ABORTED                7

// downloader will use 16 byte chunks
#define WS_UPGRADE_READ_CHUNK                   20

// write to eeprom in 64 byte chunks
#define WS_UPGRADE_CHUNK_SIZE_TO_COMMIT         60


/******************************************************
 *               Variables Definitions
 ******************************************************/

UINT8   ws_upgrade_state   = WS_UPGRADE_STATE_IDLE;

UINT16  ws_upgrade_client_configuration;        // characteristic client configuration descriptor
UINT8   ws_upgrade_status;                      // Current status
UINT16  ws_upgrade_current_offset;              // Offset in the image to store the data
UINT16  ws_upgrade_total_len;                   // Total length expected from the host
UINT16  ws_upgrade_patch_len;					// length of the patch from the host

WS_UPGRADE_APP_INFO ws_upgrade_info;
extern WS_UPGRADE_APP_INFO WsUpgradeAppInfo;

int     ws_upgrade_current_block_offset;
int     ws_upgrade_total_offset;
int     ws_upgrade_signature_offset;

UINT8   ws_upgrade_sig[WS_UPGRADE_RSA_SIGNATURE_LEN];
UINT8   ws_upgrade_sha256sum[32];

rsa_context rsaCtx;
extern UINT8 rsa_pub_key[];

char    ws_upgrade_read_buffer[WS_UPGRADE_CHUNK_SIZE_TO_COMMIT];

static void mgf_mask( unsigned char *dst, size_t dlen, unsigned char *src,
                      size_t slen, md_context_t *md_ctx )
{
    unsigned char mask[POLARSSL_MD_MAX_SIZE];
    unsigned char counter[4];
    unsigned char *p;
    unsigned int hlen;
    size_t i, use_len;

    memset( mask, 0, POLARSSL_MD_MAX_SIZE );
    memset( counter, 0, 4 );

    hlen = md_ctx->md_info->size;

    // Generate and apply dbMask
    //
    p = dst;

    while( dlen > 0 )
    {
        use_len = hlen;
        if( dlen < hlen )
            use_len = dlen;

        md_starts( md_ctx );
        md_update( md_ctx, src, slen );
        md_update( md_ctx, counter, 4 );
        md_finish( md_ctx, mask );

        for( i = 0; i < use_len; ++i )
            *p++ ^= mask[i];

        counter[3]++;

        dlen -= use_len;
    }
}

/*
 * Implementation of the PKCS#1 v2.1 RSASSA-PSS-VERIFY function
 */
int rsa_rsassa_pss_verify_( rsa_context *ctx,
                           int mode,
                           int hash_id,
                           unsigned int hashlen,
                           const unsigned char *hash,
                           unsigned char *sig )
{
    int ret;
    size_t siglen;
    unsigned char *p;
    unsigned char buf[POLARSSL_MPI_MAX_SIZE];
    unsigned char result[POLARSSL_MD_MAX_SIZE];
    unsigned char zeros[8];
    unsigned int hlen;
    size_t slen, msb;
    const md_info_t *md_info;
    md_context_t md_ctx;

    if( ctx->padding != RSA_PKCS_V21 )
        return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );

    siglen = ctx->len;

    if( siglen < 16 || siglen > sizeof( buf ) )
        return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );

    ret = ( mode == RSA_PUBLIC )
          ? rsa_public(  ctx, sig, buf )
          : rsa_private( ctx, sig, buf );

    if( ret != 0 )
        return( ret );

    p = buf;

    if( buf[siglen - 1] != 0xBC )
    {
    	ble_trace2("0x%02x not 0xBC siglen:%d", buf[siglen - 1], siglen);
        return( POLARSSL_ERR_RSA_INVALID_PADDING );
    }
    switch( hash_id )
    {
        /* BRCM: We will not support MD2 and MD4. */
        /*
        case SIG_RSA_MD2:
        case SIG_RSA_MD4:
        */
        case SIG_RSA_MD5:
            hashlen = 16;
            break;

        case SIG_RSA_SHA1:
            hashlen = 20;
            break;

        case SIG_RSA_SHA224:
            hashlen = 28;
            break;

        case SIG_RSA_SHA256:
            hashlen = 32;
            break;

        case SIG_RSA_SHA384:
            hashlen = 48;
            break;

        case SIG_RSA_SHA512:
            hashlen = 64;
            break;

        default:
            return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );
    }

    md_info = md_info_from_type( ctx->hash_id );
    if( md_info == NULL )
        return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );

    hlen = md_get_size( md_info );
    slen = siglen - hlen - 1;

    memset( zeros, 0, 8 );

    // Note: EMSA-PSS verification is over the length of N - 1 bits
    //
    msb = mpi_msb( &ctx->N ) - 1;

    // Compensate for boundary condition when applying mask
    //
    if( msb % 8 == 0 )
    {
        p++;
        siglen -= 1;
    }
    if( buf[0] >> ( 8 - siglen * 8 + msb ) )
        return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );

    md_init_ctx( &md_ctx, md_info );

    mgf_mask( p, siglen - hlen - 1, p + siglen - hlen - 1, hlen, &md_ctx );

    buf[0] &= 0xFF >> ( siglen * 8 - msb );

    while( *p == 0 && p < buf + siglen )
        p++;

    if( p == buf + siglen ||
        *p++ != 0x01 )
    {
        md_free_ctx( &md_ctx );
    	ble_trace1("not 0x01 siglen:%d", siglen);
        return( POLARSSL_ERR_RSA_INVALID_PADDING );
    }

    slen -= p - buf;

    // Generate H = Hash( M' )
    //
    md_starts( &md_ctx );
    md_update( &md_ctx, zeros, 8 );
    md_update( &md_ctx, hash, hashlen );
    md_update( &md_ctx, p, slen );
    md_finish( &md_ctx, result );

    md_free_ctx( &md_ctx );

    if( memcmp( p + slen, result, hlen ) == 0 )
        return( 0 );
    else
        return( POLARSSL_ERR_RSA_VERIFY_FAILED );
}

// Initialize peripheral UART upgrade procedure
int ws_upgrade_ota_init(void)
{
    ws_upgrade_state = WS_UPGRADE_STATE_IDLE;

    // register memory management function
    brcmcryptoglue_set_allocator(cfa_mm_Alloc);
    brcmcryptoglue_set_deallocator(cfa_mm_Free);

    // initialize padding scheme and hash algorithm
    rsa_init(&rsaCtx, RSA_PKCS_V21, POLARSSL_MD_SHA256);

    mpi_read_binary( &rsaCtx.E, &rsa_pub_key[0], 3);
    mpi_read_binary( &rsaCtx.N, &rsa_pub_key[3], WS_UPGRADE_RSA_SIGNATURE_LEN + 1);

    ws_upgrade_init();
    return TRUE;
}

// verify function is called after all the data has been received and stored
// in the NV and after the signature has been downloaded.  The function reads back
// data from the NV and calculates the hash.
// Function returns TRUE if signature verifications passes
int ws_upgrade_verify(void)
{
    unsigned int i;
    sha2_context ctx;
    int          error;
    int          bytesToRead;

    sha2_starts(&ctx, FALSE);

    // first few bytes in the stream was version information
    sha2_update(&ctx, (UINT8 *)&ws_upgrade_info, sizeof (ws_upgrade_info));

    // read patch data and update hash
    for (i = 0; i < ws_upgrade_patch_len; i += WS_UPGRADE_READ_CHUNK)
    {
        char memory_chunk[WS_UPGRADE_READ_CHUNK];

        bytesToRead = i + WS_UPGRADE_READ_CHUNK < ws_upgrade_patch_len ? WS_UPGRADE_READ_CHUNK : ws_upgrade_patch_len - i;

        ws_upgrade_retrieve_from_nv(i, memory_chunk, bytesToRead);

        sha2_update(&ctx, memory_chunk, bytesToRead);
    }

    sha2_finish(&ctx, ws_upgrade_sha256sum);

    ble_trace0("hash:\n");
    ble_tracen(ws_upgrade_sha256sum, sizeof (ws_upgrade_sha256sum));

    ble_trace0("signature:\n");
    ble_tracen(ws_upgrade_sig, sizeof (ws_upgrade_sig));

    memset( &ctx, 0, sizeof(sha2_context));

    rsaCtx.len = WS_UPGRADE_RSA_SIGNATURE_LEN;
    error = rsa_rsassa_pss_verify_(&rsaCtx, RSA_PUBLIC, SIG_RSA_SHA256, sizeof(ws_upgrade_sha256sum), ws_upgrade_sha256sum, ws_upgrade_sig);

    // just for testing verify that stack was not corrupted
    ble_trace1("StackOverflow3:%d\n", blecm_DidStackOverflow());

    ble_trace1("ws_verify err:%x\n", error);

    return (error == 0);
}

// the only time we send indication instead of notification is when we are done with upgrade, reboot
void ws_upgrade_IndicationConf(void)
{
    // sanity check
    if (ws_upgrade_state == WS_UPGRADE_STATE_VERIFIED)
    {
        ws_upgrade_finish();
    }
}

//
// Check if client has registered for notification and indication and send message if appropriate
//
int ws_upgrade_send_status(int status, int wait_for_ack)
{
    UINT8 data = (UINT8)status;

    // If client has not registered for indication or notification, do not need to do anything
    if (ws_upgrade_client_configuration == 0)
    {
        ble_trace0("send_status failed\n");
        return 0;
    }
    ble_trace1("send_status %d\n", status);
    if (wait_for_ack)
    {
        bleprofile_sendIndication(HANDLE_WS_UPGRADE_CONTROL_POINT, &data, 1, ws_upgrade_IndicationConf);
    }
    else
    {
        bleprofile_sendNotification(HANDLE_WS_UPGRADE_CONTROL_POINT, &data, 1);
    }
    return 1;
}

// handle commands received over the control point
int ws_upgrade_ota_handle_command(UINT8 *data, int len)
{
    UINT8 command = data[0];

    ble_trace2("Command:%d State:%d\n", command, ws_upgrade_state);
    if (command == WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD)
    {
        ws_upgrade_state = WS_UPGRADE_STATE_READY_FOR_DOWNLOAD;
        ws_upgrade_send_status(WS_UPGRADE_STATUS_OK, 0);
        return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
    }
    if (command == WS_UPGRADE_COMMAND_ABORT)
    {
        ws_upgrade_state = WS_UPGRADE_STATE_ABORTED;
        ws_upgrade_send_status(WS_UPGRADE_STATUS_OK, 0);
        return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
    }

    switch (ws_upgrade_state)
    {
    case WS_UPGRADE_STATE_IDLE:
        return (WS_UPGRADE_WRITE_STATUS_SUCCESS);

    case WS_UPGRADE_STATE_READY_FOR_DOWNLOAD:
        if (command == WS_UPGRADE_COMMAND_DOWNLOAD)
        {
            // command to start upgrade should be accompanied by 2 bytes with the image size
            if (len < 3)
            {
                ble_trace1("Bad Download len:%d\n", len);
                return (FALSE);
            }

            if (!ws_upgrade_init_nv_locations())
            {
            	ws_upgrade_send_status(WS_UPGRADE_STATUS_INVALID_IMAGE, 0);
            	return (FALSE);
            }

            ws_upgrade_current_offset       = 0;
            ws_upgrade_current_block_offset = 0;
            ws_upgrade_total_offset         = 0;
            ws_upgrade_signature_offset     = 0;

            ws_upgrade_total_len            = data[1] + (data[2] << 8);
            ws_upgrade_patch_len            = ws_upgrade_total_len - WS_UPGRADE_RSA_SIGNATURE_LEN - sizeof (ws_upgrade_info);

            // total transfer length should include version information in the
            // beginning and signature at the end, make sure that it is reasonable
            ws_upgrade_state                = WS_UPGRADE_STATE_VERSION_TRANSFER;

            ble_trace2("state:%d total_len:%d\n", ws_upgrade_state, ws_upgrade_total_len);

            if (ws_upgrade_total_len <= sizeof (ws_upgrade_info) + WS_UPGRADE_RSA_SIGNATURE_LEN)
            {
            	return WS_UPGRADE_WRITE_STATUS_TOO_SHORT;
            }
            // to make upgrade faster request host to reduce connection interval to minimum
            lel2cap_sendConnParamUpdateReq(6, 6, 0, 700);

            ws_upgrade_send_status(WS_UPGRADE_STATUS_OK, 0);
            return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
        }
        break;

    case WS_UPGRADE_STATE_SIGNATURE_TRANSFER:
        if (command == WS_UPGRADE_COMMAND_VERIFY)
        {
            if (ws_upgrade_verify())
            {
                ws_upgrade_state = WS_UPGRADE_STATE_VERIFIED;
                ws_upgrade_send_status(WS_UPGRADE_STATUS_OK, 1);
            }
            else
            {
                ws_upgrade_state = WS_UPGRADE_STATE_ABORTED;
                ws_upgrade_send_status(WS_UPGRADE_STATUS_VERIFICATION_FAILED, 0);
            }
            return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
        }
        break;

    case WS_UPGRADE_STATE_ABORTED:
    default:
        return (WS_UPGRADE_WRITE_STATUS_ABORTED);
    }
    // if we fall through command has not been executed
    ws_upgrade_send_status(WS_UPGRADE_STATUS_ILLEGAL_STATE, 0);
    return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
}

// process the next data chunk
int ws_upgrade_ota_handle_configuration (UINT8 *data, int len)
{
    ws_upgrade_client_configuration = data[0] + (data[1] << 8);
    return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
}

int ws_upgrade_ota_handle_data (UINT8 *data, int len)
{
    // if this is the beginning of the download first bytes should be application info
    if (ws_upgrade_state == WS_UPGRADE_STATE_VERSION_TRANSFER)
    {
    	BT_MEMCPY (&ws_upgrade_info, data, sizeof (ws_upgrade_info));

    	ble_trace3("ws_upgrade_info %04x %02x %02x\n",  ws_upgrade_info.ID, ws_upgrade_info.Version_Major, ws_upgrade_info.Version_Minor);

        if (ws_upgrade_info.ID != WsUpgradeAppInfo.ID)
        {
            return WS_UPGRADE_WRITE_STATUS_BAD_ID;
        }
        if (ws_upgrade_info.Version_Major < WsUpgradeAppInfo.Version_Major)
        {
            return WS_UPGRADE_WRITE_STATUS_BAD_MAJOR;
        }
        ws_upgrade_state = WS_UPGRADE_STATE_DATA_TRANSFER;
        ws_upgrade_total_len -= sizeof (ws_upgrade_info);

        return WS_UPGRADE_WRITE_STATUS_SUCCESS;
    }
    else if (ws_upgrade_state == WS_UPGRADE_STATE_DATA_TRANSFER)
    {
        if ((ws_upgrade_total_offset + ws_upgrade_current_block_offset + len > ws_upgrade_total_len) ||
            (ws_upgrade_current_block_offset + len > WS_UPGRADE_CHUNK_SIZE_TO_COMMIT))
        {
            ble_trace4("Too much data offsets:%d + %d total:%d len:%d\n",
                       ws_upgrade_total_offset, ws_upgrade_current_block_offset, ws_upgrade_total_len, len);
            return WS_UPGRADE_WRITE_STATUS_TOO_MUCH_DATA;
        }

        if (ws_upgrade_total_offset + len >= ws_upgrade_patch_len)
        {
            BT_MEMCPY (&ws_upgrade_read_buffer[ws_upgrade_current_block_offset], data, ws_upgrade_patch_len - ws_upgrade_total_offset);

            ws_upgrade_current_block_offset += (ws_upgrade_patch_len - ws_upgrade_total_offset);
        	len  -= (ws_upgrade_patch_len - ws_upgrade_total_offset);
        	data += (ws_upgrade_patch_len - ws_upgrade_total_offset);

            ws_upgrade_state = WS_UPGRADE_STATE_SIGNATURE_TRANSFER;

        }
        else
        {
			BT_MEMCPY (&ws_upgrade_read_buffer[ws_upgrade_current_block_offset], data, len);
			ws_upgrade_current_block_offset += len;
        }
        if ((ws_upgrade_current_block_offset == WS_UPGRADE_CHUNK_SIZE_TO_COMMIT) ||
            (ws_upgrade_total_offset + ws_upgrade_current_block_offset == ws_upgrade_patch_len))
        {
            ws_upgrade_store_to_nv(ws_upgrade_total_offset, ws_upgrade_read_buffer, ws_upgrade_current_block_offset);

            ws_upgrade_total_offset        += ws_upgrade_current_block_offset;
            ws_upgrade_current_block_offset = 0;
        }
    }
    if ((ws_upgrade_state == WS_UPGRADE_STATE_SIGNATURE_TRANSFER) && (len != 0))
    {
        if (ws_upgrade_signature_offset + len > WS_UPGRADE_RSA_SIGNATURE_LEN)
        {
            ble_trace2("Too much data sig offset:%d len:%d\n", ws_upgrade_signature_offset, len);
            return WS_UPGRADE_WRITE_STATUS_TOO_MUCH_DATA;
        }
		BT_MEMCPY (&ws_upgrade_sig[ws_upgrade_signature_offset], data, len);
		ws_upgrade_signature_offset += len;
    }
    return (WS_UPGRADE_WRITE_STATUS_SUCCESS);
}

