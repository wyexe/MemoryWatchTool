// ExpListdlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemoryWatchDLL.h"
#include "ExpListdlg.h"
#include "afxdialogex.h"


// CExpListdlg dialog

IMPLEMENT_DYNAMIC(CExpListdlg, CDialogEx)

CExpListdlg::CExpListdlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CExpListdlg::IDD, pParent)
{

}

CExpListdlg::~CExpListdlg()
{
}

void CExpListdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExpListdlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CExpListdlg::OnNMCustomdrawList1)
END_MESSAGE_MAP()

BOOL CExpListdlg::OnInitDialog()
{
	CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_LIST1));
	pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_GRIDLINES | WS_HSCROLL/*水平滚动条*/);

	CEdit* pEdit = reinterpret_cast<CEdit*>(GetDlgItem(IDC_EDIT_EXPRESSION_2));
	pEdit->SetWindowTextW(wsExpressionText.c_str());
	pEdit->SetFocus();
	return TRUE;
}


// CExpListdlg message handlers


void CExpListdlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}


void CExpListdlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ExpAnalysis.StopRecord();
	release();
	CDialogEx::OnClose();
}

void CExpListdlg::release()
{
	if (pthis != nullptr)
		delete pthis;
}

void CExpListdlg::SetDlgPtr(CExpListdlg* p)
{
	pthis = p;
}



void CExpListdlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect RectDlg;
	this->GetClientRect(&RectDlg);
	ReapintListControl(RectDlg);
	RepaintEdit(RectDlg);
	Invalidate();
}

void CExpListdlg::ReapintListControl(_In_ CONST CRect& dlgRect)
{
	CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(this->GetDlgItem(IDC_LIST1));
	if (pListCtrl == nullptr)
		return;

	CRect RectEdit;
	pListCtrl->GetWindowRect(&RectEdit);
	this->ScreenToClient(&RectEdit);

	RectEdit.right = dlgRect.right - 20;
	RectEdit.bottom = dlgRect.bottom - 20;
	pListCtrl->MoveWindow(&RectEdit);
}

BOOL CExpListdlg::AnalysisExpression(_In_ cwstring& wsText)
{
	static CListCtrl* pListCtrl = nullptr;
	pListCtrl = reinterpret_cast<CListCtrl*>(GetDlgItem(IDC_LIST1));

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


void CExpListdlg::OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult)
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
		CListCtrl* pListCtrl = reinterpret_cast<CListCtrl*>(this->GetDlgItem(IDC_LIST1));
		auto wstr = pListCtrl->GetItemText(static_cast<DWORD>(pLVCD->nmcd.dwItemSpec), pLVCD->iSubItem);
		if (wstr.Find(L"!") != -1)
			pLVCD->clrText = RGB(255, 0, 0); // Red
		else
			pLVCD->clrText = RGB(0, 0, 0); // Dark
	}
}


BOOL CExpListdlg::PreTranslateMessage(MSG* pMsg)
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

void CExpListdlg::RepaintEdit(_In_ CONST CRect& dlgRect)
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

void CExpListdlg::SetExpressionText(_In_ cwstring& wsText)
{
	wsExpressionText = wsText;
}
