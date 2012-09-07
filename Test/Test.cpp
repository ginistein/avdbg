// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
	char szBuffer[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH,szBuffer);
	printf("当前目录：%s",szBuffer);

	OutputDebugStringA("ANSI DebugString");
	OutputDebugStringW(L"Unicode Debug String");

	system("pause");
	return 0;
}

