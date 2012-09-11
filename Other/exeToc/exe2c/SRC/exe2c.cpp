// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "stdafx.h"
#include "hxd_Lib.h"

#include "Cexe2c.h"
#include "CFuncStep1.h"
#include "SVarType.h"
#include "..\LibScanner.H"
#include "HPP.h"
#include "CClassManage.h"
#include "CEnumMng.H"

// -------- 全局变量 ---------
Cexe2c* g_Cexe2c = NULL;

CFunc* g_Cur_Func = NULL;
CVarTypeMng* g_VarTypeManage = NULL;
I_LIBSCANNER* g_LIBSCANNER = NULL;

CHpp* g_Hpp = NULL;
DefineList* g_DefineList = NULL;	// 宏定义列表,#define One,1
CClassManage* g_ClassManage = NULL;
Enum_mng * g_enum_mng = NULL;

HANDLE	hOutPutFile	=NULL;

// ------- 全局函数 ------------
BOOL hpp_init();
BOOL hpp_exit();

void CExprManage_cpp_Init();
void CExprManage_cpp_exit();


class CSelfInit
{
public:
	CSelfInit()
	{
		// dll初始化
		//_CrtSetBreakAlloc(40327); 

		CExprManage_cpp_Init();	// 创建变量类型管理器,如void,BYTE等各种类型信息
		hpp_init();

	}

	~CSelfInit()
	{
		CExprManage_cpp_exit();
		hpp_exit();

		lib_exit();
	}
};

CSelfInit self;

//leave blank