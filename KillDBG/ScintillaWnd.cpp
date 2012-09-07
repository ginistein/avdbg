// ScintillaWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "KillDBG.h"
#include "ScintillaWnd.h"


// CScintillaWnd

IMPLEMENT_DYNAMIC(CScintillaWnd, CWnd)

CScintillaWnd::CScintillaWnd()
{

}

CScintillaWnd::~CScintillaWnd()
{
}


BEGIN_MESSAGE_MAP(CScintillaWnd, CWnd)
END_MESSAGE_MAP()

BOOL CScintillaWnd::Create( DWORD dwExStyle, DWORD dwStyle,const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if (!CWnd::CreateEx(dwExStyle,_T("Scintilla"),_T(""),dwStyle,rect,pParentWnd,nID))
	{
		return FALSE;
	}
	LRESULT	Ret = SendMessage(SCI_GETDIRECTFUNCTION);
	m_fnDirect = (::SciFnDirect) Ret;
	m_ptrDirect = (sptr_t)SendMessage(SCI_GETDIRECTPOINTER);

	if (!m_fnDirect || !m_ptrDirect)
	{
		return FALSE;
	}

	this->SciFnDirect(SCI_CLEARALL);
	this->SciFnDirect(EM_EMPTYUNDOBUFFER);
	this->SciFnDirect(SCI_SETSAVEPOINT);
	this->SciFnDirect(SCI_CANCEL);
	this->SciFnDirect(SCI_SETUNDOCOLLECTION,0);
// 	this->SciFnDirect(SCI_SETCODEPAGE,SC_CP_UTF8);
	return TRUE;
}


// CScintillaWnd 消息处理程序


