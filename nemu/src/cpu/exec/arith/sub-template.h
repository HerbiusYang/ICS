
#include "cpu/exec/template-start.h"
#define instr sub

static void do_execute () 
{
	DATA_TYPE res = op_dest->val - op_src->val;
	op_dest -> val = res;

	if(res == 0) cpu.ZF = 1;
	else if(res >= 0){
		cpu.SF = cpu.CF = 0;
	}
	else{
		cpu.SF = cpu.CF = 1;
	}

	if(op_dest->val < 0 && op_src->val > 0 && res < 0)
		cpu.OF = 1;
	else cpu.OF = 0;
	print_asm_template2();
}

make_instr_helper(si2rm)

#include "cpu/exec/template-end.h"