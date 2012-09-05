// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "ChildView.h"
#include "ScintillaWnd.h"

class CMainFrame : public CXTPFrameWnd
{

public:
	CMainFrame();
	virtual ~CMainFrame();
protected:
	DECLARE_DYNAMIC(CMainFrame)

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	HANDLE	m_hDebugThread;
	CString	m_strExePath;
	CString	m_strParam;

	CXTPOfficeBorder<CScintillaWnd> m_wndRegister;
	//CXTPOfficeBorder<CEdit> m_wndProperties;
	CXTPOfficeBorder<CScintillaWnd> m_wndDisasm;
	CXTPOfficeBorder<CScintillaWnd> m_wndMemory;
	CXTPOfficeBorder<CScintillaWnd> m_wndStack;
	CXTPOfficeBorder<CScintillaWnd> m_wndOutput;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CXTPStatusBar  m_wndStatusBar;
	CChildView    m_wndView;
	CXTPDockingPaneManager m_paneManager;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnCustomize();
	afx_msg LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnButtondiswnd();
	afx_msg void OnViewRegister();
	afx_msg void OnViewMemory();
	afx_msg void OnViewStack();
	afx_msg void OnViewOutput();
//	afx_msg void OnDestroy();
	afx_msg void OnFileOpen();
	// 系统是否退出
	bool m_bExit;
	// 创建可停靠窗口
	BOOL SetupDockPane(void);
};
