#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "ax-vm.h"

int ref(val addr, int n, val *out)
{
	return -1;
}

int reg(int num, val *out)
{
	return -1;
}

void trace(val addr, val size)
{
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
	vm.ref = ref;
	vm.reg = reg;
	vm.trace = trace;

	eval(&vm);

	while (vm.sp-- > 0) {
		printf("stack[%d] = " VAL_FMT "\n", vm.sp, vm.stack[vm.sp]);
	}

	return 0;
}
