// MemoryWatchDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MemoryWatchDLL.h"
#include "ExpressionDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CMemoryWatchDLLApp

BEGIN_MESSAGE_MAP(CMemoryWatchDLLApp, CWinApp)
END_MESSAGE_MAP()


// CMemoryWatchDLLApp construction

CMemoryWatchDLLApp::CMemoryWatchDLLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMemoryWatchDLLApp object

CMemoryWatchDLLApp theApp;


// CMemoryWatchDLLApp initialization
DWORD WINAPI _ShowDlgThread(LPVOID lpParm)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CExpressionDlg dlg;
	dlg.DoModal();
	return 0;
}

BOOL CMemoryWatchDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	static HANDLE hThread = NULL;
	if (hThread == NULL)
	{
		hThread = cbBEGINTHREADEX(NULL, NULL, _ShowDlgThread, NULL, NULL, NULL);
	}
	return TRUE;
}

__declspec(dllexport) int WINAPI BBB()
{
	return 0xF;
}

__declspec(dllexport) BOOL WINAPI ReleaseDLL()
{
	return TRUE;
}