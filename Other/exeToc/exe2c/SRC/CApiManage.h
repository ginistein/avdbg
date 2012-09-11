// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com

//	CApiManage.h

#pragma once

#include "datatype.h"
#define	INVALID_STACK	11

typedef	DWORD	H_NAMEID;

class CApi
{
public:
	BYTE	m_stack_purge;	//7 for invalid
	ea_t	address;
    char	name[80];

    CFuncType* m_functype;

	CApi()
    {
    	m_stack_purge = INVALID_STACK;
    }

	~CApi(){}
};

typedef	CList<CApi*,CApi*> API_LIST;

class CApiManage
{
public:
	API_LIST*	apilist;

public:
	CApiManage()
	{ 
		this->apilist = new API_LIST;
	}    //new_API_LIST

	~CApiManage()
	{
		delete this->apilist; 
		this->apilist = NULL;
	}

	bool	new_api(ea_t address,int stacksub);
	CApi*	get_api(ea_t address);

    void New_ImportAPI(PSTR pstr, DWORD apiaddr);
};

extern	CApiManage* g_ApiManage;



