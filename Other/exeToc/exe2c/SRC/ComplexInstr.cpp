// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "stdafx.h"
//	ComplexInstr.cpp

#include	"CISC.H"
bool g_f_Step_by_Step = false;
bool g_any1_return_TRUE = false;
bool Step_by_Step();


const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
const char finger_long_if [] 	= "0_jxx1_jmp2_from1_0_from2_";
const char finger_if [] 	= "0_jxx1_0_from1_";
const char finger_if_else[] = "0_jxx1_0_jmp2_from1_0_from2_";
const char finger_while[] 	= "from1_0_jxx2_0_jmp1_from2_";
const char finger_dowhile[] = "from1_0_jxx1_";
const char finger_dowhile_2[] = "from1_0_jxx1_from2_";	
						
//	如果其中有break，就会出现这种情况，我暂时还找不出更好的办法
	// for(1;3;2)
	// {
	//	4
	// }
// 释放内存
void Hxd_DeleteUsedJumpList(JUMPOPTM_LIST *used_list)
{
	if (used_list)
	{
		POSITION pos = used_list->GetHeadPosition();
		while (pos)
		{
			POSITION savpos = pos;
			st_JumpOptm * p = used_list->GetNext(pos);

			used_list->RemoveAt(savpos);
			SAFE_DELETE(p);
		}

	}
}

// 开始跳转流程分析
bool CInstrList::Hxd_JumpFlow()
{
	JUMPOPTM_LIST used_list;	//	分析过的instr
	char hxd_tbl_c[256];

	Get_Jump_Use_Flow(used_list);//先得到变量使用图
	while(Optim_Jump_flow_NT(used_list,hxd_tbl_c))
	{
		log_prtl("%s",hxd_tbl_c);
		Hxd_DeleteUsedJumpList(&used_list);
		return true;
	}

	log_prtl("%s",hxd_tbl_c);
	Hxd_DeleteUsedJumpList(&used_list);
	return false;
}

// 得到跳转标识
char  CInstrList::HowJumpUse_Char(st_JumpOptm* the)
{
	PINSTR pinstr = the->pinstr;

	switch(pinstr->type)
	{
	case i_Jump:
		if(pinstr->jmp.jmp_type == JMP_jmp)
		{
			if(the->JumpType == Jump_Down)	// 向上跳
				return 'J';		// Jump
			else
				return 'C';		// Continue
		}
		else
		{
			if(the->JumpType == Jump_Down)	// 向上跳
				return 'I';		// If
			else
				return 'W';		// While
		}
		break;
	case i_Label:
		switch(the->iLableType)
		{
		case Form_Up:
			return 'E';	// end if
		case Form_Down:
			return 'D';	// do
		case Form_All:
			return 'T'; // two,以上两都有
		}
		break;
	case i_Return:
		return 'R';

	}

	return 'U';

}


// 得到跳转使用伪代码列表
void CInstrList::Get_Jump_Use_Flow(JUMPOPTM_LIST& used_list)
{
	INSTR_LIST* list = this->m_list;		// 伪代码表
    POSITION pos = list->GetHeadPosition();

    while (pos)		// 保存当前变量使用信息、i_Label、i_Return、i_Jump
    {
        POSITION savpos = pos;
        PINSTR p = list->GetNext(pos);

		if(p->bJmpFlowOk)
			continue;

		st_JumpOptm* the;
		PINSTR pJmp;

		bool bUp =false;
		bool bDown =false;

		switch(p->type)
		{
		case i_Jump:	// 跳转
			the = new st_JumpOptm;        
            the->pinstr = p;
			the->pos = savpos;
			the->bJxx = true;
			// 跳转的方向
			if(p->jmp.linear < p->jmp.jmpto_off)	// 当前地址小于跳转后的地址，说明是向下跳
				the->JumpType = Jump_Down;		// 向下跳
			else
				the->JumpType = Jump_Up;		// 向上跳
            used_list.AddTail(the);
			break;

		case i_Label:	// 标识
			the = new st_JumpOptm;
            the->pinstr = p;
			the->pos = savpos;
			the->bLable = true;
			// 得到jump来的数量与地址
			pJmp=p->label.ref_instr;
			the->iLableJmpFromNum = 0;
			the->dwJumpFrom[the->iLableJmpFromNum++]=pJmp->jmp.linear;	// i_Jump指令的地址

			if(pJmp->jmp.linear < p->label.label_off)
				bUp = true;			// 从上跳下来的
			else
				bDown = true;
			
			while(pJmp->jmp.next_ref_of_this_label)
			{
				pJmp = pJmp->jmp.next_ref_of_this_label;
				the->dwJumpFrom[the->iLableJmpFromNum++]=pJmp->jmp.linear;
				if(pJmp->jmp.linear < p->label.label_off)
					bUp = true;			// 从上跳下来的
				else
					bDown = true;
			}
			if(p->label.bJmpFlowNum == the->iLableJmpFromNum)	// 已经全部处理过了
			{
				list->RemoveAt(savpos);		// 处理完成后删除
				//p->bJmpFlowOk =true;
			}
			else
			{
				if(bUp)
				{
					if(bDown)
						the->iLableType = Form_All;
					else
						the->iLableType = Form_Up;	// 向上跳下来的
				}
				else
					the->iLableType = Form_Down;

				used_list.AddTail(the);
			}
			break;

		case i_Return:
			the = new st_JumpOptm;       
            the->pinstr = p;
			the->pos = savpos;
            used_list.AddTail(the);
			break;
		default:
			break;
		}

    }

}

// 分析跳转流程
bool CInstrList::Optim_Jump_flow_NT(JUMPOPTM_LIST& Jumplist,char hxd_tbl_c[])
{
    int n = 0;
    char tbl_c[256];
    PJUMPOPTM tbl_JumpOptm[256];

    POSITION pos = Jumplist.GetHeadPosition();
    while (pos)
    {
        PJUMPOPTM the = Jumplist.GetNext(pos);
        char b = HowJumpUse_Char(the);

        tbl_c[n] = b; tbl_c[n+1] = '\0';
        tbl_JumpOptm[n] = the;
        n++;

		memcpy(hxd_tbl_c,tbl_c,n+1);

        //if (n >= 1)
        //{
        //    const char* pb = tbl_c + n - 1;		// 最后一个特征码
        //    PINSTR* pi = tbl_pinstr + n - 1;		// 最后一个伪代码

        //    if (strcmp(pb, "A") == 0 )
        //    {
  
        //    }

        //}

        if (n >= 2)		// 最后两个特征码
        {
            const char* pb = tbl_c + n - 2;
            PJUMPOPTM* pi = tbl_JumpOptm + n - 2;

			if ( strcmp(pb, "RR") == 0 
			|| strcmp(pb, "RJ") == 0
			|| strcmp(pb, "JR") == 0
			|| strcmp(pb, "JJ") == 0
			)
			{
				pi[1]->pinstr->bJmpFlowOk =true;	// 第二个不用管了
				return true;
			}

            if ( strcmp(pb, "IE") == 0 )
            {
				if(pi[0]->pinstr->jmp.jmpto_off == pi[1]->pinstr->label.label_off)
				{
					// 标明已经处理过了
					pi[0]->pinstr->bJmpFlowOk =true;
					pi[1]->pinstr->label.bJmpFlowNum++;

					// 处理
					Hxd_If(pi[0]->pos,pi[1]->pos,COMP_if);
					return true;
				}

            }

			if ( strcmp(pb, "DW") == 0 )
            {
				if(pi[1]->pinstr->jmp.jmpto_off == pi[0]->pinstr->label.label_off)
				{
					// 标明已经处理过了
					pi[0]->pinstr->label.bJmpFlowNum++;
					pi[1]->pinstr->bJmpFlowOk =true;

					// 处理，与 if格式相同
					Hxd_do_While(pi[0]->pos,pi[1]->pos);
					return true;
				}
            }

			if (strcmp(pb, "IW") == 0)		// while 中的一个break
			{
				pi[0]->pinstr->bJmpFlowOk =true;

				Hxd_Break(pi[0]->pos,pi[1]->pos);
				return true;
			}

        }

		if (n >= 3)				// 最后三位特征码
		{
			const char* pb = tbl_c + n - 3;
			PJUMPOPTM* pi = tbl_JumpOptm + n - 3;

			if (strcmp(pb, "IRE") == 0)	// 要继续处理
			{
				if(pi[0]->pinstr->jmp.jmpto_off == pi[2]->pinstr->label.label_off)
				{
					// 标明已经处理过了
					pi[0]->pinstr->bJmpFlowOk =true;
					pi[1]->pinstr->bJmpFlowOk =true;
					pi[2]->pinstr->label.bJmpFlowNum++;

					// 处理
					Hxd_If(pi[0]->pos,pi[2]->pos,COMP_if);

					return true;
				}

			}

		}

		if (n >= 4)
		{
			const char* pb = tbl_c + n - 4;
			PJUMPOPTM* pi = tbl_JumpOptm + n - 4;

			if (strcmp(pb, "DICE") == 0)	// while
			{
				bool bOne =	false;
				bool bTwo = false;

				if(pi[1]->pinstr->jmp.jmpto_off == pi[3]->pinstr->label.label_off)
					bOne = true;


				if(pi[2]->pinstr->jmp.jmpto_off == pi[0]->pinstr->label.label_off)
					bTwo = true;

				if(bOne &&	bTwo)
				{
					// 标明已经处理过了
					pi[0]->pinstr->label.bJmpFlowNum++;
					pi[1]->pinstr->bJmpFlowOk =true;
					pi[2]->pinstr->bJmpFlowOk =true;
					pi[3]->pinstr->label.bJmpFlowNum++;

					// 处理
					Hxd_While(pi[0]->pos,pi[1]->pos,pi[2]->pos,pi[3]->pos);
					return true;
				}
			}

			if (strcmp(pb, "IIEE") == 0)	// IF_OR
			{
				bool bOne =	false;
				bool bTwo = false;

				if(pi[0]->pinstr->jmp.jmpto_off == pi[2]->pinstr->label.label_off)
					bOne = true;

				if(pi[1]->pinstr->jmp.jmpto_off == pi[3]->pinstr->label.label_off)
					bTwo = true;

				if(bOne &&	bTwo)
				{
					// 标明已经处理过了
					
					pi[0]->pinstr->bJmpFlowOk =true;
					pi[1]->pinstr->bJmpFlowOk =true;
					pi[2]->pinstr->label.bJmpFlowNum++;
					pi[3]->pinstr->label.bJmpFlowNum++;

					// 处理
					Hxd_If_Or(pi[0]->pos,pi[1]->pos,pi[2]->pos,pi[3]->pos);
					return true;
				}

			}

			if (strcmp(pb, "IJEE") == 0)	// IF_else
			{
				bool bOne =	false;
				bool bTwo = false;

				if(pi[0]->pinstr->jmp.jmpto_off == pi[2]->pinstr->label.label_off)
					bOne = true;

				if(pi[1]->pinstr->jmp.jmpto_off == pi[3]->pinstr->label.label_off)
					bTwo = true;

				if(bOne &&	bTwo)
				{
					// 标明已经处理过了
					
					pi[0]->pinstr->bJmpFlowOk =true;
					pi[1]->pinstr->bJmpFlowOk =true;
					pi[2]->pinstr->label.bJmpFlowNum++;
					pi[3]->pinstr->label.bJmpFlowNum++;

					// 处理
					Hxd_If_else(pi[0]->pos,pi[1]->pos,pi[2]->pos,pi[3]->pos);
					return true;
				}

			}

		}	// n> =4

    }	// while

    return false;
}

void	CInstrList::Hxd_Break(POSITION Jxx,POSITION While)
{
	Hxd_Add_CplxBegin_CplxEnd(Jxx,Jxx,COMP_break);

	// 如果While 后面就是对应的i_Label 就不管了，否则要复制if

	INSTR_LIST* list = this->m_list;

	PINSTR pJxx = list->GetAt(Jxx);
	POSITION pos = While;
	list->GetNext(pos);
	PINSTR p2 = list->GetNext(pos);

	if( p2->type == i_Label)
	{
		if(pJxx->jmp.jmpto_off == p2->label.label_off)
		{
			p2->label.bJmpFlowNum++;
			return;
		}
	}

	// 复制一个i_Jxx伪代码插入

	PINSTR pnew = new INSTR;
	pnew->type = i_Jump;
	//pnew->bJmpFlowOk = true;	// 已经处理过

	pnew->jmp.jmp_type = pJxx->jmp.jmp_type;
	pnew->jmp.jmpto_off = pJxx->jmp.jmpto_off;
	pnew->jmp.linear = pJxx->jmp.linear;

	//pnew->var_w = pJxx->var_w;
	pnew->var_r1 = pJxx->var_r1;
	pnew->var_r2 = pJxx->var_r2;
	pnew->va_r1 = pJxx->va_r1;
	pnew->va_r2 = pJxx->va_r2;

	list->InsertAfter(While,pnew);


	return;
}

void	CInstrList::Hxd_While(POSITION firstpos, POSITION jxx,POSITION jump,POSITION endpos)
{
	Hxd_Add_Begin_End(firstpos,jxx);
	Hxd_Add_Begin_End(jxx,jump);
	Hxd_Add_CplxBegin_CplxEnd(firstpos,endpos,COMP_while);
	return;
}

void	CInstrList::Hxd_do_While(POSITION firstpos,POSITION endpos)
{
	Hxd_Add_Begin_End(firstpos,endpos);
	Hxd_Add_CplxBegin_CplxEnd(firstpos,endpos,COMP_do_while);
	return;
}

void	CInstrList::Hxd_If(POSITION firstpos,POSITION endpos,enum_COMP comptype)
{
	Hxd_Add_Begin_End(firstpos,endpos);
	Hxd_Add_CplxBegin_CplxEnd(firstpos,endpos,comptype);
	return;
}

void	CInstrList::Hxd_If_Or(POSITION firstpos, POSITION jxx,POSITION label,POSITION endpos)
{
	INSTR_LIST* list = this->m_list;

	POSITION pos = firstpos;

	list->GetNext(pos);

	if(jxx != pos)	// 不是相连的两个判断
		Hxd_Add_Begin_End(firstpos,jxx);

	Hxd_Add_Begin_End(label,endpos);
	Hxd_Add_CplxBegin_CplxEnd(firstpos,endpos,COMP_if_Or);
	return;
}

void	CInstrList::Hxd_If_else(POSITION firstpos, POSITION jxx,POSITION label,POSITION endpos)
{
	Hxd_Add_Begin_End(firstpos,jxx);
	Hxd_Add_Begin_End(label,endpos);
	Hxd_Add_CplxBegin_CplxEnd(firstpos,endpos,COMP_if_else);
	return;
}


// 添加 i_CplxBegin 与 i_CplxEnd指令
// 新增i_Label伪代码
void CInstrList::Hxd_Add_CplxBegin_CplxEnd(POSITION firstpos, POSITION endpos,enum_COMP comptype)
{
	INSTR_LIST* list = this->m_list;

	PINSTR begin = new INSTR;
	PINSTR end = new INSTR;

	begin->type = i_CplxBegin;		// 添加 i_CplxBegin 指令
	end->type = i_CplxEnd;			// 添加 i_CplxEnd   指令
	begin->begin.m_end = end;

	begin->begin.type=comptype;		// 跳转的类型

	PINSTR p = list->GetAt(firstpos);

	begin->begin.m_break = p->begin.m_break;	//	继承
	begin->begin.m_conti = p->begin.m_conti;	//	继承

	for (;;)
	{
		if (begin)
		{
			PINSTR p = list->GetAt(firstpos);
			if (p->type != i_Label)
			{
				list->InsertBefore(firstpos,begin);
				begin = NULL;	//	把它清掉，以后不再处理
			}
		}
		if (end)
		{
			PINSTR p = list->GetAt(endpos);
			if (p->type != i_Label)
			{
				list->InsertAfter(endpos,end);
				end = NULL;
			}

		}
		if (begin == NULL && end == NULL)
			return;	//	都解决掉了，没什么可作的了

		// 复制一个i_Label伪代码插入
		PINSTR pnew = new INSTR;
		pnew->type = i_Label;
		pnew->bJmpFlowOk = true;	// 已经处理过
		pnew->label.ref_instr = 0;

		PINSTR p;
		if (begin)	//	先任取一个吧
		{
			p = list->GetAt(firstpos);
			list->InsertAfter(firstpos,pnew);
			list->InsertAfter(firstpos,begin);
			begin = NULL;	//	把它清掉，以后不再处理
		}
		else
		{
			p = list->GetAt(endpos);

			list->InsertBefore(endpos,pnew);
			list->InsertBefore(endpos,end);
			end = NULL;		//	把它清掉，以后不再处理
		}
		pnew->label.label_off = p->label.label_off;
	}

	return;
}

// 添加i_Begin / i_End
bool	CInstrList::Hxd_Add_Begin_End(POSITION firstpos, POSITION endpos)
{
	INSTR_LIST* list = m_list;

	PINSTR begin = new INSTR;
	PINSTR end = new INSTR;
	begin->type = i_Begin;
	end->type = i_End;
	begin->begin.m_end = end;

	PINSTR p = list->GetAt(firstpos);
	begin->begin.m_break = p->begin.m_break;	//	继承
	begin->begin.m_conti = p->begin.m_conti;	//	继承

	list->InsertAfter(firstpos,begin);
	list->InsertBefore(endpos,end);

	return true;
}

bool Step_by_Step()
{	//	这是为了在非step_by_step状态时，能提高点速度
	//	而在step_by_step状态时，又能使每一步都分得很细
	if (g_f_Step_by_Step)
		return true;
	g_any1_return_TRUE = true;
	return false;
}

int CInstrList_Finger::search_and_add(DWORD* buf,DWORD val,int* pn)
{//static function
	int n = *pn;
	for (int i=0;i<n;i++)
	{
		if (buf[i] == val)
			return i+1;
	}
	buf[n] = val;
	*pn = n+1;
	return n+1;
}
bool	CInstrList_Finger::finger_compare(PSTR f1,const char* f2)
{//static function
	for (;;)
	{
		if (*f1 == *f2)
		{
			if (*f1 == 0)
				return true;
			f1++;
			f2++;
			continue;
		}
		if (*f2 == '0' && f2[1] == '_')
		{
			f2 += 2;
			continue;
		}
		return false;
	}
}

bool	CInstrList::if_Ly_In(PINSTR p, POSITION firstpos, POSITION endpos)
{	
    INSTR_LIST* list = m_list;

	assert(endpos);
	POSITION pos = firstpos;
	while (pos)
	{
		PINSTR pinstr = list->GetNext(pos);
		if (pinstr == p)
			return true;
		if (pos == endpos)
		{
			if (p->type != i_Label)
				return false;
			pinstr = list->GetNext(pos);
			if (p == pinstr)
				return true;
			return false;
		}
	}
	return false;
}


// 确定一对匹配的i_Jump 与 i_Lable
bool	CInstrList::ifOneStatement(PINSTR pNode, POSITION firstpos, POSITION endpos)
{
    INSTR_LIST* list = this->m_list;		// 伪代码表

	assert(firstpos);
	assert(endpos);

	bool ffirst = true;
	POSITION pos = firstpos;
	while (pos && pos != endpos)
	{
		PINSTR p = list->GetNext(pos);

		if (ffirst && p->type == i_Label)	// 第二条就是对应的i_Label
		{
			ffirst = false;

			if (pos == endpos)
				return false;
			//continue;
		}
		
		if (p->type == i_Label)
		{
			PINSTR pr = p->label.ref_instr;
			while (pr)
			{
				if (! if_Ly_In(pr, firstpos, endpos) )
					return false;
				pr = pr->jmp.next_ref_of_this_label;
			}
		}
		else if (p->type == i_Jump)
		{
			if (p->jmp.jmp_type == JMP_jmp)
			{
				//if (p->jmp.the_label != pNode->begin.m_conti)
				//	alert("why?");
				if (p->jmp.the_label == pNode->begin.m_break)
					continue;
				if (p->jmp.the_label == pNode->begin.m_conti
					&& p != pNode->begin.m_not_conti)
					continue;
			}
			if (! if_Ly_In(p->jmp.the_label, firstpos, endpos) )
				return false;
		}
	}
	if (pos == endpos)
		return true;
	return false;
}

// 后一条代码
PINSTR	instr_next(INSTR_LIST* list,PINSTR p)
{
	POSITION pos = list->Find(p);
	if (pos == NULL)
		return NULL;
	list->GetNext(pos);
	if (pos == NULL)
		return NULL;
	return list->GetAt(pos);
}

// 前一条代码
PINSTR	instr_prev(INSTR_LIST* list, PINSTR p)
{
	POSITION pos = list->Find(p);
	if (pos == NULL)
		return NULL;
	list->GetPrev(pos);
	if (pos == NULL)
		return NULL;
	return list->GetAt(pos);
}

void	CInstrList_Finger::prt_partern(PINSTR phead,PSTR partern_buf)
{
    INSTR_LIST* list = this->m_list;

	if (phead->type != i_CplxBegin)
		return;

	PINSTR p = instr_next(list,phead);

	int	t = 0;
	DWORD buf[20];
	int n = 0;
	
	while (p != NULL && p != phead->begin.m_end)
	{
		if (p->type == i_Jump)
		{
            int i = search_and_add(buf,(DWORD)p->jmp.the_label,&n);
			if (p->jmp.jmp_type == JMP_jmp)
			{
                t += sprintf(partern_buf+t,"jmp%d_",i);
			}
			else
				t += sprintf(partern_buf+t,"jxx%d_",i);

		}
		else if (p->type == i_Label)
		{
			int i = search_and_add(buf,(DWORD)p,&n);
			t += sprintf(partern_buf+t,"from%d_",i);

		}
		else if (p->type == i_Begin)
		{
            t += sprintf(partern_buf+t,"0_");
			p = p->begin.m_end;
		}
		else
			;//why here
		p = instr_next(list,p);

		if (t > 120)
		{
            t += sprintf(partern_buf+t,"...");
			t;	//	avoid warning
			break;
		}
	}
}
    
bool	CInstrList_Finger::Finger_check_partern_for1(PINSTR p)
{
    CFunc_InstrList instrl(this->m_list);

#if 0
    int i = 0;  //p1
    do
    {
        i++;    //p3
    }
    while (i < 100);    //p2
#endif
    INSTR_LIST* list = this->m_list;
    PINSTR p1 = instr_prev(list,p);	
    if (p1->var_r1.type != v_Immed)
        return false;
    if (p1->var_r2.type != v_Invalid)
        return false;   //这两个条件的意思是，只接收前面是i=n的才算
    
    PINSTR p2;
    {
        PINSTR ptem = instr_next(list,p);
        if (ptem->type != i_Label)
            return false;
        p2 = ptem->label.ref_instr;
    }
    if (p1 == NULL || p2 == NULL)
        return false;
    
    if (VAR::IsSame(&p1->var_w, &p2->var_r1)
        || VAR::IsSame(&p1->var_w, &p2->var_r2))
    {
    }
    else
        return false;

    {
        list->RemoveAt(list->Find(p1));
        POSITION pos = list->Find(p);	//	insert after i_CplxBegin

        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_Begin;
        end->type = i_End;
        begin->begin.m_end = end;
        list->InsertAfter(pos,end);
        list->InsertAfter(pos,p1);
        list->InsertAfter(pos,begin);
    }

    {
        //p2是条件跳，它前面应该是i_end
        PINSTR pend = instrl.instr_prev_in_func(p2);
        assert(pend->type == i_End);

        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_Begin;
        end->type = i_End;
        begin->begin.m_end = end;
        list->InsertAfter(list->Find(pend), end);
        list->InsertAfter(list->Find(pend), begin);

        for (;;)
        {
            PINSTR p3 = instrl.instr_prev_in_func(pend);
            if (p3->type != i_Add && p3->type != i_Sub)
                break;
            list->RemoveAt(list->Find(p3));
            list->InsertAfter(list->Find(begin), p3);
        }
    }

    return true;
}

bool	CInstrList_Finger::Finger_check_partern(PINSTR p)
	// 检查 p 的partern
{
    INSTR_LIST* list = this->m_list;
	char buf[140];
	this->prt_partern(p,buf);
	
	if (finger_compare(buf, finger_if))
		p->begin.type = COMP_if;
	else if (finger_compare(buf, finger_long_if))	//	一定要放在if_else前
		p->begin.type = COMP_long_if;
	else if (finger_compare(buf, finger_if_else))
		p->begin.type = COMP_if_else;
	else if (finger_compare(buf, finger_while))
		p->begin.type = COMP_while;
	else if (finger_compare(buf, finger_dowhile))
    {
		p->begin.type = COMP_do_while;
        //特殊的do-while最好写为for
        if (this->Finger_check_partern_for1(p))
        {
            log_prtl("find for1");
            p->begin.type = COMP_for1;
        }
    }
	else if (finger_compare(buf, finger_dowhile_2))
		p->begin.type = COMP_do_while;
	else if (finger_compare(buf, finger_for))
	{	//const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
		p->begin.type = COMP_for;
		//	下面，让我们对 for 作一些调整
		PINSTR p1 = instr_next(list,p);	//	p1指向 label
		assert(p1->type == i_Jump);
		p1 = instr_next(list,p1->jmp.the_label);	//p1指向条件
		if (p1->type == i_Jump
			&& p1->jmp.jmp_type != JMP_jmp
			&& p1->var_r1.type != 0)
		{
			VAR* pvar = &p1->var_r1;
			PINSTR p2 = instr_prev(list,p);
			if (VAR::IsSame(&p2->var_w,pvar))
			{
				list->RemoveAt(list->Find(p2));
				POSITION pos = list->Find(p);	//	insert after i_CplxBegin

				PINSTR begin = new INSTR;
				PINSTR end = new INSTR;
				begin->type = i_Begin;
				end->type = i_End;
				begin->begin.m_end = end;
				list->InsertAfter(pos,end);
				list->InsertAfter(pos,p2);
				list->InsertAfter(pos,begin);
			}
		}
	}
	else
		return false;
	return true;
}

void	CFunc::Finger_it()
{
	INSTR_LIST *list = this->m_instr_list;
    CInstrList_Finger the(list);
	POSITION pos = list->GetHeadPosition();
	while (pos)
	{
		PINSTR p = list->GetNext(pos);
		if (p->type == i_CplxBegin)
		{
			the.Finger_check_partern(p);
		}
	}
}

bool	CInstrList::Flow_c(PINSTR phead)
{	//	设法从中找出子 statement 来，括住后再call Flow_a
    INSTR_LIST* list = this->m_list;

	assert(phead->type == i_CplxBegin);

	POSITION s_pos = list->Find(phead);
	POSITION e_pos = list->Find(phead->begin.m_end);
	assert(s_pos);
	assert(e_pos);
	if (phead->begin.type == COMP_switch_case)
	{	//	对swith_case，有些特殊。从第一个label作起
		POSITION pos = s_pos;
		while (pos)
		{
			PINSTR p = list->GetNext(pos);
			if (p->type == i_Label)
				break;
		}
		return Flow_cc(phead,pos,e_pos);
	}
    if (phead->begin.type == COMP_switch_case_multcomp)
    {
        bool f = false;
		POSITION pos = s_pos;
		while (pos)
		{
            POSITION savpos = pos;
			PINSTR p = list->GetNext(pos);
			if (p->type == i_Label)
				break;
            if (p->type == i_Jump && p->jmp.jmp_type == JMP_jz)
                f=true;
            if (f)
            {
                if (p->type == i_Begin)
                {
                    pos = list->Find(p->begin.m_end);
                    list->GetNext(pos);
                    continue;
                }
                if (p->type != i_Jump && p->type != i_Begin)
                {
                    //break;  //这是没有jump的default
                    return Flow_cc(phead, savpos,e_pos);
                }
            }
		}
		return Flow_cc(phead,pos,e_pos);
    }
	
	list->GetNext(s_pos);				//	skip the i_CplxBegin
	PINSTR p = list->GetNext(s_pos);	//	skip first 1
	if (p->type == i_Begin)
	{	//	比如象for, 允许头上有一个小的statement
        if (this->Flow_a(p))
			return true;
		s_pos = list->Find(p->begin.m_end);
		list->GetNext(s_pos);	//	skip i_End
		list->GetNext(s_pos);	//	skip first 1
	}
	return Flow_cc(phead,s_pos,e_pos);
}

bool	CInstrList::Flow_cc(PINSTR pNode, POSITION firstpos, POSITION endpos)
{	//	这是用来在cplx中再试图找出几个小的begin_end来
    INSTR_LIST* list = m_list;

//	三种，有头无尾，要从中找到包含头的最大的一个statement，并用i_Begin,i_End括起来
//	再从i_End到尾继续
//	Flow_c(INSTR_LIST* list, POSITION firstpos,POSITION endpos);
//	对找到的i_Begin,i_End，用Flow_a继续

	//	pNode 是检查区段所属的begin_end，只是为了提供 m_Break

	assert(firstpos);
	assert(endpos);
	if (firstpos == endpos)
		return false;

	PINSTR phead = list->GetAt(firstpos);
	if (phead->type == i_Label)
	{
		PINSTR p = instr_next(list,phead);
		if (p->type == i_Begin)	//	因为i_label后跟i_begin会使后面出死循环
		{
			phead = p;	//	放到下一项进行处理
		}
	}
	if (phead->type == i_Begin)
	{
        if (this->Flow_a(phead))
			return true;
    	POSITION pos1 = list->Find(phead->begin.m_end);
		list->GetNext(pos1);	//	skip i_End
		return Flow_cc(pNode,pos1,endpos);
	}

	POSITION okpos;
	POSITION pos1;
	pos1 = okpos = firstpos;

    do
	{
		list->GetNext(pos1);
		if (ifOneStatement(pNode, firstpos,pos1))
			okpos = pos1;	// record the last success
	} while (pos1 != endpos);

	if (okpos == firstpos)	// not find anything
	{
		pos1 = firstpos;
		list->GetNext(pos1);
		if (pos1 && pos1 != endpos)
		{
			return Flow_cc(pNode, pos1, endpos);		//	start from next
		}
		return false;	
	}

	{
		PINSTR begin = new INSTR;
		PINSTR end = new INSTR;
		begin->type = i_Begin;
		end->type = i_End;
		begin->begin.m_end = end;

		Add_Begin_End(firstpos, okpos, begin, end);

		begin->begin.m_break = pNode->begin.m_break;	//	继承
		begin->begin.m_conti = pNode->begin.m_conti;	//	继承

        this->Flow_a(begin);
	}

	//if (Step_by_Step())
	//	return true;
	
	return Flow_cc(pNode, okpos, endpos);	//next part
}

void CInstrList::Add_Begin_End(POSITION firstpos, POSITION endpos, PINSTR begin, PINSTR end)
{
    this->Add_Begin_End_1(firstpos,endpos,begin,end);

    INSTR_LIST* list = this->m_list;
    POSITION pos = list->GetHeadPosition();
    while (pos)
    {
        POSITION savpos = pos;
        PINSTR p = list->GetNext(pos);
        if (p->type == i_Nop)
        {
            list->RemoveAt(savpos);
        }
    }
}

//	因为我要两次借用同一个功能，又不想把它单独写出来，所以写一个for(;;)算了

void CInstrList::Add_Begin_End_1(POSITION firstpos, POSITION endpos, PINSTR begin, PINSTR end)
{
    INSTR_LIST* list = this->m_list;


	for (;;)
	{
		if (begin)
		{
			PINSTR p = list->GetAt(firstpos);
			if (p->type != i_Label)
			{
				list->InsertBefore(firstpos,begin);
				begin = NULL;	//	把它清掉，以后不再处理
			}
		}
		if (end)
		{
			PINSTR p = list->GetAt(endpos);
			if (p->type != i_Label)
			{
				list->InsertAfter(endpos,end);
				end = NULL;
			}

		}
		if (begin == NULL && end == NULL)
			return;	//	都解决掉了，没什么可作的了

		// 复制一个i_Label伪代码插入
		PINSTR pnew = new INSTR;
		pnew->type = i_Label;
		pnew->bJmpFlowOk = true;	// 已经处理过
		pnew->label.ref_instr = 0;

		PINSTR p;
		if (begin)	//	先任取一个吧
		{
			p = list->GetAt(firstpos);
			list->InsertAfter(firstpos,pnew);
			list->InsertAfter(firstpos,begin);
			begin = NULL;	//	把它清掉，以后不再处理
		}
		else
		{
			p = list->GetAt(endpos);

			list->InsertBefore(endpos,pnew);
			list->InsertBefore(endpos,end);
			end = NULL;		//	把它清掉，以后不再处理
		}
		pnew->label.label_off = p->label.label_off;
	
		// 有什么用呢
		PINSTR p_in = 0;
		PINSTR p_out = 0;
		PINSTR p1 = p->label.ref_instr;	// 跳转来的Jump伪代码

		while (p1)			
		{
			PINSTR pnext = p1->jmp.next_ref_of_this_label;
			if (if_Ly_In(p1,firstpos,endpos))
			{
				p1->jmp.the_label = pnew;
				p1->jmp.next_ref_of_this_label = p_in;
				p_in = p1;
			}
			else
			{
				//p1->jmp.the_label = p;	//need not
				p1->jmp.next_ref_of_this_label = p_out;
				p_out = p1;
			}
			p1 = pnext;
		}

		if (p_in)	//	it must be
			pnew->label.ref_instr = p_in;
		else
			pnew->type = i_Nop;		//	因为原来是label，就不用清var了

		if (p_out)
			p->label.ref_instr = p_out;
		else
			p->type = i_Nop;
		
	}
}

bool CInstrList::IsSwitchCase_multcomp(PINSTR begin)
{	//	判断是不是多重判断的那一种switch_case
    INSTR_LIST* list = this->m_list;

	assert(begin->type == i_CplxBegin);
	POSITION pos = list->Find(begin);
	M_t* v;

	int first = 0;
	while (pos)
	{
        POSITION savpos = pos;
		PINSTR p = list->GetNext(pos);
		first++;
		if (first == 1)
			continue;	//	第一个，肯定是i_CplxBegin，就不用看了
		if (first == 2)
		{				//	第一条指令，必须是jz sth == n
			if (p->type != i_Jump || p->jmp.jmp_type != JMP_jz
				|| p->var_r1.type == 0)
				return false;
			v = p->var_r1.thevar;	//	记下来，以后的要跟这个一样才行
			continue;
		}
		if (p->type == i_Jump)
		{
			if (p->jmp.jmp_type == JMP_jz)
			{
				if (v != p->var_r1.thevar)
					return false;
			}
			else if (p->jmp.jmp_type == JMP_jmp)	//	找到default了
			{
				if (first < 4)		//	检查太少了，不算switch case
					return false;
				return true;
			}
			else
				return false;

			continue;
		}
        //不是Jump了
        if (first < 4)		//	检查太少了，不算switch case
            return false;
        return true;
	}
	return false;
}

bool CInstrList::IsSwitchCase(PINSTR begin)
{
    INSTR_LIST* list = m_list;

	assert(begin->type == i_CplxBegin);
	POSITION pos = list->Find(begin);

	bool first = true;
	while (pos)
	{
		PINSTR p = list->GetNext(pos);
		if (p->type == i_JmpAddr)
		{
			return true;
		}
		if (p->type == i_Return)
			return false;
		if (p->type == i_Label)
			return false;
		if (p->type == i_Jump)
		{	//	只允许有一个jmp
			if (first)
				first = false;
			else
				return false;
		}
	}
	return false;
}

void	CInstrList::Flow_b(PINSTR pParentNode, POSITION firstpos, POSITION endpos)
{	//	紧凑分析
    INSTR_LIST* list = this->m_list;

	//	last not include
	if (firstpos == endpos)
	{
		//alert_prtf("why firstpos == endpos");
		return;
	}

	PINSTR begin = new INSTR;
	PINSTR end = new INSTR;
	begin->type = i_CplxBegin;		// 添加 i_CplxBegin 指令
	end->type = i_CplxEnd;			// 添加 i_CplxEnd   指令
	begin->begin.m_end = end;

	Add_Begin_End(firstpos, endpos, begin, end);
	
	POSITION pos = endpos;
	list->GetPrev(pos);		//now it point to i_CplxEnd
	assert(list->GetAt(pos) == end);
	list->GetPrev(pos);		//now it point to last instr in body
	PINSTR plast = list->GetAt(pos);

	PINSTR plast2 = instr_prev(list,plast);	//前一条指令

	PINSTR pNode = begin;
	pNode->begin.m_break = pParentNode->begin.m_break;	//	继承
	pNode->begin.m_conti = pParentNode->begin.m_conti;	//	继承

	PINSTR pfirst = instr_next(list,begin);
	PINSTR psecond = instr_next(list,pfirst);

	if (pfirst->type == i_Label
		|| (pfirst->type == i_Jump && pfirst->jmp.jmp_type == JMP_jmp && psecond->type == i_Label)
		)
	{	//	这是我认为 break 的条件 ！
		PINSTR pconti;
		if (pfirst->type == i_Label)	//	如果是第一种情况
			pconti = pfirst;
		else
			pconti = psecond;
		pconti->label.f_conti = true;

		if (plast->type == i_Label)		//	如果最后一条指令是个label，那它肯定是break
			pNode->begin.m_break = plast;
		else
			pNode->begin.m_break = 0;
		
		pNode->begin.m_conti = pconti;

		if (plast->type == i_Label)
		{
			if (plast2->type == i_Jump
				&& plast2->jmp.jmp_type == JMP_jmp
				&& plast2->jmp.the_label == pconti)
			{	//	这是 while !
				pNode->begin.m_not_conti = plast2;
			}
		}
		else if (plast->type == i_Jump
				 && plast->jmp.jmp_type == JMP_jmp
				 && plast->jmp.the_label == pconti)
		{
			pNode->begin.m_not_conti = plast;
		}
		else
		{
			//do_while的continue 以后再处理
		}
	}
    else if (plast->type == i_Label && IsSwitchCase(begin))
	{
		pNode->begin.m_break = plast;
		//conti not change
		//	这是switch_case 的特点，就是只用break,却不用continue,非常特殊。
		pNode->begin.type = COMP_switch_case;
		//alert("case find 00");
	}
	else if (plast->type == i_Label && IsSwitchCase_multcomp(begin))
	{
		pNode->begin.m_break = plast;
		//conti not change
		//	这是switch_case 的特点，就是只用break,却不用continue,非常特殊。
		pNode->begin.type = COMP_switch_case_multcomp;
		//alert("case find 01");
	}
	//if (Step_by_Step())
	//	return;		//	我们已经加了个i_CplxBegin_End就算干过活了

	Flow_c(begin);	//	对这个i_CplxBegin再作些处理
}

//	流程分析第一步
//	对这个区间进行分析。以后可以递归
bool	CInstrList::Flow_a(PINSTR pNode)
{
    INSTR_LIST* list = this->m_list;		// 伪代码列表
	assert(pNode->type == i_Begin);			// 断言

	POSITION endpos = list->Find(pNode->begin.m_end);	// 与begin对应的end 位置
	
	POSITION firstpos = list->Find(pNode);	// begin位置
	list->GetNext(firstpos);				// 从第二条指令开始
	assert(firstpos);						// 断言
	if (firstpos == endpos)					// 第二条就是end ，不用处理了
		return false;
	
	//	Flow_a 是指向一个begin_end
	//	而Flow_aa是指向一个区间，并不一定是begin_end

	return Flow_aa(firstpos,endpos);
}

// 松散分析
// return true表示分析有进展
// pBlockHeadNode 是块头的那个begin,用于查询break和continue地址
// 分析 i_Jump 或 i_Label指令
bool	CInstrList::Flow_aa(POSITION firstpos, POSITION endpos)
{	

    INSTR_LIST* list = this->m_list;	// 伪代码列表
	
	assert(firstpos);
    assert(endpos);
	
	if (firstpos == endpos)
		return false;

	POSITION pos = firstpos;

	while(pos && pos != endpos)
	{
		POSITION savepos = pos;
		PINSTR pCur = list->GetNext(pos);		// 下一条

		switch(pCur->type)
		{
		case i_Jump:
			{
				if (pCur->jmp.jmp_type == JMP_jmp)
				{
					if (pCur->jmp.the_label == pCur->begin.m_break)
					{
						return Flow_aa(savepos, endpos);
					}
					else if (pCur->jmp.the_label == pCur->begin.m_conti
						&& pCur != pCur->begin.m_not_conti)
					{
						return Flow_aa(savepos, endpos);
					}
				}
				else
				{
					PINSTR p1 = pCur->jmp.the_label;	// i_Label 伪代码
					POSITION pos1 = list->Find(p1);		// i_Label位置
					assert(pos1);

					while (pos1)
					{
						if (pos1 == endpos)				// i_Label位置 是就endpos位置
							break;
						else if (ifOneStatement(pCur, savepos,pos1))	//	找到一个紧凑结构
						{	
							Flow_b(pCur, savepos,pos1);
							//if (Step_by_Step())
							//	return true;	//	可以在这里直接return TRUE了
							return Flow_aa(firstpos,endpos);;
						}

						list->GetNext(pos1);	// 下一条指令
					}
				}
			}
			break;

		case i_Label:
			{
				PINSTR p1 = pCur->label.ref_instr;
				POSITION pos1 = list->Find(p1);
				while (pos1 && pos1 != endpos)
				{
					list->GetNext(pos1);
					assert(pos1);
					if (ifOneStatement(pCur, savepos,pos1))
					{
						Flow_b(pCur, savepos,pos1);
						//if (Step_by_Step())
						//	return true;	//	可以在这里直接return TRUE了
						return Flow_aa(pos1,endpos);
					}
					//assert(pos1);
				}
			}
			break;

		case i_Begin:
			{
				if (this->Flow_a(pCur))			// 递归
					return true;

				// 下面的语句有什么用
				POSITION pos1 = list->Find(pCur->begin.m_end);
				assert(pos1);
				return Flow_aa(pos1,endpos);
			}

		case i_CplxBegin:
			{
				POSITION pos1 = list->Find(pCur->begin.m_end);
				assert(pos1);

				if (Flow_c(pCur))
					return true;

				if (pCur->begin.type == COMP_unknown)	//	这时应该试图识别它
				{	
					CInstrList_Finger the(list);
					if (the.Finger_check_partern(pCur))
						return true;
				}

				return Flow_aa(pos1, endpos);	
			}
		}
		
	}

	return false;
}
/*
	有三种分析，一定要分清楚。

	一种是，有头有尾，已知这是一个松散的statement,要在其中找到一个个的complex statement
	Flow_a(PINSTR phead, INSTR_LIST* list);
	对找到的complex,用Flow_b处理
	
	二种，有头有尾，且已知这是一个单一的complex statement，需要自己括自己
	Flow_b(INSTR_LIST* list, POSITION firstpos,POSITION endpos);
	括住自己后，call一个Flow_c

	三种，输入一个单一的complex statement,设法要从中找到包含头的最大的一个statement，并用i_Begin,i_End括起来
	再从i_End到尾继续
	Flow_c(PINSTR phead, INSTR_LIST* list);
	对找到的i_Begin,i_End，用Flow_a继续
	
*/

// --------------------------------------------------------------
void	CFunc_Prt::add_default_entry(CasePrt_List* list, PINSTR thelabel)
{	//	delete all same as default
    //static function

    if (thelabel->type == i_Label)
    {
        POSITION pos = list->GetHeadPosition();
        while (pos)
        {
            POSITION savpos = pos;
            OneCase* p = list->GetNext(pos);
            if (p->thelabel->label.label_off == thelabel->label.label_off)
            {
                //alert_prtf("delete case %d",p->case_n);
                list->RemoveAt(savpos);
            }
        }
    }
	OneCase* pnew = new OneCase;
	pnew->case_n = 0xffffffff;		//	不能只检查这一个条件，最后一个才是最重要的
	pnew->thelabel = thelabel;
	list->AddTail(pnew);
}

void	CFunc_Prt::Add_case_entry(CasePrt_List* list, int case_n, PINSTR thelabel)
{	//	必须先把switch case的所有项保存起来，最后一起打印
	//alert("add case entry");
    //static function

	POSITION pos = list->GetHeadPosition();
	while (pos)
	{
		POSITION savpos = pos;
		OneCase* p = list->GetNext(pos);
		if (p->case_n == case_n)
			error("why same case");
        if (p->thelabel->label.label_off >= thelabel->label.label_off)
		{
			pos = savpos;
			break;
		}
	}

	OneCase* pnew = new OneCase;
	pnew->case_n = case_n;
	pnew->thelabel = thelabel;
	if (pos)
		list->InsertBefore(pos, pnew);
	else
		list->AddTail(pnew);
	return;
}
