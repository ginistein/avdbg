// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

// INSTR.h
#ifndef INSTR__H
#define INSTR__H

class	CFunc;

class INSTR;
typedef	INSTR*	PINSTR;

enum em_InstrAddOn
{
    IA_Nothing = 0,
    IA_AddImmed,    // 简单地加立即数
    IA_MulImmed,    // 简单地乘立即数
    IA_ReadPointTo, // *
    IA_GetAddress,  // &
};

// 这个结果有什么用处呢？
struct st_InstrAddOn
{
    em_InstrAddOn type;

    union
    {
        struct
        {
            signed int iAddon;
        } addimmed; //for IA_AddImmed
    };

    st_InstrAddOn* pChild;

    st_InstrAddOn()
    {
        ZeroMemory(this, sizeof(st_InstrAddOn));

    }
	~st_InstrAddOn()
	{

		if(pChild)
		{
			delete pChild;
			pChild = NULL;
		}

	}
    static bool IsSame(st_InstrAddOn* p1, st_InstrAddOn* p2);
};

typedef st_InstrAddOn* Pst_InstrAddOn;

struct VAR_ADDON
{
	VAR*	pv;			// 变量信息
    st_InstrAddOn* pao; // point add on

    VAR_ADDON()
    {
        pv = NULL;
        pao = NULL;
    }
    ~VAR_ADDON()
    {
        //do not delete pv
        if (pao)
            delete pao;
    }
};

// 定义伪代码类
class INSTR
{
public:
	DWORD	id;			//  自动新增
	HLType	type;		//	伪代码的类型,如i_Assign ...
	bool	bJmpFlowOk;	// 处理过没
    union
	{
		struct
		{
			JxxType	jmp_type;	//跳转的类型，如JMP_???
			DWORD	linear;		//  跳转时，当前指令的地址
			DWORD   jmpto_off;
			PINSTR the_label;
			PINSTR next_ref_of_this_label;	//这里组成个链，用来保存对一个label的所有ref
		} jmp;					//for type = i_Jump only

		struct
		{
			PINSTR  ref_instr;		// for type = i_Label only
			ea_t	label_off;
			int		bJmpFlowNum;	// 流程分析时，已经成功处理过几次
			bool	f_conti;
		} label;

		struct
		{
			CFunc*  call_func;		// for i_Call
			CApi*	papi;			// for i_CallApi
			signed int	esp_level;
            PINSTR      p_callpara;
            PINSTR      p_callret;
		} call;

        struct
        {
            PINSTR      p_thecall;
        } call_addon;   //for i_CallPara and i_CallRet

		struct	// for,while i_CplxBegin的属性
		{
			PINSTR		m_end;			// 结束i_End伪代码
			enum_COMP	type;			// 跳转指令类型，如COMP_if
			PINSTR		m_break;		// 如果允许 break 的话，这是break到的label
			PINSTR		m_conti;		// 如果允许 continue 的话，这是continue到的label
			PINSTR		m_not_conti;	// 尽管这个jmp指向m_conti，它仍然不是continue.
		} begin;						// 这个特例是为了照顾while

        struct
        {
            signed int esp_level;
            UINT howlen;
        } espreport;		// for i_EspReport
	};

	VAR			var_w;		// 写操作的操作数信息
    VAR_ADDON	va_r1;		// va=Var AddOn
    VAR_ADDON	va_r2;		// 
	VAR			var_r1;		// 读操作的操作数1
	VAR			var_r2;		// 读操作的操作数2
	DWORD	i1;				// =4,     例如：当type == i_Address时，[eax+ebx*4+8888]
	DWORD	i2;				// =8888

	INSTR();
	~INSTR();
};

typedef	CList<PINSTR,PINSTR> INSTR_LIST;

// 保存跳转指令的属性
struct st_JumpOptm
{
    PINSTR pinstr;
	POSITION pos;	// 当前语句在伪代码中的位置
	bool   bJxx;	// 跳转
	em_JumpType  JumpType;	// 跳转的方向，向下为 1,向上为 2
	bool   bLable;
	int    iLableType;	// 跳来的方向，从上跳来的为 1,从下跳来的为 2，上下都有是3
    int	   iLableJmpFromNum;	// 跳转到此i_Lable的Jmp数量
	DWORD  dwJumpFrom[250];		// 跳转的地址

    st_JumpOptm()
    {
        ZeroMemory(this, sizeof(st_JumpOptm));
    }
    bool IsJump()
    {
        return (bJxx && pinstr->type == i_Jump);
    }
};
typedef CList<st_JumpOptm*, st_JumpOptm*> JUMPOPTM_LIST;
typedef	st_JumpOptm*	PJUMPOPTM;


class CInstrList
{
    bool	if_Ly_In(PINSTR p, POSITION firstpos, POSITION endpos);
    bool	IsSwitchCase_multcomp(PINSTR begin);
    bool	IsSwitchCase(PINSTR begin);
    bool	ifOneStatement(PINSTR pNode, POSITION firstpos, POSITION endpos);
    bool	Flow_c(PINSTR pNode);
    void	Flow_b(PINSTR pParentNode, POSITION firstpos, POSITION endpos);

    bool	Flow_aa(POSITION firstpos, POSITION endpos);

    bool	Flow_cc(PINSTR pNode, POSITION firstpos, POSITION endpos);
    void	Add_Begin_End(POSITION firstpos, POSITION endpos, PINSTR begin, PINSTR end);
    void	Add_Begin_End_1(POSITION firstpos, POSITION endpos, PINSTR begin, PINSTR end);

	// 以下内容，红尘岁月添加
	void	Get_Jump_Use_Flow(JUMPOPTM_LIST& used_list);		// 得到跳转使用伪代码列表
	char	HowJumpUse_Char(st_JumpOptm* the);
	bool	Optim_Jump_flow_NT(JUMPOPTM_LIST& Jumplist,char hxd_tbl_c[]);	// 分析跳转流程

	void	Hxd_Add_CplxBegin_CplxEnd(POSITION firstpos, POSITION endpos,enum_COMP comptype);
	bool	Hxd_Add_Begin_End(POSITION firstpos, POSITION endpos);

	void	Hxd_While(POSITION firstpos, POSITION jxx,POSITION jump,POSITION endpos);
	void	Hxd_do_While(POSITION firstpos,POSITION endpos);
	void	Hxd_If(POSITION firstpos,POSITION endpos,enum_COMP comptype);
	void	Hxd_If_Or(POSITION firstpos, POSITION jxx,POSITION label,POSITION endpos);
	void	Hxd_If_else(POSITION firstpos, POSITION jxx,POSITION label,POSITION endpos);
	void	Hxd_Break(POSITION Jxx,POSITION While);

	// end 红尘岁月
    
    INSTR_LIST* m_list; //要尽量把它private

public:
    CInstrList(INSTR_LIST* p)
    {
        m_list = p;
    }

    bool	Flow_a(PINSTR pNode);
	bool	Hxd_JumpFlow();
};

class CInstrList_Finger
{
    void	prt_partern(PINSTR phead, PSTR partern_buf);
    static int search_and_add(DWORD* buf,DWORD val,int* pn);
    static bool	finger_compare(PSTR f1,const char* f2);
public:
    INSTR_LIST* m_list; //要尽量把它private
    CInstrList_Finger(INSTR_LIST* p)
    {
        m_list = p;
    }
    bool	Finger_check_partern(PINSTR p);
    bool	Finger_check_partern_for1(PINSTR p);
};




#endif // INSTR__H
