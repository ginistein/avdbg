///////////////////////////////////////////////////////////////
//
// Cexe2c.h
// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
// Created at 2005.2.1
// Description:	The standard header file of the component implement
// History:		
//
///////////////////////////////////////////////////////////////

#pragma once

#include "..\..\Public_Interface\exe2c.h"

#include "cisc.h"

#include "CFuncStep1.h"

class Cexe2c : public I_EXE2C
{
public:
	Cexe2c(){}
	~Cexe2c();

private:	// 成员
	//Add member here
    I_E2COUT* m_E2COut;
    CNameMng* m_api_name_manager;   // 用于保存API的函数名列表
    FileLoader* m_FileLoader;		
    FUNC_LIST* m_func_list;			// 函数列表


public:
///////////// DO NOT EDIT THIS //////////////
	virtual bool	__stdcall BaseInit();	//override the origin function, it's a class creator!
///////////// DO NOT EDIT THIS //////////////

	//Add interface here
	virtual bool	__stdcall test();		//Test interface
    void __stdcall exe2c_main(PSTR fname);

	void __stdcall Hxd_prtout_Pcode(I_XmlOut* iOut)		// 显示伪代码信息
    { 	
		if (g_Cur_Func != NULL)
		{
			if(g_Cur_Func->m_nStep < 10) // 第10步变量命名完成，才可以显示
			{
				log_prtl("第10步后才能查看伪代码列表，现在为第 %d 步",g_Cur_Func->m_nStep);
				return;
			}
			CXmlOutPro out(iOut);
			CFunc_Prt prt(g_Cur_Func);
			prt.Hxd_prtout_Pcode(&out);
		}
    }


    void __stdcall prtout_cpp(I_XmlOut* iOut)		// 显示反编译代码
    {
		if (g_Cur_Func != NULL)
		{
			CXmlOutPro out(iOut);
			CFunc_Prt prt(g_Cur_Func);
			prt.prtout_cpp(&out);
		}
    }
	
	void __stdcall hxd_prtout_Asm(I_XmlOut* iOut)	// 显示反汇编代码
    {
		if (g_Cur_Func != NULL)
		{
			CXmlOutPro out(iOut);
			CFunc_Prt prt(g_Cur_Func);
			prt.hxd_prtout_Asm(&out);
		}
    }

    void __stdcall Init(I_E2COUT* i_E2COut)
    {
        m_E2COut = i_E2COut;
    }

    void prt_log(PSTR str)
    {
        if (m_E2COut != NULL)
        {
            m_E2COut->prt_log(str);
        }
    }

    HANDLE __stdcall GetFirstFuncHandle();
    void   __stdcall GetFuncInfo(HANDLE h, OUT st_FuncInfo* info);
    HANDLE __stdcall GetNextFuncHandle(HANDLE h);
    HANDLE __stdcall GetCurFuncHandle();

	void   __stdcall ClearFuncList();

    void __stdcall SetCurFunc_by_Name(PSTR funcname);

    bool __stdcall analysis_Once() 
	{
		if (g_Cur_Func != NULL)
			return (g_Cur_Func->analysis_step_by_step());
		return false;
    }
    void __stdcall analysis_All() {
        g_Cur_Func->analysis();
    }
    bool __stdcall RenameCurFuncName(PSTR name);
    virtual void __stdcall DoCommandLine(PSTR cmd);
    virtual bool __stdcall Rename(enum XMLTYPE xmltype, void* handle, PSTR newname);
    virtual void __stdcall ReType(int colorindex, void* handle, PSTR newtype);
    virtual void __stdcall Change_Array(int colorindex, void* handle, int newarray);
    virtual void __stdcall HotKey(int colorindex, void* handle, char key);
    virtual void __stdcall LineHotKey(void* hline, char nChar);
	//Add interface here

    void Recurse_Analysis();
    void Recurse_Optim();
    CFunc* func_new(ea_t start);
    CFunc* GetFunc(ea_t start);

    
private:
    CFunc*	FindFuncByName(PSTR pname);
    void	do_exe2c(ea_t entry);
};

extern Cexe2c* g_Cexe2c;

