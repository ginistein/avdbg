// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "stdafx.h"
#include	"CISC.H"
#include "cexe2c.h"
#include "CFuncStep1.h"

static void	Add_in_order(EALIST *list, ea_t i)
{
	if (list->IsEmpty())
	{
		list->AddHead(i);
		return;
	}
	POSITION pos = list->GetHeadPosition();
	while (pos)
	{
		POSITION savpos = pos;
		ea_t ea = list->GetNext(pos);
		if (ea == i)
			return;		//	已经有了，就算了
		if (ea > i)
		{
			list->InsertBefore(savpos,i);
			return;
		}
	}
    list->AddTail(i);
}

//==========================================

//	判断是否为enter,leave或mov esp,??之类的指令
static bool	isLeave(AsmCode* p)
{	
	if (p->xcpu.opcode == C_LEAVE)
		return true;
	if (p->xcpu.opcode == C_MOV
		&& (p->xcpu.op[0].mode == OP_Register)
		&& (p->xcpu.op[0].reg.reg_index == _ESP_)	//mov esp,?
		)
		return true;
	return false;
}

static bool	any_free_ea(EALIST *jxxlist, EALIST *usedlist, ea_t* pea)	// 返回不在usedlistg列表中的地址
{
	POSITION pos = jxxlist->GetHeadPosition();
	while (pos)
	{
		ea_t ea = jxxlist->GetNext(pos);
		if (! usedlist->Find(ea))
		{
			*pea = ea;
			return true;
		}
	}
	return false;
}

void CFuncStep1::check_if_switch_case(ea_t cur_off, CaseList* pcaselist,EALIST* pjxxlist,XCPUCODE* pxcpu)
{//static function
	if (pxcpu->opcode != C_JMP)
		return;
	if (pxcpu->op[0].mode != OP_Address)
		return;
	if (pxcpu->op[0].addr.base_reg_index != _NOREG_)
		return;
	if (pxcpu->op[0].addr.off_reg_index == _NOREG_)
		return;
	if (pxcpu->op[0].addr.off_reg_scale != 4)
		return;
	if (pxcpu->op[0].addr.off_value <= 0x401000)
		return;
	//alert("switch case find 1");
	//Code_SwitchCase1();

	ea_t ptbl = pxcpu->op[0].addr.off_value;
	//alert_prtf("table is %x",ptbl);
	if (! IfInWorkSpace(ptbl))
		return;		//	确认这个table有效
	
	//alert("switch case find 2");

	ea_t d = Peek_D(ptbl);
	if (! IfInWorkSpace(d))
		return;		//	确认第一条有效
	
	//alert("switch case find 3");

	ea_t break_off = 0;	//	下面，要确定break_off的值
	POSITION pos = pjxxlist->GetHeadPosition();
	while (pos)
	{
		ea_t ea = pjxxlist->GetNext(pos);
		if (ea > cur_off)
		{
			break_off = ea;
			break;
		}
	}
	if (break_off == 0)
		return;		//	没找到条件跳?不对劲
	
	//alert("switch case find 4");


	//if (pjxx->jmp.jmp_type != JMP_ja)
	//	return;		//	不是ja,也不对
	if (d < cur_off || d > break_off)
		return;		//	第一个就没对准，不行

	//alert("switch case find 5");

	//alert("rreally switch case");
	CASE_t *pnew = new CASE_t;
	pnew->jxx_opcode = cur_off;
	pnew->caselist = new EALIST;
	pcaselist->AddHead(pnew);

	for (int i=0;;i++)
	{
		d = Peek_D(ptbl+i*4);
//		if (! IfInWorkSpace(d))
		if (d < cur_off || d > break_off)
			break;
		Add_in_order(pjxxlist,d);
		pnew->caselist->AddTail(d);
	}
}

void CFuncStep1::CheckIfJustSwitchCase(CaseList& caselist, ea_t ea)
{
    POSITION pos1 = caselist.GetHeadPosition();
    while (pos1)
    {
        CASE_t *p1 = caselist.GetNext(pos1);
        if (p1->jxx_opcode == ea)	//	really
        {	//	now, add some jcase instruction
            POSITION pos2 = p1->caselist->GetHeadPosition();
            while (pos2)
            {
                ea_t case_ea = p1->caselist->GetNext(pos2);
                assert(case_ea);

                AsmCode *pnew = AsmCode::new_AsmCode();
                pnew->linear = ea;
                pnew->xcpu.opcode = C_JCASE;
                pnew->xcpu.op[0].mode = OP_Near;
                pnew->xcpu.op[0].nearptr.offset = case_ea;
                this->m_asmlist->AddTail(pnew);
                //alert("insert 1 C_JCAES");
            }
            break;	//	only one can be true
        }
    }
}

void CFuncStep1::CreateNewFunc_if_CallNear()
{
	POSITION pos1 = m_asmlist->GetHeadPosition();
	while (pos1)
	{
		AsmCode* pasm = m_asmlist->GetNext(pos1);
		XCPUCODE* pxcpu = &pasm->xcpu;
		if ( pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
		{
				g_Cexe2c->func_new(pxcpu->op[0].nearptr.offset);	// 创建函数信息
		}
	}
}

bool CFuncStep1::Step_1(ea_t head_off) // 生反汇编列表，保存函数结束地址，设定函数最多3000行
{
	//assert(m_nStep == 0);

	CaseList caselist;
	EALIST jxxlist;		// 跳转语句列表
	EALIST usedlist;	// 已分析过的语句列表

    ea_t ea = head_off;
    assert(ea < 0x10000000);	// 0x400000 <= 反汇编的地址 < 0x10000000
    assert(ea >= 0x400000);

	jxxlist.AddHead(ea);

	// 找到当前函数所有的地址，保存到usedlist中
    while (any_free_ea(&jxxlist,&usedlist,&ea))							//	走遍所有的jxx
    {	
		for (;;)
		{
			Add_in_order(&usedlist,ea);									// 添加到已处理的列表中

            CDisasm the;
			the.Disasm_OneCode(ea); //ea会递增的

			if (the.get_xcpu()->IsJxx() || the.get_xcpu()->IsJmpNear())	// 判断是否为跳转指令
			{
				Add_in_order(&jxxlist, the.get_xcpu()->op[0].nearptr.offset);	// 添加到跳转列表中
			}
			else														// 判断是否为switch指令
			{
				check_if_switch_case(ea,&caselist,&jxxlist, the.get_xcpu());
			}

			if (the.get_xcpu()->opcode == C_RET || the.get_xcpu()->opcode == C_JMP )
				break;

			//if (the.get_xcpu()->opcode == C_CALL )	// ExitProcess也要退出
			//	  break;

			if (usedlist.Find(ea))		// 地址已经处理过
				break;
		}
    }
	
	// 反汇编usedlist,最多3000行汇编
	POSITION pos = usedlist.GetHeadPosition();
	int i_MaxLines=0;
	while (pos && i_MaxLines<3000)									//	生成asm opcode list
	{
		if (this->m_asmlist == NULL)
		{
			this->m_asmlist = new AsmCodeList;
		}

		i_MaxLines++;
		
		ea_t ea = usedlist.GetNext(pos);
		AsmCode *p = AsmCode::new_AsmCode();
		p->linear = ea;								// 反汇编地址

        CDisasm the;
		p->opsize = the.Disasm_OneCode(ea);			// 反汇编,ea自动增加
		p->xcpu = *the.get_xcpu();

		// 红尘岁月添加
		// 0000 ADD [EAX], AL 指令不管
		/*
		if(p->xcpu.opcode == C_ADD
		&& (p->xcpu.op[0].mode==OP_Address && p->xcpu.op[0].addr.base_reg_index==0x00)
		&& (p->xcpu.op[1].mode==OP_Register && p->xcpu.op[1].reg.reg_index==0x00)
			)				
		  continue;
		*/

		this->m_asmlist->AddTail(p);				// 保存反汇编结果

        this->CheckIfJustSwitchCase(caselist, ea);	// 查找是不是刚过一个swith case

	}

	AsmCode *pasm = this->m_asmlist->GetTail();	// 获取最近一个反汇编语句信息
	m_end_off = pasm->linear + pasm->opsize;	// 得到函数结束地址

	return true;
}

AsmCode* CFuncLL::ea2pasm(ea_t ea)
{
	POSITION pos = m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* p = m_asmlist->GetNext(pos);
		if (p->linear == ea)
			return p;
	}
	log_prtl("0x%08X 反汇编失败",ea);
	//assert_msg(0,"whye here");
	return NULL;
}

PSTR check_if_jmp_api(const BYTE* phead);	// 判断是否是跳转到API
// 分析当前语句与下一条语句
bool	CFuncLL::stack_stack(AsmCode* p0, AsmCode* p1)
{
    signed int & esp0 = p0->esp_level;
    signed int & esp0_next = p0->esp_level_next;
    signed int & esp1 = p1->esp_level;

	//	首先，是p0自身的建设-------
    if (isLeave(p0))					// 是否为Leave 与 MOV esp,? 指令
	{
		if (esp0_next == ESP_UNKNOWN)	// 未知类型
		{
			signed int esp = ESP_UNKNOWN;
			if (Asm_Code_Change_ESP(esp, &p0->xcpu))
			{
				esp0_next = esp;
				return true;
			}
		}
	}
	else
	{
		if (esp0_next == ESP_UNKNOWN)		// 下一个指令的位置是未知的
		{
			if (p0->xcpu.opcode == C_JMP || p0->xcpu.opcode == C_RET)	// 跳转与返回
			{
				esp0_next = ESP_IGNORE;
				return true;
			}
			if (esp0 != ESP_UNKNOWN)		// 当前指令位置为已知，下一指令未知时调用
			{
				signed int esp = esp0;
				if ( Asm_Code_Change_ESP(esp, &p0->xcpu) )
				{
					esp0_next = esp;
					return true;
				}
			}
		}

		if (esp0_next != ESP_UNKNOWN
			&& (esp0_next != ESP_IGNORE)
			&& (esp0 == ESP_UNKNOWN)
			)	// 反查
		{
			signed int esp = 0;
			if ( Asm_Code_Change_ESP(esp, &p0->xcpu) )
			{
				//esp0_next = esp0 + esp;
				esp0 = esp0_next - esp;
				return true;
			}
		}
		// call --------------------
		if (esp0 != ESP_UNKNOWN
			&& esp0_next != ESP_UNKNOWN
			&& p0->xcpu.opcode == C_CALL
			&& p0->xcpu.op[0].mode == OP_Address
			&& p0->xcpu.op[0].addr.base_reg_index == _NOREG_
			&& p0->xcpu.op[0].addr.off_reg_index == _NOREG_
			)
		{
			ea_t address = p0->xcpu.op[0].addr.off_value;
			
			CApi *papi = g_ApiManage->get_api(address);	//find it
			if (papi)
			{
				if (esp0_next != esp0 + papi->m_stack_purge)
					return false;	//find error
			}
			else	// 在API函数中没有找到
			{	//not find, insert it
				//alert_prtf("error not find api %x", address);
				//g_ApiManage->new_api(address,esp0_next - esp0);

				PBYTE temp01=ea2ptr(address);
				ea_t addr= *(DWORD*)temp01;

				g_Cexe2c->func_new(addr);
			}
		}
		
	}

	// Jmp -------------------------
    if (p1->xcpu.IsJxx() || p1->xcpu.IsJmpNear())
	{
		ea_t jmpto = p1->xcpu.op[0].nearptr.offset;
		AsmCode* p = ea2pasm(jmpto);
		if (p->esp_level == ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
		{
			p->esp_level = esp1;
			return true;
		}
		if (p->esp_level != ESP_UNKNOWN && esp1 == ESP_UNKNOWN)
		{
			esp1 = p->esp_level;
			return true;
		}
		if (p->esp_level != ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
		{
			if (p->esp_level != esp1)
				return false;
		}
	}
	// ret ------------------------
	if (p1->xcpu.opcode == C_RET)
	{
		if (esp1 == ESP_UNKNOWN)
		{
			esp1 = 0;
			return true;
		}
		if (esp1 != 0)
			return false;
	}
	// -----------------------------------
	if (esp0_next == ESP_UNKNOWN && esp1 != ESP_UNKNOWN)
	{
		esp0_next = esp1;
		return true;
	}

	if (esp0_next != ESP_UNKNOWN
		&& esp0_next != ESP_IGNORE
		&& esp1 == ESP_UNKNOWN)
	{
		esp1 = esp0_next;
		return true;
	}
	if (esp0_next != ESP_UNKNOWN
		&& esp0_next != ESP_IGNORE
		&& esp1 != ESP_UNKNOWN)
	{	//do some check
		if (esp0_next != esp1)
			return false;
	}
	return false;
}

ea_t FindApiAddress_Reg(DWORD regindex, XCPUCODE* pxcpu1, AsmCodeList* asmlist)
{
    //意思是，我要正确处理call eax
    //到前面去找一个mov eax, [405070] 或 mov eax, 405070
    ea_t retn = 7;

	POSITION pos = asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* p = asmlist->GetNext(pos);
        XCPUCODE* pxcpu = &p->xcpu;
		if (pxcpu == pxcpu1)
			return retn;
        if (pxcpu->opcode == C_MOV
            && pxcpu->op[0].mode == OP_Register
            && pxcpu->op[0].reg.reg_index == regindex)
        {
            if (pxcpu->op[1].mode == OP_Address 
                && pxcpu->op[1].addr.base_reg_index == _NOREG_ 
                && pxcpu->op[1].addr.off_reg_index == _NOREG_
                )   //mov eax, [405070] // call [eax]
            {
                ea_t address = pxcpu->op[1].addr.off_value;
                retn = address;
            }
			else if (pxcpu->op[1].mode == OP_Immed )	//mov eax, 405070 //call [eax]
            {
                ea_t address = pxcpu->op[1].immed.immed_value;

				PBYTE temp01=ea2ptr(address);
				ea_t addr= *(DWORD*)temp01;
				return addr;
            }
        }
	}
    return retn;
}


//  算出来esp,就return true
//	让我们假定一个func中只出现一次mov ebp,esp

//	影响stack的有这几种情况：
//	enter leave
//	push ebp,mov ebp,esp,.....,mov esp,ebp,pop ebp
//	push,pop
//	call near
//	ret时检查是否已平衡
//	对call	[immed]等，暂不考虑s
bool	CFuncLL::Asm_Code_Change_ESP(int &esp, XCPUCODE* pxcpu)
{
	if (pxcpu->opcode == C_MOV					// mov esp,ebp
        && (pxcpu->op[0].mode == OP_Register)
        && (pxcpu->op[0].reg.reg_index == _ESP_)
        && (pxcpu->op[1].mode == OP_Register)
        && (pxcpu->op[1].reg.reg_index == _EBP_))
    {
        if (this->m_EBP_base != Not_EBP_based
            && esp == ESP_UNKNOWN)
        {
            esp = this->m_EBP_base;				// mov esp,ebp
            return true;
        }
    }
	if (pxcpu->opcode == C_MOV					// mov ebp,esp
        && (pxcpu->op[0].mode == OP_Register)
        && (pxcpu->op[0].reg.reg_index == _EBP_)
        && (pxcpu->op[1].mode == OP_Register)
        && (pxcpu->op[1].reg.reg_index == _ESP_))
    {
        if (esp != ESP_UNKNOWN
            && this->m_EBP_base == Not_EBP_based)
        {
            this->m_EBP_base = esp;				//mov ebp,esp
            return true;
        }
    }

	if (pxcpu->opcode == C_LEAVE)	//  leave
	{
		esp = m_EBP_base;			//	mov	esp,ebp
		esp += 4;					//	pop	ebp

		return true;
	}

	if (pxcpu->opcode == C_ENTER)	//  enter N,L
	{
		esp -= 4;					//	push ebp
		this->m_EBP_base = esp;		//  mov ebp,esp

		if(pxcpu->op[1].immed.immed_value == 0) // enter N,0
			esp -= pxcpu->op[0].immed.immed_value;

		return true;
	}

	if (pxcpu->opcode == C_PUSH)	// push ?
	{
		esp -= 4;
		return true;
	}

	if (pxcpu->opcode == C_POP)		// pop  ?
	{
		esp += 4;
		return true;
	}

	if (pxcpu->opcode == C_SUB		// sub esp,imm
		&&(pxcpu->op[0].mode == OP_Register)
		&&(pxcpu->op[0].reg.reg_index == _ESP_)
		&&(pxcpu->op[1].mode == OP_Immed))
	{
		esp -= pxcpu->op[1].immed.immed_value;
		return true;
	}

	if (pxcpu->opcode == C_ADD		// add esp,imm
		&&(pxcpu->op[0].mode == OP_Register)
		&&(pxcpu->op[0].reg.reg_index == _ESP_)
		&&(pxcpu->op[1].mode == OP_Immed))
	{
		esp += pxcpu->op[1].immed.immed_value;
		return true;
	}

	if (pxcpu->opcode == C_CALL)	// call
	{
		if (pxcpu->op[0].mode == OP_Near)
		{
			CFunc* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
			if (pfunc == NULL)
                return false;
			if (pfunc->m_IfLibFunc)
			{
				//esp += pfunc->m_stack_purge;
				//assert(pfunc->m_functype);
				if(pfunc->m_functype)	// 要存在
					esp += pfunc->m_functype->get_stack_purge();
		        return true;
			}
			if (pfunc->m_nStep < STEP_IDA_1)
				return false;
			esp += pfunc->m_stack_purge;
            return true;
		}
        else if (pxcpu->opcode == C_CALL &&
            pxcpu->op[0].mode == OP_Address &&
            pxcpu->op[0].addr.base_reg_index == _NOREG_ &&
            pxcpu->op[0].addr.off_reg_index == _NOREG_
            )   //call [405070]
        {
            ea_t address = pxcpu->op[0].addr.off_value;
            CApi* papi = g_ApiManage->get_api(address);// find API 函数
            if (papi)
            {
                esp += papi->m_stack_purge;
                return true;
            }
			else
			{
				PBYTE temp01=ea2ptr(address);
				ea_t addr= *(DWORD*)temp01;

				g_Cexe2c->func_new(addr);
			}
        }
        else if (pxcpu->opcode == C_CALL &&
            pxcpu->op[0].mode == OP_Register
            )   //call eax
        {
            ea_t address = FindApiAddress_Reg(pxcpu->op[0].reg.reg_index, pxcpu, this->m_asmlist);
		
			if(address<0x10000)
				return false;

			CApi* papi = g_ApiManage->get_api(address);// find it
			if (papi)
			{
				esp += papi->m_stack_purge;
				return true;
			}
			
        }
		else if (pxcpu->opcode == C_CALL &&
            pxcpu->op[0].mode == OP_Address &&
            pxcpu->op[0].addr.base_reg_index != _NOREG_
            )   // 如call [ESI]  红尘岁月添加 
		{
			return true;
		}
		else
			return false;
	}

	// 红尘岁月添加 
	if (pxcpu->opcode == C_RDTSC)		// RDTSC
		return true;

	// 红尘岁月添加 end

    if (esp == ESP_UNKNOWN)
        return false;
	return true;
}

void CFuncLL::AddRemoveSomeInstr()
{
    POSITION pos = m_asmlist->GetHeadPosition();
    while (pos)
    {
        AsmCode* pasm = m_asmlist->GetNext(pos);
        if (pasm->iAddRemove != 0)				// 已经处理过的，不再处理
            continue;
        
        XCPUCODE* pxcpu = &pasm->xcpu;
        if (pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
        {
            CFunc* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            if (pfunc == NULL)
                continue;
            if (pfunc->m_IfLibFunc && pfunc->m_functype != NULL)	// 有定义的库函数
            {
                CFuncType* fctype = pfunc->m_functype;
                if (!strcmp(fctype->m_internal_name, "_EH_prolog"))
                {
                    //strcmp(fctype->m_internal_name, "_EH_prolog");
                    pasm->iAddRemove = 1;

                    AsmCode* pnew;
                    //push ebp
                    pnew = AsmCode::new_AsmCode();
                    pnew->iAddRemove = 2;
                    pnew->xcpu.opcode = C_PUSH;
                    pnew->xcpu.op[0].mode = OP_Register;
                    pnew->xcpu.op[0].opersize = 4;
                    pnew->xcpu.op[0].reg.reg_index = _EBP_;
                    m_asmlist->InsertBefore(pos, pnew);

                    //mov ebp, esp
                    pnew = AsmCode::new_AsmCode();
                    pnew->iAddRemove = 2;
                    pnew->xcpu.opcode = C_MOV;
                    pnew->xcpu.op[0].mode = OP_Register;
                    pnew->xcpu.op[0].opersize = 4;
                    pnew->xcpu.op[0].reg.reg_index = _EBP_;
                    pnew->xcpu.op[1].mode = OP_Register;
                    pnew->xcpu.op[1].opersize = 4;
                    pnew->xcpu.op[1].reg.reg_index = _ESP_;
                    m_asmlist->InsertBefore(pos, pnew);

                }
            }
        }
    }
}

void CFuncLL::Prepare_CallFunc()	// 把本函数中call到的函数都prepare一遍
{
    POSITION pos = m_asmlist->GetHeadPosition();
    while (pos)
    {
        AsmCode* pasm = m_asmlist->GetNext(pos);
        XCPUCODE* pxcpu = &pasm->xcpu;
        if (pxcpu->opcode == C_CALL && pxcpu->op[0].mode == OP_Near)
        {
            CFunc* pfunc = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            if (pfunc )
               pfunc->PrepareFunc();
        }
    }
}

// 检查该函数的stack是否平衡，有leave的情况，可以堆栈不平
// 许多情况堆栈是不平的
bool	CFuncLL::Check_Stack()	
{
	POSITION pos = this->m_asmlist->GetHeadPosition();
	signed int lastesp = 0;	

	while (pos)
	{
		AsmCode* p = this->m_asmlist->GetNext(pos);
		signed int esp = p->esp_level;
		if (esp == ESP_UNKNOWN)		// 堆栈位置不能为未知
			return false;

		if (lastesp != ESP_UNKNOWN 	// 堆栈位置不能为未知
			&& lastesp != ESP_IGNORE 
			&& esp != lastesp
			)
			return false;

        if (p->xcpu.IsJxx() || p->xcpu.IsJmpNear())
		{
			ea_t jmpto = p->xcpu.op[0].nearptr.offset;
			AsmCode* p = ea2pasm(jmpto);
			if (esp != p->esp_level)
				return false;
		}
		if (p->xcpu.opcode == C_RET)
		{
			if (esp != 0)
				return false;
		}
		lastesp = p->esp_level_next;
	}
	return true;
}

bool	CFuncLL::Fill_Stack_Info()	// 计算堆栈位置
{
	POSITION pos = m_asmlist->GetHeadPosition();
	AsmCode* pasm = 0;
	if (pos)
		pasm = m_asmlist->GetNext(pos);// 当前语句
	else
		return false;

	pasm->esp_level = 0;		// 初始第一行的堆栈位置为0

	int i;
	while (pos)
	{
		AsmCode* p0 = pasm;					// 当前语句
		pasm = m_asmlist->GetNext(pos);		// 下一条语句

		i=0;
		while(stack_stack(p0,pasm) && i<8)	// 循环调用，计算stack位置
			;
	}

	//Fill_Stack_1();	
	// 有leave函数或最后调用mov esp,ebp 的情况，可以堆栈不平衡,所以不检查了
	//if (! Check_Stack())
	//{
	//	log_prtl("处理堆栈平衡失败");
	//}

    return true;
}

int CFuncLL::Get_Ret_Purge()	// return -1 for fail
{
	int retn = -1;
	POSITION pos = this->m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* pasm = this->m_asmlist->GetNext(pos);
		XCPUCODE* pxcpu = &pasm->xcpu;
		if (pxcpu->opcode == C_RET)		//找到了ret语句
		{
			int r = 0;
			if (pxcpu->op[0].mode == OP_Immed)	// means RET n
			{
				r = pxcpu->op[0].immed.immed_value;
			}

			if (retn == -1)
				retn = r;
			else if (retn != r) //两个Ret不一致？
				return -1;
		}
	}
	if (retn == -1)
	{
		log_prtl("函数没有 Ret 指令");
		return 0;	// 没有就默认为ret 0;
	}
    return retn;
}

CString CFuncLL::GetLabelName(ea_t ea)
{
    CString retn;

    CFunc* pfunc = g_Cexe2c->GetFunc(ea);
    if (pfunc != NULL)
    {
        retn = pfunc->m_funcname;
        return retn;
    }
    AsmCode* pasm = this->ea2pasm(ea);
    if (pasm != NULL && pasm->h.label.ref_j != NULL)
    {
        retn = pasm->h.label.label_name;
        return retn;
    }
    return retn;
}

void	CFuncLL::prtout_asm(CFunc* pfunc, CVarLL* pvarll, CXmlOutPro* out)
{
    out->XMLbegin(XT_Function, pfunc);

    out->XMLbegin(XT_FuncName, pfunc);
    out->prtt(pfunc->m_funcname);
    out->XMLend(XT_FuncName);

    out->prtspace();
    out->prtt("proc");
    out->endline();

    pvarll->prtout(out);

    out->endline();
    
    this->prtout_asm_1(pvarll, out);

    out->XMLbegin(XT_FuncName, pfunc);		// 结尾的函数名称
    out->prtt(pfunc->m_funcname);
    out->XMLend(XT_FuncName);

    out->prtspace();
    out->prtt("endp");
    out->endline();

    out->XMLend(XT_Function);
}

void	CFuncLL::prtout_asm_1(CVarLL* pvarll, CXmlOutPro* out)
{   //	按运行代码以ASM显示func，对其中的GAP能指出来

	ea_t last = 0xffffffff;
	POSITION pos = this->m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* pasm = this->m_asmlist->GetNext(pos);
		if (pasm->iAddRemove == 2)
			continue;
		ea_t ea = pasm->linear;

		char	buf[280];
		DWORD n;
		if (pasm->xcpu.opcode == C_JCASE)
		{
			n = 0;
			sprintf(buf,"case jmp to %x",pasm->xcpu.op[0].nearptr.offset);
		}
		else
        {
            st_IDA_OUT idaout;
            CDisasm the;
            //n = the.Disassembler(buf, ea2ptr(ea), ea);
            n = the.Disassembler_X(ea2ptr(ea), ea, &idaout);
            XCPUCODE* pxcpu = the.get_xcpu();
            if (pxcpu->op[0].mode == OP_Near)
            {
                ea_t linear = pxcpu->op[0].nearptr.offset;
                CString labelname = this->GetLabelName(linear);
                if (!labelname.IsEmpty())
                {
                    idaout.Par1Str = labelname;
                }
            }
            else if (pxcpu->op[0].mode == OP_Address)
            {
                OPERITEM* op = &pxcpu->op[0];
                if (op->addr.base_reg_index == _ESP_
                    || (op->addr.base_reg_index == _NOREG_
                        && op->addr.off_reg_index == _ESP_
                        && op->addr.off_reg_scale == 1))
                {
                    signed int level = pasm->esp_level + op->addr.off_value;
                    st_VarLL* p = pvarll->LookUp_VarLL(level- pvarll->m_VarRange_L);
                    if (p != NULL)
                    {
                        idaout.Par1Str += '.';
                        idaout.Par1Str += p->Name;
                    }
                }
            }
            else if (pxcpu->op[1].mode == OP_Address)
            {
                OPERITEM* op = &pxcpu->op[1];
                if (op->addr.base_reg_index == _ESP_
                    || (op->addr.base_reg_index == _NOREG_
                        && op->addr.off_reg_index == _ESP_
                        && op->addr.off_reg_scale == 1))
                {
                    signed int level = pasm->esp_level + op->addr.off_value;
                    st_VarLL* p = pvarll->LookUp_VarLL(level- pvarll->m_VarRange_L);
                    if (p != NULL)
                    {
                        idaout.Par2Str += '.';
                        idaout.Par2Str += p->Name;
                    }
                }
            }
            idaout.output(buf);
        }
		if (last != 0xffffffff && ea != last)
			out->prtl("//      gap here");
        
        if (pasm->h.label.ref_j != NULL)
        {
            //asm_prtl("%s:", pasm->h.label.label_name);
            out->prtf("%s:", pasm->h.label.label_name);
            out->endline();
        }

        //asm_prtl("%4x %x %s",-pasm->esp_level, ea, buf);
        if (pasm->esp_level == ESP_UNKNOWN)
        {
            out->prtt("    ");  //四个空格的位置要留
        }
        else
            out->prtf("%4x", -pasm->esp_level);

        out->prtspace();

        out->XMLbegin(XT_AsmOffset, (void*)ea);
        out->prtf("%x", ea);
        out->XMLend(XT_AsmOffset);

        out->prtt(buf);
        out->endline();

		last = ea+n;
	}
}

/*
如果一个函数开头是：
  0 401010 SUB    ESP,00000190
190 401016 LEA    ECX,[ESP+00]    
则 VarRange_L = -190h
则 VarRange_H = 0
写作
  0 401010 SUB    ESP,00000190
190 401016 LEA    ECX,[ESP+v_00]    
*/
// 查询变量申请的范围
void CFuncLL::GetVarRange(signed int& VarRange_L, signed int& VarRange_H)
{
    signed int L = 0;
    signed int H = 0;

	POSITION pos = this->m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* pasm = this->m_asmlist->GetNext(pos);

        signed int last = pasm->esp_level;
        signed int here = pasm->esp_level_next;
        if (pasm->xcpu.opcode == C_SUB		// 变量一般是通过sub 、add 在堆栈中申请空间
		 || pasm->xcpu.opcode == C_ADD
		 )
        {
            if (last - here > H - L)
            {
                H = last;
                L = here;
            }
        }
    }
    if (H - L > 0)
    {
        VarRange_H = H;
        VarRange_L = L;
    }
}


void CVarLL::prtout(CXmlOutPro* out)
{
    int curlevel = 0;
    int maxlevel = this->m_VarRange_H - this->m_VarRange_L;

    POSITION pos = this->m_varll_list.GetHeadPosition();
    while (pos)
    {
        st_VarLL* p = this->m_varll_list.GetNext(pos);
        if (curlevel > p->off)
        {
            out->prtl("error, var collapse!!!");
            curlevel = p->off;
        }
        else if (curlevel < p->off)
        {
            out->prtl("gap len = %x", p->off - curlevel);
            curlevel = p->off;
        }
        /*asm_prtl("%s equ %s %x", p->Name, 
                 (p->size == 1) ? "BYTE ptr" :
                 (p->size == 2) ? "WORD ptr" :
                 (p->size == 4) ? "DWORD ptr" : "",
                 p->off); */

        out->prtspace(4);
        out->XMLbegin(XT_Symbol, p);
        out->prtt(p->Name);
        out->XMLend(XT_Symbol);
        out->prtt("equ");
        out->prtspace();
        out->prtt((p->size == 1) ? "BYTE ptr" :
                          (p->size == 2) ? "WORD ptr" :
                          (p->size == 4) ? "DWORD ptr" : "");
        out->prtspace();
        if (p->array != 1)
        {
            out->XMLbegin(XT_Number, NULL);
            out->prtf("%xh", p->array);
            out->XMLend(XT_Number);
            out->prtt("dup");
            out->prtspace();
        }
        out->XMLbegin(XT_Number, NULL);
        out->prtf("%xh", p->off);
        out->XMLend(XT_Number);
        out->endline();

        curlevel += p->size * p->array;
    }
    
    if (curlevel < maxlevel)
    {
        out->prtl("    gap len = %xh", maxlevel - curlevel);
    }
}
st_VarLL* CVarLL::LookUp_VarLL(int off)
{
    POSITION pos = this->m_varll_list.GetHeadPosition();
    while (pos)
    {
        st_VarLL* p = this->m_varll_list.GetNext(pos);
        if (p->off == off)
            return p;
    }
    return NULL;
}
void CVarLL::AddRef(signed int level, int opersize)
{
    if (level < this->m_VarRange_H && level >= this->m_VarRange_L)
    {
    }
    else
        return;
    int off = level - this->m_VarRange_L; //这个是>=0的
    st_VarLL* pnew = this->LookUp_VarLL(off);
    if (pnew != NULL)
        return;
    pnew = new st_VarLL;
    pnew->off = off;
    pnew->size = opersize;
    sprintf(pnew->Name, "v_%x", off);

    if (this->m_varll_list.IsEmpty())
    {
        this->m_varll_list.AddTail(pnew);
    }
    else
    {//有序列表
        POSITION pos = this->m_varll_list.GetHeadPosition();
        while (pos)
        {
            POSITION savpos = pos;
            st_VarLL* p = this->m_varll_list.GetNext(pos);
            if (p->off > off)
            {
                this->m_varll_list.InsertBefore(savpos, pnew);
                return;
            }
        }
        this->m_varll_list.AddTail(pnew);
    }

}
void CFuncLL::VarLL_Analysis_1(CVarLL* pvarll, OPERITEM* op, AsmCode* pasm)
{
    if (op->mode != OP_Address)
        return;
	if (op->addr.base_reg_index == _ESP_
        || (op->addr.base_reg_index == _NOREG_
                        && op->addr.off_reg_index == _ESP_
                        && op->addr.off_reg_scale == 1))
    {
        signed int level = pasm->esp_level + op->addr.off_value;
        pvarll->AddRef(level, op->opersize);
    }
	if (op->addr.base_reg_index == _EBP_)
    {
        //怎么写？
    }
}
void CFuncLL::VarLL_Analysis(CVarLL* pvarll)
{
    POSITION pos = this->m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* pasm = this->m_asmlist->GetNext(pos);
        if (pasm->xcpu.op[0].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[0], pasm);
        if (pasm->xcpu.op[1].mode == OP_Address)
            this->VarLL_Analysis_1(pvarll, &pasm->xcpu.op[1], pasm);
    }
}

AsmCode* ea2pasm(ea_t ea, AsmCodeList* m_asmlist)	// 
{
	POSITION pos = m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* p = m_asmlist->GetNext(pos);
		if (p->linear == ea)
			return p;
	}
	assert_msg(0,"取指定地址的反汇编信息出错！");
	return NULL;
}

void CJxxLabel::Label_Analysis()
{
    POSITION pos = this->m_asmlist->GetHeadPosition();
    while (pos)
    {
		AsmCode* p = this->m_asmlist->GetNext(pos);
        assert(p->h.type == i_Nothing);				// 以前没对h分析过

        if (p->xcpu.IsJxx() || p->xcpu.IsJmpNear())
		{
			ea_t jmpto = p->xcpu.op[0].nearptr.offset;
			AsmCode* plabel = ea2pasm(jmpto, m_asmlist);

            p->h.type = i_Jump;
            if (p->xcpu.IsJmpNear())
                p->h.jmp.jmp_type = JMP_jmp;
            p->h.jmp.the_label = plabel;

            if (plabel->h.label.ref_j == NULL)
            {
                plabel->h.label.ref_j = p;
                sprintf(plabel->h.label.label_name, "loc_%x", plabel->linear);
            }
            else
            {
                p->h.jmp.next_ref_of_this_label = plabel->h.label.ref_j;
                plabel->h.label.ref_j = p;
            }
		}
    }
}
