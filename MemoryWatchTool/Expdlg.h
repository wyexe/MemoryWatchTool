#pragma once

#include "Common/ExpressionAnalysis.h"

// CExpdlg dialog

class CExpdlg : public CDialogEx
{
	DECLARE_DYNAMIC(CExpdlg)

public:
	CExpdlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExpdlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	BOOL AnalysisExpression(_In_ cwstring& wsText);
	void RepaintEdit(_In_ CONST CRect& dlgRect);
	void ReapintListControl(_In_ CONST CRect& dlgRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();

	void SetDlgPtr(CExpdlg* p);
	void SetExpressionText(_In_ cwstring& wsText);
	void release();
	cwstring& GetErrText();
private:
	CExpdlg* pthis;
	CExpressionAnalysis ExpAnalysis;
	std::wstring wsExpression;
public:
	afx_msg void OnClose();
	afx_msg void OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
