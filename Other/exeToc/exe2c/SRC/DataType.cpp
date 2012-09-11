// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com

// DataType.cpp: implementation of the CDataTypeMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataType.h"
#include "ParseHead.h"
//#include	"CISC.H"

static bool __stdcall DataDefine_Init()
{
    if (true)//GetFileAttributes("e2c_define.h") != -1)
    {
        //CParseHead ph;
        //ph.ParseHeadFile("e2c_define.h");
        
//        g_FuncDefineMng.prtall();
    }
    else
    {
        MessageBox(NULL,"not find e2c_define.h", "error", 0);
    }
	return true;
}


static void __stdcall DataDefine_Exit()
{
}

class CDataType_cpp
{
public:
    CDataType_cpp()
    {
        DataDefine_Init();
    }
    ~CDataType_cpp()
    {
        DataDefine_Exit();
    }
};

CDataType_cpp g_CDataType_cpp;


    
