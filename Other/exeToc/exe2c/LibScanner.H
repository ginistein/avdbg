///////////////////////////////////////////////////////////////
//
// LibScanner.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//
///////////////////////////////////////////////////////////////
//#include "..\..\LibScanner\LibScanner.H"

#pragma once

#include "..\I_KSUNKNOWN\KsFrame.h"

#pragma warning(disable:4200)

typedef struct REFSYMBOL
{
	CHAR    RefSymbol[_MAX_PATH];	//引用符号
	WORD	RefType;				//such as IMAGE_REL_I386_REL32
	DWORD	RefOffset;				//引用在函数中的偏移
} *PREFSYMBOL;

typedef struct tagFUNCTION_SYMBOL
{
	DWORD       dwFuncLen;
	BYTE*		FunRawData;
	CHAR		ObjName[_MAX_PATH];		// 对象名称
	CHAR		FunctionName[_MAX_PATH];// 函数名称
	DWORD		RefCount;			// 引用的符号次数
	REFSYMBOL	RefInfo[];			// 引用信息
}FUNCTION_SYMBOL,* PFUNCTION_SYMBOL;

class I_LIBSCANNER : public I_KSUNKNOWN
{
public:
	//Add interface here
    
	virtual BOOL __stdcall ScanLib(LPCSTR szLib)=0;
	virtual PFUNCTION_SYMBOL __stdcall GetFunctionInfo(LPCSTR szFun)=0;
    virtual PSTR __stdcall CheckIfLibFunc(PCBYTE phead) = 0;
};

KS_DECLARE_INTERFACE(LibScanner, LIBSCANNER)

extern I_LIBSCANNER* g_LIBSCANNER;

/*	这里放接口函数的详细解释
	
*/	
