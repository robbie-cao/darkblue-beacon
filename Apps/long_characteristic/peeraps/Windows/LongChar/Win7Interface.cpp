
// LongCharDlg.cpp : implementation file
//

#include "stdafx.h"
#include <setupapi.h>
#include "LongChar.h"
#include "Win7Interface.h"
#include "LongCharDlg.h"

void GattConnectCallback(LPVOID pRefData, BLUETOOTH_ADDRESS *pDevice, BOOL bConnected)
{
    ods ("Connected:%d\n", bConnected);
    PostMessage(((CLongCharDlg *)pRefData)->m_hWnd, WM_CONNECTED, (WPARAM)bConnected, NULL); 
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
    if ((Op != BTW_GATT_OPTYPE_NOTIFICATION) && (Op != BTW_GATT_OPTYPE_INDICATION))
        return ERROR_SUCCESS;

    if (*pGuidService != guidSvcLongChar)
    {
        ods ("Notification from %04x dropped\n", pGuidService->Data1);
        return ERROR_SUCCESS;
    }

    BTW_GATT_VALUE *p = (BTW_GATT_VALUE *)malloc (sizeof (BTW_GATT_VALUE));
    if (!p)
        return ERROR_SUCCESS;

    *p = *pValue;
    PostMessage(((CLongCharDlg *)pRefData)->m_hWnd, WM_USER + (pGuidChar->Data1 & 0xff), (WPARAM)dwCharInstance, (LPARAM)p);
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

BOOL CBtWin7Interface::Init(GUID uuid)
{
    m_guid = uuid;

    if (m_hLib != NULL)
    {
        FP_BtwGattRegister p = (FP_BtwGattRegister)GetProcAddress(m_hLib, "BtwGattRegister");
        if (p)
        {
            DWORD rc = (*p)(&m_bth, &GattConnectCallback, NULL, &GattRequestCallback, m_NotificationContext, &m_hReg);
            return (rc == ERROR_SUCCESS);
        }
    }
    return FALSE;
}

BOOL CBtWin7Interface::ReadCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue)
{
    ods("+%S\n", __FUNCTIONW__);

    GUID guid = uuidChar;

    FP_BtwGattReadCharacteristic pReadChar = (FP_BtwGattReadCharacteristic)GetProcAddress(m_hLib, "BtwGattReadCharacteristic");
    DWORD dwResult = 0;
    if (pReadChar)
    {
        dwResult = (*pReadChar)(m_hReg, &m_bth, &m_guid, 0, &guid, inx, 0, pValue, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}


BOOL CBtWin7Interface::WriteCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue)
{
    ods("+%S\n", __FUNCTIONW__);

    GUID guid = uuidChar;

    DWORD dwResult = 0;
    FP_BtwGattWriteCharacteristic pWriteChar = (FP_BtwGattWriteCharacteristic)GetProcAddress(m_hLib, "BtwGattWriteCharacteristic");
    if (pWriteChar)
    {
        dwResult = (*pWriteChar)(m_hReg, &m_bth, &m_guid, 0, &guid, inx, 0, pValue, TRUE, this);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}

BOOL CBtWin7Interface::GetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue)
{
    ods("+%S\n", __FUNCTIONW__);
    DWORD dwResult;
    BTW_GATT_VALUE value = {0};
    GUID guidChar   = uuidChar;
    GUID guidDescr  = guidBT;
    guidDescr.Data1 = uuidDescr;

    FP_BtwGattReadDescriptor pReadDescr = (FP_BtwGattReadDescriptor)GetProcAddress(m_hLib, "BtwGattReadDescriptor");
    if (pReadDescr)
    {
        value.value[0] = 0;
        value.value[1] = 0;
        value.len = 2;

        dwResult = (*pReadDescr)(m_hReg, &m_bth, &m_guid, 0, &guidChar, inx, &guidDescr, 0, pValue, TRUE, NULL);
        if (dwResult == ERROR_SUCCESS)
        {
            ods("-%S %d\n", __FUNCTIONW__, dwResult);
            return TRUE;
        }
    }
    ods("-%S %d\n", __FUNCTIONW__, dwResult);
    return FALSE;
}

BOOL CBtWin7Interface::SetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue)
{
    GUID guidChar   = uuidChar;
    GUID guidDescr  = guidBT;
    guidDescr.Data1 = uuidDescr;

    ods("+%S\n", __FUNCTIONW__);

    FP_BtwGattWriteDescriptor pWriteDescr = (FP_BtwGattWriteDescriptor)GetProcAddress(m_hLib, "BtwGattWriteDescriptor");
    if (pWriteDescr)
    {
        DWORD dwResult = (*pWriteDescr)(m_hReg, &m_bth, &m_guid, 0, &guidChar, inx, &guidDescr, 0, pValue, TRUE, NULL);
        ods("-%S %d\n", __FUNCTIONW__, dwResult);
        return (dwResult == ERROR_SUCCESS);
    }
    ods("-%S\n", __FUNCTIONW__);
    return FALSE;
}

