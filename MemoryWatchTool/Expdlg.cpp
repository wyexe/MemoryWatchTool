// Expdlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemoryWatchTool.h"
#include "Expdlg.h"
#include "afxdialogex.h"
#include "Resource.h"

// CExpdlg dialog

IMPLEMENT_DYNAMIC(CExpdlg, CDialogEx)

CExpdlg::CExpdlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CExpdlg::IDD, pParent), pthis(nullptr)
{

}

CExpdlg::~CExpdlg()
{
}

void CExpdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExpdlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CExpdlg::OnNMCustomdrawList2)
END_MESSAGE_MAP()


// CExpdlg message handlers


BOOL CExpdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CListCtrl* pListCtrl = nullptr;
	pListCtrl = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_LIST2));
	pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_GRIDLINES | WS_HSCROLL/*水平滚动条*/);

	CEdit* pEdit = reinterpret_cast<CEdit*>(GetDlgItem(IDC_EDIT_EXPRESSION_2));
	pEdit->SetWindowTextW(wsExpression.c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CExpdlg::AnalysisExpression(_In_ cwstring& wsText)
{
	static CListCtrl* pListCtrl = nullptr;
	pListCtrl = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_LIST2));

	ExpAnalysis.SetColumner([](int nWidth, cwstring& wsColumnText)
	{
		UINT uColumnCount = pListCtrl->GetHeaderCtrl()->GetItemCount();
		return pListCtrl->InsertColumn(uColumnCount, wsColumnText.c_str(), LVCFMT_LEFT, nWidth);
	});

	ExpAnalysis.SetRowText([](int nColumnIndex, int nRowIndex, cwstring& wsText)
	{
		while (pListCtrl->GetItemCount() <= nRowIndex)
			pListCtrl->InsertItem(pListCtrl->GetItemCount(), L"Empty");

		pListCtrl->SetItemText(nRowIndex, nColumnIndex, wsText.c_str());
	});

	ExpAnalysis.SetPrinter([](cwstring& wsText)
	{
		if (pListCtrl->GetHeaderCtrl()->GetItemCount() == 0)
			pListCtrl->InsertColumn(0, L"ErrText", LVCFMT_LEFT, 200);

		pListCtrl->InsertItem(pListCtrl->GetItemCount(), wsText.c_str());
	});

	ExpAnalysis.SetCleaner([]
	{
		pListCtrl->DeleteAllItems();
		while (pListCtrl->GetHeaderCtrl()->GetItemCount() != 0)
			pListCtrl->DeleteColumn(0);
	});

	return ExpAnalysis.Analysis(wsText);
}

void CExpdlg::ReapintListControl(_In_ CONST CRect& dlgRect)
{
	CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(this->GetDlgItem(IDC_LIST2));
	if (pListCtrl == nullptr)
		return;

	CRect RectEdit;
	pListCtrl->GetWindowRect(&RectEdit);
	this->ScreenToClient(&RectEdit);

	RectEdit.right = dlgRect.right - 20;
	RectEdit.bottom = dlgRect.bottom - 20;
	pListCtrl->MoveWindow(&RectEdit);
}


void CExpdlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect RectDlg;
	this->GetClientRect(&RectDlg);
	ReapintListControl(RectDlg);
	Invalidate();
	// TODO: Add your message handler code here
}


void CExpdlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}

void CExpdlg::SetDlgPtr(CExpdlg* p)
{
	pthis = p;
}

void CExpdlg::release()
{
	if (pthis != nullptr)
		delete pthis;
}


void CExpdlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ExpAnalysis.StopRecord();
	release();
	CDialogEx::OnClose();
}


void CExpdlg::OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = 0;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(this->GetDlgItem(IDC_LIST2));
		auto wstr = pListCtrl->GetItemText(static_cast<DWORD>(pLVCD->nmcd.dwItemSpec), pLVCD->iSubItem);
		if (wstr.Find(L"!") != -1)
			pLVCD->clrText = RGB(255, 0, 0);
	}

}


BOOL CExpdlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		CEdit* pEdit = reinterpret_cast<CEdit*>(this->GetDlgItem(IDC_EDIT_EXPRESSION_2));
		if (pMsg->hwnd == pEdit->GetSafeHwnd())
		{
			WCHAR wszText[1024] = { 0 };
			pEdit->GetWindowTextW(wszText, 1024);
			ExpAnalysis.Analysis(wszText);
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CExpdlg::RepaintEdit(_In_ CONST CRect& dlgRect)
{
	CEdit* pEdit = reinterpret_cast<CEdit*>(this->GetDlgItem(IDC_EDIT_EXPRESSION_2));
	if (pEdit == nullptr)
		return;

	CRect RectEdit;
	pEdit->GetWindowRect(&RectEdit);
	this->ScreenToClient(&RectEdit);

	RectEdit.right = dlgRect.right - 20;
	pEdit->MoveWindow(&RectEdit);
}

void CExpdlg::SetExpressionText(_In_ cwstring& wsText)
{
	wsExpression = wsText;
}
