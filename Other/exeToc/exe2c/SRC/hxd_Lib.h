#pragma once

//bool check_if_API(BYTE* dwAddr);	// 判断是否是跳转到API

void OutToTxt(const char * szText);	// 信息保存到“c:\\OutTxt.txt”文件中

void nop();	// 空函数

PSTR GetMyExePath();	// Exe2C程序的路径

void lib_init();
void lib_exit();

PSTR my_itoa(int i);
