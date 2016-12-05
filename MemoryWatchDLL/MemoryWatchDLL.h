// MemoryWatchDLL.h : main header file for the MemoryWatchDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMemoryWatchDLLApp
// See MemoryWatchDLL.cpp for the implementation of this class
//

class CMemoryWatchDLLApp : public CWinApp
{
public:
	CMemoryWatchDLLApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
