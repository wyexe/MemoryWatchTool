
// MemoryWatchToolDlg.h : header file
//

#pragma once


// CMemoryWatchToolDlg dialog
class CMemoryWatchToolDlg : public CDialogEx
{
// Construction
public:
	CMemoryWatchToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MEMORYWATCHTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void RepaintEdit(_In_ CONST CRect& dlgRect);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
};
