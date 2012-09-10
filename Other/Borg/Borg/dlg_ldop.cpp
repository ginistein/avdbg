/************************************************************************
*                  dlg_ldopt.cpp                                        *
* Contains the dialog routines for the load file dialogboxes            *
************************************************************************/

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "exeload.h"
#include "proctab.h"
#include "dasm.h"
#include "disasm.h"
#include "help.h"
#include "debug.h"

/************************************************************************
* forward declarations                                                  *
************************************************************************/
BOOL CALLBACK CheckOptDlgProc(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK moreoptions(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
* checktypebox                                                          *
* - after a file has been chosen to load and before the file is loaded  *
*   this is displayed for the user to set options for analysis, file    *
*   type, etc.                                                          *
************************************************************************/
//dialog proc for verifying type and
// initial options
BOOL CALLBACK CheckOptDlgProc(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	char segtext[20],offstext[20];
	static int exetype;
	int i;
	dword segd;
	switch(message)
	{
	case WM_INITDIALOG:
		exetype=g_floader.getexetype();
		i=0;
		g_options.loadaddr.segm=0x1000;
		g_options.loadaddr.offs=0x00;
		switch(exetype)		//根据前面判断出的类型进行显示
		{
		case NE_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"NE Executable");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_80486;
			g_options.mode16=true;
			break;
		case COM_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_80386;
			g_options.mode16=true;
			g_options.loadaddr.offs=0x100;
			break;
		case SYS_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"SYS File");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_80386;
			g_options.mode16=true;
			g_options.loadaddr.offs=0x00;
			break;
		case PE_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"PE Executable");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_PENTIUM;
			g_options.mode16=false;
			break;
		case OS2_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"OS2 Executable");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_PENTIUM;
			g_options.mode16=false;
			break;
		case LE_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"LE Executable");
			CheckDlgButton(hdwnd,IDC_DEFAULTBUTTON,true);
			g_options.processor=PROC_80486;
			g_options.mode16=false;
			break;
		case MZ_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			CheckDlgButton(hdwnd,IDC_DOSBUTTON,true);
			g_options.processor=PROC_80386;
			g_options.mode16=true;
			break;
		default:
		case BIN_EXE:
			SetDlgItemText(hdwnd,IDC_DEFAULTBUTTON,"COM File");
			CheckDlgButton(hdwnd,IDC_BINBUTTON,true);
			g_options.processor=PROC_8086;
			g_options.mode16=true;
			break;
		}

		g_options.mode32=!g_options.mode16;
		CheckDlgButton(hdwnd,load_debug,g_options.loaddebug);
		CheckDlgButton(hdwnd,demangle_names,g_options.demangle);
		CheckDlgButton(hdwnd,IDC_16DASM,g_options.mode16);
		CheckDlgButton(hdwnd,IDC_32DASM,g_options.mode32);
		CheckDlgButton(hdwnd,IDC_LOADDATA,g_options.loaddata);
		CheckDlgButton(hdwnd,IDC_LOADRESOURCES,g_options.loadresources);

		while(procnames[i].num)		//循环添加处理器类型
		{
			SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_ADDSTRING,0,(LPARAM) (LPCTSTR)procnames[i].name);
			if(g_options.processor==procnames[i].num)
				SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_SETCURSEL,i,0);
			i++;
		}

		//设置加载段（？？貌似对PE没啥用）
		segd=g_options.loadaddr.segm;
		wsprintf(segtext,"%x",segd);
		wsprintf(offstext,"%lx",g_options.loadaddr.offs);
		SendDlgItemMessage(hdwnd,IDC_SEGEDIT,WM_SETTEXT,0,(LPARAM)segtext);
		SendDlgItemMessage(hdwnd,IDC_OFFSEDIT,WM_SETTEXT,0,(LPARAM)offstext);
		return false;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(!IsDlgButtonChecked(hdwnd,IDC_DEFAULTBUTTON))
			{
				if(IsDlgButtonChecked(hdwnd,IDC_DOSBUTTON))
					g_floader.setexetype(MZ_EXE);
				else
					g_floader.setexetype(BIN_EXE);
			}
			else if((exetype==BIN_EXE)||(exetype==MZ_EXE))
				g_floader.setexetype(COM_EXE);
			g_options.processor=procnames[SendDlgItemMessage(hdwnd,IDC_LISTBOX1,LB_GETCURSEL,0,0)].num;
			EndDialog(hdwnd,NULL);
			return true;
		case IDC_SEGEDIT:
			if(HIWORD(wParam)==EN_CHANGE)
			{
				SendDlgItemMessage(hdwnd,IDC_SEGEDIT,WM_GETTEXT,(WPARAM)18,(LPARAM)segtext);
				sscanf(segtext,"%x",&g_options.loadaddr.segm);
			}
			return true;
		case IDC_OFFSEDIT:
			if(HIWORD(wParam)==EN_CHANGE)
			{
				SendDlgItemMessage(hdwnd,IDC_OFFSEDIT,WM_GETTEXT,(WPARAM)18,(LPARAM)offstext);
				sscanf(offstext,"%lx",&g_options.loadaddr.offs);
			}
			return true;
		case IDC_HELPBUTTON1:
			DialogBox(g_hInst,MAKEINTRESOURCE(HELPDIALOG_1),hdwnd,(DLGPROC)helpbox1);
			return true;
		case more_options:
			DialogBox(g_hInst,MAKEINTRESOURCE(Advanced_Options),hdwnd,(DLGPROC)moreoptions);
			return true;
		case load_debug:
			g_options.loaddebug=!g_options.loaddebug;
			CheckDlgButton(hdwnd,load_debug,g_options.loaddebug);
			return true;
		case demangle_names:
			g_options.demangle=!g_options.demangle;
			CheckDlgButton(hdwnd,demangle_names,g_options.demangle);
			return true;
		case IDC_16DASM:
			g_options.mode16=!g_options.mode16;
			CheckDlgButton(hdwnd,IDC_16DASM,g_options.mode16);
			return true;
		case IDC_32DASM:
			g_options.mode32=!g_options.mode32;
			CheckDlgButton(hdwnd,IDC_32DASM,g_options.mode32);
			return true;
		case IDC_LOADDATA:
			g_options.loaddata=!g_options.loaddata;
			CheckDlgButton(hdwnd,IDC_LOADDATA,g_options.loaddata);
			return true;
		case IDC_LOADRESOURCES:
			g_options.loadresources=!g_options.loadresources;
			CheckDlgButton(hdwnd,IDC_LOADRESOURCES,g_options.loadresources);
			return true;
		default:
			break;
		}
	}
	return false;
}

#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* moreoptions                                                           *
* - advanced loading options                                            *
************************************************************************/
#ifdef __BORLANDC__
#pragma warn -par
#endif
BOOL CALLBACK moreoptions(HWND hdwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case IDOK:
				g_options.codedetect=0;
				if(IsDlgButtonChecked(hdwnd,advanced_pushbp))
					g_options.codedetect|=CD_PUSHBP;
				if(IsDlgButtonChecked(hdwnd,advanced_aggressive))
					g_options.codedetect|=CD_AGGRESSIVE;
				if(IsDlgButtonChecked(hdwnd,advanced_enter))
					g_options.codedetect|=CD_ENTER;
				if(IsDlgButtonChecked(hdwnd,advanced_movbx))
					g_options.codedetect|=CD_MOVBX;
				if(IsDlgButtonChecked(hdwnd,advanced_moveax))
					g_options.codedetect|=CD_MOVEAX;
				if(IsDlgButtonChecked(hdwnd,advanced_eaxfromesp))
					g_options.codedetect|=CD_EAXFROMESP;
				EndDialog(hdwnd,NULL);
				return true;
			default:
				break;
			}
		}
		break;
	case WM_INITDIALOG:
		CheckDlgButton(hdwnd,advanced_pushbp,g_options.codedetect&CD_PUSHBP);
		CheckDlgButton(hdwnd,advanced_aggressive,g_options.codedetect&CD_AGGRESSIVE);
		CheckDlgButton(hdwnd,advanced_enter,g_options.codedetect&CD_ENTER);
		CheckDlgButton(hdwnd,advanced_movbx,g_options.codedetect&CD_MOVBX);
		CheckDlgButton(hdwnd,advanced_moveax,g_options.codedetect&CD_MOVEAX);
		CheckDlgButton(hdwnd,advanced_eaxfromesp,g_options.codedetect&CD_EAXFROMESP);
		return false;
	default:
		break;
	}
	return false;
}
#ifdef __BORLANDC__
#pragma warn +par
#endif

/************************************************************************
* load                                                                  *
* - checks file header info, identifies the possible types of files,    *
*   gets the users file loading options and calls the specific exe      *
*   format loading routines.                                            *
************************************************************************/
bool loadexefile(char *fname)
{
	char mzhead[2],exthead[2];
	dword pe_offset;
	dword num;
	dword fsize;
	if(g_floader.efile!=INVALID_HANDLE_VALUE)	//判断是否已经加载过
		return false;

	// just grab the file size first
	//获取文件大小
	g_floader.efile=CreateFile(fname,GENERIC_READ,1,NULL,OPEN_EXISTING,0,NULL);
	fsize=GetFileSize(g_floader.efile,NULL);
	CloseHandle(g_floader.efile);
	if(!fsize)		//获取文件大小失败或为零
	{
		MessageBox(g_hMainWnd,"打开文件失败或文件大小为零！！","Borg Message",MB_OK);
		return false;
	}

	g_floader.efile=CreateFile(fname,GENERIC_READ|GENERIC_WRITE,1,NULL,OPEN_EXISTING,0,NULL);	//打开文件
	if(g_floader.efile==INVALID_HANDLE_VALUE)		//如果打开失败
	{
		g_floader.efile=CreateFile(fname,GENERIC_READ,1,NULL,OPEN_EXISTING,0,NULL);		//以只读方式重新打开一遍
		if(g_floader.efile==INVALID_HANDLE_VALUE)		//还是失败
			return false;

		g_options.readonly=true;		//以只读方式打开成功
		MessageBox(g_hMainWnd,"无法获得文件的写入权限\n文件是只读的，这将无法保存对文件的任何修改","Borg Message",MB_OK);
	}

	if(GetFileType(g_floader.efile)!=FILE_TYPE_DISK)	//如果不是磁盘上的文件，打开失败
		return false;

	g_floader.exetype=BIN_EXE;
	if(ReadFile(g_floader.efile,mzhead,2,&num,NULL))	//读取MZ
	{
		if((num==2)&&(((mzhead[0]=='M')&&(mzhead[1]=='Z'))||	//判断是否是PE文件或dos的exe文件
			((mzhead[0]=='Z')&&(mzhead[1]=='M'))))
		{
			SetFilePointer(g_floader.efile,0x3c,NULL,FILE_BEGIN);
			if(ReadFile(g_floader.efile,&pe_offset,4,&num,NULL))		//读取IMAGE_DOS_HEADER中的e_lfanew字段，获取到PE文件头的文件偏移位置
				SetFilePointer(g_floader.efile,pe_offset,NULL,FILE_BEGIN);
			if(ReadFile(g_floader.efile,exthead,2,&num,NULL))		//读取IMAGE_NT_HEADERS32中的Signature字段，判断文件的类型（一般都是PE文件了吧）
			{
				if(((short int *)exthead)[0]==0x4550)		//IMAGE_NT_SIGNATURE
					g_floader.exetype=PE_EXE;
				else if(((short int *)exthead)[0]==0x454e)
					g_floader.exetype=NE_EXE;
				else if(((short int *)exthead)[0]==0x454c)
					g_floader.exetype=LE_EXE;
				else if(((short int *)exthead)[0]==0x584c)
					g_floader.exetype=OS2_EXE;
				else
					g_floader.exetype=MZ_EXE;
			}
		}
		else	//未知格式，直接当成纯bin反汇编了
		{
			if(strlen(fname)>3)
			{
				if(!lstrcmpi(fname+strlen(fname)-3,"com"))	//判断是否是dos的com程序
				{
					SetFilePointer(g_floader.efile,0,NULL,FILE_BEGIN);
					g_floader.exetype=COM_EXE;
				}
				else if(!lstrcmpi(fname+strlen(fname)-3,"sys"))	//判断是否是sys（sys不也是PE文件么？）
				{
					SetFilePointer(g_floader.efile,0,NULL,FILE_BEGIN);
					g_floader.exetype=SYS_EXE;
				}
			}
		}
	}

	//申请了一个与文件大小相同的缓冲区并把文件全部读入
	g_floader.fbuff=new byte[fsize];
	SetFilePointer(g_floader.efile,0x00,NULL,FILE_BEGIN);
	ReadFile(g_floader.efile,g_floader.fbuff,fsize,&num,NULL);

	//显示打开新文件配置的对话框
	DialogBox(g_hInst,MAKEINTRESOURCE(D_checktype),g_hMainWnd,(DLGPROC)CheckOptDlgProc);

	if(!g_options.loadaddr.segm)
	{
		g_options.loadaddr.segm=0x1000;
		MessageBox(g_hMainWnd,"无法使用0作为段基地址。\n将使用默认段基地址0x1000","Borg Message",MB_OK);
	}

	g_dsm.dissettable();	//貌似是设置反汇编用的处理器的
	switch(g_floader.exetype)
	{
	case BIN_EXE:
		g_floader.readbinfile(fsize);
		break;
	case PE_EXE:
		g_floader.readpefile(pe_offset);		//好大一个函数
		break;
	case MZ_EXE:
		g_floader.readmzfile(fsize);
		break;
	case OS2_EXE:
		g_floader.reados2file();
		CloseHandle(g_floader.efile);
		g_floader.efile=INVALID_HANDLE_VALUE;
		g_floader.exetype=0;
		return false; // at the moment;
	case COM_EXE:
		g_floader.readcomfile(fsize);
		break;
	case SYS_EXE:
		g_floader.readsysfile(fsize);
		break;
	case LE_EXE:
		g_floader.readlefile();
		CloseHandle(g_floader.efile);
		g_floader.efile=INVALID_HANDLE_VALUE;
		g_floader.exetype=0;
		return false; // at the moment;
	case NE_EXE:
		g_floader.readnefile(pe_offset);
		break;
	default:
		CloseHandle(g_floader.efile);
		g_floader.efile=INVALID_HANDLE_VALUE;
		g_floader.exetype=0;
		return false;
	}
	return true;
}

/************************************************************************
* newfile                                                               *
* - handles selecting a new file and its messages, using the standard   *
*   routine GetOpenFileName                                             *
* - starts up the secondary thread when the file is loaded              *
************************************************************************/
bool newfile(void)
{
	// factor of 2 added for nt unicode
	getfiletoload(current_exe_name);	//显示打开文件对话框，返回文件路径
	if(current_exe_name[0])
	{
		if(loadexefile(current_exe_name))
		{
			StatusMessage("File Opened");
			strcat(winname," : ");
			strcat(winname,current_exe_name);
			SetWindowText(g_hMainWnd,winname);
			InThread=true;
			ThreadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread,0,0,&ThreadId);
			changemenus();
		}
		else
			MessageBox(g_hMainWnd,"File open failed ?",program_name,MB_OK|MB_ICONEXCLAMATION);
	}
	return 0;
}

