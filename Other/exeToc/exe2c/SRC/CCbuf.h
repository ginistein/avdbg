// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#pragma once

// 把程序文件读入缓冲区

class CCbuf
{
public:
	char f_str;		// 因为有'和"两种string形式
	PSTR m_p;		// 数据缓冲区
	SIZEOF	m_len;

	CCbuf();
	~CCbuf();

	void LoadFile(FILE *f);
	void OneChar(int c);

};

