#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "ax-vm.h"

void eval(struct vm *vm)
{
	val imm(int n)
	{
		int i;
		val x = 0;
		for (i = 0; i < n; i++) {
			fprintf(stderr, "0x%x ", vm->bytecode[vm->pc]);
			x |= (vm->bytecode[vm->pc++] << ((n-i-1)*8));
		}
		fprintf(stderr, "-> " VAL_FMT "\n", x);
		return x;
	}

	void push(val x)
	{
		fprintf(stderr, "stack[%d] <- " VAL_FMT "\n", vm->sp, x);
		vm->stack[vm->sp++] = x;
	}

	val pop()
	{
		fprintf(stderr, "stack[%d] -> " VAL_FMT "\n", vm->sp-1, vm->stack[vm->sp-1]);
		return vm->stack[--vm->sp];
	}

	while (vm->pc >= 0 && vm->pc < vm->bytecode_len) {
		int start = vm->pc;
		int op = vm->bytecode[vm->pc];
		vm->pc++;

		fprintf(stderr, "op 0x%x\n", op);

		switch (op) {

#define BINOP(op) { \
				val b = pop(); \
				val a = pop(); \
				push(a op b); \
			  break; }

			case aop_add: BINOP(+);
			case aop_sub: BINOP(-);
			case aop_mul: BINOP(*);
			case aop_div_signed: BINOP(/); // XXX
			case aop_div_unsigned: BINOP(/);
			case aop_rem_signed: BINOP(%); // XXX
			case aop_rem_unsigned: BINOP(%);
			case aop_lsh: BINOP(<<);
			case aop_rsh_signed: BINOP(>>);
			case aop_rsh_unsigned: BINOP(>>); // XXX
			case aop_bit_and: BINOP(&);
			case aop_bit_or: BINOP(|);
			case aop_bit_xor: BINOP(^);
			case aop_equal: BINOP(==);
			case aop_less_signed: BINOP(<); // XXX
			case aop_less_unsigned: BINOP(<);

			case aop_log_not:
				push(!pop());
				break;
			case aop_bit_not:
				push(~pop());
				break;

			case aop_ext: {
				int n = imm(1);
				val a = pop();
				val bit = 1 << (n-1);
				val mask = ~(bit - 1);
				a &= mask;
				push(a);
				break;
			}
			case aop_zero_ext: {
				int n = imm(1);
				val a = pop();
				val bit = 1 << (n-1);
				val mask = ~(bit - 1);
				int sign = !!(a & bit);
				a &= mask;
				if (sign) a |= ~mask;
				push(a);
				break;
			}

#define REF(t) { \
				push(13); \
				break; \
			}

			case aop_ref8: REF(char);
			case aop_ref16: REF(short);
			case aop_ref32: REF(int);
			case aop_ref64: REF(long);

			case aop_dup: {
				val a = pop();
				push(a);
				push(a);
				break;
			}

			case aop_swap: {
				val a = pop();
				val b = pop();
				push(a);
				push(b);
				break;
			}

			case aop_pop:
				pop();
				break;

			case aop_if_goto: {
				int offset = imm(2);
				val a = pop();
				if (a != 0)
					vm->pc = start + offset;
				break;
			}

			case aop_goto: {
				int offset = imm(2);
				vm->pc = start + offset;
				break;
			}

#define IMM(n) push(imm(n)); break
			case aop_const8: IMM(1);
			case aop_const16: IMM(2);
			case aop_const32: IMM(4);
			case aop_const64: IMM(8);

			case aop_reg: {
				int num = imm(2);
				push(42);
				break;
			}

			case aop_trace: abort();
			case aop_trace_quick: abort();
			case aop_trace16: abort();

			case aop_end:
				vm->pc = -1;
				break;

			default:
				fprintf(stderr, "unknown opcode %x\n", op);
				exit(1);
		}
	}

	fprintf(stderr, "terminated\n");

	while (vm->sp-- > 0) {
		printf("stack[%d] = " VAL_FMT "\n", vm->sp, vm->stack[vm->sp]);
	}
}
