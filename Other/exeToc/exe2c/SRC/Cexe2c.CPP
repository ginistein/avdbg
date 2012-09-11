///////////////////////////////////////////////////////////////
//
// Cexe2c.cpp
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
// Created at 2005.2.1
// Description:	The main cpp file of the component
// History:		
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Cexe2c.h"
#include "CFuncStep1.h"
#include "DataType.h"
#include "ParseHead.h"
#include "..\LibScanner.H"
#include "hpp.h"
#include "cfunctype.h"

// 全局变量
KS_DECLARE_COMPONENT(exe2c, EXE2C)

PSTR check_if_jmp_api(PCBYTE phead);
static CString CheckIf_libfunc(PCBYTE phead);
static DWORD str_to_dword(PSTR cmd);


void __stdcall outstring_in_log(PSTR str)
{
    g_Cexe2c->prt_log(str);
}

bool __stdcall Cexe2c::BaseInit()
{
	//KICK_MFC();
	g_Cexe2c = this;
    
	m_func_list = new FUNC_LIST;
    m_api_name_manager = new CNameMng;

	m_E2COut = NULL;
    m_FileLoader = NULL;

	return true;
}

Cexe2c::~Cexe2c()
{
	//KICK_MFC();
	if(m_func_list)
	{
		POSITION pos = m_func_list->GetHeadPosition();
		while (pos != NULL)
		{
			CFunc* p = m_func_list->GetNext(pos);
			
			p->~CFunc();
		}
		SAFE_DELETE(m_func_list);
	}

	SAFE_DELETE(m_api_name_manager);
	SAFE_DELETE(m_FileLoader);

	g_Cexe2c = NULL;
}

void __stdcall Cexe2c::exe2c_main(PSTR fname)
{
    lib_init();	// 初始化lib库文件

    //	把需要反编译的文件调入
    if (m_FileLoader != NULL)
    	delete m_FileLoader;

	m_FileLoader = new FileLoader;
	bool f = m_FileLoader->load(fname);
    if (!f)
    {
    	alert_prtf("文件 %s 加载失败",fname);
        return;
    }

    PBYTE entry_buf;
    ea_t entry_offset;
    m_FileLoader->GetEntrance(entry_buf, entry_offset);

	// 因为文件的调入地址与虚拟地址不同，所以要记住这个差值
    // 以后主程序只以offset来访问，不管实际buffer
    Disassembler_Init_offset(entry_buf, entry_offset);

	//	开始分析
    this->do_exe2c(entry_offset);
}

bool __stdcall Cexe2c::test()
{
	//KICK_MFC();
	return true;
}

void	Cexe2c::Recurse_Analysis()
{
	POSITION pos = m_func_list->GetHeadPosition();
    while (pos != NULL)
    {
        CFunc* p = m_func_list->GetNext(pos);
		
		log_prtl("Recurse_analysis %x",p->m_head_off);

		if (p->m_nStep != STEP_100)
			continue;

		p->analysis();
    }
}

void	Cexe2c::Recurse_Optim()
{
	POSITION pos = m_func_list->GetHeadPosition();
    while (pos != NULL)
    {
        CFunc* p = m_func_list->GetNext(pos);
		
		log_prtl("Recurse_Optim %x",p->m_head_off);

		if (p->m_nStep < STEP_6)
			continue;
    }
}

CFunc*	Cexe2c::FindFuncByName(PSTR pname)	// 查找函数名称
{
	if (m_func_list == NULL)
    	return NULL;
	POSITION pos = m_func_list->GetHeadPosition();
    while (pos != NULL)
    {
        CFunc* p = m_func_list->GetNext(pos);
        if (_stricmp(p->m_funcname,pname) == 0)
            return p;
    }
    return NULL;
}

void	Cexe2c::do_exe2c(ea_t start)	//	开始分析
{
    ea_t pmain = Find_Main(start);

	CFunc* pfunc = this->func_new(pmain);// 第一步，根据 start，创建一个空的 CFunc

	if (pmain == start)
		strcpy(pfunc->m_funcname,"start");
	else
		strcpy(pfunc->m_funcname,"main");

	g_Cur_Func = pfunc;	//	设置当前的CFunc

    g_Cur_Func->PrepareFunc();
}

CFunc* Cexe2c::GetFunc(ea_t start)
{
	POSITION pos = m_func_list->GetHeadPosition();	// 函数列表的入口地址
    while (pos != NULL)
    {
        CFunc* p = m_func_list->GetNext(pos);
        if (p->m_head_off == start)					// 判断函数入口地址是否已存在
            return p;
    }
    return NULL;
}


//	根据 start，创建一个空的 CFunc ，并加入 m_func_list
//	如果该地址的 CFunc 已经存在，则直接返回它
CFunc* Cexe2c::func_new(ea_t start)
{
	if(0x100000>start)
		return NULL;

	CFunc* p1 = GetFunc(start);	// 检查本func是否已经在func链中了
	if (p1 != NULL)				// 函数地址已存在,无需处理，直接反馈
		return p1;

	log_prtl("New func %x",start);

	CFunc* p = new CFunc(start); // new_CFunc

	fill_func_info(start, p);	//	填入 CFunc 的一些其它信息

	CString pname = CheckIf_libfunc(ea2ptr(p->m_head_off));		// 判断是否为库函数

	if (!pname.IsEmpty())
	{
		p->m_IfLibFunc = true;		// 标明是库函数
		p->m_functype = Get_FuncDefine_from_internal_name(pname.GetBuffer());
		if (p->m_functype)
			strcpy(p->m_funcname, p->m_functype->m_pname);
		else
			strcpy(p->m_funcname, pname);
	}

	m_func_list->AddTail(p);	// insert cur Func to m_func_list

	return p;
}


void __stdcall Cexe2c::Change_Array(int colorindex, void* handle, int newarray)
{
    if (handle == NULL)
        return;
    
    if (colorindex == COLOR_Var)
    {
        st_VarLL* p = (st_VarLL*)handle;
        if (p->array < newarray)
        {
        }
        p->array = newarray;
    }
    if (colorindex == COLOR_VarH)
    {
        M_t* p = (M_t*)handle;
        //g_Cur_Func->m_exprs->Change_Array(p, newarray);
    }
}

void __stdcall Cexe2c::LineHotKey(void* hline, char key)
{
    if (key == 'i' || key == 'I')
    {//i for internal
        static bool flag = false;
        flag = !flag;
        if (flag)
        {
            //prtout_itn();
        }
        else
        {
            //prtout_cpp();
        }
    }
    if (hline == NULL)
        return;
    
    if (key == 'd' || key == 'D')
    {
        g_Cur_Func->MakeDownInstr(hline);
    }
}

void __stdcall Cexe2c::HotKey(int colorindex, void* handle, char key)
{
    if (handle == NULL)
        return;
    if (key == 'p' || key == 'P')
    {
    }
    else
        return;

    if (colorindex == COLOR_VarH)
    {
        M_t* p = (M_t*)handle;
        //g_Cur_Func->m_exprs->namemanager->Rename(p->nameid,newname);
    }
}
    
void __stdcall Cexe2c::ReType(int colorindex, void* handle, PSTR newtype)
{
    if (handle == NULL)
        return;
    else if (colorindex == COLOR_VarH || colorindex == COLOR_type)
    {
        M_t* p = (M_t*)handle;
        g_Cur_Func->ReType(p, newtype);
    }
}

bool __stdcall Cexe2c::Rename(enum XMLTYPE xmltype, void* handle, PSTR newname)
{
    if (handle == NULL)
        return false;
    
    if (xmltype == XT_FuncName)
    {
        CFunc* p = (CFunc*)handle;
        strcpy(p->m_funcname, newname);
        return true;
    }
    else if (xmltype == XT_Symbol)
    {
        M_t* p = (M_t*)handle;
        strcpy(p->namestr, newname);
        return true;
    }
    /*
    else if (colorindex == COLOR_Var)
    {
        st_VarLL* p = (st_VarLL*)handle;
        strcpy(p->Name, newname);
    }
    else if (colorindex == COLOR_type)
    {
        M_t* p = (M_t*)handle;
        g_Cur_Func->ReType(p, newname);
    }
    */
    return false;
}

HANDLE Cexe2c::GetFirstFuncHandle()
{
    if (m_func_list == NULL)
        return 0;
    POSITION pos = m_func_list->GetHeadPosition();
    return (HANDLE)pos;
}

HANDLE Cexe2c::GetCurFuncHandle()
{
    if (m_func_list == NULL)
        return 0;
    POSITION pos = m_func_list->GetHeadPosition();
    while (pos)
    {
        POSITION sav_pos = pos;
        CFunc* p = m_func_list->GetNext(pos);
        if (p == g_Cur_Func)
        {
            return sav_pos;
        }
    }
    return 0;
}

void Cexe2c::SetCurFunc_by_Name(PSTR funcname)
{
	CFunc *p = this->FindFuncByName((PSTR)funcname);
    if (p == NULL)
    	return;
    g_Cur_Func = p;
}

void   Cexe2c::GetFuncInfo(HANDLE h, OUT st_FuncInfo* info)
{
    if (m_func_list == NULL)
        return;
    if (h == 0)
        return;
    
    POSITION pos = (POSITION)h;
    CFunc* p = m_func_list->GetNext(pos);

    strcpy(info->name, p->m_funcname);
    info->nStep = p->m_nStep;
    info->headoff = p->m_head_off;
    info->endoff = p->m_end_off;
    info->stack_purge = p->m_stack_purge;
    info->m_EBP_base = p->m_EBP_base;
    info->m_args = p->m_args;
	info->m_IfLibFunc =  p->m_IfLibFunc;
}

HANDLE Cexe2c::GetNextFuncHandle(HANDLE h)
{
    if (m_func_list == NULL)
        return NULL;
    if (h == 0)
        return 0;
    
    POSITION pos = (POSITION)h;
    m_func_list->GetNext(pos);
    return (HANDLE)pos;
}

bool Cexe2c::RenameCurFuncName(PSTR name)	// 重命名当前函数名称
{
	CFunc *p = g_Cur_Func;
    if (p == NULL)
    	return false;
	if (!IfValideFuncName(name))
    	return false;	// name not valid
    if (this->FindFuncByName(name) != NULL )
    	return false;	// name already use
    strcpy(p->m_funcname, name);
    return true;
}

void Cexe2c::ClearFuncList()	// 清空函数列表
{
    if (m_func_list == NULL)
        return;
	m_func_list->RemoveAll();
    return;
}

void __stdcall Cexe2c::DoCommandLine(PSTR cmd)
{
    //MessageBox(NULL,"asdf","dddd",0);
    //if (g_Cur_Func == NULL)
        //return;
    if (memcmp(cmd, "var ", 4) == 0)
    {
        PSTR varname = cmd + 4;
        CFuncOptim the(g_Cur_Func);
        the.Prt_Var_Flow(varname);
    }
    else if (memcmp(cmd, "funcinfo", 8) == 0)		// 显示当前函数信息
    {
        g_Cur_Func->report_info();
    }
    else if (memcmp(cmd, "funcproto ", 10) == 0)	//当前函数的预定义
    {
        cmd += 10;
        CCInfo * pnew = new CCInfo;
        CFuncType* pfunctype = pnew->do_func_proto(cmd);
        g_Cur_Func->m_functype = pfunctype;
        strcpy(g_Cur_Func->m_funcname, pfunctype->m_pname);
    }
    else if (memcmp(cmd, "classof ", 8) == 0)
    {
        cmd += 8;
        VarTypeID id = g_VarTypeManage->VarType_Name2ID(cmd);
        Class_st* pclass = g_VarTypeManage->id2_Class(id);
        CFuncType* pfunctype = g_Cur_Func->m_functype;
        if (pfunctype != NULL && pclass != NULL)
        {
            pfunctype->m_class = pclass;
        }
        //this->DoCommandLine("funcproto void __cdecl func1()");
        //this->DoCommandLine("classof CTest1");
    }
    else if (memcmp(cmd, "macro1", 6) == 0)
    {
        this->DoCommandLine("funcproto ATOM __cdecl MyRegisterClass(HINSTANCE hInstance)");
    }
    else if (memcmp(cmd, "macro2", 6) == 0)
    {
        this->DoCommandLine("funcproto BOOL __cdecl InitInstance(HINSTANCE hInstance, int nCmdShow)");
    }
    else if (memcmp(cmd, "macro3", 6) == 0)
    {
        this->DoCommandLine("funcproto int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)");
    }
    else if (memcmp(cmd, "macro_test", 10) == 0)
    {
        this->DoCommandLine("funcproto void __cdecl CTest1()");
    }
    else if (memcmp(cmd, "macro5", 6) == 0)
    {
        this->DoCommandLine("classof CTest1");
    }
    else if (memcmp(cmd, "restart", 7) == 0)
    {
        g_Cur_Func->Restart();
    }
    else if (memcmp(cmd, "go", 2) == 0)
    {
        this->analysis_All();
    }
    else if (memcmp(cmd, "funcnew ", 8) == 0)	// 添加新函数
    {
        cmd += 8;
        DWORD d = str_to_dword(cmd);
        this->func_new(d);
    }

}
// 函数
static CString CheckIf_libfunc(PCBYTE phead)
{
    PSTR apiname = check_if_jmp_api(phead);	// 是否跳转到输入表函数列表中
    if (apiname)
    {
        return apiname;
    }

    if (1)
    {
        CString fcname = g_LIBSCANNER->CheckIfLibFunc(phead);	// 是否为库函数

        if (!fcname.IsEmpty())
        {
            return fcname;
        }
    }
    return "";
}


static DWORD str_to_dword(PSTR cmd)	// 字符串转换为数值
{
    char buf[80];
    strncpy(buf, cmd, 70);
    buf[70] = 0;
    _strupr(buf);
    if (buf[0] == '0' && buf[1] == 'X')
    {
        DWORD d;
        sscanf(buf+2,"%X", &d);
        return d;
    }
    DWORD d;
    sscanf(buf,"%d", &d);
    return d;
}


