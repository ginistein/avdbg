#pragma once
#include "scintillawnd.h"
class COutputWnd :
	public CScintillaWnd
{
public:
	COutputWnd(void);
	virtual ~COutputWnd(void);

	virtual BOOL PreTranslateMessage( MSG* pMsg );
};

