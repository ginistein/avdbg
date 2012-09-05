#pragma once

#include <Scintilla.h>
// CScintillaWnd

class CScintillaWnd : public CWnd
{
	DECLARE_DYNAMIC(CScintillaWnd)

public:
	CScintillaWnd();
	virtual ~CScintillaWnd();

protected:
	DECLARE_MESSAGE_MAP()

private:
	::SciFnDirect m_fnDirect;
	sptr_t m_ptrDirect;

public:
	virtual BOOL Create(DWORD dwExStyle, DWORD dwStyle,const RECT& rect, CWnd* pParentWnd, UINT nID);
	sptr_t SciFnDirect(unsigned int iMessage, uptr_t wParam=0, sptr_t lParam=0)
	{
		return m_fnDirect(m_ptrDirect,iMessage,wParam,lParam);
	}
};


