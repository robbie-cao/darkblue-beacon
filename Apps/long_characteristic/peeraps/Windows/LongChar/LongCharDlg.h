
// LongCharDlg.h : header file
//

#pragma once
#include "Win7Interface.h"
#include "Win8Interface.h"


// CLongCharDlg dialog
class CLongCharDlg : public CDialogEx
{
// Construction
public:
	CLongCharDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CLongCharDlg();

    void SetParam(BLUETOOTH_ADDRESS *bth, HMODULE hLib);

// Dialog Data
	enum { IDD = IDD_LONG_CHAR_DIALOG };

	BOOL m_bWin8;
//    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btInterface;
    BOOL ReadLongChar();
    BOOL WriteLongChar(LPBYTE pData, DWORD length);
    BOOL ReadLongCharClientCfg();
    BOOL WriteLongCharClientCfg(int ClientCfg);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    HANDLE m_hDevice;
    UINT32  m_LnFeature;
    UINT8  m_PositionQuality[16];

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
    LRESULT OnLongChar(WPARAM op, LPARAM lparam);
public:
    afx_msg void OnBnClickedReadLongChar();
    afx_msg void OnBnClickedWriteLongChar();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnCbnSelchangeLongCharClientCfg();
};
