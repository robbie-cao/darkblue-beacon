#pragma once
#include "stdafx.h"
#include "LongChar.h"
#include "BtInterface.h"

BOOL CBtInterface::GetDescriptorClientConfigValue(GUID uuidChar, USHORT inx, USHORT *Value)
{
    BTW_GATT_VALUE gattValue;
    gattValue.value[0] = 0;
    gattValue.value[1] = 0;
    gattValue.len      = 2;

    if (GetDescriptorValue(uuidChar, inx, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &gattValue))
    {
        *Value = gattValue.value[0] + (gattValue.value[1] << 8);
        return TRUE;
    }
    else
        return FALSE;
}

BOOL CBtInterface::SetDescriptorClientConfigValue(GUID uuidChar, USHORT inx, USHORT Value)
{
    BTW_GATT_VALUE gattValue = {0};
    gattValue.len      = 2;
    gattValue.value[0] = (BYTE)(Value & 0xff);
    gattValue.value[1] = (BYTE)((Value >> 8) &0xff);

    return SetDescriptorValue(uuidChar, inx, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &gattValue);
}


