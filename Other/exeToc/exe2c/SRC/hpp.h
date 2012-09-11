// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com


//	exe2c project

#pragma once

#include "CFuncType.h"
//	--------------------------------------------------------
class define_t
{	//#define src dst
public:
	PSTR src;
	PSTR dst;

	define_t();

	~define_t();
};

typedef CList<define_t*, define_t*> DefineList;
	//about this list:
	//	1. allow multi define, that is , it will save both
	//		#define A 1
	//		#define A 2
	//		but if 2 just same define, only once

//	--------------------------------------------------------

//	当我们一行一行去读CPP程序时，我们需要一个结构来记录当前的状态
class CCInfo
{	
public:
	int	comma1;	//	为1表明在()中，期待一个')'。为2表明(())
	int	comma2;	//	为1表明在{}中，期待一个'}'。为2表明{{}}
	int extern_c;	//	表明是在一个extern "C" {}中
	enum_CallC	m_default_callc;
	
	SIZEOF m_len;	// 是m_buf的长度
	char* m_buf;	// CCbuf整理后的文件内容

public:
	CCInfo();
	~CCInfo();
	
	void LoadFile(FILE *f);
	void OneLine(PSTR lbuf, PSTR &pnext);
	void do_typedef(PSTR &p);
	void do_typedef_(VarTypeID baseid, PSTR &p);
	CFuncType* do_func_proto(PSTR pstr);
	CFuncType* do_func_proto_void(PSTR pstr);
	void do_class(PSTR pleft, PSTR &pnext);
};

class CHpp
{
public:
	FuncTypeList* m_FuncTypeList;	//	保存从.h得来的全局函数定义

	CHpp();
	~CHpp();
	void newfunc_addlist(CFuncType* pnewfunc);
	void func_define(PSTR lbuf, CCInfo *p);
	CFuncType* Get_FuncDefine_from_internal_name_(PSTR pmyinternalname);
	CFuncType* Get_FuncDefine_from_name_(PSTR pmyname);
};

extern CHpp* g_Hpp;
extern DefineList* g_DefineList;

VarTypeID do_enum(PSTR &p);

//	--------------------------------------------------------

BOOL hpp_init();
BOOL hpp_onexit();

