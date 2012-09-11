// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com
#pragma once

#include "..\..\Public_typedef\pub_typedef.h"

#include "mylist.h"

PSTR	new_str(PSTR p);

int		alert_prtf(PSTR fmt, ...);
//int		log_prtf(PSTR fmt,...);

#define log_prtf
#define alert
#define log_prtt
//#define log_prtl

void nop();
