// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com

//	XmlType.h

#ifndef	XmlType_H
#define	XmlType_H

#include "enum.h"

#define	COLOR_DEFAULT	RGB(255,255,255)

COLORREF XmlType_2_Color(XMLTYPE xmltype);

void XML_Clicked(XMLTYPE xmltype, PVOID p);

#endif	//	XmlType_H
