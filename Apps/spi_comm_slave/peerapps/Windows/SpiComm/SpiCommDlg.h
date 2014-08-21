
// SpiCommDlg.h : header file
//

#pragma once
#include "Win7Interface.h"
#include "Win8Interface.h"
#include "afxcmn.h"

#define WM_CONNECTED        (WM_USER + 101)
#define WM_FRAME_RECEIVED   (WM_USER + 102)
#define WM_FRAME_ACKED      (WM_USER + 103)

// CSpiCommDlg dialog
class CSpiCommDlg : public CDialogEx
{
// Construction
public:
	CSpiCommDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CSpiCommDlg();

// Dialog Data
	enum { IDD = IDD_SPI_COMM_DIALOG };

	BOOL m_bWin8;
//    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btMasterInterface;
    CBtInterface *m_btSlaveInterface;
    void ExecuteTest();

    UINT m_OutstandingMasterFrames;
    UINT m_OutstandingSlaveFrames;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    void InitTest();
    BOOL GetDeviceName(BLUETOOTH_ADDRESS *pbth, char *name);
    void GetNextFrame(LPBYTE data, LPDWORD len);

private:
    enum {TEST_NONE, TEST_SEND_ONE_MASTER, TEST_SEND_ONE_SLAVE, TEST_HALF_DUPLEX_MASTER, TEST_HALF_DUPLEX_SLAVE, TEST_FULL_DUPLEX} m_Test;
    BOOL        m_Stop;
    int         numNotifies;
    HANDLE      m_hDevice;
    CComboBox   m_cbSlaveDevices;
    CComboBox   m_cbMasterDevices;
    BOOL        GetDeviceList();
    int         m_numSlaveDevices;
    int         m_numMasterDevices;
    HANDLE      m_hMasterFlow;
    HANDLE      m_hSlaveFlow;
// Implementation
protected:
	HICON m_hIcon;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    virtual void PostNcDestroy();
    LRESULT OnConnected(WPARAM bConnected, LPARAM lparam);
    LRESULT OnFrameReceived(WPARAM pValue, LPARAM pInterface);
    LRESULT OnFrameAcked(WPARAM numFrames, LPARAM pInterface);
public:
    afx_msg void OnBnClickedSendToMaster();
    afx_msg void OnBnClickedSendToSlave();
    BOOL m_bRandomData;
    CString m_bBytes;
    int m_numBytes;
    afx_msg void OnBnClickedRandom();
    afx_msg void OnBnClickedHalfDuplexMaster();
    afx_msg void OnBnClickedHalfDuplexSlave();
    afx_msg void OnBnClickedFullDuplex();
};
