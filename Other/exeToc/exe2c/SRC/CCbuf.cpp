// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
//------------------------//
//
//  注释日期：20110615
//  编 写 者：红尘岁月
//  功    能：解析INC目录中的.h文件内容
//  方    法：逐步分析每一个字符，除掉注释、拐行符等内容
//
//------------------------//

#include "stdafx.h"
#include "Public_Fun.h"
#include "CCbuf.h"

#include <io.h>

//	----------------------------------------------------------
CCbuf::CCbuf()
{
	f_str = 0;
}

CCbuf::~CCbuf()
{

}

void CCbuf::LoadFile(FILE *f)
{
	long flen = _filelength(_fileno(f));	// 文件长度
	char *buf = new char[flen];
	fread(buf,1,flen,f);				// 文件读入缓冲区
	
	m_p = new char[flen+1];				//thats enough
	m_len = 0;

	for (long i = 0; i < flen; i++)		// 读取每一个字节
	{
		OneChar(buf[i]);
	}
	OneChar(EOF);

	SAFE_DELETE(buf);

	assert((long)m_len <= flen+1);
}

// 处理注释内容
// 处理换行
void CCbuf::OneChar(int c3)
{	
	if (c3 == '\r')
		c3 = '\n';

	//	c3 是 input char 或 EOF
	//	如果有效，则把c3 加到p所指的buf，同时 len++

	int i = m_len;	// 文件当前处理位置
	PSTR p = m_p;

	if (c3 == EOF)	// 文件结束
	{
		assert(f_str == 0);
		OneChar(0);
		return;
	}

	char c = (char)c3;

	// ----第一步：忽略单行注释与多行注释内容----
	if ( f_str == 0 && i >= 2)	
	{
		if (p[i-2] == '/' && p[i-1] == '/')	// 处理单行注释
		{
			if (c == '\n' || c == 0)	// 一直到换行
			{
				m_len -= 2;	// 把单行注释符清理掉
				OneChar(0);
				return;
			}
			else
				return;	 // 单行注释“//”后的内容，不用保存到p指向的内存中，直接忽略，不做处理
		}

		if (p[i-2] == '/' && p[i-1] == '*')	// 处理多行注释
		{
			static char c1 = 0;
			if (c1 == '*' && c == '/')		// 注释结束
			{
				m_len -= 2;
				return;
			}
			c1 = c;		// 保存到c1中，保存*号用
			return;
		}
	}

	// --- 第二步：处理字符串-----
	if ( f_str )	// 内容是否在字符串内
	{
	
		if (c == 0 || c == '\n')	// 字符串内不能换行
			assert(("error",0));

		static BOOL f = FALSE;	//	for '\\'

		if (f)
		{
			f = FALSE;
		}
		else
		{
			if (c == '\\')		// 转义字符后面的一个字符不处理，如 \t
				f = TRUE;
			if (c == f_str)		// 字符串结果
				f_str = 0;
		}
	}
	else
	{
		if (c == '\'' || c == '\"')
			f_str = c;			// 后面的为字符串
	}
	
	
	switch (c)
	{
	case 0:
	case '\n':
		if(i==0)	// 第一个字符为换行，清除掉
			break;
		p[i] = 0;
		m_len++;
		if (i > 0)
		{
			//	把前面的空格与空行除掉
			if (p[i-1] == ' ' || p[i-1] == '\0')
			{
				p[i-1] = 0;
				m_len--;
				return;
			}
			//	检查前面的一个字符，是不是拐行符
			if ( p[i-1] == '\\')
			{
				m_len -= 2;
				return;		//	不拐了
			}
		}
		break;
	case '\t':
	case ' ':
		if (i == 0 || p[i-1] == '\0')	// 当第一字符 或者 前面的内容
			break;

		if (f_str == 0 && p[i-1] == ' ')
			break;	//	如果前面已经有一个空格，就算了

		c = ' ';
		//continue to next
	default:		// 其它不处理的，直接复制
		p[i] = c;
		m_len++;
		break;
	}
}

