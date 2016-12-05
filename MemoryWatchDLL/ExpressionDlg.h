#pragma once


#include "ExpListdlg.h"
// CExpressionDlg dialog

class CExpressionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CExpressionDlg)

public:
	CExpressionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExpressionDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void RepaintEdit(_In_ CONST CRect& dlgRect);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
};
