// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "stdafx.h"
#include "CStrategy.h"


CStrategy g_CStrategy;

bool CStrategy::DoIt_Addon(INSTR_LIST* list, CExprManage* expr)
{
    assert(m_es == ES_Instr_can_Elim_21E);
    M_t* v = this->can_elim.pvar;
    PINSTR p1 = this->can_elim.p1;
    PINSTR p2 = this->can_elim.p2;
    
    return false;
}

void AddOn_serial(Pst_InstrAddOn &p1, st_InstrAddOn* p2)
{
    Pst_InstrAddOn* pp = &p1;
    while (*pp != NULL)
    {
        pp = &p1->pChild;
    }
    *pp = p2;
}

void CStrategy::DoIt(INSTR_LIST* list, CExprManage* expr)
{
	switch (m_es)
	{
	case ES_Instr_can_Delete:
		{;
			list->RemoveAt(list->Find(this->can_delete.pinstr));	// 删除伪代码信息
		}
		break;
    case ES_Instr_can_Elim_21E:		// 普通写入操作，后读出，后结束
        {
            //if (this->DoIt_Addon(list,expr))
            //    break;
            
			PINSTR p1 = this->can_elim.p1;
			PINSTR p2 = this->can_elim.p2;

            M_t* v = this->can_elim.pvar;
            M_t* varnew = expr->CreateNewTemVar(v->size);

            p1->var_w.thevar = varnew;
            if (p2->var_r1.thevar == v)
                p2->var_r1.thevar = varnew;
            if (p2->var_r2.thevar == v)
                p2->var_r2.thevar = varnew;
        }
        break;
    case ES_Instr_can_Elim_31E:
        {
			PINSTR p1 = this->can_elim.p1;
			PINSTR p2 = this->can_elim.p2;

            assert(p2->var_r2.type == v_Immed);
            signed int d = p1->var_r2.d;
            if (p1->type == i_Add)
                d = -d;
            p2->var_r2.d += d;
			list->RemoveAt(list->Find(p1));
        }
        break;
	case ES_Instr_can_Elim_25E:
		{
			PINSTR p1 = this->can_elim.p1;
			PINSTR p2 = this->can_elim.p2;
            p1->var_w = p2->var_w;
            /*
			if (IsSame(&p2->var_r1, this->can_elim.pvar))
				p2->var_r1 = p1->var_r1;
			if (IsSame(&p2->var_r2, this->can_elim.pvar))
				p2->var_r2 = p1->var_r1;
                */

			list->RemoveAt(list->Find(p2));
		}
		break;
	// 替换并删除
	case ES_Instr_can_Elim_63:
		{
			PINSTR p1 = this->can_elim.p1;
			PINSTR p2 = this->can_elim.p2;

			if (p2->var_r1.thevar == this->can_elim.pvar)	// 第一个变量是要替换的变量
            {
				p2->var_r1 = p1->var_r1;
                AddOn_serial(p2->va_r1.pao, p1->va_r1.pao);
            }
			if (p2->var_r2.thevar == this->can_elim.pvar)	// 第二个变量是要替换的变量
            {
				p2->var_r2 = p1->var_r1;
                AddOn_serial(p2->va_r2.pao, p1->va_r1.pao);
            }

			list->RemoveAt(list->Find(p1));
		}
		break;
	// 只替换，不删除
	case ES_Instr_can_Elim_hxd_61:		// 只有当是eax =ebx 或eax= v_4 等 &&  pi[0]->va_r1.pao == NULL时才调用
		{
			PINSTR p1 = this->can_elim.p1;
			PINSTR p2 = this->can_elim.p2;

			if (p2->var_r1.thevar == this->can_elim.pvar)	// 第一个变量是要替换的变量
            {
				p2->var_r1 = p1->var_r1;

				if(p1->va_r1.pao !=NULL
				&& p1->va_r1.pao->type == IA_GetAddress
					)
				{
					st_InstrAddOn* pnew = new st_InstrAddOn;
					pnew->type = IA_GetAddress;
					pnew->pChild = p2->va_r1.pao;
					p2->va_r1.pao = pnew;
				}

				//AddOn_serial(p2->va_r1.pao, p1->va_r1.pao);
            }

			if (p2->var_r2.thevar == this->can_elim.pvar)	// 第二个变量是要替换的变量
            {
				p2->var_r2 = p1->var_r1;

				if(p1->va_r1.pao !=NULL
				&& p1->va_r1.pao->type == IA_GetAddress
					)
				{
					st_InstrAddOn* pnew = new st_InstrAddOn;
					pnew->type = IA_GetAddress;
					pnew->pChild = p2->va_r2.pao;
					p2->va_r2.pao = pnew;
				}

				//AddOn_serial(p2->va_r2.pao, p1->va_r1.pao);
            }

			//list->RemoveAt(list->Find(p1));
		}
		break;

	}

    this->m_es = ES_Error;	// 已经处理完成
}

// 输出信息
void CStrategy::PrintIt(INSTR_LIST* list, CFunc* pFunc)
{
    log_prtl("---优化处理---");
	switch (m_es)
	{
	case ES_Instr_can_Delete:
		{
			log_prtl("%s: %s", this->can_delete.reason, this->can_delete.pvar->GetName());
			CString s = this->PrintOne(list, this->can_delete.pinstr, pFunc);
			s += "  <----";
		}
		break;
	case ES_Instr_can_Elim_63:
	case ES_Instr_can_Elim_31E:
	case ES_Instr_can_Elim_25E:
	case ES_Instr_can_Elim_21E:
	case ES_Instr_can_Elim_hxd_61:
		{
			log_prtl("%s:", this->can_elim.reason);
			//log_prtl("%s: %s", this->can_elim.reason, this->can_elim.pvar->GetName());
			CString s1 = this->PrintOne(list, this->can_elim.p1, pFunc);
			CString s2 = this->PrintOne(list, this->can_elim.p2, pFunc);
			log_prtl(s1.GetBuffer());
			log_prtl(s2.GetBuffer());
		}
		break;
	}
    log_prtl("------------");
}

CString CStrategy::PrintOne(INSTR_LIST* list, PINSTR pinstr, CFunc* pFunc)
{
    POSITION pos = list->GetHeadPosition();
    while (pos)
    {
        PINSTR p = list->GetNext(pos);
        if (p == pinstr)
        {
            CFunc_Prt the(pFunc);
            POSITION pos1 = list->Find(pinstr);
            CString s = the.prt_the_instr(p);
            return s;
        }
    }
    return "";
}

void CStrategy::AddOne_CanDelete(M_t* pvar, PINSTR pinstr, PSTR reason)
{
	this->m_es = ES_Instr_can_Delete;			// 可以删除
	this->can_delete.pvar = pvar;
	this->can_delete.pinstr = pinstr;
	strcpy(this->can_delete.reason, reason);
}

void CStrategy::AddOne_CanEliminate(M_t* pvar, PINSTR p1, PINSTR p2, PSTR reason,ENUM_STRATEGY iES_Type)
{

	this->m_es = iES_Type;
	this->can_elim.pvar = pvar;
	this->can_elim.p1 = p1;
	this->can_elim.p2 = p2;
	strcpy(this->can_elim.reason, reason);
}
