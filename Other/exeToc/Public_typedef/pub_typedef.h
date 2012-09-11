///////////////////////////////////////////////////////////////
//
// pub_typedef.h
// Copyright(C) 2011-2015 ºì³¾ËêÔÂ
// Created at 2011.7.26
// Description:	
// History:		
//
///////////////////////////////////////////////////////////////
 
#pragma once

// typedef
typedef	DWORD	H_NAMEID;
typedef char *  PSTR;
// typedef char const *  PCSTR;

typedef DWORD	VarTypeID;
typedef	DWORD	SIZEOF;

typedef DWORD	ea_t;
typedef	DWORD	SIZEOF;
typedef DWORD	VarTypeID;

typedef const BYTE* PCBYTE;

// define
#define	SIZE_unknown 0xfffffffe
#define BIT32_is_4 4

#define	IID_LIBSCANNER				0x00003a91
#define	LIBSCANNER_INITORDER		0x80000000
#define	LIBSCANNER_PARENT_IID		NULL
#define	LIBSCANNER_PRIORITY			0x80000000

#define SAFE_DELETE(p)\
	if (p != NULL)\
	{\
		delete p;\
		p = NULL;\
	}
