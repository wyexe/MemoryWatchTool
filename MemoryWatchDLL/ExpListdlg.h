#pragma once
#include "../MemoryWatchTool/Common/ExpressionAnalysis.h"

// CExpListdlg dialog

class CExpListdlg : public CDialogEx
{
	DECLARE_DYNAMIC(CExpListdlg)

public:
	CExpListdlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExpListdlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	void release();
	void RepaintEdit(_In_ CONST CRect& dlgRect);
	void ReapintListControl(_In_ CONST CRect& dlgRect);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	CExpListdlg* pthis;
	CExpressionAnalysis ExpAnalysis;
	std::wstring wsExpressionText;
public:
	void SetExpressionText(_In_ cwstring& wsText);
	void SetDlgPtr(CExpListdlg* p);
	BOOL AnalysisExpression(_In_ cwstring& wsText);
	afx_msg void OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
