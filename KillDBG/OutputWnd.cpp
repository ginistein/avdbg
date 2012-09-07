#include "StdAfx.h"
#include "OutputWnd.h"


COutputWnd::COutputWnd(void)
{
}


COutputWnd::~COutputWnd(void)
{
}

BOOL COutputWnd::PreTranslateMessage( MSG* pMsg )
{
	if (pMsg->message == WM_CHAR
		||pMsg->message == WM_KEYDOWN
		||pMsg->message == WM_KEYUP)		//不允许用户输入，过滤掉所有键盘消息
	{
		return TRUE;
	}
	return FALSE;
}