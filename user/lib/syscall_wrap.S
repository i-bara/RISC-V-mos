#include <asm/asm.h>

.globl msyscall;
	.align 2;
	.type msyscall, @function;
	msyscall:           
	// Just use 'syscall' instruction and return.

	/* Exercise 4.1: Your code here. */
	ecall
	jr		ra

.end function;
.size function, .- function
