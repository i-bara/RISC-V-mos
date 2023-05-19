	.file	"init.c"
	.option nopic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.globl	mips_init
	.type	mips_init, @function
mips_init:
	addi	sp,sp,-16
	sd	s0,8(sp)
	addi	s0,sp,16
	nop
	ld	s0,8(sp)
	addi	sp,sp,16
	jr	ra
	.size	mips_init, .-mips_init
	.ident	"GCC: (g2ee5e430018) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
