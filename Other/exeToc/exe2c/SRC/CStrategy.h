// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "CStrategy.h"
#ifndef CStrategy__H
#define CStrategy__H
#include	"CISC.H"

//log_prtl("asign it, and end: %s", this->m_exprs->VarName(pvar));

class CStrategy
{
    ENUM_STRATEGY m_es;
    union
    {
        struct
        {
            M_t* pvar;  //是哪个变量
            PINSTR pinstr;  //哪一条语句需要删
            char reason[80];
        }can_delete;    //for m_es == ES_Instr_can_Delete
        struct
        {
            M_t* pvar;  //是哪个变量
            PINSTR p1;  //前一条，对我赋值
            PINSTR p2;  //后一条，使用我
            char reason[80];
        }can_elim;    //for m_es == ES_Instr_can_Elim
    };
    char m_buf[256];

    CString PrintOne(INSTR_LIST* list, PINSTR p, CFunc* pFunc);

public:
    CStrategy()
    {
        m_es = ES_Error;
    }
    void AddOne_CanDelete(M_t* pvar, PINSTR pinstr, PSTR reason);

	void AddOne_CanEliminate(M_t* pvar, PINSTR p1, PINSTR p2, PSTR reason,ENUM_STRATEGY iES_Type);

    void PrintIt(INSTR_LIST* list, CFunc* pFunc);
    void DoIt(INSTR_LIST* list, CExprManage* expr);
    bool DoIt_Addon(INSTR_LIST* list, CExprManage* expr);
    bool IfAny()
    {
        return this->m_es != ES_Error;
    }
};

extern CStrategy g_CStrategy;

#endif // CStrategy__H