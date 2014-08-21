
// WsSecOtaUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\..\ws_sec_upgrade_ota.h"
#include "WsOtaDownloader.h"

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
    // to simplify processing on the device side we will send prefix, patch and 
    // signature in a separate packets
    m_btInterface->SendWsUpgradeData(&m_Patch[0], WS_UPGRADE_PREFIX_LEN);

    m_offset = WS_UPGRADE_PREFIX_LEN;

    while ((m_PatchSize - WS_UPGRADE_RSA_SIGNATURE_LEN > m_offset) && (m_state != WS_UPGRADE_STATE_ABORTED))
    {
        DWORD dwBytes = m_PatchSize - WS_UPGRADE_RSA_SIGNATURE_LEN - m_offset;
        if (dwBytes > 20)
            dwBytes = 20;
        m_btInterface->SendWsUpgradeData(&m_Patch[m_offset], dwBytes);
        m_offset += dwBytes;
        PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_DATA_TRANSFER, (LPARAM)m_offset);
    }
    while ((m_PatchSize > m_offset) && (m_state != WS_UPGRADE_STATE_ABORTED))
    {
        DWORD dwBytes = m_PatchSize - m_offset;
        if (dwBytes > 20)
            dwBytes = 20;
        m_btInterface->SendWsUpgradeData(&m_Patch[m_offset], dwBytes);
        m_offset += dwBytes;
        PostMessage(m_hWnd, WM_PROGRESS, (WPARAM)WS_UPGRADE_STATE_DATA_TRANSFER, (LPARAM)m_offset);
    }
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
            m_btInterface->SendWsUpgradeCommand(WS_UPGRADE_COMMAND_VERIFY);
        }
        else if ((Event == WS_UPGRADE_ABORT) || (Event == WS_UPGRADE_RESPONSE_FAILED))
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

