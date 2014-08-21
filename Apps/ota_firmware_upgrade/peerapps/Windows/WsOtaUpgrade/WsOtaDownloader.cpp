
// WsOtaUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WsOtaDownloader.h"
#include "..\..\..\ws_upgrade_ota.h"

#define WM_PROGRESS     (WM_USER + 103)

WSDownloader::WSDownloader(CBtInterface *pBtInterface, LPBYTE pPatch, DWORD dwPatchSize, HWND hWnd)
{
    m_state       = WS_UPGRADE_STATE_IDLE;
    m_offset      = 0;
    m_btInterface = pBtInterface;
    m_PatchSize   = dwPatchSize;
    m_Patch       = pPatch;
    m_hWnd        = hWnd;
}

WSDownloader::~WSDownloader()
{
}

DWORD WINAPI DownloadWorker (void *Context)
{
    WSDownloader *p = (WSDownloader *)Context;

    p->TransferData();
    return 0;
}

void WSDownloader::TransferData()
{
    m_offset = 0;
    m_crc32 = INITIAL_REMAINDER;
    while ((m_PatchSize > m_offset) && (m_state != WS_UPGRADE_STATE_ABORTED))
    {
        DWORD dwBytes = m_PatchSize - m_offset;
        if (dwBytes > 20)
            dwBytes = 20;
        m_crc32 = crcSlow(m_crc32, &m_Patch[m_offset], dwBytes);
        m_btInterface->SendWsUpgradeData(&m_Patch[m_offset], dwBytes);
        m_offset += dwBytes;
        PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_DATA_TRANSFER, (LPARAM)m_offset);
    }
    m_crc32 = crcComplete(m_crc32);
    if (m_state == WS_UPGRADE_STATE_ABORTED)
    {
        m_btInterface->SendWsUpgradeCommand(WS_UPGRADE_COMMAND_ABORT);
        PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_ABORTED, (LPARAM)1);
    }
    return;
}

void WSDownloader::ProcessEvent(BYTE Event)
{
    switch (m_state)
    {
    case WS_UPGRADE_STATE_IDLE:
        m_bConnected = TRUE;
        if (Event == WS_UPGRADE_CONNECTED)
        {
            // register for notifications and indications with the status
            USHORT ClientConfDescrControlPoint = 3;
            m_btInterface->SetDescriptorValue(ClientConfDescrControlPoint);
            m_btInterface->SendWsUpgradeCommand(WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD);
            m_state = WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD;
            PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD, (LPARAM)m_PatchSize);
        }
        break;

    case WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD:
        if (Event == WS_UPGRADE_RESPONSE_OK)
        {
            m_offset = 0;
			m_state = WS_UPGRADE_STATE_DATA_TRANSFER;
            m_btInterface->SendWsUpgradeCommand(WS_UPGRADE_COMMAND_DOWNLOAD, (USHORT)m_PatchSize);
        }
        break;

    case WS_UPGRADE_STATE_DATA_TRANSFER:
        if (Event == WS_UPGRADE_RESPONSE_OK)
        {
            // Create thread reading unsolicited events
            CreateThread( NULL, 0, DownloadWorker, this, 0, NULL);
        }
        else if (Event == WS_UPGRADE_START_VERIFICATION)
        {
			m_state = WS_UPGRADE_STATE_VERIFICATION;
            m_btInterface->SendWsUpgradeCommand(WS_UPGRADE_COMMAND_VERIFY, m_crc32);
        }
        else if (Event == WS_UPGRADE_ABORT)
        {
			m_state = WS_UPGRADE_STATE_ABORTED;
        }
        break;

    case WS_UPGRADE_STATE_VERIFICATION:
        if (Event == WS_UPGRADE_RESPONSE_OK)
        {
            m_state = WS_UPGRADE_STATE_VERIFIED;
            PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_VERIFIED, (LPARAM)m_PatchSize);
        }
        else if (Event == WS_UPGRADE_RESPONSE_FAILED)
        {
			m_state = WS_UPGRADE_STATE_ABORTED;
            PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_ABORTED, (LPARAM)m_PatchSize);
        }
    }
}


/*********************************************************************
 *
 * Function:    reflect()
 * 
 * Description: Reorder the bits of a binary sequence, by reflecting
 *				them about the middle position.
 *
 * Notes:		No checking is done that nBits <= 32.
 *
 * Returns:		The reflection of the original data.
 *
 *********************************************************************/
static unsigned long reflect(unsigned long data, unsigned char nBits)
{
	unsigned long  reflection = 0x00000000;
	unsigned char  bit;

    // Reflect the data about the center bit.
	for (bit = 0; bit < nBits; ++bit)
	{
        // If the LSB bit is set, set the reflection of it.
		if (data & 0x01)
		{
			reflection |= (1 << ((nBits - 1) - bit));
		}
		data = (data >> 1);
	}
	return (reflection);

}	/* reflect() */



unsigned long crcSlow(unsigned long  crc32, unsigned char const message[], int nBytes)
{
	int            byte;
	unsigned char  bit;

    // Perform modulo-2 division, a byte at a time.
    for (byte = 0; byte < nBytes; ++byte)
    {
        // Bring the next byte into the crc32.
        crc32 ^= (REFLECT_DATA(message[byte]) << (WIDTH - 8));

        // Perform modulo-2 division, a bit at a time.
        for (bit = 8; bit > 0; --bit)
        {
            // Try to divide the current data bit.
            if (crc32 & TOPBIT)
            {
                crc32 = (crc32 << 1) ^ POLYNOMIAL;
            }
            else
            {
                crc32 = (crc32 << 1);
            }
        }
    }
    return crc32;

}   /* crcSlow() */

unsigned long crcComplete(unsigned long crc32)
{
    // The final crc32 is the CRC result.
    return (REFLECT_REMAINDER(crc32) ^ FINAL_XOR_VALUE);
}

