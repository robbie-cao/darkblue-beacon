
// WsSecOtaUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"
#include <setupapi.h>
#include "WsSecOtaUpgrade.h"
#include "WsSecOtaUpgradeDlg.h"
#include "..\..\..\ws_sec_upgrade_ota.h"

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


// WsSecOtaUpgradeDlg dialog



CWsSecOtaUpgradeDlg::CWsSecOtaUpgradeDlg(LPBYTE pPatch, DWORD dwPatchSize, CWnd* pParent /*=NULL*/)
    : CDialogEx(CWsSecOtaUpgradeDlg::IDD, pParent),
    m_pPatch(pPatch),
    m_dwPatchSize(dwPatchSize)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_btInterface = NULL;
    m_pDownloader = NULL;
}

CWsSecOtaUpgradeDlg::~CWsSecOtaUpgradeDlg()
{
    delete m_btInterface;
    delete m_pDownloader;
}

void CWsSecOtaUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_LIST, m_cbDevices);
    DDX_Control(pDX, IDC_UPGRADE_PROGRESS, m_Progress);
}

BEGIN_MESSAGE_MAP(CWsSecOtaUpgradeDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_CONNECTED, OnConnected)
    ON_MESSAGE(WM_NOTIFIED, OnNotified)
    ON_MESSAGE(WM_PROGRESS, OnProgress)
    ON_BN_CLICKED(IDC_START, &CWsSecOtaUpgradeDlg::OnBnClickedStart)
END_MESSAGE_MAP()


// CWsSecOtaUpgradeDlg message handlers

BOOL CWsSecOtaUpgradeDlg::OnInitDialog()
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

    GetDeviceList();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWsSecOtaUpgradeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWsSecOtaUpgradeDlg::OnPaint()
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
HCURSOR CWsSecOtaUpgradeDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



void CWsSecOtaUpgradeDlg::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();
}

BOOL CWsSecOtaUpgradeDlg::GetDeviceList()
{
    HDEVINFO                            hardwareDeviceInfo;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
    WCHAR								szBda[13] = {0};
    HANDLE								hDevice = INVALID_HANDLE_VALUE;

    m_numDevices = 0;

    if ((hardwareDeviceInfo = SetupDiGetClassDevs (NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT)) != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA DeviceInfoData;

        memset(&DeviceInfoData, 0, sizeof(DeviceInfoData));
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);

        WCHAR szService[80];
        GUID guid;
        if (m_bWin8)
            guid = GUID_WS_SECURE_UPGRADE_SERVICE;
        else
        {
            guid.Data1 = (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[4]     ) + (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[5] << 8) + (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[6] << 16) + (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[7] << 24);
            guid.Data2 = (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[2]     ) + (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[3] << 8);
            guid.Data3 = (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[0]     ) + (GUID_WS_SECURE_UPGRADE_SERVICE.Data4[1] << 8);
            guid.Data4[0] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data3      ) & 0xff;
            guid.Data4[1] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data3 >> 8 ) & 0xff;
            guid.Data4[2] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data2      ) & 0xff;
            guid.Data4[3] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data2 >> 8 ) & 0xff;
            guid.Data4[4] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data1      ) & 0xff;
            guid.Data4[5] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data1 >> 8 ) & 0xff;
            guid.Data4[6] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data1 >> 16) & 0xff;
            guid.Data4[7] = (GUID_WS_SECURE_UPGRADE_SERVICE.Data1 >> 24) & 0xff;
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
                            m_cbDevices.AddString(buf);
                            m_numDevices++;
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

    GetDlgItem(IDC_UPGRADE_PROGRESS)->ShowWindow((m_numDevices == 0) ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_DEVICE_LIST)->ShowWindow((m_numDevices == 0) ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_NO_DEVICES)->ShowWindow((m_numDevices == 0) ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_START)->SetWindowText((m_numDevices == 0) ? L"Done" : L"Start");
    return TRUE;
}


LRESULT CWsSecOtaUpgradeDlg::OnConnected(WPARAM bConnected, LPARAM lparam)
{
    SetDlgItemText(IDC_DEVICE_STATE, bConnected ? L"Connected" : L"Disconnected");

    if (!bConnected)
        return S_OK;

    SetDlgItemText(IDC_STATUS, L"Ready");
    m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_CONNECTED);
    m_errCode = WS_UPGRADE_STATUS_OK;
    return S_OK;
}

LRESULT CWsSecOtaUpgradeDlg::OnNotified(WPARAM bConnected, LPARAM lparam)
{
    BTW_GATT_VALUE *pValue = (BTW_GATT_VALUE *)lparam;
    if (pValue->len == 1)
    {
        if (pValue->value[0] == WS_UPGRADE_STATUS_OK)
        {
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_OK);
        }
        else
        {
            m_errCode = pValue->value[0];
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_FAILED);
        }
    }
    free (pValue);

    return S_OK;
}

LRESULT CWsSecOtaUpgradeDlg::OnProgress(WPARAM state, LPARAM param)
{
    static UINT total;
    if (state == WSDownloader::WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD)
    {
        total = (UINT)param;
        m_Progress.SetRange(0, (short)param);
        SetDlgItemText(IDC_STATUS, L"Transfer");
        SetDlgItemText(IDC_START, L"Abort");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_DATA_TRANSFER)
    {
        m_Progress.SetPos((int)param);
        if (param == total)
        {
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_START_VERIFICATION);
            SetDlgItemText(IDC_STATUS, L"Download verification");
        }
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_VERIFIED)
    {
        SetDlgItemText(IDC_STATUS, L"Success");
        SetDlgItemText(IDC_START, L"Done");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_ABORTED)
    {
        m_Progress.SetPos(total);
        switch (m_errCode)
        {
        case WS_UPGRADE_STATUS_VERIFICATION_FAILED:
            SetDlgItemText(IDC_STATUS, L"Firmware verification failed");
            break;
        case WS_UPGRADE_STATUS_INVALID_IMAGE:
            SetDlgItemText(IDC_STATUS, L"Firmware invalid");
            break;
        case WS_UPGRADE_STATUS_INVALID_IMAGE_SIZE:
            SetDlgItemText(IDC_STATUS, L"Firmware invalid size");
            break;
        case WS_UPGRADE_STATUS_INVALID_APPID:
            SetDlgItemText(IDC_STATUS, L"Invalid Product ID");
            break;
        case WS_UPGRADE_STATUS_INVALID_VERSION:
            SetDlgItemText(IDC_STATUS, L"Invalid Major Version");
            break;
        default:
            SetDlgItemText(IDC_STATUS, L"Aborted");
            break;
        }
        SetDlgItemText(IDC_START, L"Done");
    }
    return S_OK;
}

void CWsSecOtaUpgradeDlg::OnBnClickedStart()
{
    // if no devices exit
    if (m_numDevices == 0)
    {
        OnCancel();
        return;
    }
    // if downloader is completed exit
    if ((m_pDownloader != NULL)
     && ((m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_VERIFIED) 
      || (m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_ABORTED)))
    {
        OnCancel();
        return;
    }
    // if transfer do abort
    if ((m_pDownloader != NULL) && (m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_DATA_TRANSFER))
    {
        m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_ABORT);
        return;
    }

    int sel = m_cbDevices.GetCurSel();
    WCHAR buf[13];
    m_cbDevices.GetLBText(m_cbDevices.GetCurSel(), buf);
    int bda[6];
    BLUETOOTH_ADDRESS bth = {0};
    if (swscanf_s(buf, L"%02x%02x%02x%02x%02x%02x", &bda[0], &bda[1], &bda[2], &bda[3], &bda[4], &bda[5]) == 6)
    {
        for (int i = 0; i < 6; i++)
            bth.rgBytes[5 - i] = (BYTE)bda[i];
    }

    if (m_bWin8) 
        m_btInterface = new CBtWin8Interface(&bth, hLib, this);
    else
        m_btInterface = new CBtWin7Interface(&bth, hLib, this);

    m_btInterface->Init();

    m_pDownloader = new WSDownloader(m_btInterface, m_pPatch, m_dwPatchSize, m_hWnd);

    // on Win7 we will receive notification when device is connected and will intialize dialog there
    if (!m_bWin8)
        return;

    CBtWin8Interface *pWin8BtInterface = dynamic_cast<CBtWin8Interface *>(m_btInterface);

    // Assume that we are connected.  Failed attempt to read battery will change that to FALSE.
    pWin8BtInterface->m_bConnected = TRUE;

    if (pWin8BtInterface->m_bConnected)
    {
        // register for notifications with the status
        USHORT ClientConfDescrControlPoint = 1;
        pWin8BtInterface->m_bConnected = m_btInterface->SetDescriptorValue(ClientConfDescrControlPoint);
    }
    pWin8BtInterface->RegisterNotification();

    if (pWin8BtInterface->m_bConnected)
        OnConnected(TRUE, NULL);
//    m_btInterface->SendWsUpgradeCommand(1);
}
