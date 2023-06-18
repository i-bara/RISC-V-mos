#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

#ifdef RISCV32

// clang-format off
.macro SAVE_ALL
// .set noreorder
// .set noat
// 	csrw    sscratch, sp
// .set reorder
// 	bltz    sp, 1f
// 	li      sp, KSTACKTOP
// .set noreorder
// 1:
	csrrw	sp, sscratch, sp
	li		sp, KSTACKTOP
	sw		ra, TF_REG1 - TF_SIZE(sp)
	sw      sp, TF_REG2 - TF_SIZE(sp)
	addi    sp, sp, -TF_SIZE

	csrr	ra, sie
	sw		ra, TF_SIE(sp)
	csrr	ra, sip
	sw		ra, TF_SIP(sp)
	csrr	ra, sepc
	sw		ra, TF_SEPC(sp)
	csrr	ra, stvec
	sw		ra, TF_STVEC(sp)
	csrr	ra, scause
	sw		ra, TF_SCAUSE(sp)
	csrr	ra, stval
	sw		ra, TF_STVAL(sp)
	csrr	ra, sstatus
	sw		ra, TF_SSTATUS(sp)
	csrr	ra, sscratch
	sw		ra, TF_SSCRATCH(sp)

	sw      x0, TF_REG0(sp)

	sw      x3, TF_REG3(sp)
	sw      x4, TF_REG4(sp)
	sw      x5, TF_REG5(sp)
	sw      x6, TF_REG6(sp)
	sw      x7, TF_REG7(sp)
	sw      x8, TF_REG8(sp)
	sw      x9, TF_REG9(sp)
	sw      x10, TF_REG10(sp)
	sw      x11, TF_REG11(sp)
	sw      x12, TF_REG12(sp)
	sw      x13, TF_REG13(sp)
	sw      x14, TF_REG14(sp)
	sw      x15, TF_REG15(sp)
	sw      x16, TF_REG16(sp)
	sw      x17, TF_REG17(sp)
	sw      x18, TF_REG18(sp)
	sw      x19, TF_REG19(sp)
	sw      x20, TF_REG20(sp)
	sw      x21, TF_REG21(sp)
	sw      x22, TF_REG22(sp)
	sw      x23, TF_REG23(sp)
	sw      x24, TF_REG24(sp)
	sw      x25, TF_REG25(sp)
	sw      x26, TF_REG26(sp)
	sw      x27, TF_REG27(sp)
	sw      x28, TF_REG28(sp)
	sw      x29, TF_REG29(sp)
	sw      x30, TF_REG30(sp)
	sw      x31, TF_REG31(sp)
// .set at
// .set reorder
.endm
/*
 * Note that we restore the IE flags from stack. This means
 * that a modified IE mask will be nullified.
 */
.macro RESTORE_SOME
// .set noreorder
// .set noat
	lw		ra, TF_SIE(sp)
	csrw	sie, ra
	lw		ra, TF_SIP(sp)
	csrw	sip, ra
	lw		ra, TF_SEPC(sp)
	csrw	sepc, ra
	lw		ra, TF_STVEC(sp)
	csrw	stvec, ra
	lw		ra, TF_SCAUSE(sp)
	csrw	scause, ra
	lw		ra, TF_STVAL(sp)
	csrw	stval, ra
	lw		ra, TF_SSTATUS(sp)
	csrw	sstatus, ra
	lw		ra, TF_SSCRATCH(sp)
	csrw	sscratch, ra

	lw      x31, TF_REG31(sp)
	lw      x30, TF_REG30(sp)
	lw      x29, TF_REG29(sp)
	lw      x28, TF_REG28(sp)
	lw      x27, TF_REG27(sp)
	lw      x26, TF_REG26(sp)
	lw      x25, TF_REG25(sp)
	lw      x24, TF_REG24(sp)
	lw      x23, TF_REG23(sp)
	lw      x22, TF_REG22(sp)
	lw      x21, TF_REG21(sp)
	lw      x20, TF_REG20(sp)
	lw      x19, TF_REG19(sp)
	lw      x18, TF_REG18(sp)
	lw      x17, TF_REG17(sp)
	lw      x16, TF_REG16(sp)
	lw      x15, TF_REG15(sp)
	lw      x14, TF_REG14(sp)
	lw      x13, TF_REG13(sp)
	lw      x12, TF_REG12(sp)
	lw      x11, TF_REG11(sp)
	lw      x10, TF_REG10(sp)
	lw      x9, TF_REG9(sp)
	lw      x8, TF_REG8(sp)
	lw      x7, TF_REG7(sp)
	lw      x6, TF_REG6(sp)
	lw      x5, TF_REG5(sp)
	lw      x4, TF_REG4(sp)
	lw      x3, TF_REG3(sp)

	lw      x1, TF_REG1(sp)
// .set at
// .set reorder
.endm

.macro PRINT_REG reg
// .set noreorder
// .set noat
	SAVE_ALL
		move	a0, \reg
		la		t0, print_reg
		jalr	t0
	RESTORE_SOME
	lw		sp, TF_REG29(sp)
.endm

.macro PRINT_CP0 cp0
// .set noreorder
// .set noat
	SAVE_ALL
		mfc0	a0, \cp0
		la		t0, print_reg
		jalr	t0
	RESTORE_SOME
	lw		sp, TF_REG29(sp)
.endm

.macro PRINT_REGS
	SAVE_ALL
	lw		a0, TF_REG0(sp)
	la		t0, print_reg_zero
	jalr	t0
	lw		a0, TF_REG1(sp)
	la		t0, print_reg_at
	jalr	t0
	lw		a0, TF_REG2(sp)
	la		t0, print_reg_v0
	jalr	t0
	lw		a0, TF_REG3(sp)
	la		t0, print_reg_v1
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG4(sp)
	la		t0, print_reg_a0
	jalr	t0
	lw		a0, TF_REG5(sp)
	la		t0, print_reg_a1
	jalr	t0
	lw		a0, TF_REG6(sp)
	la		t0, print_reg_a2
	jalr	t0
	lw		a0, TF_REG7(sp)
	la		t0, print_reg_a3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG8(sp)
	la		t0, print_reg_t0
	jalr	t0
	lw		a0, TF_REG9(sp)
	la		t0, print_reg_t1
	jalr	t0
	lw		a0, TF_REG10(sp)
	la		t0, print_reg_t2
	jalr	t0
	lw		a0, TF_REG11(sp)
	la		t0, print_reg_t3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG12(sp)
	la		t0, print_reg_t4
	jalr	t0
	lw		a0, TF_REG13(sp)
	la		t0, print_reg_t5
	jalr	t0
	lw		a0, TF_REG14(sp)
	la		t0, print_reg_t6
	jalr	t0
	lw		a0, TF_REG15(sp)
	la		t0, print_reg_t7
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG16(sp)
	la		t0, print_reg_s0
	jalr	t0
	lw		a0, TF_REG17(sp)
	la		t0, print_reg_s1
	jalr	t0
	lw		a0, TF_REG18(sp)
	la		t0, print_reg_s2
	jalr	t0
	lw		a0, TF_REG19(sp)
	la		t0, print_reg_s3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG20(sp)
	la		t0, print_reg_s4
	jalr	t0
	lw		a0, TF_REG21(sp)
	la		t0, print_reg_s5
	jalr	t0
	lw		a0, TF_REG22(sp)
	la		t0, print_reg_s6
	jalr	t0
	lw		a0, TF_REG23(sp)
	la		t0, print_reg_s7
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG24(sp)
	la		t0, print_reg_t8
	jalr	t0
	lw		a0, TF_REG25(sp)
	la		t0, print_reg_t9
	jalr	t0
	lw		a0, TF_REG26(sp)
	la		t0, print_reg_k0
	jalr	t0
	lw		a0, TF_REG27(sp)
	la		t0, print_reg_k1
	jalr	t0
	la		t0, print_endl
	jalr	t0
	lw		a0, TF_REG28(sp)
	la		t0, print_reg_gp
	jalr	t0
	lw		a0, TF_REG29(sp)
	la		t0, print_reg_sp
	jalr	t0
	lw		a0, TF_REG30(sp)
	la		t0, print_reg_fp
	jalr	t0
	lw		a0, TF_REG31(sp)
	la		t0, print_reg_ra
	jalr	t0
	la		t0, print_endl
	jalr	t0
	RESTORE_SOME
	lw		sp, TF_REG2(sp)
.endm

#else // riscv64

// clang-format off
.macro SAVE_ALL
// .set noreorder
// .set noat
// 	csrw    sscratch, sp
// .set reorder
// 	bltz    sp, 1f
// 	li      sp, KSTACKTOP
// .set noreorder
// 1:
	csrrw	sp, sscratch, sp
	li		sp, KSTACKTOP
	sd		ra, TF_REG1 - TF_SIZE(sp)
	sd      sp, TF_REG2 - TF_SIZE(sp)
	addi    sp, sp, -TF_SIZE

	csrr	ra, sie
	sd		ra, TF_SIE(sp)
	csrr	ra, sip
	sd		ra, TF_SIP(sp)
	csrr	ra, sepc
	sd		ra, TF_SEPC(sp)
	csrr	ra, stvec
	sd		ra, TF_STVEC(sp)
	csrr	ra, scause
	sd		ra, TF_SCAUSE(sp)
	csrr	ra, stval
	sd		ra, TF_STVAL(sp)
	csrr	ra, sstatus
	sd		ra, TF_SSTATUS(sp)
	csrr	ra, sscratch
	sd		ra, TF_SSCRATCH(sp)

	sd      x0, TF_REG0(sp)

	sd      x3, TF_REG3(sp)
	sd      x4, TF_REG4(sp)
	sd      x5, TF_REG5(sp)
	sd      x6, TF_REG6(sp)
	sd      x7, TF_REG7(sp)
	sd      x8, TF_REG8(sp)
	sd      x9, TF_REG9(sp)
	sd      x10, TF_REG10(sp)
	sd      x11, TF_REG11(sp)
	sd      x12, TF_REG12(sp)
	sd      x13, TF_REG13(sp)
	sd      x14, TF_REG14(sp)
	sd      x15, TF_REG15(sp)
	sd      x16, TF_REG16(sp)
	sd      x17, TF_REG17(sp)
	sd      x18, TF_REG18(sp)
	sd      x19, TF_REG19(sp)
	sd      x20, TF_REG20(sp)
	sd      x21, TF_REG21(sp)
	sd      x22, TF_REG22(sp)
	sd      x23, TF_REG23(sp)
	sd      x24, TF_REG24(sp)
	sd      x25, TF_REG25(sp)
	sd      x26, TF_REG26(sp)
	sd      x27, TF_REG27(sp)
	sd      x28, TF_REG28(sp)
	sd      x29, TF_REG29(sp)
	sd      x30, TF_REG30(sp)
	sd      x31, TF_REG31(sp)
// .set at
// .set reorder
.endm
/*
 * Note that we restore the IE flags from stack. This means
 * that a modified IE mask will be nullified.
 */
.macro RESTORE_SOME
// .set noreorder
// .set noat
	ld		ra, TF_SIE(sp)
	csrw	sie, ra
	ld		ra, TF_SIP(sp)
	csrw	sip, ra
	ld		ra, TF_SEPC(sp)
	csrw	sepc, ra
	ld		ra, TF_STVEC(sp)
	csrw	stvec, ra
	ld		ra, TF_SCAUSE(sp)
	csrw	scause, ra
	ld		ra, TF_STVAL(sp)
	csrw	stval, ra
	ld		ra, TF_SSTATUS(sp)
	csrw	sstatus, ra
	ld		ra, TF_SSCRATCH(sp)
	csrw	sscratch, ra

	ld      x31, TF_REG31(sp)
	ld      x30, TF_REG30(sp)
	ld      x29, TF_REG29(sp)
	ld      x28, TF_REG28(sp)
	ld      x27, TF_REG27(sp)
	ld      x26, TF_REG26(sp)
	ld      x25, TF_REG25(sp)
	ld      x24, TF_REG24(sp)
	ld      x23, TF_REG23(sp)
	ld      x22, TF_REG22(sp)
	ld      x21, TF_REG21(sp)
	ld      x20, TF_REG20(sp)
	ld      x19, TF_REG19(sp)
	ld      x18, TF_REG18(sp)
	ld      x17, TF_REG17(sp)
	ld      x16, TF_REG16(sp)
	ld      x15, TF_REG15(sp)
	ld      x14, TF_REG14(sp)
	ld      x13, TF_REG13(sp)
	ld      x12, TF_REG12(sp)
	ld      x11, TF_REG11(sp)
	ld      x10, TF_REG10(sp)
	ld      x9, TF_REG9(sp)
	ld      x8, TF_REG8(sp)
	ld      x7, TF_REG7(sp)
	ld      x6, TF_REG6(sp)
	ld      x5, TF_REG5(sp)
	ld      x4, TF_REG4(sp)
	ld      x3, TF_REG3(sp)

	ld      x1, TF_REG1(sp)
// .set at
// .set reorder
.endm

.macro PRINT_REG reg
// .set noreorder
// .set noat
	SAVE_ALL
		move	a0, \reg
		la		t0, print_reg
		jalr	t0
	RESTORE_SOME
	ld		sp, TF_REG29(sp)
.endm

.macro PRINT_CP0 cp0
// .set noreorder
// .set noat
	SAVE_ALL
		mfc0	a0, \cp0
		la		t0, print_reg
		jalr	t0
	RESTORE_SOME
	ld		sp, TF_REG29(sp)
.endm

.macro PRINT_REGS
	SAVE_ALL
	ld		a0, TF_REG0(sp)
	la		t0, print_reg_zero
	jalr	t0
	ld		a0, TF_REG1(sp)
	la		t0, print_reg_at
	jalr	t0
	ld		a0, TF_REG2(sp)
	la		t0, print_reg_v0
	jalr	t0
	ld		a0, TF_REG3(sp)
	la		t0, print_reg_v1
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG4(sp)
	la		t0, print_reg_a0
	jalr	t0
	ld		a0, TF_REG5(sp)
	la		t0, print_reg_a1
	jalr	t0
	ld		a0, TF_REG6(sp)
	la		t0, print_reg_a2
	jalr	t0
	ld		a0, TF_REG7(sp)
	la		t0, print_reg_a3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG8(sp)
	la		t0, print_reg_t0
	jalr	t0
	ld		a0, TF_REG9(sp)
	la		t0, print_reg_t1
	jalr	t0
	ld		a0, TF_REG10(sp)
	la		t0, print_reg_t2
	jalr	t0
	ld		a0, TF_REG11(sp)
	la		t0, print_reg_t3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG12(sp)
	la		t0, print_reg_t4
	jalr	t0
	ld		a0, TF_REG13(sp)
	la		t0, print_reg_t5
	jalr	t0
	ld		a0, TF_REG14(sp)
	la		t0, print_reg_t6
	jalr	t0
	ld		a0, TF_REG15(sp)
	la		t0, print_reg_t7
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG16(sp)
	la		t0, print_reg_s0
	jalr	t0
	ld		a0, TF_REG17(sp)
	la		t0, print_reg_s1
	jalr	t0
	ld		a0, TF_REG18(sp)
	la		t0, print_reg_s2
	jalr	t0
	ld		a0, TF_REG19(sp)
	la		t0, print_reg_s3
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG20(sp)
	la		t0, print_reg_s4
	jalr	t0
	ld		a0, TF_REG21(sp)
	la		t0, print_reg_s5
	jalr	t0
	ld		a0, TF_REG22(sp)
	la		t0, print_reg_s6
	jalr	t0
	ld		a0, TF_REG23(sp)
	la		t0, print_reg_s7
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG24(sp)
	la		t0, print_reg_t8
	jalr	t0
	ld		a0, TF_REG25(sp)
	la		t0, print_reg_t9
	jalr	t0
	ld		a0, TF_REG26(sp)
	la		t0, print_reg_k0
	jalr	t0
	ld		a0, TF_REG27(sp)
	la		t0, print_reg_k1
	jalr	t0
	la		t0, print_endl
	jalr	t0
	ld		a0, TF_REG28(sp)
	la		t0, print_reg_gp
	jalr	t0
	ld		a0, TF_REG29(sp)
	la		t0, print_reg_sp
	jalr	t0
	ld		a0, TF_REG30(sp)
	la		t0, print_reg_fp
	jalr	t0
	ld		a0, TF_REG31(sp)
	la		t0, print_reg_ra
	jalr	t0
	la		t0, print_endl
	jalr	t0
	RESTORE_SOME
	ld		sp, TF_REG2(sp)
.endm

#endif

// .macro PRINT_REGS
// 	SAVE_ALL
// 	ld		a0, TF_REG0(sp)
// 	jal		print_reg_zero
// 	ld		a0, TF_REG1(sp)
// 	jal		print_reg_at
// 	ld		a0, TF_REG2(sp)
// 	jal		print_reg_v0
// 	ld		a0, TF_REG3(sp)
// 	jal		print_reg_v1
// 	jal		print_endl
// 	ld		a0, TF_REG4(sp)
// 	jal		print_reg_a0
// 	ld		a0, TF_REG5(sp)
// 	jal		print_reg_a1
// 	ld		a0, TF_REG6(sp)
// 	jal		print_reg_a2
// 	ld		a0, TF_REG7(sp)
// 	jal		print_reg_a3
// 	jal		print_endl
// 	ld		a0, TF_REG8(sp)
// 	jal		print_reg_t0
// 	ld		a0, TF_REG9(sp)
// 	jal		print_reg_t1
// 	ld		a0, TF_REG10(sp)
// 	jal		print_reg_t2
// 	ld		a0, TF_REG11(sp)
// 	jal		print_reg_t3
// 	jal		print_endl
// 	ld		a0, TF_REG12(sp)
// 	jal		print_reg_t4
// 	ld		a0, TF_REG13(sp)
// 	jal		print_reg_t5
// 	ld		a0, TF_REG14(sp)
// 	jal		print_reg_t6
// 	ld		a0, TF_REG15(sp)
// 	jal		print_reg_t7
// 	jal		print_endl
// 	ld		a0, TF_REG16(sp)
// 	jal		print_reg_s0
// 	ld		a0, TF_REG17(sp)
// 	jal		print_reg_s1
// 	ld		a0, TF_REG18(sp)
// 	jal		print_reg_s2
// 	ld		a0, TF_REG19(sp)
// 	jal		print_reg_s3
// 	jal		print_endl
// 	ld		a0, TF_REG20(sp)
// 	jal		print_reg_s4
// 	ld		a0, TF_REG21(sp)
// 	jal		print_reg_s5
// 	ld		a0, TF_REG22(sp)
// 	jal		print_reg_s6
// 	ld		a0, TF_REG23(sp)
// 	jal		print_reg_s7
// 	jal		print_endl
// 	ld		a0, TF_REG24(sp)
// 	jal		print_reg_t8
// 	ld		a0, TF_REG25(sp)
// 	jal		print_reg_t9
// 	ld		a0, TF_REG26(sp)
// 	jal		print_reg_k0
// 	ld		a0, TF_REG27(sp)
// 	jal		print_reg_k1
// 	jal		print_endl
// 	ld		a0, TF_REG28(sp)
// 	jal		print_reg_gp
// 	ld		a0, TF_REG29(sp)
// 	jal		print_reg_sp
// 	ld		a0, TF_REG30(sp)
// 	jal		print_reg_fp
// 	ld		a0, TF_REG31(sp)
// 	jal		print_reg_ra
// 	jal		print_endl
// 	// ld		a0, TF_REG32(sp)
// 	// jal		print_reg_status
// 	// ld		a0, 132(sp)
// 	// jal		print_reg_hi
// 	// ld		a0, 136(sp)
// 	// jal		print_reg_lo
// 	// jal		print_endl
// 	// ld		a0, 140(sp)
// 	// jal		print_reg_badvaddr
// 	// ld		a0, 144(sp)
// 	// jal		print_reg_cause
// 	// ld		a0, 148(sp)
// 	// jal		print_reg_epc
// 	// jal		print_endl
// 	RESTORE_SOME
// 	ld		sp, TF_REG2(sp)
// .endm
