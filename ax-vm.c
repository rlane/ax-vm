#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

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

typedef unsigned long val;

struct vm {
	unsigned char *bytecode;
	int bytecode_len;
	val stack[1024];
	int sp, pc;
};

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
		fprintf(stderr, "-> 0x%lx\n", x);
		return x;
	}

	void push(val x)
	{
		fprintf(stderr, "stack[%d] <- 0x%lx\n", vm->sp, x);
		vm->stack[vm->sp++] = x;
	}

	val pop()
	{
		fprintf(stderr, "stack[%d] -> 0x%lx\n", vm->sp-1, vm->stack[vm->sp-1]);
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
				t *addr = (void*) pop(); \
				push(*addr); \
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
		printf("stack[%d] = 0x%lx\n", vm->sp, vm->stack[vm->sp]);
	}
}

int main(int argc, int **argv)
{
	struct vm vm;
	char buf[1024];
	int c, n = 0;

	while ((c = read(STDIN_FILENO, buf+n, sizeof(buf)-n)) > 0)
		n += c;

	if (c < 0) {
		perror("read");
		exit(1);
	}

	vm.pc = 0;
	vm.sp = 0;
	vm.bytecode = buf;
	vm.bytecode_len = n;

	eval(&vm);

	return 0;
}
