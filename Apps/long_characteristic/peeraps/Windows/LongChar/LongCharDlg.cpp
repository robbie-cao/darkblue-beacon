
// LongCharDlg.cpp : implementation file
//

#include "stdafx.h"
#include <setupapi.h>
#include "LongChar.h"
#include "LongCharDlg.h"
#include "afxdialogex.h"
#include "../../../long_characteristic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CLongCharDlg dialog



CLongCharDlg::CLongCharDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLongCharDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_btInterface = NULL;
}

CLongCharDlg::~CLongCharDlg()
{
	delete m_btInterface;
}

void CLongCharDlg::SetParam(BLUETOOTH_ADDRESS *bth, HMODULE hLib)
{
    if (m_bWin8) 
        m_btInterface = new CBtWin8Interface(bth, hLib, this);
    else
        m_btInterface = new CBtWin7Interface(bth, hLib, this);
}

void CLongCharDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLongCharDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_CONNECTED, OnConnected)
    ON_MESSAGE(WM_LONG_CHAR, OnLongChar)
    ON_BN_CLICKED(IDC_WRITE_LONG_CHAR_VALUE, &CLongCharDlg::OnBnClickedWriteLongChar)
    ON_BN_CLICKED(IDCANCEL, &CLongCharDlg::OnBnClickedCancel)
    ON_CBN_SELCHANGE(IDC_LONG_CHAR_CLIENT_CFG, &CLongCharDlg::OnCbnSelchangeLongCharClientCfg)
    ON_BN_CLICKED(IDC_READ_LONG_CHAR, &CLongCharDlg::OnBnClickedReadLongChar)
END_MESSAGE_MAP()


// CLongCharDlg message handlers

BOOL CLongCharDlg::OnInitDialog()
{
    BOOL bConnected = TRUE;  // assume that device is connected which should generally be the case for location sensor

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

    USHORT ClientConfDescrNotify = 0;

    m_btInterface->Init(guidSvcLongChar);

    // on Win7 we will receive notification when device is connected and will intialize dialog there
    if (!m_bWin8)
        return TRUE;

    CBtWin8Interface *pWin8BtInterface = dynamic_cast<CBtWin8Interface *>(m_btInterface);

    // Assume that we are connected.  
    pWin8BtInterface->m_bConnected = TRUE;

    if (pWin8BtInterface->m_bConnected)
    {
        if (!ReadLongChar())
            pWin8BtInterface->m_bConnected = FALSE;
    }

    pWin8BtInterface->RegisterNotification();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLongCharDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CLongCharDlg::OnPaint()
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
HCURSOR CLongCharDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLongCharDlg::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();
}


LRESULT CLongCharDlg::OnConnected(WPARAM bConnected, LPARAM lparam)
{
    SetDlgItemText(IDC_DEVICE_STATE, bConnected ? L"Connected" : L"Disconnected");

    if (!bConnected)
        return S_OK;

    if (!m_bWin8)
    {
        ReadLongChar();
    }
    return S_OK;
}

BOOL CLongCharDlg::WriteLongChar(LPBYTE pData, DWORD length)
{
    BTW_GATT_VALUE value = {0};
    memcpy (value.value, pData, length);
    value.len = (USHORT)length;

    return (m_btInterface->WriteCharacteristic(guidCharLongChar, 0, &value));
}

BOOL CLongCharDlg::ReadLongChar()
{
    BTW_GATT_VALUE value;
    if (m_btInterface->ReadCharacteristic(guidCharLongChar, 0, &value))
    {
        WCHAR buf[1024] = {0};
        for (int i = 0; i < value.len; i++)
            wsprintf(&buf[wcslen(buf)], L"%02x ", value.value[i]);
        SetDlgItemText(IDC_LONG_CHAR_VALUE, buf);
        return TRUE;
    }
    return FALSE;
}

BOOL CLongCharDlg::WriteLongCharClientCfg(int ClientCfg)
{
    BTW_GATT_VALUE value = {0};
    value.len = 2;
    value.value[0] = ClientCfg;
    value.value[1] = 0;

    return (m_btInterface->SetDescriptorValue(guidCharLongChar, 0, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &value));
}

LRESULT CLongCharDlg::OnLongChar(WPARAM Instance, LPARAM lparam)
{
    BTW_GATT_VALUE *pValue = (BTW_GATT_VALUE *)lparam;
 
    WCHAR buf[1024] = {0};
    for (int i = 0; i < pValue->len; i++)
        wsprintf(&buf[wcslen(buf)], L"%02x ", pValue->value[i]);
    SetDlgItemText(IDC_LONG_CHAR_VALUE, buf);

    free (pValue);
    return S_OK;
}

void CLongCharDlg::OnBnClickedReadLongChar()
{
    ReadLongChar();
}

void CLongCharDlg::OnBnClickedWriteLongChar()
{
    WCHAR buf[1024];
    BYTE long_char[512];
    int val;
    int j = 0, i = 0;
    GetDlgItemText(IDC_LONG_CHAR_VALUE, buf, sizeof(buf));
    while (swscanf_s(&buf[i], L"%02x ", &val) == 1)
    {
        long_char[j++] = (BYTE)val;
        i += 3;
    }
    WriteLongChar(long_char, j);
}


void CLongCharDlg::OnCbnSelchangeLongCharClientCfg()
{
    int Sel = ((CComboBox *)GetDlgItem(IDC_LONG_CHAR_CLIENT_CFG))->GetCurSel();
    WriteLongCharClientCfg(Sel);
}


void CLongCharDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}


