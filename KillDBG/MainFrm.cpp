// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "KillDBG.h"

#include "DBGKernel.h"
#include "MainFrm.h"
#include "scintilla/Scintilla.h"
#include <scintilla/SciLexer.h>
#include "FileOpenDlg.h"
#include "DebugThread.h"
#include "AttachProcessDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_COMMAND(XTP_ID_CUSTOMIZE, OnCustomize)
	ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)
	ON_COMMAND(ID_BUTTONDISWND, &CMainFrame::OnButtondiswnd)
	ON_COMMAND(ID_VIEW_REGISTER, &CMainFrame::OnViewRegister)
	ON_COMMAND(ID_VIEW_MEMORY, &CMainFrame::OnViewMemory)
	ON_COMMAND(ID_VIEW_STACK, &CMainFrame::OnViewStack)
	ON_COMMAND(ID_VIEW_OUTPUT, &CMainFrame::OnViewOutput)
//	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_FILE_ATTACH, &CMainFrame::OnFileAttach)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_ATTACH, &CMainFrame::OnUpdateFileAttach)
	ON_MESSAGE(WM_USER_DEBUGSTOP, &CMainFrame::OnDebugStop)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

static UINT uHideCmds[] =
{
	ID_FILE_PRINT,
	ID_FILE_PRINT_PREVIEW,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	:m_pDbgKrnl(NULL)
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}


	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Initialize the command bars
	if (!InitCommandBars())
		return -1;

	// Get a pointer to the command bars object.
	CXTPCommandBars* pCommandBars = GetCommandBars();
	if(pCommandBars == NULL)
	{
		TRACE0("Failed to create command bars object.\n");
		return -1;      // fail to create
	}

	// Add the menu bar
	CXTPCommandBar* pMenuBar = pCommandBars->SetMenu(
		_T("Menu Bar"), IDR_MAINFRAME);
	if(pMenuBar == NULL)
	{
		TRACE0("Failed to create menu bar.\n");
		return -1;      // fail to create
	}

	// Create ToolBar
	CXTPToolBar* pToolBarNoraml = pCommandBars->Add(_T("Standard"), xtpBarTop);
	if (!pToolBarNoraml || !pToolBarNoraml->LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	CXTPToolBar* pToolBarWnd = pCommandBars->Add(_T("Window"), xtpBarTop);
	if (!pToolBarWnd || !pToolBarWnd->LoadToolBar(IDR_TOOLBARWND))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	// Set Office 2003 Theme
	CXTPPaintManager::SetTheme(xtpThemeOffice2003);

	// Hide array of commands
	pCommandBars->HideCommands(uHideCmds, _countof(uHideCmds));

	// Set "Always Show Full Menus" option to the FALSE
	pCommandBars->GetCommandBarsOptions()->bAlwaysShowFullMenus = FALSE;

	pCommandBars->GetShortcutManager()->SetAccelerators(IDR_MAINFRAME);

	// Load the previous state for toolbars and menus.
	LoadCommandBars(_T("CommandBars"));

	SetupDockPane();

	return 0;
}

BOOL CMainFrame::SetupDockPane(void)
{
	// Initialize the docking pane manager and set the
	// initial them for the docking panes.  Do this only after all
	// control bars objects have been created and docked.
	m_paneManager.InstallDockingPanes(this);
	m_paneManager.SetDefaultPaneOptions(xtpPaneHasMenuButton);
	m_paneManager.HideClient(TRUE);		//不显示客户区
	m_paneManager.SetAlphaDockingContext(TRUE);		//显示透明停靠Context
	m_paneManager.SetShowDockingContextStickers(TRUE);		//显示停靠导航
	m_paneManager.SetTheme(xtpPaneThemeVisualStudio2008);	//VS2008风格
	m_paneManager.SetStickyFloatingFrames(TRUE);	//可以边缘吸附

	// Create docking panes.
	CXTPDockingPane* pPaneRegister = m_paneManager.CreatePane(IDR_PANE_REGISTER, CRect(0, 0,200, 120), xtpPaneDockLeft);
	CXTPDockingPane* pPaneDisasm = m_paneManager.CreatePane(IDR_PANE_DISASM, CRect(0, 0,200, 120), xtpPaneDockRight, pPaneRegister);
	CXTPDockingPane* pPaneMemory = m_paneManager.CreatePane(IDR_PANE_MEMORY, CRect(0, 0,200, 120), xtpPaneDockBottom);
	CXTPDockingPane* pPaneOutput = m_paneManager.CreatePane(IDR_PANE_OUTPUT, CRect(0, 0,200, 120), xtpPaneDockBottom, pPaneMemory);
	CXTPDockingPane* pPaneStack = m_paneManager.CreatePane(IDR_PANE_STACK, CRect(0, 0,200, 120), xtpPaneDockBottom);

	//创建显示反汇编代码的窗口
	if (m_wndDisasm.Create(NULL,WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0))
	{
		//关键字
		const char* szKeywords= "ja jae jb jbe jc jcxz je jg jge "
			"jl jle jmp jna jnae jnb jnbe jnc jne jng jnge jnl jnle "
			"jno jnp jns jnz jo jp jpe jpo js jz jcxz jecxz jrcxz "
			"loop loope loopne loopz loopnz call ret add sub adc "
			"sbb neg cmp inc dec and or xor not test shl shr sal "
			"sar shld shrd rol ror rcl rcr cbw cwd cwde cdq cdqe "
			"cqo bsf bsr bt btc btr bts idiv imul div mul bswap nop"
			"lea mov movsx movsxd movzx xlatb bound xchg xadd "
			"cmpxchg cmpxchg8b cmpxchg16b push pop pushad popad "
			"pushf popf pushfd popfd pushfq popfq seta setae setb "
			"setbe setc sete setg setge setl setle setna setnae "
			"setnb setnbe setnc setne setng setnge setnl setnle "
			"setno setnp setns setnz seto setp setpe setpo sets "
			"setz salc clc cld stc std cmc lahf sahf cmovo cmovno "
			"cmovb cmovc cmovnae cmovae cmovnb cmovnc cmove cmovz "
			"cmovne cmovnz cmovbe cmovna cmova cmovnbe cmovs "
			"cmovns cmovp cmovpe cmovnp cmovpo cmovl cmovnge "
			"cmovge cmovnl cmovle cmovng cmovg cmovnle"; 
		//m_wndDisasm.SciFnDirect(SCI_SETREADONLY,1);
		// 							m_wndDisasm.SciFnDirect(SCI_SETHSCROLLBAR,0);	//去掉横向滚动条
		// 							m_wndDisasm.SciFnDirect(SCI_STYLESETFONT, STYLE_DEFAULT,(sptr_t)"Courier New");	//设置字体
		// 							m_wndDisasm.SciFnDirect(SCI_STYLESETSIZE, STYLE_DEFAULT,16);	//字体大小
		// 							m_wndDisasm.SciFnDirect(SCI_STYLECLEARALL);
		// 
		m_wndDisasm.SciFnDirect(SCI_SETLEXER, SCLEX_ASM);	//ASM语法
		// 							m_wndDisasm.SciFnDirect(SCI_SETKEYWORDS, 0, (sptr_t)szKeyWords);//设置关键字
		// 							m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3); //字符串 
		// 							m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3); //字符 

		//C++语法解析 
		//m_wndDisasm.SciFnDirect(SCI_SETLEXER, SCLEX_CPP); 
		m_wndDisasm.SciFnDirect(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords);//设置关键字 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_DEFAULT, 0x00FF0000);	//普通字符 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_COMMENT, 0x00008000);	//注释 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_COMMENTBLOCK, 0x00008000);	//注释块 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_NUMBER, 0x00990040);	//数字 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_STRING, 0x00000088);	//字符串 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_OPERATOR, 0x00000000);	//操作符 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_IDENTIFIER, 0x00008000);	//？ 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_CPUINSTRUCTION, 0x00FF001A);	//指令 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_MATHINSTRUCTION, 0x00008000);	//数学指令？？ 
		m_wndDisasm.SciFnDirect(SCI_STYLESETFORE, SCE_ASM_REGISTER, 0x0000FFFF);	//寄存器 

		m_wndDisasm.SciFnDirect(SCI_SETCARETLINEVISIBLE, TRUE); 
		m_wndDisasm.SciFnDirect(SCI_SETCARETLINEBACK, 0xb0ffff); 
	}

	//寄存器窗口
	if (m_wndRegister.Create(NULL,WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0))
	{
		m_wndRegister.SciFnDirect(SCI_SETMARGINWIDTHN,1,0);	//去掉Margin
		m_wndRegister.SciFnDirect(SCI_SETREADONLY,1);
		m_wndRegister.SciFnDirect(SCI_SETHSCROLLBAR,0);
	}

	//内存窗口
	if (m_wndMemory.Create(NULL,WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0))
	{
		m_wndMemory.SciFnDirect(SCI_SETMARGINWIDTHN,1,80);	//加宽Margin用于显示地址
		m_wndMemory.SciFnDirect(SCI_SETREADONLY,1);
		m_wndMemory.SciFnDirect(SCI_SETHSCROLLBAR,0);
	}

	//堆栈窗口
	if (m_wndStack.Create(NULL,WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0))
	{
		m_wndStack.SciFnDirect(SCI_SETMARGINWIDTHN,1,0);	//去掉Margin
		m_wndStack.SciFnDirect(SCI_SETREADONLY,1);
		m_wndStack.SciFnDirect(SCI_SETHSCROLLBAR,0);
	}

	//输出窗口
	if (m_wndOutput.Create(NULL,WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0))
	{
		m_wndOutput.SciFnDirect(SCI_SETMARGINWIDTHN,1,0);	//去掉Margin
		//m_wndOutput.SciFnDirect(SCI_SETREADONLY,1);
		m_wndOutput.SciFnDirect(SCI_SETHSCROLLBAR,0);
	}

	// Set the icons for the docking pane tabs.
	int nIDIcons[] = {IDR_PANE_REGISTER, IDR_PANE_DISASM};
	m_paneManager.SetIcons(IDB_BITMAP_ICONS, nIDIcons,_countof(nIDIcons), RGB(0, 255, 0));

	// Load the previous state for docking panes.
	CXTPDockingPaneLayout layoutNormal(&m_paneManager);
	if (layoutNormal.Load(_T("NormalLayout")))
	{
		m_paneManager.SetLayout(&layoutNormal);
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



void CMainFrame::OnClose()
{

	// Save the current state for toolbars and menus.
	SaveCommandBars(_T("CommandBars"));

	// Save the current state for docking panes.
	CXTPDockingPaneLayout layoutNormal(&m_paneManager);
	m_paneManager.GetLayout(&layoutNormal);
	layoutNormal.Save(_T("NormalLayout"));

	CFrameWnd::OnClose();
}


void CMainFrame::OnCustomize()
{
	// Get a pointer to the command bars object.
	CXTPCommandBars* pCommandBars = GetCommandBars();
	if(pCommandBars != NULL)
	{
		// Instanciate the customize dialog object.
		CXTPCustomizeSheet dlg(pCommandBars);

		// Add the keyboard page to the customize dialog.
		CXTPCustomizeKeyboardPage pageKeyboard(&dlg);
		dlg.AddPage(&pageKeyboard);
		pageKeyboard.AddCategories(IDR_MAINFRAME);

		// Add the options page to the customize dialog.
		CXTPCustomizeOptionsPage pageOptions(&dlg);
		dlg.AddPage(&pageOptions);

		// Add the commands page to the customize dialog.
		CXTPCustomizeCommandsPage* pCommands = dlg.GetCommandsPage();
		pCommands->AddCategories(IDR_MAINFRAME);

		// Use the command bar manager to initialize the
		// customize dialog.
		pCommands->InsertAllCommandsCategory();
		pCommands->InsertBuiltInMenus(IDR_MAINFRAME);
		pCommands->InsertNewMenuCategory();

		// Dispaly the dialog.
		dlg.DoModal();
	}
}

LRESULT CMainFrame::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_DPN_SHOWWINDOW)
	{
		CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
		if (!pPane->IsValid())
		{
			switch (pPane->GetID())
			{
			case IDR_PANE_REGISTER:
				pPane->Attach(&m_wndRegister);
				break;
			case IDR_PANE_DISASM:
				pPane->Attach(&m_wndDisasm);
				break;
			case IDR_PANE_MEMORY:
				pPane->Attach(&m_wndMemory);
				break;
			case IDR_PANE_OUTPUT:
				pPane->Attach(&m_wndOutput);
				break;
			case IDR_PANE_STACK:
				pPane->Attach(&m_wndStack);
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
}


void CMainFrame::OnButtondiswnd()
{
	m_paneManager.ShowPane(IDR_PANE_DISASM);
}


void CMainFrame::OnViewRegister()
{
	m_paneManager.ShowPane(IDR_PANE_REGISTER);
}


void CMainFrame::OnViewMemory()
{
	m_paneManager.ShowPane(IDR_PANE_MEMORY);
}


void CMainFrame::OnViewStack()
{
	m_paneManager.ShowPane(IDR_PANE_STACK);
}


void CMainFrame::OnViewOutput()
{
	m_paneManager.ShowPane(IDR_PANE_OUTPUT);
}


//void CMainFrame::OnDestroy()
//{
//	CXTPFrameWnd::OnDestroy();
//}


void CMainFrame::OnFileOpen()
{
	//CFileDialog	dlg(TRUE,)
	CFileOpenDlg	dlg(this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}	
	m_pDbgKrnl = new CDBGKernel();
	if (!m_pDbgKrnl->CreateDbgProcess(dlg.m_strPath,dlg.m_strParam,dlg.m_strRunDir))
	{
		MessageBox(_T("创建调试进程失败"),NULL,MB_OK|MB_ICONERROR);
	}
}


// 创建可停靠窗口


void CMainFrame::OnFileAttach()
{
	CAttachProcessDlg	dlg(this);

	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	m_pDbgKrnl = new CDBGKernel();
	if (!m_pDbgKrnl->AttachDbgProcess(dlg.m_dwPID))
	{
		MessageBox(_T("附加指定进程失败"),NULL,MB_OK|MB_ICONERROR);
	}
}

LRESULT CMainFrame::OnDebugStop( WPARAM wParam, LPARAM lParam )
{
	if (m_pDbgKrnl)
	{
		delete m_pDbgKrnl;
		m_pDbgKrnl = NULL;
	}
	return FALSE;
}

void CMainFrame::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	if (m_pDbgKrnl)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
	}
}


void CMainFrame::OnUpdateFileAttach(CCmdUI *pCmdUI)
{
	OnUpdateFileOpen(pCmdUI);
}
