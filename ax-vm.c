#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "ax-vm.h"

int verbose = 1;

#ifdef DEBUG
#define debug(fmt, ...) ({ if (verbose) fprintf(stderr, fmt, ##  __VA_ARGS__); })
#else
#define debug(fmt, ...)
#endif

void eval(struct vm *vm)
{
	val imm(int n)
	{
		int i;
		val x = 0;
		for (i = 0; i < n; i++) {
			debug("0x%x ", vm->bytecode[vm->pc]);
			x |= (vm->bytecode[vm->pc++] << ((n-i-1)*8));
		}
		debug("-> " VAL_FMT "\n", x);
		return x;
	}

	void push(val x)
	{
		debug("stack[%d] <- " VAL_FMT "\n", vm->sp, x);
		vm->stack[vm->sp++] = x;
	}

	val pop()
	{
		debug("stack[%d] -> " VAL_FMT "\n", vm->sp-1, vm->stack[vm->sp-1]);
		return vm->stack[--vm->sp];
	}

	while (vm->pc >= 0 && vm->pc < vm->bytecode_len) {
		int start = vm->pc;
		int op = vm->bytecode[vm->pc];
		vm->pc++;

		debug("op 0x%x\n", op);

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

#define REF(n) { \
				val x; \
        int ret = vm->ref(pop(), n, &x); \
	      if (ret) { \
				  debug("ref failed\n"); \
					exit(1); \
				} \
				push(x); \
				break; \
			}

			case aop_ref8: REF(1);
			case aop_ref16: REF(2);
			case aop_ref32: REF(4);
			case aop_ref64: REF(8);

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
				val out;
				if (vm->reg(num, &out)) {
					debug("reg failed\n");
					exit(1);
				}
				push(out);
				break;
			}

			case aop_trace: {
				val size = pop();
				val addr = pop();
				vm->trace(addr, size);
				break;
			}
			case aop_trace_quick: {
				val size = imm(1);
				val addr = pop();
				vm->trace(addr, size);
				break;
			}
			case aop_trace16: {
				val size = imm(2);
				val addr = pop();
				vm->trace(addr, size);
				break;
			}

			case aop_end:
				vm->pc = -1;
				break;

			default:
				debug("unknown opcode %x\n", op);
				exit(1);
		}
	}

	debug("terminated\n");
}
