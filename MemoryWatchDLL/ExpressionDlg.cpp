// ExpressionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemoryWatchDLL.h"
#include "ExpressionDlg.h"
#include "afxdialogex.h"

// CExpressionDlg dialog

IMPLEMENT_DYNAMIC(CExpressionDlg, CDialogEx)

CExpressionDlg::CExpressionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CExpressionDlg::IDD, pParent)
{

}

CExpressionDlg::~CExpressionDlg()
{
}

void CExpressionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExpressionDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CExpressionDlg::OnInitDialog()
{
	return TRUE;
}


// CExpressionDlg message handlers


void CExpressionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect RectDlg;
	this->GetClientRect(&RectDlg);
	RepaintEdit(RectDlg);
	Invalidate();
}

void CExpressionDlg::RepaintEdit(_In_ CONST CRect& dlgRect)
{
	CEdit* pEdit = reinterpret_cast<CEdit*>(this->GetDlgItem(IDC_EDIT_EXPRESSION));
	if (pEdit == nullptr)
		return;

	CRect RectEdit;
	pEdit->GetWindowRect(&RectEdit);
	this->ScreenToClient(&RectEdit);

	RectEdit.right = dlgRect.right - 20;
	pEdit->MoveWindow(&RectEdit);
}


BOOL CExpressionDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		WCHAR wszText[1024] = { 0 };
		CEdit* pEdit = reinterpret_cast<CEdit*>(this->GetDlgItem(IDC_EDIT_EXPRESSION));
		pEdit->GetWindowTextW(wszText, _countof(wszText) - 1);

		CExpListdlg * dlg = new CExpListdlg;
		dlg->SetDlgPtr(dlg);
		dlg->SetExpressionText(wszText);
		dlg->Create(IDD_DIALOG2, this);
		dlg->ShowWindow(SW_SHOW);
		dlg->AnalysisExpression(cwstring(wszText));

	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CExpressionDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}
