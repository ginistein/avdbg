/************************************************************************
*                  Dasm.cpp                                             *
*                                                                       *
*              Borg Disassembler                                        *
*                    v2.28                                              *
*                 by Cronos                                             *
*                                                                       *
* Contributors:                                                         *
* Thanks to Mark Ogden for many bugfixes and mods for assembly under    *
* VC++, in its early stages (v2 beta builds). Also changes from         *
* wvsprintf to wsprintf to allow compilation under VC++.                *
* Thanks to Eugen Polukhin for some interesting alternate code and      *
* ideas around v2.11 and feedback on fonts, etc.                        *
* Thanks to Pawe3 Kunio for a lot of coding ideas and advice re C++     *
* usage around v2.22                                                    *
* Thanks to Howard Chu for some coding additions on default names, and  *
* more block functions                                                  *
************************************************************************/

/************************************************************************
* dasm.cpp                                                              *
* - this whole file is a declaration and global routine dumping area    *
* - it includes winmain and the main message callback routine, along    *
*   with initialisation code and the registry read/write code, odd      *
*   dialog boxes and helper functions for the main routines. The whole  *
*   file is a bit of a mish-mash of stuff which hasnt found its way     *
*   elsewhere.                                                          *
* - a lot of the code in here has grown, and i mean grown from the      *
*   ground up, and at some point it will require reorganisation, or     *
*   maybe that point was long ago....                                   *
************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "exeload.h"
#include "data.h"
#include "schedule.h"
#include "proctab.h"
#include "disasm.h"
#include "dasm.h"
#include "mainwind.h"
#include "gname.h"
#include "xref.h"
#include "relocs.h"
#include "srch.h"
#include "disio.h"
#include "range.h"
#include "decrypt.h"
#include "registry.h"
#include "help.h"
#include "user_dlg.h"
#include "database.h"
#include "dlg_ldop.h"
#include "user_fn.h"

/************************************************************************
* decaration dumping ground........                                     *
************************************************************************/
CRITICAL_SECTION g_hCs;
volatile bool KillThread;
volatile bool InThread;
globaloptions g_options;

fileloader g_floader;
dataseg dta;
schedule g_scheduler;
disasm g_dsm;
disio dio;
xref xrefs;
gname import;
gname expt;
gname name;
relocs reloc;
range blk;
decrypt decrypter;

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK shutbox(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
bool TestThread(void);
void optionsinit(void);
bool savefile_text(HWND hwnd,bool printaddrs,bool block);
void setupfont(void);
HFONT cf=NULL;

char winname[300];
HINSTANCE g_hInst;
HWND hwndStatusBar;
HANDLE ThreadHandle;
HWND g_hMainWnd;
DWORD ThreadId;
RECT mainwnd,mainwndsize,StatusWindowSize;
int cxChar,cyChar;
HMENU rmenu;
bool charinputenabled=false;

char szWinName[]="Dasm";
char program_name[]="Borg Disassembler";
char current_exe_name[MAX_PATH*2];

/************************************************************************
* WinMain..... it all starts here                                       *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
int WINAPI WinMain(HINSTANCE hThisInst,HINSTANCE hPrevInst,LPSTR lpszArgs,int nWinMode)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wcl;
	int cx,cy;
	INITCOMMONCONTROLSEX icc;

	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_BAR_CLASSES;

	InitCommonControlsEx(&icc);

	cx=GetSystemMetrics(SM_CXFULLSCREEN);
	cy=GetSystemMetrics(SM_CYFULLSCREEN);
	/* Define window class */
	wcl.hInstance=hThisInst;
	wcl.lpszClassName=szWinName;
	wcl.lpfnWndProc=MainWndProc;
	wcl.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;

	wcl.cbSize=sizeof(WNDCLASSEX);

	wcl.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wcl.hIconSm=LoadIcon(NULL,IDI_WINLOGO);

	wcl.hCursor=LoadCursor(NULL,IDC_ARROW);
	wcl.lpszMenuName=MAKEINTRESOURCE(main_menu);

	wcl.cbClsExtra=0;
	wcl.cbWndExtra=0;
	// this was causing the flickering.........damn thing
	wcl.hbrBackground=NULL;//(HBRUSH)(COLOR_APPWORKSPACE+1);

	g_hInst=hThisInst;

	/* Register window class */
	if(!RegisterClassEx(&wcl))
		return 0;

	/* Create window */
	sprintf(winname,"Borg Disassembler v%0d.%2d",(int)BORG_VER/100,BORG_VER%100);
	hwnd=CreateWindow(szWinName,winname,WS_OVERLAPPEDWINDOW,
		cx/2-320,cy/2-230,640,480,
		NULL,NULL,hThisInst,NULL);

	mainwndsize.top=0;
	mainwndsize.bottom=cy-1;
	mainwndsize.left=0;
	mainwndsize.right=cx-1;

	/* Display window */
	ShowWindow(hwnd,nWinMode);
	UpdateWindow(hwnd);

	/* Message Loop */
	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* dasm                                                                  *
* - dasm is the application main window.                                *
* - everything the main window does is in this routine (for now) and    *
*   where a response is quick it appears in one of its helper functions *
*   later in this file, otherwise it has been substantial enough to     *
*   warrant its own file and routines........                           *
* - this is long                                                        *
************************************************************************/
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	TEXTMETRIC tm;
	POINT point;
	lptr scrll;
	int killcount;
	g_hMainWnd=hwnd;
	RECT tmp_rect;
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case file_exit:
			SendMessage(g_hMainWnd,WM_CLOSE,0,0);
			break;
		case file_save:
			savefile_text(hwnd,true,false);
			break;
		case get_comment:
			getcomment();
			break;
		case jump_to:
			jumpto();
			break;
		case change_oep:
			changeoep();
			break;
		case save_database:
			savedb();
			break;
		case load_database:
			loaddb();
			break;
		case save_asm:
			savefile_text(hwnd,false,false);
			break;
		case block_saveasm:
			savefile_text(hwnd,false,true);
			break;
		case block_savetext:
			savefile_text(hwnd,true,true);
			break;
		case cm_decrypt:
			decrypterdialog();
			break;
		case file_open:
			newfile();
			break;
		case view_segment:
			segviewer();
			break;
		case view_names:
			namesviewer();
			break;
		case view_imports:
			importsviewer();
			break;
		case view_exports:
			exportsviewer();
			break;
		case view_xrefs:
			xrefsviewer();
			break;
		case make_code:
			g_scheduler.addtask(user_makecode,priority_userrequest,nlptr,NULL);
			break;
		case make_dword:
			g_scheduler.addtask(user_makedword,priority_userrequest,nlptr,NULL);
			break;
		case float_single:
			g_scheduler.addtask(user_makesingle,priority_userrequest,nlptr,NULL);
			break;
		case float_double:
			g_scheduler.addtask(user_makedouble,priority_userrequest,nlptr,NULL);
			break;
		case float_longdouble:
			g_scheduler.addtask(user_makelongdouble,priority_userrequest,nlptr,NULL);
			break;
		case make_word:
			g_scheduler.addtask(user_makeword,priority_userrequest,nlptr,NULL);
			break;
		case make_string:
			g_scheduler.addtask(user_makestring,priority_userrequest,nlptr,NULL);
			break;
		case pascal_string:
			g_scheduler.addtask(user_pascalstring,priority_userrequest,nlptr,NULL);
			break;
		case uc_string:
			g_scheduler.addtask(user_ucstring,priority_userrequest,nlptr,NULL);
			break;
		case up_string:
			g_scheduler.addtask(user_upstring,priority_userrequest,nlptr,NULL);
			break;
		case dos_string:
			g_scheduler.addtask(user_dosstring,priority_userrequest,nlptr,NULL);
			break;
		case general_string:
			g_scheduler.addtask(user_generalstring,priority_userrequest,nlptr,NULL);
			break;
		case argover_dec:
			g_scheduler.addtask(user_argoverdec,priority_userrequest,nlptr,NULL);
			break;
		case arg_single:
			g_scheduler.addtask(user_argsingle,priority_userrequest,nlptr,NULL);
			break;
		case argover_hex:
			g_scheduler.addtask(user_argoverhex,priority_userrequest,nlptr,NULL);
			break;
		case argnegate:
			g_scheduler.addtask(user_argnegate,priority_userrequest,nlptr,NULL);
			break;
		case offset_dseg:
			g_scheduler.addtask(user_argoveroffsetdseg,priority_userrequest,nlptr,NULL);
			break;
		case argover_char:
			g_scheduler.addtask(user_argoverchar,priority_userrequest,nlptr,NULL);
			break;
		case undefine_line:
			g_scheduler.addtask(user_undefineline,priority_userrequest,nlptr,NULL);
			break;
		case undefine_lines:
			g_scheduler.addtask(user_undefinelines,priority_userrequest,nlptr,NULL);
			break;
		case undefine_lines_long:
			g_scheduler.addtask(user_undefinelines_long,priority_userrequest,nlptr,NULL);
			break;
		case block_undefine:
			g_scheduler.addtask(user_undefineblock,priority_userrequest,nlptr,NULL);
			break;
		case block_view:
			blockview();
			break;
		case block_top:
			g_scheduler.addtask(user_marktopblock,priority_userrequest,nlptr,NULL);
			break;
		case block_bottom:
			g_scheduler.addtask(user_markbottomblock,priority_userrequest,nlptr,NULL);
			break;
		case line_jumpto:
			g_scheduler.addtask(user_jumpto,priority_userrequest,nlptr,NULL);
			break;
		case line_jumptoarg2:
			g_scheduler.addtask(user_jumptoarg2,priority_userrequest,nlptr,NULL);
			break;
		case Name_Location:
			namelocation();
			break;
		case help_short:
			DialogBox(g_hInst,MAKEINTRESOURCE(help_shortcuts),hwnd,(DLGPROC)helpshortcuts);
			break;
		case help_about:
			DialogBox(g_hInst,MAKEINTRESOURCE(D_help_about),hwnd,(DLGPROC)habox);
			break;
		case Jump_Back:
			g_scheduler.addtask(user_jumpback,priority_userrequest,nlptr,NULL);
			break;
		case main_search:
			searchengine();
			break;
		case search_again:
			searchmore();
			break;
		case set_bg_color:
			g_options.bgcolor=choosecolour(g_options.bgcolor);
			GetClientRect(g_hMainWnd,&tmp_rect);
			InvalidateRect(g_hMainWnd,&tmp_rect,true);
			g_scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
			break;
		case set_high_color:
			g_options.highcolor=choosecolour(g_options.highcolor);
			GetClientRect(g_hMainWnd,&tmp_rect);
			InvalidateRect(g_hMainWnd,&tmp_rect,true);
			g_scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
			break;
		case set_text_color:
			g_options.textcolor=choosecolour(g_options.textcolor);
			GetClientRect(g_hMainWnd,&tmp_rect);
			InvalidateRect(g_hMainWnd,&tmp_rect,true);
			g_scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
			break;
		case font_system:
			g_options.font=systemfont;
			setupfont();
			break;
		case font_courier:
			g_options.font=courierfont;
			setupfont();
			break;
		case font_courier10:
			g_options.font=courierfont10;
			setupfont();
			break;
		case font_courier12:
			g_options.font=courierfont12;
			setupfont();
			break;
		case font_ansi:
			g_options.font=ansifont;
			setupfont();
			break;
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
		}
		break;
	case WM_CHAR:
		if(charinputenabled)
			switch(wParam)
		{
			case 'c':
				g_scheduler.addtask(user_makecode,priority_userrequest,nlptr,NULL);
				break;
			case 'C':
				g_scheduler.addtask(user_argoverchar,priority_userrequest,nlptr,NULL);
				break;
			case 'd':
				g_scheduler.addtask(user_makedword,priority_userrequest,nlptr,NULL);
				break;
			case 'D':
				g_scheduler.addtask(user_argoverdec,priority_userrequest,nlptr,NULL);
				break;
			case 'H':
				g_scheduler.addtask(user_argoverhex,priority_userrequest,nlptr,NULL);
				break;
			case '-':
				g_scheduler.addtask(user_argnegate,priority_userrequest,nlptr,NULL);
				break;
			case 'n':
				namelocation();
				break;
			case ';':
				getcomment();
				break;
			case 'o':
				g_scheduler.addtask(user_argoveroffsetdseg,priority_userrequest,nlptr,NULL);
				break;
			case 'p':
				g_scheduler.addtask(user_pascalstring,priority_userrequest,nlptr,NULL);
				break;
			case 's':
				g_scheduler.addtask(user_makestring,priority_userrequest,nlptr,NULL);
				break;
			case 'u':
				g_scheduler.addtask(user_undefineline,priority_userrequest,nlptr,NULL);
				break;
			case 'U':
				g_scheduler.addtask(user_undefinelines,priority_userrequest,nlptr,NULL);
				break;
			case 'w':
				g_scheduler.addtask(user_makeword,priority_userrequest,nlptr,NULL);
				break;
			case 't':
				g_scheduler.addtask(user_marktopblock,priority_userrequest,nlptr,NULL);
				break;
			case 'b':
				g_scheduler.addtask(user_markbottomblock,priority_userrequest,nlptr,NULL);
				break;
			default:
				break;
		}
		break;
	case WM_LBUTTONDOWN:
		dio.setpos(HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		dio.setpos(HIWORD(lParam));
		point.x=LOWORD(lParam);
		point.y=HIWORD(lParam);
		ClientToScreen(g_hMainWnd,&point);
		TrackPopupMenu(rmenu,0,point.x,point.y,0,g_hMainWnd,NULL);
		break;
	case WM_PAINT:
		if(!KillThread)
			DoPaint(hwnd,cxChar,cyChar);
		else
			PaintBack(hwnd);
		ValidateRect(g_hMainWnd,NULL);
		break;
	case WM_CLOSE:
		if(MessageBox(g_hMainWnd,"Are you sure that you want to exit Borg ?\n\rHit Yes To Exit\n\rHit No to Stay","Borg Disassembler",
			MB_ICONEXCLAMATION|MB_YESNO)==IDNO)
			break;
		g_scheduler.stopthread();
		g_scheduler.addtask(quitborg,priority_quit,nlptr,NULL);
		KillThread=true;
		if(InThread)
			SetThreadPriority(ThreadHandle,THREAD_PRIORITY_TIME_CRITICAL);
		DestroyWindow(g_hMainWnd);
		return 0;
	case WM_DESTROY:
		save_reg_entries();
		KillThread=true;
		killcount=0;
		Sleep(0);
		SetPriorityClass(ThreadHandle,HIGH_PRIORITY_CLASS);
		if(InThread)
			while(TestThread())
			{
				killcount++;
				if(killcount>2)
				{
					// this is a nasty way of getting out.
					// sometimes the thread just will not exit nicely when its busy.
					if(TerminateThread(ThreadHandle,1))
					{
						CloseHandle(ThreadHandle);
						break;
					}
				}
			}
			DeleteCriticalSection(&g_hCs);
			PostQuitMessage(0);
			break;
	case WM_SIZE:
		if(wParam==SIZE_MAXIMIZED)
			g_options.winmax=true;
		else if (wParam==SIZE_RESTORED)
			g_options.winmax=false;
		mainwndsize.top=0;
		mainwndsize.left=0;
		mainwndsize.right=LOWORD(lParam);
		mainwndsize.bottom=HIWORD(lParam);
		GetWindowRect(hwndStatusBar,&StatusWindowSize);
		GetWindowRect(g_hMainWnd,&mainwnd);
		MoveWindow(hwndStatusBar,0,mainwndsize.bottom-StatusWindowSize.bottom+StatusWindowSize.top,
			mainwndsize.right,StatusWindowSize.bottom-StatusWindowSize.top,true);
		break;
	case WM_MOUSEWHEEL:
		if (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT)
		{
			scrll.assign(0,GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA);
			g_scheduler.addtask(hscroll,priority_userrequest,scrll,NULL);
		} 
		else
		{
			scrll.assign(0,-GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA);
			if(InThread)
				g_scheduler.addtask(scrolling,priority_userrequest,scrll,NULL);
		}
		break;
	case WM_VSCROLL:
		switch(LOWORD(wParam))
		{
		case SB_TOP:
			break;
		case SB_BOTTOM:
			break;
		case SB_LINEUP:
			scrll.assign(0,-1);
			if(InThread)
				g_scheduler.addtask(scrolling,priority_userrequest,scrll,NULL);
			break;
		case SB_LINEDOWN:
			scrll.assign(0,1);
			if(InThread)
				g_scheduler.addtask(scrolling,priority_userrequest,scrll,NULL);
			break;
		case SB_PAGEUP:
			scrll.assign(0,-mainwndsize.bottom/cyChar+1);
			if(InThread)
				g_scheduler.addtask(scrolling,priority_userrequest,scrll,NULL);
			break;
		case SB_PAGEDOWN:
			scrll.assign(0,mainwndsize.bottom/cyChar-1);
			if(InThread)
				g_scheduler.addtask(scrolling,priority_userrequest,scrll,NULL);
			break;
		case SB_THUMBPOSITION:
			scrll.assign(0,HIWORD(wParam));
			if(InThread)
				g_scheduler.addtask(vthumbposition,priority_userrequest,scrll,NULL);
			break;
		default:
			break;
		}
		break;
	case WM_HSCROLL:
		switch(LOWORD(wParam))
		{
		case SB_LINEUP:
			scrll.assign(0,-1);
			g_scheduler.addtask(hscroll,priority_userrequest,scrll,NULL);
			break;
		case SB_LINEDOWN:
			scrll.assign(0,1);
			g_scheduler.addtask(hscroll,priority_userrequest,scrll,NULL);
			break;
		case SB_PAGEUP:
			scrll.assign(0,-8);
			g_scheduler.addtask(hscroll,priority_userrequest,scrll,NULL);
			break;
		case SB_PAGEDOWN:
			scrll.assign(0,8);
			g_scheduler.addtask(hscroll,priority_userrequest,scrll,NULL);
			break;
		case SB_THUMBPOSITION:
			scrll.assign(0,HIWORD(wParam));
			if(InThread)
				g_scheduler.addtask(hthumbposition,priority_userrequest,scrll,NULL);
			break;
		default:
			break;
		}
		break;
	case WM_REPEATNAMEVIEW:
		namesviewer();
		break;
	case WM_REPEATXREFVIEW:
		xrefsviewer();
		break;
		// maximises window, used when the reg is read in at the start to maximise
		// the main window after initialisation of it
	case WM_MAXITOUT:
		ShowWindow(g_hMainWnd,SW_MAXIMIZE);
		break;
	case WM_CREATE:
		optionsinit();
		hdc=GetDC(hwnd);
		SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		GetTextMetrics(hdc,&tm);
		cxChar=tm.tmAveCharWidth;
		cyChar=tm.tmHeight+tm.tmExternalLeading;
		ReleaseDC(hwnd,hdc);
		InitializeCriticalSection(&g_hCs);
		hwndStatusBar=CreateStatusWindow(WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|CCS_BOTTOM,
			"No File Loaded",hwnd,2);
		GetWindowRect(hwndStatusBar,&StatusWindowSize);
		GetWindowRect(g_hMainWnd,&mainwnd);
		SetScrollRange(hwnd,SB_VERT,0,VERTSCROLLRANGE,false);
		SetScrollPos(hwnd,SB_VERT,0,false);
		KillThread=false;
		InThread=false;
		rmenu=LoadMenu(g_hInst,MAKEINTRESOURCE(right_click_menu));
		rmenu=GetSubMenu(rmenu,0);
		load_reg_entries();
		setupfont();
		break;
	case WM_KEYDOWN:
		if(!charinputenabled) break;
		switch(wParam)
		{
		case VK_HOME:
			SendMessage(hwnd,WM_VSCROLL,SB_TOP,0L);
			break;
		case VK_PRIOR:
			SendMessage(hwnd,WM_VSCROLL,SB_PAGEUP,0L);
			break;
		case VK_NEXT:
			SendMessage(hwnd,WM_VSCROLL,SB_PAGEDOWN,0L);
			break;
		case VK_DOWN:
			SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,0L);
			break;
		case VK_UP:
			SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,0L);
			break;
		case VK_LEFT:
			SendMessage(hwnd,WM_HSCROLL,SB_PAGEUP,0L);
			break;
		case VK_RIGHT:
			SendMessage(hwnd,WM_HSCROLL,SB_PAGEDOWN,0L);
			break;
		case VK_RETURN:
			if(GetKeyState(VK_SHIFT)&0x8000)
				g_scheduler.addtask(user_jumptoarg2,priority_userrequest,nlptr,NULL);
			else
				g_scheduler.addtask(user_jumpto,priority_userrequest,nlptr,NULL);
			break;
		case VK_ESCAPE:
			g_scheduler.addtask(user_jumpback,priority_userrequest,nlptr,NULL);
			break;
		case VK_F1:
			DialogBox(g_hInst,MAKEINTRESOURCE(help_shortcuts),hwnd,(DLGPROC)helpshortcuts);
			break;
		case VK_F3:
			searchmore();
			break;
		default:
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

/************************************************************************
* changemenus                                                           *
* erm..... everytime I added a menu function I just added another line  *
* in here :)                                                            *
************************************************************************/
void changemenus(void)
{
	HMENU hMenu;
	hMenu=GetMenu(g_hMainWnd);
	EnableMenuItem(hMenu,file_open,MF_GRAYED);
	EnableMenuItem(hMenu,file_save,MF_ENABLED);
	EnableMenuItem(hMenu,save_asm,MF_ENABLED);
	EnableMenuItem(hMenu,view_segment,MF_ENABLED);
	EnableMenuItem(hMenu,view_names,MF_ENABLED);
	EnableMenuItem(hMenu,view_xrefs,MF_ENABLED);
	EnableMenuItem(hMenu,view_imports,MF_ENABLED);
	EnableMenuItem(hMenu,view_exports,MF_ENABLED);
	EnableMenuItem(hMenu,make_code,MF_ENABLED);
	EnableMenuItem(hMenu,undefine_line,MF_ENABLED);
	EnableMenuItem(hMenu,undefine_lines,MF_ENABLED);
	EnableMenuItem(hMenu,undefine_lines_long,MF_ENABLED);
	EnableMenuItem(hMenu,line_jumpto,MF_ENABLED);
	EnableMenuItem(hMenu,line_jumptoarg2,MF_ENABLED);
	EnableMenuItem(hMenu,make_dword,MF_ENABLED);
	EnableMenuItem(hMenu,make_word,MF_ENABLED);
	EnableMenuItem(hMenu,make_string,MF_ENABLED);
	EnableMenuItem(hMenu,pascal_string,MF_ENABLED);
	EnableMenuItem(hMenu,uc_string,MF_ENABLED);
	EnableMenuItem(hMenu,up_string,MF_ENABLED);
	EnableMenuItem(hMenu,dos_string,MF_ENABLED);
	EnableMenuItem(hMenu,general_string,MF_ENABLED);
	EnableMenuItem(hMenu,Name_Location,MF_ENABLED);
	EnableMenuItem(hMenu,Jump_Back,MF_ENABLED);
	EnableMenuItem(hMenu,argover_dec,MF_ENABLED);
	EnableMenuItem(hMenu,argover_hex,MF_ENABLED);
	EnableMenuItem(hMenu,argover_char,MF_ENABLED);
	EnableMenuItem(hMenu,argnegate,MF_ENABLED);
	EnableMenuItem(hMenu,offset_dseg,MF_ENABLED);
	EnableMenuItem(hMenu,main_search,MF_ENABLED);
	EnableMenuItem(hMenu,save_database,MF_ENABLED);
	EnableMenuItem(hMenu,load_database,MF_GRAYED);
	EnableMenuItem(hMenu,get_comment,MF_ENABLED);
	EnableMenuItem(hMenu,block_top,MF_ENABLED);
	EnableMenuItem(hMenu,block_bottom,MF_ENABLED);
	EnableMenuItem(hMenu,block_view,MF_ENABLED);
	EnableMenuItem(hMenu,block_saveasm,MF_ENABLED);
	EnableMenuItem(hMenu,block_savetext,MF_ENABLED);
	EnableMenuItem(hMenu,block_undefine,MF_ENABLED);
	EnableMenuItem(hMenu,float_single,MF_ENABLED);
	EnableMenuItem(hMenu,float_double,MF_ENABLED);
	EnableMenuItem(hMenu,float_longdouble,MF_ENABLED);
	EnableMenuItem(hMenu,arg_single,MF_ENABLED);
	EnableMenuItem(hMenu,search_again,MF_ENABLED);
	EnableMenuItem(hMenu,cm_decrypt,MF_ENABLED);
	EnableMenuItem(hMenu,change_oep,MF_ENABLED);
	EnableMenuItem(hMenu,jump_to,MF_ENABLED);
	EnableMenuItem(rmenu,cm_decrypt,MF_ENABLED);
	EnableMenuItem(rmenu,arg_single,MF_ENABLED);
	EnableMenuItem(rmenu,float_single,MF_ENABLED);
	EnableMenuItem(rmenu,float_double,MF_ENABLED);
	EnableMenuItem(rmenu,float_longdouble,MF_ENABLED);
	EnableMenuItem(rmenu,block_top,MF_ENABLED);
	EnableMenuItem(rmenu,block_bottom,MF_ENABLED);
	EnableMenuItem(rmenu,make_code,MF_ENABLED);
	EnableMenuItem(rmenu,get_comment,MF_ENABLED);
	EnableMenuItem(rmenu,undefine_line,MF_ENABLED);
	EnableMenuItem(rmenu,undefine_lines,MF_ENABLED);
	EnableMenuItem(rmenu,undefine_lines_long,MF_ENABLED);
	EnableMenuItem(rmenu,line_jumpto,MF_ENABLED);
	EnableMenuItem(rmenu,line_jumptoarg2,MF_ENABLED);
	EnableMenuItem(rmenu,make_dword,MF_ENABLED);
	EnableMenuItem(rmenu,make_word,MF_ENABLED);
	EnableMenuItem(rmenu,make_string,MF_ENABLED);
	EnableMenuItem(rmenu,pascal_string,MF_ENABLED);
	EnableMenuItem(rmenu,uc_string,MF_ENABLED);
	EnableMenuItem(rmenu,up_string,MF_ENABLED);
	EnableMenuItem(rmenu,dos_string,MF_ENABLED);
	EnableMenuItem(rmenu,general_string,MF_ENABLED);
	EnableMenuItem(rmenu,Name_Location,MF_ENABLED);
	EnableMenuItem(rmenu,view_xrefs,MF_ENABLED);
	EnableMenuItem(rmenu,argover_dec,MF_ENABLED);
	EnableMenuItem(rmenu,argover_hex,MF_ENABLED);
	EnableMenuItem(rmenu,argover_char,MF_ENABLED);
	EnableMenuItem(rmenu,argnegate,MF_ENABLED);
	EnableMenuItem(rmenu,offset_dseg,MF_ENABLED);
	charinputenabled=true;
}

/************************************************************************
* Thread                                                                *
* - this is the secondary thread interface. It just keeps calling the   *
*   scheduler to process any items in the queue, until such time as the *
*   main thread wants to quit.                                          *
************************************************************************/
void Thread(PVOID pvoid)
{
	// needs to setup initial schedule list
	// continually monitor schedule list for jobs
	do
	{
		if(g_scheduler.process())
			if(!KillThread)
				StatusMessage("Processing Completed");
		Sleep(1);
	} while(!KillThread);
	InThread=false;
	ExitThread(0);
}

/************************************************************************
* savefile_text                                                         *
* - this is the savefile as text dialog which asks the user to select   *
*   a file for the save. The filedump is then controlled by disio       *
* - this is for text or asm saves                                       *
************************************************************************/
bool savefile_text(HWND hwnd,bool printaddrs,bool block)
{
	char savefname[MAX_PATH*2];
	HANDLE sbox;
	if(g_scheduler.sizelist())
	{
		MessageBox(g_hMainWnd,"There are still items to process yet","Borg Warning",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	savefname[0]=0;
	getfiletosave(savefname);
	if(savefname[0])
	{
		sbox=CreateDialog(g_hInst,MAKEINTRESOURCE(save_box),g_hMainWnd,(DLGPROC)savemessbox);
		if(block)
			dio.dumpblocktofile(savefname,printaddrs);
		else
			dio.dumptofile(savefname,printaddrs);
		DestroyWindow((HWND)sbox);
	}
	return 0;
}

/************************************************************************
* StatusMessage                                                         *
* - output a simple message to the status bar                           *
************************************************************************/
void StatusMessage(char *msg)
{
	/*EnterCriticalSection(&cs);*/     // was causing locks (possibly)
	Status_SetText(hwndStatusBar,0,0,msg);
	/*LeaveCriticalSection(&cs);    */
}

/************************************************************************
* StatusMessageNItems                                                   *
* - shows how many items there are left to process                      *
************************************************************************/
void StatusMessageNItems(dword nolistitems)
{
	static char buff[40];
	buff[39]=0; // just in case
	wsprintf(buff,"Items to Process :%lu",nolistitems);
	StatusMessage(buff);
}

/************************************************************************
* optionsinit                                                           *
* - initialises the global options struct with some default values      *
************************************************************************/
void optionsinit(void)
{
	g_options.loaddebug=true;
	g_options.mode16=false;
	g_options.mode32=false;
	g_options.loaddata=true;
	g_options.loadresources=false;
	g_options.cfa=true;
	g_options.demangle=true;
	g_options.processor=PROC_PENTIUM;
	g_options.codedetect=CD_PUSHBP|CD_EAXFROMESP|CD_MOVEAX;
	g_options.bgcolor=GetSysColor(COLOR_APPWORKSPACE);
	g_options.highcolor=RGB(0,255,0);
	g_options.textcolor=0;
	g_options.font=ansifont;
	g_options.readonly=false;
	g_options.winmax=false;
}

/************************************************************************
* setupfont                                                             *
* - handles the setting up of a font (like selecting the object for     *
*   window painting and setting checkmarks on the menu, etc)            *
************************************************************************/
void setupfont(void)
{
	HDC hdc;
	TEXTMETRIC tm;
	RECT tmp_rect;
	HMENU hMenu;
	hMenu=GetMenu(g_hMainWnd);
	hdc=GetDC(g_hMainWnd);
	CheckMenuItem(hMenu,font_ansi,MF_UNCHECKED);
	CheckMenuItem(hMenu,font_system,MF_UNCHECKED);
	CheckMenuItem(hMenu,font_courier,MF_UNCHECKED);
	CheckMenuItem(hMenu,font_courier10,MF_UNCHECKED);
	CheckMenuItem(hMenu,font_courier12,MF_UNCHECKED);
	switch(g_options.font)
	{
	case ansifont:
		SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		CheckMenuItem(hMenu,font_ansi,MF_CHECKED);
		break;
	case systemfont:
		SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
		CheckMenuItem(hMenu,font_system,MF_CHECKED);
		break;
	case courierfont:
		if(cf!=NULL)
			DeleteObject(cf);
		cf=CreateFont(-MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72),0,0,0,FW_LIGHT,0,0,0,0,0,0,0,0,"Courier New");
		if(cf==NULL)
			SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		else
			SelectObject(hdc,cf);
		CheckMenuItem(hMenu,font_courier,MF_CHECKED);
		break;
	case courierfont10:
		if(cf!=NULL)
			DeleteObject(cf);
		cf=CreateFont(-MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72),0,0,0,FW_LIGHT,0,0,0,0,0,0,0,0,"Courier New");
		if(cf==NULL)
			SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		else
			SelectObject(hdc,cf);
		CheckMenuItem(hMenu,font_courier10,MF_CHECKED);
		break;
	case courierfont12:
		if(cf!=NULL)
			DeleteObject(cf);
		cf=CreateFont(-MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72),0,0,0,FW_LIGHT,0,0,0,0,0,0,0,0,"Courier New");
		if(cf==NULL)
			SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		else
			SelectObject(hdc,cf);
		CheckMenuItem(hMenu,font_courier12,MF_CHECKED);
		break;
	default:
		SelectObject(hdc,GetStockObject(ANSI_FIXED_FONT));
		CheckMenuItem(hMenu,font_ansi,MF_CHECKED);
		break;
	}
	GetTextMetrics(hdc,&tm);
	cxChar=tm.tmAveCharWidth;
	cyChar=tm.tmHeight+tm.tmExternalLeading;
	ReleaseDC(g_hMainWnd,hdc);
	GetClientRect(g_hMainWnd,&tmp_rect);
	InvalidateRect(g_hMainWnd,&tmp_rect,true);
	g_scheduler.addtask(windowupdate,priority_window,nlptr,NULL);
}

/************************************************************************
* TestThread                                                            *
* - this is used on exit to wait for the secondary thread to finish.    *
* - I try every possible way of getting the thread to exit normally     *
*   before we eventually kill it in the mainwindow, WM_DESTROY handler  *
************************************************************************/
bool TestThread(void)
{
	HWND sbox;
	bool ttest;
	sbox=CreateDialog(g_hInst,MAKEINTRESOURCE(Borg_Shutdown),g_hMainWnd,(DLGPROC)shutbox);
	if(InThread)
		Sleep(1000);
	DestroyWindow(sbox);
	//EnterCriticalSection(&cs);
	ttest=InThread;
	//LeaveCriticalSection(&cs);
	return ttest;
}


