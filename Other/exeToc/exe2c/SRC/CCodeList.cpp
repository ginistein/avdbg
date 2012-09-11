// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include "stdafx.h"
#include "CISC.H"
#include "cexe2c.h"

#include "CCodeList.h"

int g_hxd_instr;

// 初步构造instr_list和expr_list
void CCodeList::CreateInstrList_raw(INSTR_LIST* instr_list, AsmCodeList* asmlist, int EBP_base)
{
    this->m_instr_list = instr_list;
    this->m_asmlist = asmlist;
    this->m_EBP_base = EBP_base;

	PINSTR p_begin = new INSTR;     // new_INSTR
	p_begin->type = i_Begin;

	PINSTR p_end = new INSTR;		// new_INSTR	总是同时new
	p_end->type = i_End;

	p_begin->begin.m_end = p_end;

	InstrAddTail(p_begin);	// 在开始处添加i_Begin 标识

    signed int esp_level = 3;

	g_hxd_instr = 0;		// 初始

	POSITION pos = this->m_asmlist->GetHeadPosition();
	while (pos)
	{
		AsmCode* cur = this->m_asmlist->GetNext(pos);
        assert(cur);
		assert(cur->linear);

        CCodeList_Maker the(this,cur);	// 初始化CCodeList_Maker
		
		//	在每条指令前加一个label,唯一的例外，C_JCASE前不加label
        if (cur->xcpu.opcode != C_JCASE)
		{	
            PINSTR p = new INSTR;   //new_INSTR
            p->type = i_Label;
            p->label.label_off = cur->linear;
            InstrAddTail(p);
		}
		// 添加出栈标记
        if (esp_level != 3 
			&& esp_level < cur->esp_level
			)
        {
            //cur->xcpu.opcode;
            PINSTR	p = new INSTR;  //new_INSTR
            p->type = i_EspReport;
            p->espreport.esp_level = cur->esp_level;
            p->espreport.howlen = cur->esp_level - esp_level;

            InstrAddTail(p);
        }
		

		the.AddTail_Cur_Opcode();		// 把当前的xcpu转化为伪码，同时在intr_list中addtail

        esp_level = cur->esp_level;
    }

	// 函数是没有ret 指令,添加一条i_Return
	AsmCode* Last=this->m_asmlist->GetTail();

	if(Last->xcpu.opcode != C_RET)
	{
		PINSTR	p = new INSTR;  // new_INSTR
		p->type = i_Return;
		InstrAddTail(p);
	}
	
	InstrAddTail(p_end);	//	在后面加一条i_End 标识
}

void CCodeList::InstrAddTail(PINSTR p)
{
	/*
	if (p->var_w.type)
    	if (p->var_w.opsize == 0)
			nop();//assert(p->var_w.opsize);
	if (p->var_r1.type)
    	if (p->var_r1.opsize == 0)
			nop();//assert(p->var_r1.opsize);
	if (p->var_r2.type)
    	if (p->var_r2.opsize == 0)
			nop();//assert(p->var_r2.opsize);
	*/

	p->id= g_hxd_instr++;
	this->m_instr_list->AddTail(p);
}

void	set_address(OPERITEM* op,PINSTR p)	// 填充信息，如[eax + ebx*4 + 8888]
{
	if (op->addr.base_reg_index != _NOREG_)	// 寄存器，对应的为 eax
	{
		p->var_r1.type = v_Reg;
		p->var_r1.opsize = BIT32_is_4;
		p->var_r1.reg = regindex_2_regoff(op->addr.base_reg_index);
	}

	if (op->addr.off_reg_index != _NOREG_) // 寄存器 ，对应的为 ebx
	{
		p->var_r2.type = v_Reg;
		p->var_r2.opsize = BIT32_is_4;
		p->var_r2.reg = regindex_2_regoff(op->addr.off_reg_index);
	}

	p->i1 = op->addr.off_reg_scale;			// 对应的为 4
	p->i2 = op->addr.off_value;				// 对应的为 8888
}
//-------------------------------------------------------
#define m_instr_list Q->m_instr_list
#define InstrAddTail Q->InstrAddTail
#define m_EBP_base Q->m_EBP_base

ea_t FindApiAddress_Reg(DWORD regindex, XCPUCODE* pxcpu1, AsmCodeList* asmlist);

//	把当前的xcpu转化为伪码，同时在intr_list中addtail
void	CCodeList_Maker::AddTail_Cur_Opcode()
{
    XCPUCODE* pxcpu = &cur->xcpu;

	switch (pxcpu->opcode)
	{
		// 逻辑指令处理
	case C_NOT:		
		Code_general(enum_A, i_Not); 
		// 不影响跳转
		break;
	case C_AND:
		{
			PINSTR	p = new INSTR;
			p->type = i_Assign;
			
			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			TransVar(&p->var_r2, 1);	//	1 means	xcpu.op[1]

			if ( pxcpu->op[0].mode == OP_Register
              && pxcpu->op[1].mode == OP_Register
			  && VAR::IsSame(&p->var_r1,&p->var_r2)
			  )	//	and eax,eax 等于自己给自己赋值
			{	
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			}
			else
			{
				PINSTR p = Code_general(enum_AR, i_And);;   // 一个伪指令

				// 结果是否为 0 的比较指令
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			
			}
			break;
		}
		
	case C_OR: 
		{
			//PINSTR p = Code_general(enum_AR, i_Or);;   // 一个伪指令

			PINSTR	p = new INSTR;
			p->type = i_Assign;
			
			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			TransVar(&p->var_r2, 1);	//	1 means	xcpu.op[1]

			if ( pxcpu->op[0].mode == OP_Register
              && pxcpu->op[1].mode == OP_Register
			  && VAR::IsSame(&p->var_r1,&p->var_r2)
			  )	//	or eax,eax 等于自己给自己赋值
			{	
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			}
			else
			{
				PINSTR p = Code_general(enum_AR, i_Or);;   // 一个伪指令

				// 结果是否为 0 的比较指令
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			
			}

			break;
		}

	case C_XOR: 
		{
			PINSTR p = new INSTR;   // 一个伪指令

			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			TransVar(&p->var_r2, 1);	//	0 means	xcpu.op[0]

			if ( pxcpu->op[0].mode == OP_Register
              && pxcpu->op[1].mode == OP_Register
			  && VAR::IsSame(&p->var_r1,&p->var_r2)
			  )	//	xor eax,eax means mov eax,0 下面的内容有作用吗
			{	
				PINSTR	p = new INSTR;
				p->type = i_Assign;

				TransVar(&p->var_w, 0);		//	0 means	xcpu.op[0]

				p->var_r1.type = v_Immed;	//  类型为立即数
				p->var_r1.opsize = BIT32_is_4;
				p->var_r1.d=0;				//  数据为0

				InstrAddTail(p);

			}
			else
			{
				Code_general(enum_AR, i_Xor);

				// 结果是否为 0 的比较指令
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			
			}

			break;
		}

	case C_TEST:
		{
			VAR v;
			new_temp(&v);			// 添加一个变量

			PINSTR p = new INSTR;   // 一个伪指令
			p->type = i_And;
			p->var_w = v;			// 要写入一个临时变量操作符

			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			TransVar(&p->var_r2, 1);	//	0 means	xcpu.op[0]

			if ( pxcpu->op[0].mode == OP_Register
              && pxcpu->op[1].mode == OP_Register
			  && VAR::IsSame(&p->var_r1,&p->var_r2)
			  )	//	xor eax,eax means mov eax,0 下面的内容有作用吗
			{	
				OPERITEM* op0 = &pxcpu->op[0];
				OPERITEM* op1 = &pxcpu->op[1];
				op1->mode = OP_Immed;
				op1->opersize = op0->opersize;
				op1->immed.immed_value = 0;
				Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
			}
			else
			{
				InstrAddTail(p);

				// 再增中一个比较的伪指令
				PINSTR p2 = new INSTR;   // 一个伪指令

				p2->type = i_Cmp;

				p2->var_r1 =v;

				p2->var_r2.type = v_Immed;
				p2->var_r2.opsize = 4;
				p2->var_r2.d=0;

				InstrAddTail(p2);

			}

			break;
		}

		// 逻辑指令处理 end

    case C_INC:
        {
            OPERITEM* op0 = &pxcpu->op[0];	// 第一个操作数
            OPERITEM* op1 = &pxcpu->op[1];

            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 1;
        }

		Code_general(enum_AR, i_Add);    //2005.2.1加，把它当时一个加1的Add 指令处理
        break;
    case C_DEC:
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 1;
        }

		Code_general(enum_AR, i_Sub);    //2005.2.1加，，把它当时一个加1的SUB 指令处理
        {	// 结果是否为 0 的比较指令
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 0;
            Code_general(enum_RR, i_Cmp);	// 添加一个op0是否为零的比较指令
        }
        break;
	case C_JCASE:
		{
			//alert("C_JCASE find ");
			PINSTR p = new INSTR;   //new_INSTR
			p->type = i_Jump;
			p->jmp.jmp_type = JMP_case;
			p->jmp.jmpto_off = pxcpu->op[0].nearptr.offset;

			PINSTR plast = m_instr_list->GetTail();	//	我们看看它前一条是什么
			if (plast->type == i_JmpAddr)
			{	//	means this is case 0
				p->var_r1 = plast->var_r2;	// index reg
				p->var_r2.type = v_Immed;
				p->var_r2.d = 0;
			}
			else if (plast->type == i_Jump && plast->jmp.jmp_type == JMP_case)
			{
				p->var_r1 = plast->var_r1;
				p->var_r2.type = v_Immed;
				p->var_r2.d = plast->var_r2.d + 1;	//	next case
			}
			else
			{
				alert_prtf("type is %s", hlcode_name(plast->type));
				assert(0);
			}
			InstrAddTail(p);
		}
		break;

	case C_LEA:
		Code_general(enum_WR, i_Lea);
		break;
	case C_MOV:
		Code_general(enum_WR, i_Assign);
		break;
	case C_MOVZX:
		Code_general(enum_WR, i_NosignExpand);
		break;
	case C_MOVSX:
		Code_general(enum_WR, i_SignExpand);
		break;
	
	case C_ADD: Code_general(enum_AR, i_Add);    break;
	case C_IMUL: Code_general(enum_AR, i_Imul);    break;
	case C_SUB: 
        Code_general(enum_AR, i_Sub);    
		// 添加一条比较命令
        {
            OPERITEM* op0 = &pxcpu->op[0];
            OPERITEM* op1 = &pxcpu->op[1];
//            OPERITEM* op2 = &this->cur->xcpu.op[2];
            op1->mode = OP_Immed;
            op1->opersize = op0->opersize;
            op1->immed.immed_value = 0;
            Code_general(enum_RR, i_Cmp);
        }
        break;
	case C_ROL: Code_general(enum_AR, i_Rol);    break;
	case C_ROR: Code_general(enum_AR, i_Ror);    break;

	case C_SAR: Code_general(enum_AR, i_Sar);    break;
	case C_SHL: Code_general(enum_AR, i_Shl);    break;
	case C_SHR: Code_general(enum_AR, i_Shr);    break;

	case C_CMP:
		Code_general(enum_RR, i_Cmp);
		break;
	case C_PUSH:
		{
			PINSTR	p = new INSTR;
			p->type = i_Assign;

			p->var_w.type = v_Var;
			p->var_w.opsize = BIT32_is_4;
			p->var_w.var_off = stack2varoff(cur->esp_level - 4);	// or esp_level_next

			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			InstrAddTail(p);
		}
		break;
	case C_POP:
		{
			PINSTR	p = new INSTR;  //new_INSTR
			p->type = i_Assign;

			p->var_r1.type = v_Var;
			p->var_r1.opsize = BIT32_is_4;
			p->var_r1.var_off = stack2varoff(cur->esp_level);

			TransVar(&p->var_w, 0);	//	0 means	xcpu.op[0]
			InstrAddTail(p);
		}
		break;
	case C_ENTER:	// 用于堆栈平衡，忽略指令
		break;	
	case C_LEAVE:	// 用于堆栈平衡，忽略指令
		break;

	case C_JO: 	Code_Jxx(JMP_jo);	break;
	case C_JNO: Code_Jxx(JMP_jno);	break;
	case C_JB: 	Code_Jxx(JMP_jb);	break;
	case C_JNB: Code_Jxx(JMP_jnb);	break;
	case C_JZ:	Code_Jxx(JMP_jz);	break;
	case C_JNZ: Code_Jxx(JMP_jnz);	break;
	case C_JNA: Code_Jxx(JMP_jna);	break;
	case C_JA:	Code_Jxx(JMP_ja);	break;
	case C_JS: 	Code_Jxx(JMP_js);	break;
	case C_JNS:	Code_Jxx(JMP_jns);	break;
	case C_JP: 	Code_Jxx(JMP_jp);	break;
	case C_JNP: Code_Jxx(JMP_jnp);	break;
	case C_JL: 	Code_Jxx(JMP_jl);	break;
	case C_JNL: Code_Jxx(JMP_jnl);	break;
	case C_JLE:	Code_Jxx(JMP_jle);	break;
	case C_JNLE:Code_Jxx(JMP_jnle);	break;
	case C_JMP:
		if (pxcpu->op[0].mode == OP_Near)
			Code_Jxx(JMP_jmp);
		else
		{
			if (pxcpu->op[0].mode == OP_Address)
			{
				PINSTR p = new INSTR;   //new_INSTR
				p->type = i_JmpAddr;	//	就是说，我先把这个jmp [edx*4+402000]记下来
				//	并期望后面有 C_JCASE

				void	set_address(OPERITEM* op,PINSTR p);

				set_address(&pxcpu->op[0], p);

				InstrAddTail(p);
			}
		}
		break;
	case C_CALL:
		if (pxcpu->op[0].mode == OP_Near)
		{
			PINSTR p = new INSTR;  //new_INSTR
			p->type = i_Call;
			p->call.esp_level = cur->esp_level;
			p->call.call_func = g_Cexe2c->GetFunc(pxcpu->op[0].nearptr.offset);
            p->call.p_callpara = NULL;
            p->call.p_callret = NULL;
			InstrAddTail(p);
		}
        else if (pxcpu->op[0].mode == OP_Address)
		{
			if (pxcpu->op[0].addr.base_reg_index == _NOREG_ &&
				pxcpu->op[0].addr.off_reg_index == _NOREG_)
			{
				ea_t address = pxcpu->op[0].addr.off_value;
				CApi* papi = g_ApiManage->get_api(address);	//find it
				if (papi)
				{
					PINSTR	p = new INSTR;  //new_INSTR
					p->type = i_CallApi;
					p->call.papi = papi;
					p->call.esp_level = cur->esp_level;
                    p->call.p_callpara = NULL;
                    p->call.p_callret = NULL;
					InstrAddTail(p);
				}
				else
				{
					//Code_general(0, i_Unknown);

					PBYTE temp01=ea2ptr(address);
					ea_t addr= *(DWORD*)temp01;

					PINSTR p = new INSTR;  //new_INSTR
					p->type = i_Call;
					p->call.esp_level = cur->esp_level;
					p->call.call_func = g_Cexe2c->GetFunc(addr);
					p->call.p_callpara = NULL;
					p->call.p_callret = NULL;
					InstrAddTail(p);
				}
			}
			else if (pxcpu->op[0].addr.base_reg_index != _NOREG_
				&& pxcpu->op[0].addr.off_reg_index == _NOREG_)
			{	// CALL [ESI]
				ea_t address = FindApiAddress_Reg(pxcpu->op[0].addr.base_reg_index, pxcpu, this->Q->m_asmlist);

				if(address<0x10000)
				{
					Code_general(0, i_Unknown);
					break;			// 没有找到
				}
				CApi* papi = g_ApiManage->get_api(address);	//find it
				if (papi)
				{
					PINSTR	p = new INSTR;  //new_INSTR
					p->type = i_CallApi;
					p->call.papi = papi;
					p->call.esp_level = cur->esp_level;
					p->call.p_callpara = NULL;
					p->call.p_callret = NULL;
					InstrAddTail(p);
				}
				else // 不是API函数
				{
					PINSTR p = new INSTR;  //new_INSTR
					p->type = i_Call;
					p->call.esp_level = cur->esp_level;

					if(g_Cexe2c->GetFunc(address))
						p->call.call_func = g_Cexe2c->GetFunc(address);
					else
						p->call.call_func = g_Cexe2c->func_new(address);
					p->call.p_callpara = NULL;
					p->call.p_callret = NULL;
					InstrAddTail(p);
				}
			}
			else
				Code_general(0, i_Unknown);
		}
        else if (pxcpu->op[0].mode == OP_Register)
        {
            ea_t address = FindApiAddress_Reg(pxcpu->op[0].reg.reg_index, pxcpu, this->Q->m_asmlist);
            CApi* papi = g_ApiManage->get_api(address);	//find it
            if (papi)
            {
                PINSTR	p = new INSTR;  //new_INSTR
                p->type = i_CallApi;
                p->call.papi = papi;
                p->call.esp_level = cur->esp_level;
                p->call.p_callpara = NULL;
                p->call.p_callret = NULL;
                InstrAddTail(p);
            }
            else
                Code_general(0, i_Unknown);
        }
        else
            Code_general(0, i_Unknown);
		break;
	case C_RET:	
		Code_general(0, i_Return);	
		break;

	case C_RDTSC:
		break;

	case C_NOP:    
		break;

	case C_LODSB:  
		break;

	case C_SETNZ:	
		Code_general(enum_W, i_Setnz);
		break;

	default:	
		Code_general(0, i_Unknown);	break;

	}
}

void	CCodeList_Maker::Code_Jxx(JxxType t)
{
	PINSTR p = new INSTR;   // new_INSTR
	p->type = i_Jump;
	p->jmp.jmp_type = t;	// 跳转的类型
	p->jmp.jmpto_off = cur->xcpu.op[0].nearptr.offset;
	p->jmp.linear = cur->linear ;
	InstrAddTail(p);
}

//	只有type == enum_RR时，返回值才有用
PINSTR	CCodeList_Maker::Code_general(int type, HLType t)
{	
	PINSTR	p = new INSTR;  // new_INSTR
	p->type = t;			// 伪指令类型，如i_Assign，i_Add

	switch (type)
	{
	case enum_00:	// 没有操作数的伪指令
		InstrAddTail(p);
		return p;
	case enum_RR:	// 两个操作符都是读操作
		{
			TransVar(&p->var_r1, 0);	//	0 means	xcpu.op[0]
			TransVar(&p->var_r2, 1);	//	1 means	xcpu.op[1]
			VarRead(p->va_r1);
			VarRead(p->va_r2);
		}
		InstrAddTail(p);
		return p;
	case enum_WR:	// 第一个写操作
		{
			TransVar(&p->var_w, 0);		//	0 means	xcpu.op[0]
			TransVar(&p->var_r1, 1);	//	1 means	xcpu.op[1]
			if (t == i_Lea)
			{
				p->type = i_Assign;
				if (p->var_r1.type != v_Tem)
				{	//	比如象 lea eax,[ebp]
					p->type = i_GetAddr;
				}
			}
			else
			{
				VarRead(p->va_r1);
				//VarWrite(&p->var_w);
				if (p->var_w.type == v_Tem)
				{
					WriteToAddress(p);
					return p;
				}
			}
		}
		InstrAddTail(p);
		return p;
	case enum_AR:	// 第一个操作数既读也写，第两个只读取
		{
			VAR v;
			TransVar(&v, 0);
			TransVar(&p->var_r2, 1);
			p->var_r1 = v;
			p->var_w = v;
			VarRead(p->va_r2);
			VarRead(p->va_r1);
			//VarWrite(&p->var_w);
			if (p->var_w.type == v_Tem)
			{
				WriteToAddress(p);
				return p;
			}
		}
		InstrAddTail(p);
		return p;
	case enum_A:	 // 一个操作数，即读也写，如 not指令
		{
			VAR v;
			TransVar(&v, 0);
			p->var_w = v;
			p->var_r1 = v;
			VarRead(p->va_r1);
			if (p->var_w.type == v_Tem)
			{
				WriteToAddress(p);
				return p;
			}
		}
		InstrAddTail(p);
		return p;
	case enum_W:	 // 一个操作数，只写Setnz
		{
			VAR v;
			TransVar(&v, 0);
			p->var_w = v;
		}
		InstrAddTail(p);
		return p;
	default:
		alert("CCodeList_Maker::Code_general 出错");	// 没有定义的类型报错
		return NULL;
	}
	//return NULL;
}
void	CCodeList_Maker::TransVar(VAR* pvar,int no)
{
	TransVar_(pvar,no);
	if (pvar->type)
	{
		assert(pvar->opsize);
	}
}
// 把第no个操作数转为伪代码信息
void	CCodeList_Maker::TransVar_(VAR* pvar,int no)
//SuperC_func: 只在＜CCodeList_Maker::TransVar＞中使用
{
	OPERITEM* op = &this->cur->xcpu.op[no];
	switch (op->mode)
	{
	case OP_Register:	// 寄存器
		pvar->type = v_Reg;
		pvar->opsize = op->opersize;
		pvar->reg = regindex_2_regoff(op->reg.reg_index);
		return;
	case OP_Immed:		// 立即数
		pvar->type = v_Immed;
		pvar->opsize = op->opersize;
		pvar->d = op->immed.immed_value;
		return;
	case OP_Address:	// []形式
		if (op->addr.base_reg_index == _NOREG_
			&& op->addr.off_reg_index == _NOREG_)
		{
            if (op->addr.off_value == 0 && op->addr.seg_index == _FS_)
            {//判断 fs:[0]
                pvar->type = v_Volatile;    //现在只用于 fs:0
                pvar->opsize = op->opersize;
                pvar->temno = 222;			//只要是偶数就行
                return;
            }
			pvar->type = v_Global;
			pvar->opsize = op->opersize;
			pvar->off = op->addr.off_value;
			return;
		}
		if (op->addr.base_reg_index == _ESP_		// 基址为esp
			&& op->addr.off_reg_index == _NOREG_)	// 不是寄存器
		{
			pvar->opsize = op->opersize;
			signed long l = this->cur->esp_level + (signed int)op->addr.off_value;
			if (l >= 0)
			{
				pvar->par_off = l;
				pvar->type = v_Par;
			}
			else
			{
				pvar->var_off = stack2varoff(l);
				pvar->type = v_Var;
			}
			return;
		}
		if (op->addr.base_reg_index == _EBP_		// 基址为ebp
			&& this->m_EBP_base != Not_EBP_based
			&& op->addr.off_reg_index == _NOREG_)	// 不是寄存器
		{
			pvar->opsize = op->opersize;
			signed long l = this->m_EBP_base + (signed int)op->addr.off_value;
			if (l >= 0)
			{
				pvar->par_off = l;
				pvar->type = v_Par;
			}
			else
			{
				pvar->var_off = stack2varoff(l);
				pvar->type = v_Var;
			}
			return;
		}
		//	其它的形式，如 [ebx+8]
		{
			VAR v;
			new_temp(&v);

			PINSTR p = new INSTR;   //new_INSTR
			p->type = i_Address;
			
			p->var_w = v;

			set_address(op, p);		// 

			InstrAddTail(p);

			*pvar = v;
		}
		return;
	default:
		//warn_msg(0,"op mode unknown");
		break;
	}
}

//	这个函数的意思是，如果这是个v_Tem,则加一条 i_Readpointto
void	CCodeList_Maker::VarRead(VAR_ADDON& va)
{	
    VAR* pvar = va.pv;
    Pst_InstrAddOn &pAddOn = va.pao;
	if (pvar->type != v_Tem)
		return;

    Pst_InstrAddOn pnew = new st_InstrAddOn;
    pnew->type = IA_ReadPointTo;
    pnew->pChild = pAddOn;
    pAddOn = pnew;

    return;
}

void	CCodeList_Maker::WriteToAddress(PINSTR p)
{	//	这个函数的意思是，如果是向一个address写，则加一条 i_Writepointto
	
//	对于add [ebx+4],6,变为
//		tem_1 = i_addr(ebx,4);
//		tem_2 = i_readpointto(tem_1);
//		tem_3 = tem_2 + 6;
//		i_writepointto(tem_1, tem_3);

	
	//	当前的情况是：
	//		tem1 addr eax,ebx*4,401000
	//		tem1 = ????
	//	要改成
	//		tem1 addr eax,ebx*4,401000
	//		tem2 = ????
	//		Writepointto(tem1, tem2);
	if (p->var_w.type != v_Tem)
	{	//	实际上不会来这里
		InstrAddTail(p);
		return;
	}
	
	VAR tem1 = p->var_w;	//	sav it
	VAR tem2;

	new_temp(&tem2);

	p->var_w = tem2;
	InstrAddTail(p);	//	add this

	PINSTR pnew = new INSTR;    //new_INSTR
	pnew->type = i_Writepointto;
	pnew->var_r1 = tem1;				// the pointer
	pnew->var_r2 = tem2;					// the value
		//	对 i_Writepointto, 是var_r1是指针，var_r2是值
	InstrAddTail(pnew);
	
}

extern int g_newtemno;
void	CCodeList_Maker::new_temp(VAR* pvar)
{
    pvar->type = v_Tem;
    pvar->temno = g_newtemno;
    g_newtemno += 2;
	pvar->opsize = BIT32_is_4;	//	temp var always DWORD
}
