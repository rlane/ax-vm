#include <stdint.h>

#ifndef _AX_VM_H

enum agent_op {
	aop_float = 0x01,
	aop_add = 0x02,
	aop_sub = 0x03,
	aop_mul = 0x04,
	aop_div_signed = 0x05,
	aop_div_unsigned = 0x06,
	aop_rem_signed = 0x07,
	aop_rem_unsigned = 0x08,
	aop_lsh = 0x09,
	aop_rsh_signed = 0x0a,
	aop_rsh_unsigned = 0x0b,
	aop_trace = 0x0c,
	aop_trace_quick = 0x0d,
	aop_log_not = 0x0e,
	aop_bit_and = 0x0f,
	aop_bit_or = 0x10,
	aop_bit_xor = 0x11,
	aop_bit_not = 0x12,
	aop_equal = 0x13,
	aop_less_signed = 0x14,
	aop_less_unsigned = 0x15,
	aop_ext = 0x16,
	aop_ref8 = 0x17,
	aop_ref16 = 0x18,
	aop_ref32 = 0x19,
	aop_ref64 = 0x1a,
	aop_ref_float = 0x1b,
	aop_ref_double = 0x1c,
	aop_ref_long_double = 0x1d,
	aop_l_to_d = 0x1e,
	aop_d_to_l = 0x1f,
	aop_if_goto = 0x20,
	aop_goto = 0x21,
	aop_const8 = 0x22,
	aop_const16 = 0x23,
	aop_const32 = 0x24,
	aop_const64 = 0x25,
	aop_reg = 0x26,
	aop_end = 0x27,
	aop_dup = 0x28,
	aop_pop = 0x29,
	aop_zero_ext = 0x2a,
	aop_swap = 0x2b,
	aop_trace16 = 0x30,
	aop_last
};

typedef uint64_t val;

struct vm {
	unsigned char *bytecode;
	int bytecode_len;
	val stack[1024];
	int sp, pc;
};

void eval(struct vm *vm);

#endif
