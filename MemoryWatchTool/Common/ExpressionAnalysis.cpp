#include "stdafx.h"
#include "ExpressionAnalysis.h"
#include <locale>

CExpressionAnalysis::CExpressionAnalysis() : hThread(NULL)
{
	
}

BOOL CExpressionAnalysis::Analysis(_In_ cwstring& wsText) throw()
{
	Clear();

	vector<std::wstring> VecParm;
	CCharacter::Split(wsText, L" ", VecParm, Split_Option_KeepOnly | Split_Option_RemoveEmptyEntries);

	ExpContent.bDisablePrint = FALSE;
	ExpContent.emCmdType = GetCmdType(VecParm.at(0));
	switch (ExpContent.emCmdType)
	{
	case em_Cmd_Type::em_Cmd_Type_Invalid:
		SetErrText(L"invalid cmd type:%s", VecParm.at(0).c_str());
		return FALSE;
	case em_Cmd_Type::em_Cmd_Type_Clear:
		Cleaner();
		return TRUE;
	case em_Cmd_Type::em_Cmd_Type_Help:
		Help();
		return TRUE;
	case em_Cmd_Type::em_Cmd_Type_StopRecord:
		StopRecord();
		return TRUE;
	default:
		break;
	}

	if (VecParm.size() < 2)
	{
		SetErrText(L"parm < 2");
		return FALSE;
	}

	wsExpText = VecParm.at(1);
	if (!GetExpression())
		return FALSE;

	for (UINT i = 2; i < VecParm.size(); ++i)
	{
		auto itm = CCharacter::MakeTextToLower(VecParm.at(i));
		if (itm == L"-record")
		{
			ExpContent.bRecord = TRUE;
			continue;
		}
		if (itm == L"-!record")
		{
			ExpContent.bRecord = FALSE;
			continue;
		}

		if (CCharacter::GetCount_By_CharacterW(itm, L"-") == 0)
		{
			SetErrText(L"Invalid Format!!");
			return FALSE;
		}

		std::wstring wsNumber;
		CCharacter::GetRemoveRight(itm, L"-", wsNumber);
		if (!IsNumber(wsNumber))
		{
			SetErrText(L"Invalid Format:%s", itm.c_str());
			return FALSE;
		}
		ExpContent.uMemWatchCount = static_cast<UINT>(_wtoi(wsNumber.c_str()));
	}

	if (!GetRpn())
		return FALSE;

	if (!ExpContent.bKeep)
	{
		// Clear List
		Cleaner();
	}

	switch (ExpContent.emCmdType)
	{
	case em_Cmd_Type::em_Cmd_Type_Calc:
		AddColumner(300,L"Process");
		return CalcResult_By_Rpn(nullptr);
	case em_Cmd_Type::em_Cmd_Type_dd:
		return ReadMemoryData();
	default:
		break;
	}
	
	SetErrText(L"UnFinish Cmd Type!");
	return FALSE;
}

VOID CExpressionAnalysis::SetPrinter(_In_ std::function<VOID(CONST std::wstring)> fnPrinter) throw()
{
	Printer = fnPrinter;
}

CExpressionAnalysis::em_Cmd_Type CExpressionAnalysis::GetCmdType(_In_ cwstring& wsText) throw()
{
	struct CmdText
	{
		em_Cmd_Type emCmdType;
		std::wstring wsCmdText;
	};
	static CONST std::vector<CmdText> VecCmdText = 
	{ 
		{ em_Cmd_Type_dd, L"dd" },					{ em_Cmd_Type_Calc, L"calc" }, 
		{ em_Cmd_Type_Clear, L"clear" },			{ em_Cmd_Type_Help, L"?" }, 
		{ em_Cmd_Type_StopRecord ,L"stoprecord"} 
	};

	auto wsLowerText = CCharacter::MakeTextToLower(wsText);

	auto p = CLPublic::Vec_find_if_Const(VecCmdText, [wsLowerText](CONST CmdText& CmdText_){ return CmdText_.wsCmdText == wsLowerText; });

	return p == nullptr ? em_Cmd_Type::em_Cmd_Type_Invalid : p->emCmdType;
}

CExpressionAnalysis::em_Text_Type CExpressionAnalysis::GetTextType(_In_ CONST UINT& uIndex) throw()
{
	static CONST WCHAR wszNumberText[] = { L"0123456789xABCDEFabcdef" };
	static CONST WCHAR wchSymbol[] = { L"+-*/%^|~&" };

	if (std::find_if(std::begin(wszNumberText), std::end(wszNumberText), [this, uIndex](CONST WCHAR wch){ return wch == wsExpText.at(uIndex); }) != std::end(wszNumberText))
	{
		return em_Text_Type::em_Text_Type_Number;
	}

	if (std::find_if(std::begin(wchSymbol), std::end(wchSymbol), [this, uIndex](CONST WCHAR wsSymbol){ return wsSymbol == wsExpText.at(uIndex); }) != std::end(wchSymbol))
	{
		return em_Text_Type::em_Text_Type_Symbol;
	}

	switch (wsExpText.at(uIndex))
	{
	case L'(': case L')': case L'[': case L']':
		return em_Text_Type::em_Text_Type_Bracket;
	case L' ': case L'\0':
		return em_Text_Type::em_Text_Type_NULL;
	case L'<': case L'>':
		if (uIndex + 1 >= wsExpText.length()) // two '<<' at least
			em_Text_Type::em_Text_Type_Invalid;

		if (wsExpText.at(uIndex) == L'<' && wsExpText.at(uIndex + 1) != L'<')
			return em_Text_Type::em_Text_Type_Invalid;
		if (wsExpText.at(uIndex) == L'>' && wsExpText.at(uIndex + 1) != L'>')
			return em_Text_Type::em_Text_Type_Invalid;

		return em_Text_Type::em_Text_Type_Symbol;
	default:
		break;
	}

	return em_Text_Type::em_Text_Type_Invalid;
}

CExpressionAnalysis::em_Content_Type CExpressionAnalysis::GetContentType(_In_ cwstring& wsSymbolText, _In_ em_Text_Type emTextType) throw()
{
	struct TextContent
	{
		em_Content_Type emContentType;
		std::wstring    wsText;
	};

	if (emTextType == em_Text_Type::em_Text_Type_Symbol)
	{
		CONST static std::vector<TextContent> Vec =
		{
			{ em_Content_Type::em_Content_Type_Symbol_Add, L"+" },
			{ em_Content_Type::em_Content_Type_Symbol_Sub, L"-" },
			{ em_Content_Type::em_Content_Type_Symbol_Mul, L"*" },
			{ em_Content_Type::em_Content_Type_Symbol_Div, L"/" },
			{ em_Content_Type::em_Content_Type_Symbol_Mod, L"%" },
			{ em_Content_Type::em_Content_Type_Symbol_ExOr, L"^" },
			{ em_Content_Type::em_Content_Type_Symbol_InOr, L"|" },
			{ em_Content_Type::em_Content_Type_Symbol_Comp, L"~" },
			{ em_Content_Type::em_Content_Type_Symbol_And, L"&" },
			{ em_Content_Type::em_Content_Type_Symbol_LeftShift, L"<<" },
			{ em_Content_Type::em_Content_Type_Symbol_RightShift, L">>" },
		};

		auto p = CLPublic::Vec_find_if_Const(Vec, [wsSymbolText](CONST TextContent& TC){ return TC.wsText == wsSymbolText; });
		return p != nullptr ? p->emContentType : em_Content_Type::em_Content_Type_None;
	}
	else if (emTextType == em_Text_Type::em_Text_Type_Bracket)
	{
		if (wsSymbolText == L"(")
			return em_Content_Type::em_Content_Type_LeftBracket;
		else if (wsSymbolText == L")")
			return em_Content_Type::em_Content_Type_RightBracket;
		else if (wsSymbolText == L"[")
			return em_Content_Type::em_Content_Type_LeftAddress;
		else if (wsSymbolText == L"]")
			return em_Content_Type::em_Content_Type_RightAddress;

	}

	return em_Content_Type::em_Content_Type_None;
}

BOOL CExpressionAnalysis::GetSymbolText(_In_ _Out_ UINT& uIndex, _Out_ std::wstring& wsSymbolText) throw()
{
	if (wsExpText.at(uIndex) == L'<')
	{
		if (wsExpText.at(uIndex + 1) != L'<')
			return FALSE;
		wsSymbolText = L"<<";
		uIndex += 1;
	}
	else if (wsExpText.at(uIndex) == L'>')
	{
		if (wsExpText.at(uIndex + 1) != L'>')
			return FALSE;
		wsSymbolText = L">>";
		uIndex += 1;
	}
	else
	{
		wsSymbolText.push_back(wsExpText.at(uIndex));
	}
	return TRUE;
}

BOOL CExpressionAnalysis::GetExpression() throw()
{
	std::wstring wsNumber;
	em_Text_Type emLastTextType = em_Text_Type::em_Text_Type_NULL;
	BOOL bNegative = FALSE;
	UINT uIndex = 0;

	while (uIndex < wsExpText.length())
	{
		auto emTextType = GetTextType(uIndex);
		switch (emTextType)
		{
		case em_Text_Type_Invalid:
			SetErrText(L"Invalid Character '%c'", wsExpText.at(uIndex));
			return FALSE;
		case em_Text_Type_Number:
			wsNumber.push_back(wsExpText.at(uIndex));
			break;
		case em_Text_Type_Symbol: case em_Text_Type_Bracket:
		{
			if (!wsNumber.empty())
			{
				if (bNegative)
					wsNumber = L"-" + wsNumber;

				ExpContent.ExpressionVec.push_back(ExpressionInfo{ em_Content_Type_Number, wsNumber });
				wsNumber.clear();
				bNegative = FALSE;
			}

			// 2次连续出现符号, 这次的符号必须是-的才行
			if (emTextType != em_Text_Type_Bracket && emLastTextType == em_Text_Type_Symbol)
			{
				if (wsExpText.at(uIndex) != L'-')
				{
					SetErrText(L"Prohibition of the continuous emergence of symbols: '%c'", wsExpText.at(uIndex));
					return FALSE;
				}
				bNegative = TRUE;
				break;
			}


			std::wstring wsSymbolText;
			if (!GetSymbolText(uIndex, wsSymbolText))
			{
				SetErrText(L"Invalid Symbol '%c'", wsExpText.at(uIndex));
				return FALSE;
			}

			auto emContentType = GetContentType(wsSymbolText, emTextType);
			if (emContentType == em_Content_Type::em_Content_Type_None)
			{
				SetErrText(L"Invalid Symbol '%c'", wsExpText.at(uIndex));
				return FALSE;
			}

			ExpContent.ExpressionVec.push_back(ExpressionInfo{ emContentType, wsSymbolText });
		}
		break;
		case em_Text_Type_NULL:
			break;
		default:
			break;
		}

		uIndex += 1;
		emLastTextType = emTextType;
	}

	if (!wsNumber.empty())
	{
		if (bNegative)
			wsNumber = L"-" + wsNumber;

		ExpContent.ExpressionVec.push_back(ExpressionInfo{ em_Content_Type_Number, wsNumber });
		wsNumber.clear();
	}
	return TRUE;
}

VOID CExpressionAnalysis::SetErrText(_In_ LPCWSTR pwszFormatText, ...) throw()
{
	va_list		args;
	wchar_t		szBuff[1024] = { 0 };

	va_start(args, pwszFormatText);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormatText, args);
	va_end(args);

	Printer(szBuff);
}

int CExpressionAnalysis::GetPriority(_In_ em_Content_Type emContentType) throw()
{
	if (emContentType == em_Content_Type::em_Content_Type_Symbol_Add || emContentType == em_Content_Type::em_Content_Type_Symbol_Sub)
		return 0x1;
	return 0x2;
}

BOOL CExpressionAnalysis::CompPrioity(_In_ em_Content_Type emTextType1, _In_ em_Content_Type emTextType2) throw()
{
	return GetPriority(emTextType1) > GetPriority(emTextType2);
}

BOOL CExpressionAnalysis::GetRpn() throw()
{
	std::stack<ExpressionInfo> StackSymbol;
	std::stack<ExpressionInfo> tmpRpn;
	for (CONST auto& itm : ExpContent.ExpressionVec)
	{
		if (itm.emContentType == em_Content_Type::em_Content_Type_Number)
		{
			tmpRpn.push(itm);
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_LeftBracket || itm.emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			StackSymbol.push(itm);
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_RightBracket)
		{
			while (StackSymbol.top().emContentType != em_Content_Type::em_Content_Type_LeftBracket)
			{
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			}
			StackSymbol.pop();
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_RightAddress)
		{
			BOOL bExistAddress = FALSE;
			do
			{
				if (StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftAddress)
					bExistAddress = TRUE;
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			} while (StackSymbol.top().emContentType != em_Content_Type::em_Content_Type_LeftAddress);

			if (!bExistAddress)
			{
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			}
			

			// Keep '['
			continue;
		}

		if (StackSymbol.empty() || StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftBracket || StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			StackSymbol.push(itm);
			continue;
		}

		if (CompPrioity(itm.emContentType, StackSymbol.top().emContentType))
		{
			StackSymbol.push(itm);
			continue;
		}

		tmpRpn.push(std::move(StackSymbol.top()));
		StackSymbol.pop();
		StackSymbol.push(itm);
	}

	while (!StackSymbol.empty())
	{
		tmpRpn.push(std::move(StackSymbol.top()));
		StackSymbol.pop();
	}

	while (!tmpRpn.empty())
	{
		Rpn.push(std::move(tmpRpn.top()));
		tmpRpn.pop();
	}

	return TRUE;
}

DWORD CExpressionAnalysis::CalcResult_By_Parm(_In_ DWORD dwNumberLeft, _In_ DWORD dwNumberRight, _In_ em_Content_Type emSymbolType) throw()
{
	switch (emSymbolType)
	{
	case em_Content_Type_LeftAddress: case em_Content_Type_RightAddress:
		return CCharacter::ReadDWORD(dwNumberLeft);
	case em_Content_Type_Symbol_Add:
		return dwNumberLeft + dwNumberRight;
	case em_Content_Type_Symbol_Sub:
		return dwNumberLeft - dwNumberRight;
	case em_Content_Type_Symbol_Mul:
		return dwNumberLeft * dwNumberRight;
	case em_Content_Type_Symbol_Div:
		return dwNumberLeft / dwNumberRight;
	case em_Content_Type_Symbol_Mod:
		return dwNumberLeft % dwNumberRight;
	case em_Content_Type_Symbol_ExOr:
		return dwNumberLeft ^ dwNumberRight;
	case em_Content_Type_Symbol_InOr:
		return dwNumberLeft | dwNumberRight;
	case em_Content_Type_Symbol_Comp:
		return ~dwNumberLeft;
	case em_Content_Type_Symbol_And:
		return dwNumberLeft & dwNumberRight;
	case em_Content_Type_Symbol_LeftShift:
		return dwNumberLeft << dwNumberRight;
	case em_Content_Type_Symbol_RightShift:
		return dwNumberLeft >> dwNumberRight;
	default:
		break;
	}
	return 0;
}

BOOL CExpressionAnalysis::CalcResult_By_Rpn(_Out_opt_ DWORD* pdwResult) throw()
{
	std::wostringstream wstream;
	std::stack<DWORD> StackResult;
	while (!Rpn.empty())
	{
		auto& itm = Rpn.top();
		if (itm.emContentType == em_Content_Type::em_Content_Type_Number)
		{
			StackResult.push(itm.GetHex());
			Rpn.pop();
			continue;
		}

		if (StackResult.size() == 0)
		{
			SetErrText(L"calc Number UnExist Left Value in Symbol '%s' ", itm.wsText.c_str());
			return FALSE;
		}

		DWORD NumberRight = StackResult.top();
		StackResult.pop();
		

		DWORD nResult = NULL;
		DWORD NumberLeft = NULL;
		if (itm.emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			nResult = CalcResult_By_Parm(NumberRight, NULL, itm.emContentType);
			if (!ExpContent.bDisablePrint)
			{
				wstream.clear();
				wstream.str(L"");
				wstream << L"[0x" << std::hex << NumberRight << L"] = 0x" << std::hex << nResult << std::endl;
				Printer(wstream.str());
			}
			
		}
		else if (itm.emContentType == em_Content_Type::em_Content_Type_Symbol_Comp)
		{
			nResult = CalcResult_By_Parm(NumberRight, NULL, itm.emContentType);
			if (!ExpContent.bDisablePrint)
			{
				wstream.clear();
				wstream.str(L"");
				wstream << L"~0x" << std::hex << NumberRight << L" = " << nResult << std::endl;
				Printer(wstream.str());
			}
		}
		else
		{
			if (StackResult.size() == 0)
			{
				SetErrText(L"calc Number UnExist Right Value in Symbol '%s' ", itm.wsText.c_str());
				return FALSE;
			}

			NumberLeft = StackResult.top();
			StackResult.pop();

			if (itm.emContentType == em_Content_Type::em_Content_Type_Symbol_Div && NumberRight == 0)
			{
				SetErrText(L"are u kidding me? did u want to div zero?");
				return FALSE;
			}

			nResult = CalcResult_By_Parm(NumberLeft, NumberRight, itm.emContentType);
			if (!ExpContent.bDisablePrint)
			{
				wstream.clear();
				wstream.str(L"");
				wstream << L"0x" << std::hex << NumberLeft << L" " << itm.wsText.c_str() << L" 0x" << std::hex << NumberRight << L" = 0x" << std::hex << nResult << std::endl;
				Printer(wstream.str());
			}
		}

		StackResult.push(nResult);
		Rpn.pop();
	}

	if (StackResult.size() == 0)
	{
		SetErrText(L"UnExist Result!");
		return FALSE;
	}
	CLPublic::SetPtr(pdwResult, StackResult.top());
	return TRUE;
}

VOID CExpressionAnalysis::SetColumner(_In_ std::function<UINT(int, cwstring&)> fnAddColumner) throw()
{
	AddColumner = fnAddColumner;
}

VOID CExpressionAnalysis::SetRowText(_In_ std::function<void(int,int, cwstring&)> fnSetRowTexter) throw()
{
	SetRowTexter = fnSetRowTexter;
}

BOOL CExpressionAnalysis::ReadMem_By_Rpn() throw()
{
	DWORD dwResult = 0;
	if (!CalcResult_By_Rpn(&dwResult))
		return FALSE;

	/*
	->0x0
	->0x4
	->0x8
	*/
	UINT uColumnIndex = AddColumner(60,L"Address");
	for (UINT i = 0; i < ExpContent.uMemWatchCount; ++i)
		SetRowTexter(uColumnIndex,i, CCharacter::FormatText(L"->0x%X", i * 4));

	/*
	Value1,
	Value2,
	Value3,
	*/
	uColumnIndex = AddColumner(80,L"Value");
	for (UINT i = 0; i < ExpContent.uMemWatchCount; ++i)
		SetRowTexter(uColumnIndex,i, CCharacter::FormatText(L"%08X", CCharacter::ReadDWORD(dwResult + i * 0x4)));
	return TRUE;
}

BOOL CExpressionAnalysis::ReadMemoryData() throw()
{
	if (!ExpContent.bRecord)
		return ReadMem_By_Rpn();

	bRunThread = TRUE;
	hThread = cbBEGINTHREADEX(NULL, NULL, _WorkThread, this, NULL, NULL);
	return TRUE;
}

DWORD WINAPI CExpressionAnalysis::_WorkThread(LPVOID lpParm)
{
	CExpressionAnalysis* pExp = reinterpret_cast<CExpressionAnalysis*>(lpParm);

	auto fnEqual = [](std::vector<DWORD>& v1, std::vector<DWORD>& v2)
	{
		for (UINT i = 0; i < v1.size(); ++i)
		{
			if (v1.at(i) != v2.at(i))
				return FALSE;
		}
		return TRUE;
	};

	UINT uColumnIndex = pExp->AddColumner(60,L"Address");
	for (UINT i = 0; i < pExp->ExpContent.uMemWatchCount; ++i)
		pExp->SetRowTexter(uColumnIndex, i, CCharacter::FormatText(L"->0x%X", i * 4));

	pExp->ExpContent.bDisablePrint = TRUE;

	auto SaveRpn = pExp->Rpn;
	std::vector<DWORD> VecLastValue;
	while (pExp->bRunThread)
	{
		DWORD dwResult = 0;
		pExp->Rpn = SaveRpn;
		if (!pExp->CalcResult_By_Rpn(&dwResult))
		{
			pExp->SetErrText(L"CalcResult_By_Rpn Error!");
			return FALSE;
		}

		if (VecLastValue.size() == 0)
		{
			for (UINT i = 0; i < pExp->ExpContent.uMemWatchCount; ++i)
				VecLastValue.push_back(CCharacter::ReadDWORD(dwResult + i * 4));

			uColumnIndex = pExp->AddColumner(80,L"Value");
			for (UINT i = 0; i < pExp->ExpContent.uMemWatchCount; ++i)
				pExp->SetRowTexter(uColumnIndex, i, CCharacter::FormatText(L"%08X", CCharacter::ReadDWORD(dwResult + i * 0x4)));
			continue;
		}

		std::vector<DWORD> VecValue;
		for (UINT i = 0; i < pExp->ExpContent.uMemWatchCount; ++i)
			VecValue.push_back(CCharacter::ReadDWORD(dwResult + i * 4));

		if (!fnEqual(VecLastValue, VecValue))
		{
			uColumnIndex = pExp->AddColumner(80,L"Value");
			for (UINT i = 0; i < VecLastValue.size(); ++i)
			{
				pExp->SetRowTexter(uColumnIndex, i, CCharacter::FormatText(L"%s%08X", VecLastValue.at(i) != VecValue.at(i) ? L"!" : L"", VecValue.at(i)));
			}
			VecLastValue = VecValue;
		}

		::Sleep(500);
	}

	return 0;
}

VOID CExpressionAnalysis::StopRecord() throw()
{
	bRunThread = FALSE;
	if (hThread != NULL)
	{
		::WaitForSingleObject(hThread, INFINITE);
		hThread = NULL;
	}
}

BOOL CExpressionAnalysis::IsNumber(_In_ cwstring& wsText) CONST throw()
{
	for (CONST auto& itm : wsText)
		if (!isdigit(itm))
			return FALSE;
	return TRUE;
}

VOID CExpressionAnalysis::Clear() throw()
{
	ExpContent.bKeep = ExpContent.bRecord = FALSE;
	ExpContent.emCmdType = em_Cmd_Type::em_Cmd_Type_Invalid;
	ExpContent.ExpressionVec.clear();
	ExpContent.uMemWatchCount = NULL;

	StopRecord();

	while (!Rpn.empty()){ Rpn.pop(); }


}

VOID CExpressionAnalysis::SetCleaner(_In_ std::function<void(void)> fnCleaner) throw()
{
	Cleaner = fnCleaner;
}

VOID CExpressionAnalysis::Help() CONST throw()
{
	Cleaner();
	
	static CONST std::vector<std::wstring> VecHelpText = 
	{
		L"can't exist space in expression, '1+1'=true, '1 + 1'=false"
		L"dd => Show Address to list for expression result", L"calc => calc expression", L"clear => clear list", L"? => show help text",
		L"-record => record every change data", L"-!record =>stop record every change data", L"-number => count in list for 'dd'",
		L"stoprecord => stop record and don't clear any data!"
	};

	std::for_each(VecHelpText.begin(), VecHelpText.end(), [this](cwstring& wsText)
	{
		Printer(wsText);
	});
}
