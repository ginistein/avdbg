// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CMyString.h
#ifndef	CMyString_H
#define	CMyString_H

const	int MEMORY_STEP	= 2048;	// 每次扩充内存空间的大小

class CMyString
{
	UINT	m_maxlen;	// 扩充数据内存空间后的大小
	UINT	m_curlen;	// 当前数据长度
	PSTR	m_bufptr;	// 当前数据缓存区

public:
	CMyString();
	~CMyString();
	
	void strcat(PSTR str);	// 追加数据
	void Clear();
	UINT	GetLength();	// 获取当前数据长度
	PSTR	GetString();	// 获取数据
	PSTR	GetWritableString();	// 获取可写的数据
};

#endif	//	CMyString_H
