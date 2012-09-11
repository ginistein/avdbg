// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com


//	exe2c project
#ifndef	STRPARSE_H
#define	STRPARSE_H

void get_1part(OUT PSTR buf,PSTR &p);
void skip_space(PSTR &p);
void skip_eos(PSTR &p);
BOOL if_split_char(char c);
DWORD Str2Num(PSTR p);
signed int Str2Int(PSTR p);


#endif	//STRPARSE_H
