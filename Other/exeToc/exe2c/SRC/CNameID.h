// Copyright(C) 1999-2005 LiuTaoTao£¬bookaa@rorsoft.com

#pragma once

struct st_NameID
{
	H_NAMEID id;
	PSTR name;
};

typedef CList<st_NameID*,st_NameID*> CNameList;

class CNameMng
{
private:
	CNameList* list;

public:
	CNameMng();
	~CNameMng();

	PSTR id2name(H_NAMEID id);
	H_NAMEID savname(PSTR name);
	H_NAMEID LookupName(PSTR name);
    void Rename(H_NAMEID id, PSTR newname);

};

