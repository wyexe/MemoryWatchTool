#ifndef __MEMORYWATCHTOOL_EXPRESSION_EXPRESSIONANALYSIS_H__
#define __MEMORYWATCHTOOL_EXPRESSION_EXPRESSIONANALYSIS_H__

#include <stack>
#include <string>
#include <sstream>
#include <MyTools/Character.h>
#include <MyTools/CLPublic.h>
#include <MyTools/ClassInstance.h>

class CExpressionAnalysis
{
private:
	enum em_Content_Type
	{
		em_Content_Type_None,
		// 1
		em_Content_Type_Number,
		// (
		em_Content_Type_LeftBracket,
		// )
		em_Content_Type_RightBracket,
		// [
		em_Content_Type_LeftAddress,
		// ]
		em_Content_Type_RightAddress,
		// +
		em_Content_Type_Symbol_Add,
		// -
		em_Content_Type_Symbol_Sub,
		// *
		em_Content_Type_Symbol_Mul,
		// /
		em_Content_Type_Symbol_Div,
		// %
		em_Content_Type_Symbol_Mod,
		// ^
		em_Content_Type_Symbol_ExOr,
		// |
		em_Content_Type_Symbol_InOr,
		// ~
		em_Content_Type_Symbol_Comp,
		// &
		em_Content_Type_Symbol_And,
		// <<
		em_Content_Type_Symbol_LeftShift,
		// >>
		em_Content_Type_Symbol_RightShift
	};

	enum em_Text_Type
	{
		em_Text_Type_Invalid,
		em_Text_Type_Number,
		em_Text_Type_Symbol,
		em_Text_Type_Bracket,
		em_Text_Type_NULL
	};

	enum em_Cmd_Type
	{
		em_Cmd_Type_Invalid,
		em_Cmd_Type_dd,
		em_Cmd_Type_Calc,
		em_Cmd_Type_Clear,
		em_Cmd_Type_Help,
		em_Cmd_Type_StopRecord
	};

	struct ExpressionInfo
	{
		em_Content_Type emContentType;
		std::wstring wsText;
		DWORD GetHex() CONST
		{
			return wcstol(wsText.c_str(), nullptr, 16);
		}
	};

	struct ExpressionLoopSize
	{
		UINT uMinIndex;
		UINT uMaxIndex;
		ExpressionLoopSize() : uMinIndex(NULL), uMaxIndex(NULL) {}
	};

	struct ExpressionContent
	{
		em_Cmd_Type emCmdType;
		std::vector<ExpressionInfo> ExpressionVec;
		UINT uMemWatchCount;
		BOOL bRecord;
		BOOL bKeep;
		BOOL bDisablePrint;
		DWORD dwAddress;
		ExpressionContent() : emCmdType(em_Cmd_Type::em_Cmd_Type_Invalid), uMemWatchCount(NULL), bRecord(FALSE), bKeep(FALSE), dwAddress(NULL), bDisablePrint(FALSE)
		{

		}
	};
public:
	CExpressionAnalysis();
	~CExpressionAnalysis() = default;

	// Analysis Expression
	BOOL Analysis(_In_ cwstring& wsText) throw();

	VOID SetPrinter(_In_ std::function<VOID(CONST std::wstring)> fnPrinter) throw();

	VOID SetColumner(_In_ std::function<UINT(int,cwstring&)> fnAddColumner) throw();

	VOID SetRowText(_In_ std::function<void(int,int, cwstring&)> fnSetRowTexter) throw();

	VOID SetCleaner(_In_ std::function<void(void)> fnCleaner) throw();

	VOID			StopRecord() throw();
private:
	//
	em_Cmd_Type		GetCmdType(_In_ cwstring& wsText) throw();

	// 
	em_Text_Type	GetTextType(_In_ CONST UINT& uIndex) throw();

	// 
	em_Content_Type GetContentType(_In_ cwstring& wsSymbolText, _In_ em_Text_Type emTextType) throw();

	// 
	BOOL			GetSymbolText(_In_  _Out_ UINT& uIndex, _Out_ std::wstring& wsSymbolText) throw();

	// 
	BOOL			GetExpression() throw();

	//
	int				GetPriority(_In_ em_Content_Type emContentType) throw();

	//
	BOOL			CompPrioity(_In_ em_Content_Type emTextType1, _In_ em_Content_Type emTextType2) throw();

	//
	BOOL			GetRpn() throw();

	//
	DWORD			CalcResult_By_Parm(_In_ DWORD dwNumberLeft, _In_ DWORD dwNumberRight, _In_ em_Content_Type emSymbolType) throw();

	//
	BOOL			CalcResult_By_Rpn(_Out_opt_ DWORD* pdwResult) throw();

	//
	VOID			SetErrText(_In_ LPCWSTR pwszFormatText, ...) throw();

	//
	BOOL			ReadMem_By_Rpn() throw();

	//				
	BOOL			ReadMemoryData() throw();

	//
	BOOL			IsNumber(_In_ cwstring& wsText) CONST throw();

	//
	VOID			Clear() throw();

	//			
	VOID			Help() CONST throw();

	static DWORD WINAPI _WorkThread(LPVOID lpParm);
private:
	std::stack<ExpressionInfo>	Rpn;
	ExpressionContent			ExpContent;
	std::wstring				wsExpText;
	std::function<VOID(CONST std::wstring)> Printer;
	std::function<UINT(int, cwstring&)>	AddColumner;
	std::function<void(int,int, cwstring&)> SetRowTexter;
	std::function<VOID(VOID)>	Cleaner;
	HANDLE						hThread;
	BOOL						bRunThread;
};



#endif // !__MEMORYWATCHTOOL_EXPRESSION_EXPRESSIONANALYSIS_H__
