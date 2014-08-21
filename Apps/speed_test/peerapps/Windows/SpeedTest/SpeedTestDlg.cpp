
// SpeedTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include <setupapi.h>
#include "SpeedTest.h"
#include "SpeedTestDlg.h"
#include "..\..\..\speed_test.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern HMODULE hLib;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// SpeedTestDlg dialog



CSpeedTestDlg::CSpeedTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSpeedTestDlg::IDD, pParent)
    , m_bRandomData(FALSE)
    , m_bBytes(_T(""))
    , m_numBytes(0)
    , m_numFramesUnacked(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_btDeviceInterface = NULL;
    m_Test = TEST_NONE;
    m_hFlowEvent       = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CSpeedTestDlg::~CSpeedTestDlg()
{
	delete m_btDeviceInterface;
}

void CSpeedTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SPEED_TEST, m_cbDevices);
    DDX_Check(pDX, IDC_RANDOM, m_bRandomData);
    DDX_Text(pDX, IDC_SEND_BYTES, m_bBytes);
    DDX_Text(pDX, IDC_NUM_BYTES, m_numBytes);
    DDV_MinMaxInt(pDX, m_numBytes, 0, 70);
    DDX_Text(pDX, IDC_NUM_FRAMES_TO_ACK, m_numFramesUnacked);
	DDV_MinMaxInt(pDX, m_numFramesUnacked, 1, 255);
}

BEGIN_MESSAGE_MAP(CSpeedTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_CONNECTED, OnConnected)
    ON_MESSAGE(WM_FRAME_RECEIVED, OnFrameReceived)
    ON_MESSAGE(WM_FRAME_ACKED, OnFrameAcked)
    ON_BN_CLICKED(IDC_SEND, &CSpeedTestDlg::OnBnClickedSend)
    ON_BN_CLICKED(IDC_RANDOM, &CSpeedTestDlg::OnBnClickedRandom)
    ON_BN_CLICKED(IDC_PERFORMANCE_START_OUT, &CSpeedTestDlg::OnBnClickedPerformanceStartOut)
    ON_BN_CLICKED(IDC_PERFORMANCE_START_IN, &CSpeedTestDlg::OnBnClickedPerformanceStartIn)
END_MESSAGE_MAP()


// CSpeedTestDlg message handlers

BOOL CSpeedTestDlg::OnInitDialog()
{
    BOOL bConnected = TRUE;  // assume that device is connected 

	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_bRandomData = TRUE;
    ((CButton *)GetDlgItem(IDC_RANDOM))->SetCheck(TRUE);
    GetDlgItem(IDC_SEND_BYTES)->EnableWindow(FALSE);
    ((CEdit *)GetDlgItem(IDC_NUM_BYTES))->SetWindowText(L"20");
    GetDlgItem(IDC_SEND_BYTES)->SetWindowText(_T("01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14"));
    ((CEdit *)GetDlgItem(IDC_NUM_FRAMES_TO_ACK))->SetWindowText(L"100");

    GetDeviceList();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSpeedTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSpeedTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSpeedTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSpeedTestDlg::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();
}

BOOL CSpeedTestDlg::GetDeviceList()
{
    HDEVINFO                            hardwareDeviceInfo;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
	WCHAR								szBda[13] = {0};

    m_numDevices = 0;

    if ((hardwareDeviceInfo = SetupDiGetClassDevs (NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT)) != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA DeviceInfoData;

        memset(&DeviceInfoData, 0, sizeof(DeviceInfoData));
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);

        WCHAR szService[80];
        GUID guid;
        if (m_bWin8)
            guid = GUID_SPEED_TEST_SERVICE;
        else
        {
            guid.Data1 = (GUID_SPEED_TEST_SERVICE.Data4[4]     ) + (GUID_SPEED_TEST_SERVICE.Data4[5] << 8) + (GUID_SPEED_TEST_SERVICE.Data4[6] << 16) + (GUID_SPEED_TEST_SERVICE.Data4[7] << 24);
            guid.Data2 = (GUID_SPEED_TEST_SERVICE.Data4[2]     ) + (GUID_SPEED_TEST_SERVICE.Data4[3] << 8);
            guid.Data3 = (GUID_SPEED_TEST_SERVICE.Data4[0]     ) + (GUID_SPEED_TEST_SERVICE.Data4[1] << 8);
            guid.Data4[0] = (GUID_SPEED_TEST_SERVICE.Data3      ) & 0xff;
            guid.Data4[1] = (GUID_SPEED_TEST_SERVICE.Data3 >> 8 ) & 0xff;
            guid.Data4[2] = (GUID_SPEED_TEST_SERVICE.Data2      ) & 0xff;
            guid.Data4[3] = (GUID_SPEED_TEST_SERVICE.Data2 >> 8 ) & 0xff;
            guid.Data4[4] = (GUID_SPEED_TEST_SERVICE.Data1      ) & 0xff;
            guid.Data4[5] = (GUID_SPEED_TEST_SERVICE.Data1 >> 8 ) & 0xff;
            guid.Data4[6] = (GUID_SPEED_TEST_SERVICE.Data1 >> 16) & 0xff;
            guid.Data4[7] = (GUID_SPEED_TEST_SERVICE.Data1 >> 24) & 0xff;
        }
        UuidToString(szService, 80, &guid);
        ods ("%S\n", szService);
        for (DWORD n = 0; SetupDiEnumDeviceInfo(hardwareDeviceInfo, n, &DeviceInfoData); n++)
        {
            DWORD dwBytes = 0;

            SetupDiGetDeviceInstanceId(hardwareDeviceInfo, &DeviceInfoData, NULL, 0, &dwBytes);

            PWSTR szInstanceId = new WCHAR [dwBytes];
            if (szInstanceId)
            {
                if (SetupDiGetDeviceInstanceId(hardwareDeviceInfo, &DeviceInfoData, szInstanceId, dwBytes, &dwBytes))
                {
                    _wcsupr_s (szInstanceId, dwBytes);
                    if (wcsstr(szInstanceId, szService))
                    {
                        OutputDebugStringW(szInstanceId);
                        WCHAR buf[13];
                        wchar_t* pStart;
                        wchar_t* pEnd;
                        if (m_bWin8)
                        {
                            pStart = wcsrchr(szInstanceId, '_');
                            pEnd = wcsrchr(szInstanceId, '\\');
                        }
                        else
                        {
                            pStart = wcsrchr(szInstanceId, '&');
                            pEnd = wcsrchr(szInstanceId, '_');
                        }
                        if (pStart && pEnd)
                        {
                            *pEnd = 0;
                            wcscpy_s(buf, pStart + 1);

                            BLUETOOTH_ADDRESS bth;
			                int bda[6];
                            if (swscanf_s(buf, L"%02x%02x%02x%02x%02x%02x", &bda[0], &bda[1], &bda[2], &bda[3], &bda[4], &bda[5]) == 6)
                            {
                                for (int i = 0; i < 6; i++)
                                    bth.rgBytes[5 - i] = (BYTE)bda[i];
                            }
                            char name[251];
                            GetDeviceName (&bth, name);
			    
                            m_numDevices++;
                            m_cbDevices.AddString(buf);
                        }
                    }
                }
                delete[] szInstanceId;
            }
        }
        SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    }
    if (m_numDevices)
        m_cbDevices.SetCurSel(0);

    GetDlgItem(IDC_SEND)->EnableWindow(m_numDevices != 0);
    GetDlgItem(IDC_PERFORMANCE_START_OUT)->EnableWindow(m_numDevices != 0);
	return TRUE;
}


LRESULT CSpeedTestDlg::OnConnected(WPARAM wParam, LPARAM lParam)
{
    CBtInterface *pInterface = (CBtInterface *)lParam;
    pInterface->m_bConnected = (BOOL)wParam;
    return S_OK;
}

LRESULT CSpeedTestDlg::OnFrameReceived(WPARAM wParam, LPARAM lParam)
{
    BTW_GATT_VALUE *pValue = (BTW_GATT_VALUE *)wParam;
    CBtInterface *pInterface = (CBtInterface *)lParam;
    
    char buf1[GATT_MAX_ATTR_LEN * 3 + 1];
    sprintf_s (buf1, 10, "%d bytes ", pValue->len);

	char buf2[GATT_MAX_ATTR_LEN * 3 + 1];
	for (int i = 0, j = 0; i < pValue->len; i++, j += 3)
		sprintf_s(&buf2[j], (GATT_MAX_ATTR_LEN - j) * 3, "%02x ", pValue->value[i]);

	if ((LPARAM)m_btDeviceInterface == lParam)
	{
		SetDlgItemTextA(m_hWnd, IDC_BYTES, buf1);
		SetDlgItemTextA(m_hWnd, IDC_RECEIVED_BYTES, buf2);
	}

    if (m_Test == TEST_PERFORMANCE_IN)
    {
        if ((++m_NumFramesSent % 100) == 0)
        {
            DWORD speed = ((ULONGLONG)m_NumFramesSent * pValue->len * 8 * 1000) / (GetTickCount() - m_StartTime);
            SetDlgItemInt(IDC_THROUGHPUT, speed, FALSE);
            SetDlgItemInt(IDC_FRAMES_SENT, m_NumFramesSent, FALSE);
        }
    }
    free (pValue);
    return S_OK;
}

LRESULT CSpeedTestDlg::OnFrameAcked(WPARAM wParam, LPARAM lParam)
{
    CBtInterface *pInterface = (CBtInterface *)lParam;
	if ((LPARAM)m_btDeviceInterface == lParam)
	{
        m_OutstandingFrames--;
		SetEvent(m_hFlowEvent);
	}
    return S_OK;
}

void CSpeedTestDlg::InitTest()
{
    UpdateData(TRUE);

    WCHAR buf[13];
	int   bda[6];
	BLUETOOTH_ADDRESS bth = { 0 };
	int sel;
	if ((sel = m_cbDevices.GetCurSel()) >= 0)
	{
		m_cbDevices.GetLBText(m_cbDevices.GetCurSel(), buf);
		if (swscanf_s(buf, L"%02x%02x%02x%02x%02x%02x", &bda[0], &bda[1], &bda[2], &bda[3], &bda[4], &bda[5]) == 6)
		{
			for (int i = 0; i < 6; i++)
				bth.rgBytes[5 - i] = (BYTE)bda[i];
		}
	}
    // if we are running second time interfaces are probably already initialized
    if ((m_btDeviceInterface != NULL) && (m_btDeviceInterface->m_bth.ullLong == bth.ullLong))
    {
        OnConnected(m_btDeviceInterface->m_bConnected, (LPARAM)m_btDeviceInterface);
        return;
    }

    delete m_btDeviceInterface;

    if (m_bWin8)
    {
        m_btDeviceInterface = new CBtWin8Interface(&bth, hLib, this);
    }
    else
    {
        m_btDeviceInterface = new CBtWin7Interface(&bth, hLib, this);
    }
    m_btDeviceInterface->Init();

    // on Win7 we will receive notification when device is connected and will intialize dialog there
    if (!m_bWin8)
        return;

    CBtWin8Interface *pWin8BtInterface = dynamic_cast<CBtWin8Interface *>(m_btDeviceInterface);

    // Assume that we are connected.  Failed attempt to read battery will change that to FALSE.
    pWin8BtInterface->m_bConnected = TRUE;

    // register for notifications with the status
    pWin8BtInterface->m_bConnected = pWin8BtInterface->SetDescriptors(0, 0);

    pWin8BtInterface->RegisterNotification();

	if (pWin8BtInterface->m_bConnected)
		OnConnected(TRUE, (LPARAM)pWin8BtInterface);
}

void CSpeedTestDlg::GetNextFrame(LPBYTE data, LPDWORD len)
{
    char buf[GATT_MAX_ATTR_LEN * 3];
    if (m_bRandomData)
    {
        *len = m_numBytes;

        USHORT *p = (USHORT *)data;
        srand(GetTickCount());
        for (int i = 0; i < (int)((*len + 1) / sizeof (USHORT)); i++)
            *p++ = (USHORT)rand();
        
        for (int i = 0, j = 0; i < (int)*len; i++, j += 3)
            sprintf_s (&buf[j], (GATT_MAX_ATTR_LEN - j) * 3, "%02x ", data[i]);
    
        SetDlgItemTextA(m_hWnd, IDC_SEND_BYTES, buf);
    }
    else
    {
        GetDlgItemTextA(m_hWnd, IDC_SEND_BYTES, buf, sizeof (buf));
        int i;
        for (i = 0; i < GATT_MAX_ATTR_LEN; i++)
        {
            int a;
            if (sscanf_s (&buf[3 * i], "%02x ", &a) != 1)
                break;
            data[i] = (BYTE)a;
        }
        *len = i;
    }
}

DWORD WINAPI TestWorker (void *Context)
{
    CSpeedTestDlg *p = (CSpeedTestDlg *)Context;

    p->ExecuteTest();
    return 0;
}

void CSpeedTestDlg::ExecuteTest()
{
    int i;

    m_Stop = FALSE;

    if ((m_Test == TEST_SEND_ONE) || (m_Test == TEST_PERFORMANCE_OUT) || (m_Test == TEST_PERFORMANCE_IN))
    {
        for (i = 0; i < 5; i++)
        {
            if ((m_btDeviceInterface != NULL) && m_btDeviceInterface->m_bConnected)
                break;
            Sleep(200);
        }
        if (i == 5)
            return;
    }

    UINT ToSendFrames;
    UINT NumFramesBeforeAck;
    m_OutstandingFrames     = 0;

    switch (m_Test)
    {
    case TEST_SEND_ONE:
        ToSendFrames = 1;
        NumFramesBeforeAck = 1;
        break;

    case TEST_PERFORMANCE_OUT:
        ToSendFrames = 0xffffffff;
        NumFramesBeforeAck = GetDlgItemInt(IDC_NUM_FRAMES_TO_ACK);
        break;

    case TEST_PERFORMANCE_IN:
        ToSendFrames = 0;
        NumFramesBeforeAck = 0;
        break;
    }
    BOOL bLoopback = ((CButton *)GetDlgItem(IDC_DEVICE_SHOULD_RETURN))->GetCheck();
    m_btDeviceInterface->SetDescriptors(1, (bLoopback || (m_Test == TEST_PERFORMANCE_IN)) ? 1 : 0);

    DWORD FramesToSendUnacked = 0;
    m_NumFramesSent           = 0;

    SetDlgItemInt(IDC_FRAMES_SENT, 0, FALSE);

    m_StartTime = GetTickCount();

    m_btDeviceInterface->SendSpeedTestCommand(SPEED_TEST_START, (BYTE)NumFramesBeforeAck);

    while (!m_Stop && (ToSendFrames != 0))
    {
        BYTE  data[GATT_MAX_ATTR_LEN];
        DWORD len;

        if ((ToSendFrames != 0) && (m_OutstandingFrames == 0))
        {
            for (int i = 0; ((BYTE)i < NumFramesBeforeAck) && (ToSendFrames != 0); i++)
            {
                GetNextFrame(data, &len);

                ToSendFrames--;
                m_btDeviceInterface->SendSpeedTestData(data, len);
                m_NumFramesSent++;
            }
            m_OutstandingFrames++;
        }
        if (WaitForSingleObject(m_hFlowEvent, 5000) == WAIT_TIMEOUT)
        {
            ods("Ack failed\n");
            break;
        }
        DWORD speed = ((ULONGLONG)m_NumFramesSent * m_numBytes * 8 * 1000) / (GetTickCount() - m_StartTime);
        SetDlgItemInt(IDC_THROUGHPUT, speed, FALSE);
        SetDlgItemInt(IDC_FRAMES_SENT, m_NumFramesSent, FALSE);
    }
    if (m_Test != TEST_PERFORMANCE_IN)
        m_Test = TEST_NONE;
}

void CSpeedTestDlg::OnBnClickedSend()
{
    if (!UpdateData(TRUE))
        return;

    InitTest();
    
    m_Test = TEST_SEND_ONE;

    CreateThread(NULL, 0, TestWorker, this, 0, NULL);
}


BOOL CSpeedTestDlg::GetDeviceName(BLUETOOTH_ADDRESS *pbth, char *name)
{
    static GUID GUID_BLUETOOTHLE_DEVICE_INTERFACE = {0x781aee18, 0x7733, 0x4ce4, {0xad, 0xd0, 0x91, 0xf4, 0x1c, 0x67, 0xb5, 0x92}};

    if (m_bWin8)
	{
        HDEVINFO  hardwareDeviceInfo;
		BOOL bRetVal = FALSE;

		if ((hardwareDeviceInfo = SetupDiGetClassDevs ((LPGUID)&GUID_BLUETOOTHLE_DEVICE_INTERFACE,
				NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE))) == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringA ("SetupDiGetClassDevs returned INVALID_HANDLE_VALUE.\n");
			return bRetVal;
        }
		// Enumerate devices of LE_DEVICE interface class
		for (DWORD dwIndex = 0; ; dwIndex++) 
		{
			SP_DEVINFO_DATA DeviceInfoData;

			memset(&DeviceInfoData, 0, sizeof(DeviceInfoData));
			DeviceInfoData.cbSize = sizeof(DeviceInfoData);

            OutputDebugStringA("Call SetupDiEnumDeviceInfo");
            if (!SetupDiEnumDeviceInfo(hardwareDeviceInfo, dwIndex, &DeviceInfoData))
				break;

			CHAR hwid[30];
            DWORD dwBytes = dwBytes = sizeof (hwid);

            if (!SetupDiGetDeviceRegistryPropertyA(hardwareDeviceInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)hwid, dwBytes, &dwBytes))
            {
                OutputDebugStringA("Continue in loop");
				continue;
            }
			int bda0, bda1, bda2, bda3, bda4, bda5;
			sscanf_s(hwid, "BTHLE\\Dev_%02x%02x%02x%02x%02x%02x", &bda0, &bda1, &bda2, &bda3, &bda4, &bda5);

            if ((pbth->rgBytes[0] == (BYTE)bda5) && (pbth->rgBytes[1] == (BYTE)bda4) && (pbth->rgBytes[2] == (BYTE)bda3)
             && (pbth->rgBytes[3] == (BYTE)bda2) && (pbth->rgBytes[4] == (BYTE)bda1) && (pbth->rgBytes[5] == (BYTE)bda0))
			{
				DWORD dwBytes = 251;
				if (SetupDiGetDeviceRegistryPropertyA(hardwareDeviceInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)name, dwBytes, &dwBytes))
                {
					bRetVal = TRUE;
                }
				break;
			}
		}
		SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
		return bRetVal;
	}
                    
    WORD    wVersionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;

    WSAStartup(wVersionRequested, &wsaData);

    WSAQUERYSETW qs = {0};
#define OFFSET 1024
    BYTE abyBuf[sizeof(WSAQUERYSETW) + OFFSET]; // provide a sufficient large buffer for returned query set
	WSAQUERYSETW *pQS = (WSAQUERYSETW*) abyBuf;
	HANDLE hLookup = INVALID_HANDLE_VALUE;
	DWORD dwFlags = LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_RETURN_TYPE | LUP_RETURN_NAME;

	//Prepare WSAQUERYSETW
	qs.dwSize       = sizeof (WSAQUERYSETW);
	qs.dwNameSpace  = NS_BTH;
	if (WSALookupServiceBeginW(&qs, dwFlags, &hLookup) != SOCKET_ERROR)
	{
		//Retrive next record
		DWORD dwBufLen = sizeof(WSAQUERYSETW) + OFFSET;
		while (WSALookupServiceNextW(hLookup, dwFlags, &dwBufLen, pQS) != SOCKET_ERROR)
		{
	        BYTE *p = (BYTE *)pQS->lpcsaBuffer->RemoteAddr.lpSockaddr->sa_data;
            if ((pbth->rgBytes[0] == p[0]) && (pbth->rgBytes[1] == p[1]) && (pbth->rgBytes[2] == p[2])
             && (pbth->rgBytes[3] == p[3]) && (pbth->rgBytes[4] == p[4]) && (pbth->rgBytes[5] == p[5]))
            {
                if (pQS->lpszServiceInstanceName[0])
 					WideCharToMultiByte(CP_UTF8, 0, pQS->lpszServiceInstanceName, -1, name , 251, 0, 0);
                else
                    name[0] = 0;
				
				WSALookupServiceEnd(hLookup);
                return TRUE;
            }
        }
		if (hLookup != INVALID_HANDLE_VALUE)
			WSALookupServiceEnd(hLookup);
    }
    else
    {
        DWORD wsErr =  WSAGetLastError();
        ods("%d\n", wsErr);
    }
    WSACleanup();
	return FALSE;
}


void CSpeedTestDlg::OnBnClickedPerformanceStartOut()
{
    if (!UpdateData(TRUE))
        return;

    if (m_Test == TEST_NONE)
    {
        GetDlgItem(IDC_SEND)->EnableWindow(FALSE);

        SetDlgItemText(IDC_PERFORMANCE_START_OUT, _T("Stop"));
        (GetDlgItem(IDC_PERFORMANCE_START_IN))->EnableWindow (FALSE);

        InitTest();

        m_Test = TEST_PERFORMANCE_OUT;

        CreateThread(NULL, 0, TestWorker, this, 0, NULL);
    }
    else
    {
        m_Stop = TRUE;

        GetDlgItem(IDC_SEND)->EnableWindow(TRUE);

        SetDlgItemText(IDC_PERFORMANCE_START_OUT, _T("Start Send"));
        (GetDlgItem(IDC_PERFORMANCE_START_IN))->EnableWindow (TRUE);
    }
}


void CSpeedTestDlg::OnBnClickedRandom()
{
    UpdateData(TRUE);
    GetDlgItem(IDC_SEND_BYTES)->EnableWindow(!m_bRandomData);
    GetDlgItem(IDC_NUM_BYTES)->EnableWindow(m_bRandomData);
}





void CSpeedTestDlg::OnBnClickedPerformanceStartIn()
{
    if (!UpdateData(TRUE))
        return;

    if (m_Test == TEST_NONE)
    {
        GetDlgItem(IDC_SEND)->EnableWindow(FALSE);

        SetDlgItemText(IDC_PERFORMANCE_START_IN, _T("Stop"));
        (GetDlgItem(IDC_PERFORMANCE_START_OUT))->EnableWindow (FALSE);

        InitTest();

        m_Test = TEST_PERFORMANCE_IN;

        CreateThread(NULL, 0, TestWorker, this, 0, NULL);
    }
    else
    {
        m_Stop = TRUE;

        m_Test = TEST_NONE;

        GetDlgItem(IDC_SEND)->EnableWindow(TRUE);

        m_btDeviceInterface->SendSpeedTestCommand(SPEED_TEST_STOP, (BYTE)0);
        SetDlgItemText(IDC_PERFORMANCE_START_IN, _T("Start Receive"));
        (GetDlgItem(IDC_PERFORMANCE_START_OUT))->EnableWindow (TRUE);
    }
}
