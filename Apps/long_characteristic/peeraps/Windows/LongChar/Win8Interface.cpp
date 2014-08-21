
// LongCharDlg.cpp : implementation file
//

#include "stdafx.h"
#include <setupapi.h>
#include "LongChar.h"
#include "LongCharDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GUID GUID_BLUETOOTHLE_DEVICE_INTERFACE = {0x781aee18, 0x7733, 0x4ce4, {0xad, 0xd0, 0x91, 0xf4, 0x1c, 0x67, 0xb5, 0x92}};

HANDLE OpenBleDevice(BLUETOOTH_ADDRESS *btha)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
	WCHAR								szBda[13] = {0};
	HANDLE								hDevice = INVALID_HANDLE_VALUE;
	DWORD								err;

    if ((hardwareDeviceInfo = SetupDiGetClassDevs ((LPGUID)&GUID_BLUETOOTHLE_DEVICE_INTERFACE,
			NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE))) == INVALID_HANDLE_VALUE)
    {
        ods("SetupDiGetClassDevs failed: %x\n", GetLastError());
        return hDevice;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

	// Enumerate devices of LE_DEVICE interface class
    for (int i = 0; ; i++) 
	{
        if (!SetupDiEnumDeviceInterfaces (hardwareDeviceInfo, 0, 
				(LPGUID)&GUID_BLUETOOTHLE_DEVICE_INTERFACE, i, &deviceInterfaceData)) 
		{
			if ((err = GetLastError()) == ERROR_NO_MORE_ITEMS) 
				ods ("OpenBleDevice device not found\n");
			else
				ods ("OpenBleDevice:ERROR SetupDiEnumDeviceInterfaces failed:%d\n", err);
			break;
		}
        SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfo, &deviceInterfaceData,
				NULL, 0, &requiredLength, NULL);

		if ((err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) 
		{
            ods("OpenBleDevice:ERROR SetupDiGetDeviceInterfaceDetail failed %d\n", err);
            break;
        }

        predictedLength = requiredLength;

        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new CHAR[predictedLength + 2];
        if (deviceInterfaceDetailData == NULL) 
		{
            ods("OpenBleDevice:ERROR Couldn't allocate %d bytes for device interface details.\n", predictedLength);
			break;
        }
		RtlZeroMemory (deviceInterfaceDetailData, predictedLength + 2);
        deviceInterfaceDetailData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfo, &deviceInterfaceData,
			deviceInterfaceDetailData, predictedLength, &requiredLength, NULL)) 
		{
            ods("OpenBleDevice:ERROR SetupDiGetDeviceInterfaceDetail\n");
			delete deviceInterfaceDetailData;
			break;
        }

		_wcsupr_s (deviceInterfaceDetailData->DevicePath, wcslen(deviceInterfaceDetailData->DevicePath) + 1);
		BdaToString (szBda, btha);
		if (wcsstr (deviceInterfaceDetailData->DevicePath, szBda) != NULL)
		{
			ods("Opening interface:%S\n", deviceInterfaceDetailData->DevicePath);

			hDevice = CreateFile ( deviceInterfaceDetailData->DevicePath,
				GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			delete deviceInterfaceDetailData;
			break;
		}
		delete deviceInterfaceDetailData;
	}
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
	return hDevice;
}

HANDLE OpenBleService(BLUETOOTH_ADDRESS *btha, GUID *guid)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
	WCHAR								szBda[13] = {0};
	HANDLE								hService = INVALID_HANDLE_VALUE;
	DWORD								err;

    if ((hardwareDeviceInfo = SetupDiGetClassDevs ((LPGUID)guid,
			NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE))) == INVALID_HANDLE_VALUE)
    {
        ods("OpenBleServiceSetupDiGetClassDevs failed: %x\n", GetLastError());
        return hService;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

	// Enumerate devices of LE_DEVICE interface class
    for (int i = 0; ; i++) 
	{
        if (!SetupDiEnumDeviceInterfaces (hardwareDeviceInfo, 0, guid, i, &deviceInterfaceData)) 
		{
			if ((err = GetLastError()) == ERROR_NO_MORE_ITEMS) 
				ods("OpenBleService device not found\n");
			else
				ods ("OpenBleService:ERROR SetupDiEnumDeviceInterfaces failed:%d\n", err);
			break;
		}
        SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfo, &deviceInterfaceData,
				NULL, 0, &requiredLength, NULL);

		if ((err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) 
		{
            ods("OpenBleService:ERROR SetupDiGetDeviceInterfaceDetail failed %d\n", err);
            break;
        }

        predictedLength = requiredLength;

        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new CHAR[predictedLength + 2];
        if (deviceInterfaceDetailData == NULL) 
		{
            ods("OpenBleService:ERROR Couldn't allocate %d bytes for device interface details.\n", predictedLength);
			break;
        }
		RtlZeroMemory (deviceInterfaceDetailData, predictedLength + 2);
        deviceInterfaceDetailData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfo, &deviceInterfaceData,
			deviceInterfaceDetailData, predictedLength, &requiredLength, NULL)) 
		{
            ods("OpenBleService :ERROR SetupDiGetDeviceInterfaceDetail\n");
			delete deviceInterfaceDetailData;
			break;
        }

		_wcsupr_s (deviceInterfaceDetailData->DevicePath, wcslen(deviceInterfaceDetailData->DevicePath) + 1);
		BdaToString (szBda, btha);
		if (wcsstr (deviceInterfaceDetailData->DevicePath, szBda) != NULL)
		{
			ods("OpenBleService Opening interface:%S\n", deviceInterfaceDetailData->DevicePath);

			hService = CreateFile ( deviceInterfaceDetailData->DevicePath,
				GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

			if(hService == INVALID_HANDLE_VALUE)
				ods("OpenBleService (hService == INVALID_HANDLE_VALUE) GetLastError() = %d\n", GetLastError());

			delete deviceInterfaceDetailData;
			break;
		}
		delete deviceInterfaceDetailData;
	}
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
	return hService;
}

VOID CBtWin8Interface::PostNotification(BTW_GATT_VALUE *pValue, BLUETOOTH_GATT_VALUE_CHANGED_EVENT *pEvent, PVOID EventOutParameter, PVOID Context)
{
    CLongCharDlg *pDlg = (CLongCharDlg *)Context;

    for (int i = 0; i < num_chars; i++)
    {
        if (m_pchar[i].AttributeHandle == pEvent->ChangedAttributeHandle)
        {
            USHORT inx = 0;
            for (int j = 0; j < i; j++)
            {
                if (IsBthLEUuidMatch(m_pchar[i].CharacteristicUuid, m_pchar[j].CharacteristicUuid))
                {
                    inx++;
                }
            }
            pDlg->PostMessage(WM_USER + (m_pchar[i].CharacteristicUuid.Value.ShortUuid & 0xff), (WPARAM)inx, (LPARAM)pValue);
        }
    }
}

VOID WINAPI NotificationCallback(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
    ods ("EventType:%d\n", EventType);

    CLongCharDlg *pDlg = (CLongCharDlg *)Context;
    CBtWin8Interface *pBtInterface = (CBtWin8Interface *)(pDlg->m_btInterface);

    BTW_GATT_VALUE *p = (BTW_GATT_VALUE *)malloc (sizeof (BTW_GATT_VALUE));
    if (!p)
        return;

    BLUETOOTH_GATT_VALUE_CHANGED_EVENT *pEvent = (BLUETOOTH_GATT_VALUE_CHANGED_EVENT *)EventOutParameter;
    p->len = (USHORT)pEvent->CharacteristicValue->DataSize;
    memcpy (p->value, pEvent->CharacteristicValue->Data, pEvent->CharacteristicValue->DataSize);

    pBtInterface->PostNotification(p, pEvent, EventOutParameter, Context);
}


CBtWin8Interface::CBtWin8Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) 
    : CBtInterface(bth, hLib, NotificationContext) 
    , m_hDevice(INVALID_HANDLE_VALUE)
    , m_pEventHandle(INVALID_HANDLE_VALUE)
{
}

BOOL CBtWin8Interface::FindChar(GUID uuid, USHORT inx, PBTH_LE_GATT_CHARACTERISTIC pOutChar)
{
    for (int i = 0; i < num_chars; i++)
    {
        _BTH_LE_UUID leuuid;
        leuuid.IsShortUuid = FALSE;
        leuuid.Value.LongUuid = uuid;
        if (IsBthLEUuidMatch(m_pchar[i].CharacteristicUuid, leuuid))
        {
            if (inx-- == 0)
            {
                *pOutChar = m_pchar[i];
                return TRUE;
            }
        }
    }
    return FALSE;
}

VOID CBtWin8Interface::FindChar(GUID uuid, PBTH_LE_GATT_CHARACTERISTIC pOutChar, PBTH_LE_GATT_DESCRIPTOR pOutDescr, USHORT num, PBTH_LE_GATT_CHARACTERISTIC pChars)
{
    for (int i = 0; i < num; i++)
    {
        _BTH_LE_UUID leuuid;
        leuuid.IsShortUuid = FALSE;
        leuuid.Value.LongUuid = uuid;
        if (IsBthLEUuidMatch(pChars[i].CharacteristicUuid, leuuid))
        {
            *pOutChar = pChars[i];

            PBTH_LE_GATT_DESCRIPTOR pDescrs = NULL;
            USHORT numDescr = 0;

            FP_BluetoothGATTGetDescriptors pBluetoothGATTGetDescriptors = (FP_BluetoothGATTGetDescriptors)GetProcAddress(m_hLib, "BluetoothGATTGetDescriptors");
	        pBluetoothGATTGetDescriptors(m_hDevice, &pChars[i], 0, NULL, &numDescr, BLUETOOTH_GATT_FLAG_NONE);
	        if (numDescr != 0)
	        {
	            if ((pDescrs = new BTH_LE_GATT_DESCRIPTOR[numDescr]) == NULL)
		            break;

                if ((pBluetoothGATTGetDescriptors(m_hDevice, &pChars[i], numDescr, pDescrs, &numDescr, BLUETOOTH_GATT_FLAG_NONE)) != S_OK)
	            {
                    ods("LoadDescriptors\n");
		            delete [] pDescrs;
		            break;
	            }
            }
            // save descriptors
            for (int j = 0; j < numDescr; j++)
                pOutDescr[j] = pDescrs[j];

            delete []pDescrs;
        }
    }
}

BOOL CBtWin8Interface::Init(GUID uuidServ)
{
    if ((m_hDevice = OpenBleDevice(&m_bth)) == INVALID_HANDLE_VALUE)
        return FALSE;

    m_guid = uuidServ;

    num_chars = 0;

	USHORT num = 0;
	HRESULT hr;

    FP_BluetoothGATTGetServices pBluetoothGATTGetServices = (FP_BluetoothGATTGetServices)GetProcAddress(m_hLib, "BluetoothGATTGetServices");
	hr = pBluetoothGATTGetServices(m_hDevice, 0, NULL, &num, BLUETOOTH_GATT_FLAG_NONE);
    if (num == 0)
        return FALSE;

    PBTH_LE_GATT_SERVICE pServices = (PBTH_LE_GATT_SERVICE) malloc(sizeof(BTH_LE_GATT_SERVICE) * num);
    if (pServices == NULL) 
        return FALSE;

    RtlZeroMemory(pServices, sizeof(BTH_LE_GATT_SERVICE) * num);
    
    hr = pBluetoothGATTGetServices(m_hDevice, num, pServices, &num, BLUETOOTH_GATT_FLAG_NONE);
    if (S_OK != hr) 
    {
        ods("BluetoothGATTGetServices - Actual Data %x", hr);
        return FALSE;
    }

    // search for Location and Battery services.
    _BTH_LE_UUID leuuid;
    leuuid.IsShortUuid = FALSE;
    leuuid.Value.LongUuid = uuidServ;

    for (int i = 0; i < num; i++)
    {
        if (IsBthLEUuidMatch(pServices[i].ServiceUuid, leuuid))
            m_service = pServices[i];
    }

    FP_BluetoothGATTGetCharacteristics pBluetoothGATTGetCharacteristics = (FP_BluetoothGATTGetCharacteristics)GetProcAddress(m_hLib, "BluetoothGATTGetCharacteristics");
    FP_BluetoothGATTGetDescriptors pBluetoothGATTGetDescriptors = (FP_BluetoothGATTGetDescriptors)GetProcAddress(m_hLib, "BluetoothGATTGetDescriptors");

    // Go through all characteristics of the Location services to find Config and Notify
    num = 0;
	pBluetoothGATTGetCharacteristics(m_hDevice, &m_service, 0, NULL, &num, BLUETOOTH_GATT_FLAG_NONE);
	if (num != 0)
	{
	    if ((m_pchar = new BTH_LE_GATT_CHARACTERISTIC[num]) == NULL)
		    return FALSE;

        if ((pBluetoothGATTGetCharacteristics(m_hDevice, &m_service, num, m_pchar, &num, BLUETOOTH_GATT_FLAG_NONE)) != S_OK)
	    {
            ods("LoadCharacteristics hr:0x%x\n", hr);
		    delete [] m_pchar;
		    return FALSE;
	    }
    }
    num_chars = num;
    pnum_descr = new USHORT[num_chars];

    for (int i = 0; i < num_chars; i++)
    {
        pBluetoothGATTGetDescriptors(m_hDevice, &m_pchar[i], 0, NULL, &pnum_descr[i], BLUETOOTH_GATT_FLAG_NONE);
	    if (pnum_descr[i] != 0)
	    {
	        if ((m_ppdescr[i] = new BTH_LE_GATT_DESCRIPTOR[pnum_descr[i]]) == NULL)
		        break;

            if ((pBluetoothGATTGetDescriptors(m_hDevice, &m_pchar[i], pnum_descr[i], m_ppdescr[i], &pnum_descr[i], BLUETOOTH_GATT_FLAG_NONE)) != S_OK)
	        {
                ods("LoadDescriptors\n");
		        delete [] m_ppdescr[i];
		        break;
	        }
        }
    }
    free (pServices);
    return TRUE;
}

void CBtWin8Interface::RegisterNotification()
{
    // register to receive notification
    HANDLE hService = OpenBleService(&m_bth, &m_guid);
    if (hService == INVALID_HANDLE_VALUE)
        return;

    PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION p;
    if ((p = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)malloc (sizeof (BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION) + sizeof (BTH_LE_GATT_CHARACTERISTIC))) == NULL)
        return;

    p->NumCharacteristics = 1;
//    p->Characteristics[0] = NULL; //m_charDigitalIn;

    if (m_pEventHandle == INVALID_HANDLE_VALUE)
    {
        FP_BluetoothGATTRegisterEvent pReg = (FP_BluetoothGATTRegisterEvent)GetProcAddress(m_hLib, "BluetoothGATTRegisterEvent");
        if (pReg)
        {
            HRESULT hr = pReg(hService, CharacteristicValueChangedEvent, p, &NotificationCallback, m_NotificationContext, &m_pEventHandle, BLUETOOTH_GATT_FLAG_NONE);
            ods ("BluetoothGATTRegisterEvent hr:0x%x\n", hr);
        }
    }
    free (p);
    CloseHandle(hService);
}

PBTH_LE_GATT_DESCRIPTOR CBtWin8Interface::FindDescr(GUID uuidChar, USHORT inx, USHORT uuidDescr)
{
    for (int i = 0; i < num_chars; i++)
    {
        _BTH_LE_UUID leuuid;
        leuuid.IsShortUuid = FALSE;
        leuuid.Value.LongUuid = uuidChar;
        if (IsBthLEUuidMatch(m_pchar[i].CharacteristicUuid, leuuid))
        {
            if (inx-- == 0)
            {
                for (int j = 0; j < pnum_descr[i]; j++)
                {
                    _BTH_LE_UUID descuuid;
                    descuuid.IsShortUuid = TRUE;
                    descuuid.Value.ShortUuid = uuidDescr;
                   
                    if (IsBthLEUuidMatch(m_ppdescr[i][j].DescriptorUuid, descuuid))
                    {
                        return &m_ppdescr[i][j];
                    }
                }
            }
        }
    }
    return NULL;
}

BOOL CBtWin8Interface::GetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue)
{
    PBTH_LE_GATT_DESCRIPTOR pDescr = FindDescr (uuidChar, inx, uuidDescr);
    if (pDescr == NULL)
    {
        ods ("GetDescriptorValue bad UUID:0x%x\n", uuidChar);
        return FALSE;
    }
    HANDLE hService = OpenBleService(&m_bth, &m_guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    FP_BluetoothGATTGetDescriptorValue pGetDescr = (FP_BluetoothGATTGetDescriptorValue)GetProcAddress(m_hLib, "BluetoothGATTGetDescriptorValue");
    if (pGetDescr == NULL)
    {
        CloseHandle(hService);
        return FALSE;
    }

    BTH_LE_GATT_DESCRIPTOR_VALUE DescriptorValue;
    RtlZeroMemory(&DescriptorValue, sizeof(DescriptorValue));

    USHORT descValueDataSize;
    PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer;
    HRESULT hr = pGetDescr(hService, pDescr, 0, NULL, &descValueDataSize, BLUETOOTH_GATT_FLAG_NONE);
    ods ("BluetoothGATTGetDescriptorValue hr:0x%x\n", hr);

    if (descValueDataSize)
    {
        pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);
        if (pDescValueBuffer == NULL) 
        {
            ods("pDescValueBuffer out of memory\r\n");
            CloseHandle(hService);
            return FALSE;
        } 
        RtlZeroMemory(pDescValueBuffer, descValueDataSize);

        // Force reading from the device.  If failed device is not connected
        hr = pGetDescr(hService, pDescr, (ULONG)descValueDataSize, pDescValueBuffer, NULL, BLUETOOTH_GATT_FLAG_FORCE_READ_FROM_DEVICE);
        if (hr == HRESULT_FROM_WIN32(ERROR_SEM_TIMEOUT))
            m_bConnected = FALSE;
        else
        {
            if (uuidDescr == BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG)
            {
                pValue->len = 2;
                pValue->value[0] = pDescValueBuffer->ClientCharacteristicConfiguration.IsSubscribeToNotification +
                                   (pDescValueBuffer->ClientCharacteristicConfiguration.IsSubscribeToIndication << 1);
                pValue->value[1] = 0;
            }
            else
            {
                ASSERT(FALSE);
            }
        }
        free (pDescValueBuffer);
    }
    CloseHandle(hService);
    return TRUE;
}

BOOL CBtWin8Interface::SetDescriptorValue(GUID uuidChar, USHORT inx, USHORT uuidDescr, BTW_GATT_VALUE *pValue)
{
    PBTH_LE_GATT_DESCRIPTOR pDescr = FindDescr (uuidChar, inx, uuidDescr);
    if (pDescr == NULL)
    {
        ods ("SetDescriptorValue bad UUID:0x%x\n", uuidChar);
        return FALSE;
    }
    HRESULT hr = E_FAIL;
    HANDLE hService = OpenBleService(&m_bth, &m_guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    FP_BluetoothGATTSetDescriptorValue pSetDescr = (FP_BluetoothGATTSetDescriptorValue)GetProcAddress(m_hLib, "BluetoothGATTSetDescriptorValue");
    if (pSetDescr)
    {
        BTH_LE_GATT_DESCRIPTOR_VALUE DescriptorValue;

        RtlZeroMemory(&DescriptorValue, sizeof(DescriptorValue));

        if (uuidDescr == BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG)
        {
            DescriptorValue.DescriptorType = ClientCharacteristicConfiguration;
            DescriptorValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = (pValue->value[0] == 1);
            DescriptorValue.ClientCharacteristicConfiguration.IsSubscribeToIndication   = (pValue->value[0] == 2);
        }
        else
        {
            ASSERT (FALSE);
        }
        HRESULT hr = pSetDescr(hService, pDescr, &DescriptorValue, BLUETOOTH_GATT_FLAG_NONE);
        ods ("BluetoothGATTSetDescriptorValue hr:0x%x\n", hr);
    }
    CloseHandle(hService);
    return hr == S_OK;
}

BOOL CBtWin8Interface::ReadCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue)
{
    BTH_LE_GATT_CHARACTERISTIC bleChar;

    if (!FindChar(uuidChar, inx, &bleChar))
        return FALSE;

    FP_BluetoothGATTGetCharacteristicValue pReadChar = (FP_BluetoothGATTGetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTGetCharacteristicValue");
    if (pReadChar == NULL)
        return FALSE;

    HANDLE hService = OpenBleService(&m_bth, &m_guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    USHORT charValueDataSize;
    HRESULT hr = pReadChar(hService, &bleChar, 0, NULL, &charValueDataSize, BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);

    if (charValueDataSize)
    {
        PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);
        if (pCharValueBuffer)
        {
            hr = pReadChar(hService, &bleChar, (ULONG)charValueDataSize, pCharValueBuffer, &charValueDataSize, BLUETOOTH_GATT_FLAG_FORCE_READ_FROM_DEVICE | BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);
            memcpy (pValue->value, pCharValueBuffer->Data, pCharValueBuffer->DataSize);
            pValue->len = (USHORT)pCharValueBuffer->DataSize;
            free (pCharValueBuffer);
            CloseHandle (hService);
            return (hr == S_OK);
        }
    }
    CloseHandle (hService);
    return FALSE;
}


BOOL CBtWin8Interface::WriteCharacteristic(GUID uuidChar, USHORT inx, BTW_GATT_VALUE *pValue)
{
    BTH_LE_GATT_CHARACTERISTIC bleChar;

    if (!FindChar(uuidChar, inx, &bleChar))
        return FALSE;

    FP_BluetoothGATTSetCharacteristicValue pWriteChar = (FP_BluetoothGATTSetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTSetCharacteristicValue");
    if (pWriteChar == NULL)
        return FALSE;

    HANDLE hService = OpenBleService(&m_bth, &m_guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    BTH_LE_GATT_CHARACTERISTIC_VALUE Value;
    RtlZeroMemory(&Value, sizeof(Value));

    Value.DataSize = 1;
    memcpy (Value.Data, pValue->value, pValue->len);

    pWriteChar(hService, &bleChar, &Value, NULL, BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);

    CloseHandle (hService);
    return FALSE;
}

CBtWin8Interface::~CBtWin8Interface()
{
    if (m_pEventHandle != INVALID_HANDLE_VALUE)
    {
        FP_BluetoothGATTUnregisterEvent pDereg = (FP_BluetoothGATTUnregisterEvent)GetProcAddress(m_hLib, "BluetoothGATTUnregisterEvent");
        if (pDereg)
            pDereg(m_pEventHandle, BLUETOOTH_GATT_FLAG_NONE);
    }
    if (m_hDevice != INVALID_HANDLE_VALUE)
        CloseHandle (m_hDevice);
}

