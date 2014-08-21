
// Win7Interface.cpp : implementation file
//

#include "stdafx.h"
#include <setupapi.h>
#include "SpiComm.h"
#include "Win7Interface.h"
#include "SpiCommDlg.h"

void GattConnectCallback(LPVOID pRefData, BLUETOOTH_ADDRESS *pAddress, BOOL bConnected)
{
    ods ("Connected:%d\n", bConnected);

    CBtWin7Interface *pWin7Interface = (CBtWin7Interface *)pRefData;
    CSpiCommDlg *pDlg = (CSpiCommDlg *)pWin7Interface->m_NotificationContext;

    if (pWin7Interface->m_bth.ullLong == pAddress->ullLong)
        PostMessage(pDlg->m_hWnd, WM_CONNECTED, (WPARAM)bConnected, (LPARAM)pWin7Interface); 
}

DWORD GattRequestCallback(
    LPVOID               pRefData,
    BLUETOOTH_ADDRESS   *pAddress, 
    GUID                *pGuidService, 
    DWORD               dwServiceInstance, 
    GUID                *pGuidChar, 
    DWORD               dwCharInstance,
    GUID                *pGuidDescr, 
    DWORD               security, 
    BTW_GATT_OPTYPE     Op, 
    BTW_GATT_VALUE      *pValue
)
{
    ods ("Op:%d\n", Op);

    CBtWin7Interface *pWin7Interface = (CBtWin7Interface *)pRefData;
    CSpiCommDlg *pDlg = (CSpiCommDlg *)pWin7Interface->m_NotificationContext;
    
    if (pWin7Interface->m_bth.ullLong == pAddress->ullLong)
    {
        if ((*pGuidService == guidSvcSpiComm) && (*pGuidChar == guidCharSpiCommData))
        {
            if ((Op == BTW_GATT_OPTYPE_NOTIFICATION) || (Op == BTW_GATT_OPTYPE_INDICATION))
            {
                BTW_GATT_VALUE *p = (BTW_GATT_VALUE *)malloc (sizeof (BTW_GATT_VALUE));
                if (p)
                {
                    *p = *pValue;
                    PostMessage(pDlg->m_hWnd, WM_FRAME_RECEIVED, (WPARAM)p, (LPARAM)pWin7Interface);
                }
            }
        }
        else if ((*pGuidService == guidSvcSpiComm) && (*pGuidChar == guidCharSpiCommControlPoint))
        {
            if ((Op == BTW_GATT_OPTYPE_NOTIFICATION) || (Op == BTW_GATT_OPTYPE_INDICATION))
            {
                PostMessage(pDlg->m_hWnd, WM_FRAME_ACKED, (WPARAM)1, (LPARAM)pWin7Interface);
            }
        }
    }
    return ERROR_SUCCESS;
}

CBtWin7Interface::CBtWin7Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) 
    : CBtInterface(bth, hLib, NotificationContext) 
    , m_hReg(INVALID_HANDLE_VALUE)
{
}

CBtWin7Interface::~CBtWin7Interface()
{
    if (m_hReg != INVALID_HANDLE_VALUE)
    {
        FP_BtwGattDeregister pDereg = (FP_BtwGattDeregister)GetProcAddress(m_hLib, "BtwGattDeregister");
        if (pDereg)
            (*pDereg)(m_hReg);
    }
}

BOOL CBtWin7Interface::Init()
{
    if (m_hLib != NULL)
    {
        FP_BtwGattRegister p = (FP_BtwGattRegister)GetProcAddress(m_hLib, "BtwGattRegister");
        if (p)
        {
            DWORD rc = (*p)(&m_bth, &GattConnectCallback, NULL, &GattRequestCallback, this, &m_hReg);
            return (rc == ERROR_SUCCESS);
        }
    }
    return FALSE;
}

BOOL CBtWin7Interface::SendSpiCommCommand(BYTE Command)
{
    ods("+%S\n", __FUNCTIONW__);
    BTW_GATT_VALUE value = {1, Command};
    DWORD dwResult = 0;
    FP_BtwGattWriteCharacteristic pWriteChar = (FP_BtwGattWriteCharacteristic)GetProcAddress(m_hLib, "BtwGattWriteCharacteristic");
    if (pWriteChar)
    {
        dwResult = (*pWriteChar)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommControlPoint, 0, 0, &value, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}

BOOL CBtWin7Interface::SendSpiCommCommand(BYTE Command, USHORT sParam)
{
    ods("+%S\n", __FUNCTIONW__);
    BTW_GATT_VALUE value = {3, Command, sParam & 0xff, (sParam >> 8) & 0xff};
    DWORD dwResult = 0;
    FP_BtwGattWriteCharacteristic pWriteChar = (FP_BtwGattWriteCharacteristic)GetProcAddress(m_hLib, "BtwGattWriteCharacteristic");
    if (pWriteChar)
    {
        dwResult = (*pWriteChar)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommControlPoint, 0, 0, &value, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}

BOOL CBtWin7Interface::SendSpiCommCommand(BYTE Command, ULONG lParam)
{
    ods("+%S\n", __FUNCTIONW__);
    BTW_GATT_VALUE value = {5, Command, lParam & 0xff, (lParam >> 8) & 0xff, (lParam >> 16) & 0xff, (lParam >> 24) & 0xff};
    DWORD dwResult = 0;
    FP_BtwGattWriteCharacteristic pWriteChar = (FP_BtwGattWriteCharacteristic)GetProcAddress(m_hLib, "BtwGattWriteCharacteristic");
    if (pWriteChar)
    {
        dwResult = (*pWriteChar)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommControlPoint, 0, 0, &value, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}

BOOL CBtWin7Interface::SendSpiCommData(BYTE *Data, DWORD len)
{
    ods("+%S\n", __FUNCTIONW__);
    BTW_GATT_VALUE value;

    if (len > sizeof (value.value))
    {
        ods("-%S data too long\n", __FUNCTIONW__);
        return (FALSE);
    }
    value.len = (USHORT)len;
    memcpy (value.value, Data, len);

    DWORD dwResult = 0;
    FP_BtwGattWriteCharacteristic pWriteChar = (FP_BtwGattWriteCharacteristic)GetProcAddress(m_hLib, "BtwGattWriteCharacteristic");
    if (pWriteChar)
    {
        dwResult = (*pWriteChar)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommData, 0, 0, &value, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}


BOOL CBtWin7Interface::SetDescriptors()
{
    ods("+%S\n", __FUNCTIONW__);
    UINT16 Value = 1;
    BTW_GATT_VALUE value = {0};
    value.len = 2;
    value.value[0] = (BYTE)(Value & 0xff);
    value.value[1] = (BYTE)((Value >> 8) &0xff);

    FP_BtwGattWriteDescriptor pWriteDescr = (FP_BtwGattWriteDescriptor)GetProcAddress(m_hLib, "BtwGattWriteDescriptor");
    if (pWriteDescr)
    {
        value.len = 2;
        value.value[0] = (BYTE)(Value & 0xff);
        value.value[1] = (BYTE)((Value >> 8) &0xff);
        DWORD dwResult = (*pWriteDescr)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommControlPoint, 0, &guidClntConfigDesc, 0, &value, TRUE, NULL);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        if (dwResult == ERROR_SUCCESS)
        {
            value.len = 2;
            value.value[0] = (BYTE)(Value & 0xff);
            value.value[1] = (BYTE)((Value >> 8) &0xff);
            dwResult = (*pWriteDescr)(m_hReg, &m_bth, &guidSvcSpiComm, 0, &guidCharSpiCommData, 0, &guidClntConfigDesc, 0, &value, TRUE, NULL);
            ods("-%S %d\n", __FUNCTIONW__, dwResult);
        }
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S\n", __FUNCTIONW__);
    return FALSE;
}

