
// SpeedTestDlg.h : header file
//

#pragma once
#include "Win7Interface.h"
#include "Win8Interface.h"
#include "afxcmn.h"

#define WM_CONNECTED        (WM_USER + 101)
#define WM_FRAME_RECEIVED   (WM_USER + 102)
#define WM_FRAME_ACKED      (WM_USER + 103)

// CSpeedTestDlg dialog
class CSpeedTestDlg : public CDialogEx
{
// Construction
public:
	CSpeedTestDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CSpeedTestDlg();

// Dialog Data
	enum { IDD = IDD_SPEED_TEST_DIALOG };

	BOOL m_bWin8;
//    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btDeviceInterface;
    void ExecuteTest();

    UINT m_OutstandingFrames;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    void InitTest();
    BOOL GetDeviceName(BLUETOOTH_ADDRESS *pbth, char *name);
    void GetNextFrame(LPBYTE data, LPDWORD len);

private:
    enum {TEST_NONE, TEST_SEND_ONE, TEST_PERFORMANCE_OUT, TEST_PERFORMANCE_IN} m_Test;
    BOOL        m_Stop;
    int         numNotifies;
    HANDLE      m_hDevice;
    CComboBox   m_cbDevices;
    BOOL        GetDeviceList();
    int         m_numDevices;
    HANDLE      m_hFlowEvent;
    DWORD       m_NumFramesSent;
    DWORD       m_StartTime;
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
    afx_msg void OnBnClickedSend();
    BOOL m_bRandomData;
    CString m_bBytes;
    int m_numBytes;
    afx_msg void OnBnClickedRandom();
    afx_msg void OnBnClickedPerformanceStartOut();
    afx_msg void OnBnClickedPerformanceStartIn();
    int m_numFramesUnacked;
};
