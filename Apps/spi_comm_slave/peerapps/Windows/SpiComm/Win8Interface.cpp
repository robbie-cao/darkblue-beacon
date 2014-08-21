
// Win8Interface.cpp : implementation file
//

#include "stdafx.h"
#include <setupapi.h>
#include "SpiComm.h"
#include "SpiCommDlg.h"
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
                GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

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
                GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

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

VOID WINAPI NotificationCallback(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
    ods ("EventType:%d\n", EventType);
    CBtWin8Interface *pWin8Interface = (CBtWin8Interface *)Context;
    pWin8Interface->HandleNotification(EventType, EventOutParameter);
}


CBtWin8Interface::CBtWin8Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext) 
    : CBtInterface(bth, hLib, NotificationContext) 
    , m_hDevice(INVALID_HANDLE_VALUE)
    , m_pEventHandle(INVALID_HANDLE_VALUE)
{
}

void CBtWin8Interface::HandleNotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter)
{
    CSpiCommDlg *pDlg = (CSpiCommDlg *)m_NotificationContext;
    BLUETOOTH_GATT_VALUE_CHANGED_EVENT *pEvent = (BLUETOOTH_GATT_VALUE_CHANGED_EVENT *)EventOutParameter;
    if (pEvent->ChangedAttributeHandle == m_charSpiCommData.AttributeHandle)
    {
        BTW_GATT_VALUE *p = (BTW_GATT_VALUE *)malloc (sizeof (BTW_GATT_VALUE));
        if (p)
        {
            p->len = (USHORT)pEvent->CharacteristicValue->DataSize;
            memcpy (p->value, pEvent->CharacteristicValue->Data, pEvent->CharacteristicValue->DataSize);
            pDlg->PostMessage(WM_FRAME_RECEIVED, (WPARAM)p, (LPARAM)this);
        }
    }
    else if (pEvent->ChangedAttributeHandle == m_charSpiCommControlPoint.AttributeHandle)
    {
        pDlg->PostMessage(WM_FRAME_ACKED, (WPARAM)1, (LPARAM)this);
    }
}


BOOL CBtWin8Interface::Init()
{
    if ((m_hDevice = OpenBleDevice(&m_bth)) == INVALID_HANDLE_VALUE)
        return FALSE;

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

    // search for SPI COMM services.
    _BTH_LE_UUID uuidSpiComm;
    uuidSpiComm.IsShortUuid = FALSE;
    uuidSpiComm.Value.LongUuid = GUID_SPI_COMM_SERVICE;
    for (int i = 0; i < num; i++)
    {
        if (IsBthLEUuidMatch(pServices[i].ServiceUuid, uuidSpiComm))
            m_service = pServices[i];
    }

    PBTH_LE_GATT_CHARACTERISTIC pChars;

    FP_BluetoothGATTGetCharacteristics pBluetoothGATTGetCharacteristics = (FP_BluetoothGATTGetCharacteristics)GetProcAddress(m_hLib, "BluetoothGATTGetCharacteristics");

    // Go through all characteristics of the SPI COMM service to find Control Point and Data
    num = 0;
	pBluetoothGATTGetCharacteristics(m_hDevice, &m_service, 0, NULL, &num, BLUETOOTH_GATT_FLAG_NONE);
	if (num != 0)
	{
	    if ((pChars = new BTH_LE_GATT_CHARACTERISTIC[num]) == NULL)
		    return FALSE;

        if ((pBluetoothGATTGetCharacteristics(m_hDevice, &m_service, num, pChars, &num, BLUETOOTH_GATT_FLAG_NONE)) != S_OK)
	    {
            ods("LoadCharacteristics hr:0x%x\n", hr);
		    delete [] pChars;
		    return FALSE;
	    }
    }
    // search for SPI Comm service characteristics.
    m_charSpiCommControlPoint.AttributeHandle = 0;
    m_charSpiCommData.AttributeHandle = 0;

    _BTH_LE_UUID charControlPoint, charData;
    charControlPoint.IsShortUuid = FALSE;
    charControlPoint.Value.LongUuid = GUID_SPI_COMM_CHARACTERISTIC_CONTROL_POINT;
    charData.IsShortUuid = FALSE;
    charData.Value.LongUuid = GUID_SPI_COMM_CHARACTERISTIC_DATA;
    for (int i = 0; i < num; i++)
    {
        BTH_LE_GATT_DESCRIPTOR *pClientConfig;

        if (IsBthLEUuidMatch(pChars[i].CharacteristicUuid, charControlPoint))
        {
            m_charSpiCommControlPoint = pChars[i];
            pClientConfig = &m_descrSpiDataClientConfig;
        }
        else if (IsBthLEUuidMatch(pChars[i].CharacteristicUuid, charData))
        {
            m_charSpiCommData = pChars[i];
            pClientConfig = &m_descrSpiControlClientConfig;
        }
        else 
            continue;

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
                ods("LoadCharacteristics hr:0x%x\n", hr);
		        delete [] pDescrs;
		        break;
	        }
        }

        // search for client configuration descriptor.
        _BTH_LE_UUID uuidClientConfig;
        uuidClientConfig.IsShortUuid = TRUE;
        uuidClientConfig.Value.ShortUuid = BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG;

        for (int j = 0; j < numDescr; j++)
        {
            if (IsBthLEUuidMatch(pDescrs[j].DescriptorUuid, uuidClientConfig))
            {
                *pClientConfig = pDescrs[j];
                break;
            }
        }
        delete []pDescrs;
    }
    free (pServices);
    return TRUE;
}

void CBtWin8Interface::RegisterNotification()
{
    // register to receive notification
    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return;

	LPBYTE buf = (LPBYTE) malloc (sizeof (BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION) + sizeof (BTH_LE_GATT_CHARACTERISTIC) + 16);

	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION p = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)buf;
	p->NumCharacteristics = 2;
	p->Characteristics[0] = m_charSpiCommControlPoint;
	p->Characteristics[1] = m_charSpiCommData;

    if (m_pEventHandle == INVALID_HANDLE_VALUE)
    {
        FP_BluetoothGATTRegisterEvent pReg = (FP_BluetoothGATTRegisterEvent)GetProcAddress(m_hLib, "BluetoothGATTRegisterEvent");
        if (pReg)
        {
            HRESULT hr = pReg(hService, CharacteristicValueChangedEvent, p, &NotificationCallback, this, &m_pEventHandle, BLUETOOTH_GATT_FLAG_NONE);
            ods ("BluetoothGATTRegisterEvent hr:0x%x\n", hr);
        }
    }
	free(buf);
    CloseHandle(hService);
}

BOOL CBtWin8Interface::SetDescriptors()
{
    HRESULT hr = E_FAIL;
    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    FP_BluetoothGATTSetDescriptorValue pSetDescr = (FP_BluetoothGATTSetDescriptorValue)GetProcAddress(m_hLib, "BluetoothGATTSetDescriptorValue");
    if (pSetDescr)
    {
        BTH_LE_GATT_DESCRIPTOR_VALUE DescriptorValue;

        RtlZeroMemory(&DescriptorValue, sizeof(DescriptorValue));

        DescriptorValue.DescriptorType = ClientCharacteristicConfiguration;
        DescriptorValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = 1;
        // DescriptorValue.ClientCharacteristicConfiguration.IsSubscribeToIndication = 1;

        hr = pSetDescr(hService, &m_descrSpiDataClientConfig, &DescriptorValue, BLUETOOTH_GATT_FLAG_NONE);
        ods ("BluetoothGATTSetDescriptorValue hr:0x%x\n", hr);
        if (hr == S_OK)
        {
            hr = pSetDescr(hService, &m_descrSpiControlClientConfig, &DescriptorValue, BLUETOOTH_GATT_FLAG_NONE);
            ods ("BluetoothGATTSetDescriptorValue hr:0x%x\n", hr);
        }
    }
    CloseHandle(hService);
    return hr == S_OK;
}

BOOL CBtWin8Interface::SendSpiCommCommand(BYTE Command)
{
    FP_BluetoothGATTSetCharacteristicValue pWriteChar = (FP_BluetoothGATTSetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTSetCharacteristicValue");
    if (pWriteChar == NULL)
        return FALSE;

    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    BTH_LE_GATT_CHARACTERISTIC_VALUE Value;
    RtlZeroMemory(&Value, sizeof(Value));

    Value.DataSize = 1;
    Value.Data[0] = (BYTE)Command;

    pWriteChar(hService, &m_charSpiCommControlPoint, &Value, NULL, BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);

    CloseHandle (hService);
    return FALSE;
}

BOOL CBtWin8Interface::SendSpiCommCommand(BYTE Command, ULONG lParam)
{
    FP_BluetoothGATTSetCharacteristicValue pWriteChar = (FP_BluetoothGATTSetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTSetCharacteristicValue");
    if (pWriteChar == NULL)
        return FALSE;

    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    BTH_LE_GATT_CHARACTERISTIC_VALUE Value;
    RtlZeroMemory(&Value, sizeof(Value));

    Value.DataSize = 5;
    Value.Data[0] = (BYTE)Command;
    Value.Data[1] = (BYTE)(lParam & 0xFF);
    Value.Data[2] = (BYTE)((lParam << 8) & 0xFF);
    Value.Data[3] = (BYTE)((lParam << 16) & 0xFF);
    Value.Data[4] = (BYTE)((lParam << 24) & 0xFF);

    pWriteChar(hService, &m_charSpiCommControlPoint, &Value, NULL, BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);

    CloseHandle (hService);
    return FALSE;
}

BOOL CBtWin8Interface::SendSpiCommCommand(BYTE Command, USHORT sParam)
{
    FP_BluetoothGATTSetCharacteristicValue pWriteChar = (FP_BluetoothGATTSetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTSetCharacteristicValue");
    if (pWriteChar == NULL)
        return FALSE;

    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

    BTH_LE_GATT_CHARACTERISTIC_VALUE Value;
    RtlZeroMemory(&Value, sizeof(Value));

    Value.DataSize = 3;
    Value.Data[0] = (BYTE)Command;
    Value.Data[1] = (BYTE)(sParam & 0xFF);
    Value.Data[2] = (BYTE)((sParam << 8) & 0xFF);

    pWriteChar(hService, &m_charSpiCommControlPoint, &Value, NULL, BLUETOOTH_GATT_FLAG_CONNECTION_ENCRYPTED);

    CloseHandle (hService);
    return FALSE;
}

BOOL CBtWin8Interface::SendSpiCommData(BYTE *Data, DWORD len)
{
    if (len > 512)
    {
        ods("-%S data too long\n", __FUNCTIONW__);
        return (FALSE);
    }
    FP_BluetoothGATTSetCharacteristicValue pWriteChar = (FP_BluetoothGATTSetCharacteristicValue)GetProcAddress(m_hLib, "BluetoothGATTSetCharacteristicValue");
    if (pWriteChar == NULL)
        return FALSE;

    GUID guid = GUID_SPI_COMM_SERVICE;
    HANDLE hService = OpenBleService(&m_bth, &guid);
    if (hService == INVALID_HANDLE_VALUE)
        return FALSE;

	HRESULT hr;

    PBTH_LE_GATT_CHARACTERISTIC_VALUE pValue = (PBTH_LE_GATT_CHARACTERISTIC_VALUE) malloc (sizeof (BTH_LE_GATT_CHARACTERISTIC_VALUE) + 512);
    if (pValue)
    {
        RtlZeroMemory(pValue, sizeof (BTH_LE_GATT_CHARACTERISTIC_VALUE) + 512);

        pValue->DataSize = len;
        memcpy (pValue->Data, Data, len);

		hr = pWriteChar(hService, &m_charSpiCommData, pValue, NULL, BLUETOOTH_GATT_FLAG_WRITE_WITHOUT_RESPONSE);

        free (pValue);
    }
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

