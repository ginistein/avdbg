// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	analysis.cpp
#include "stdafx.h"

#include "cisc.h"
#include "CStrategy.h"

extern bool g_f_Step_by_Step;
extern bool g_any1_return_TRUE;

bool	g_Hxd_Step;

bool Step_by_Step();

typedef bool (CFuncOptim::*OPTIMFUNC)();
typedef bool (CFunc::*PROGFUNC)();

PROGFUNC tbl_Progress[] =
{
    &CFunc::Step1_Disassembly,		//0 反汇编当前函数，计算结束地址，添加CALL指令后的地址为新函数
    &CFunc::Step2_GetRetPurge,
    &CFunc::Step3_AddRemoveSomeInstr,
    &CFunc::Step4_FillStackInfo,		//3  填充堆栈位置
    &CFunc::Step5_Label_Analysis,	//4  跳转分析

	&CFunc::Step6_CreateInstrList,	//5  创建伪代码
	&CFunc::Step7_GetArgs,			//6	 获取当前函数参数数量
    &CFunc::Step8_ana_RetType,		//7  检查函数的返回值
    &CFunc::Func_FillCallParas,		//8  填充函数参数信息

    &CFunc::Step10_Var_analysis,		//9	 分析变量，创建每个变量的thevar信息 （M_t结构）
    0
};

OPTIMFUNC tbl_Ana[] =
{
	&CFuncOptim::Step1_Address_to_Add,			//0  把i_Address转换成i_Add
	&CFuncOptim::Step2_Simplify_Instr,			//1  把简单指令转换成赋值指令i_Assign
    &CFuncOptim::ClassSubFuncProcess,            //2  如果是一个class的子函数
	&CFuncOptim::DataType_Flow,	                //3  数据类型流分析
	&CFuncOptim::pcode_1,	                    //4  如果cmp和jxx紧靠，把cmp和jxx合并
	&CFuncOptim::optim_once_new,	                //5  生成变量特征码，对变量进行优化
	&CFuncOptim::ana_Flow,	                    //6	 流程分析,标出所有的i_Begin和i_End

	&CFuncOptim::VarDataType_analysis_mydefine,	//7  变量类型分析
	&CFuncOptim::VarDataType_analysis,	        //8  变量类型分析
	//CFuncOptim::Step9_Var_Split,				//10  寄存器变量拆分，放到最后
	0
};


bool	CFunc::analysis_once_1()
{
    m_prepareTrue_analysisFalse = false;

	// 第一部分
	if(m_nStep<STEP_100)
	{
		for(;m_nStep < STEP_100;m_nStep++)	// 伪代码需要第10步变量命名后才可以显示
		{
			PROGFUNC fn = tbl_Progress[m_nStep];
			if (fn != NULL)
			{
				log_prtl("step %d",this->m_nStep);
				if (!(this->*fn)())
					return false;	// 处理失败
			}
			else					// 处理完成
				break;
		}

		log_prtl("第一部分处理完成！");
		this->m_nStep = STEP_100;   //说明过关了！

		return true;	// 处理完成后停一次
	}

	// 开始处理第二部分
    CFuncOptim the(this);

	if(m_nStep_Two<STEP_100)
	{
		for (;m_nStep_Two<STEP_100;m_nStep_Two++)
		{
			OPTIMFUNC pfunc = tbl_Ana[m_nStep_Two];
			if (pfunc != NULL)
			{
				log_prtl("第二部分 step %d",m_nStep_Two);
				if ((the.*pfunc)())
					return true;		// 处理完成一小步
				else
				{
					if(g_Hxd_Step)		// 每处理完成一大步，停一次
					{
						m_nStep_Two++;
						return false;
					}
				}
			}
			else
				break;	// 处理完成
		}
		log_prtl("第二部分处理完成！");
		this->m_nStep_Two = STEP_100;   //说明过关了！
	}

	return false;
}

bool	CFunc::analysis_once()	
{
    bool f = analysis_once_1();
    if (f)
    {
        if (g_CStrategy.IfAny())	// 变量优化处理
        {
			if(g_Hxd_Step)		// 当步时才显示
				g_CStrategy.PrintIt(this->m_instr_list, this);		// 打印优化信息
            g_CStrategy.DoIt(this->m_instr_list, this->m_exprs);	// 优化处理
        }
        DeleteUnusedVar();			// 删除不需要的变量
    }
    return f;
}

void CFunc::analysis()	// 一步反编译
{
    for (;;)
    {
        g_f_Step_by_Step = true;
        g_any1_return_TRUE = false;
		g_Hxd_Step = false;
        if (!this->analysis_once())
            break;
    }
}

bool	CFunc::analysis_step_by_step() // 单步进行
{
	//	让我们显示点信息
	static CFunc* lastfunc = 0;

	if (this != lastfunc)
		lastfunc = this;

	g_f_Step_by_Step = true;
	g_any1_return_TRUE = false;

	g_Hxd_Step = true;

	bool f = analysis_once();

	g_Hxd_Step = false;

	g_f_Step_by_Step = false;
	f = f || g_any1_return_TRUE;

	return f;
}


bool	CFunc::Step8_ana_RetType()	//	检查函数的返回值
{
    VAR v;
    v.type = v_Reg;
    v.reg = enum_EAX;			//	enum_EAX = 0 = enum_AL = enum_AX 
    v.opsize = BIT32_is_4;

    if (this->m_functype != NULL)
    {
        int n = GG_VarType_ID2Size(this->m_functype->m_retdatatype_id);
        if (n == 0)
            return true;
        if (n == 2 || n == 4)
            v.opsize = n;
    }


    POSITION pos = m_instr_list->GetHeadPosition();
	while (pos)
	{
		POSITION savpos = pos;
		PINSTR p = m_instr_list->GetNext(pos);
		if (p->type == i_Return)
		{
			PINSTR pnew = new INSTR;    //  new_INSTR
			pnew->type = i_RetPar;		//	暂时认为每一个func都是ret DWORD
			pnew->var_r1 = v;
			m_instr_list->InsertBefore(savpos,pnew);
		}
	}
	return true;
}

void CFunc::PrepareFunc()				// 运行tbl_Progress前三步
{
    m_prepareTrue_analysisFalse = true;
    while (this->m_nStep < 3)
    {
        PROGFUNC fn = tbl_Progress[m_nStep];
        if (fn != NULL)
        {
            if ((this->*fn)())
            {
                m_nStep++;
                continue;
            }
            break;
        }
        else
            this->m_nStep = STEP_100;
    }
}
