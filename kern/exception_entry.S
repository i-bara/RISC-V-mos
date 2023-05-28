#include <asm/asm.h>
#include <stackframe.h>

.macro BUILD_HANDLER exception handler
NESTED(handle_\exception, TF_SIZE + 8, zero)
	move    a0, sp
	addi    sp, sp, -8
	jal     \handler
	addi    sp, sp, 8
	j       ret_from_exception
END(handle_\exception)
.endm

.section .text.exc_gen_entry
exc_gen_entry:
	SAVE_ALL
	
	
	csrr	t0, scause
	bltz	t0, interrupt_handler

exception_handler:
    li		t1, 100
	li		t2, SCAUSE_EXC
	and		t0, t0, t2
	blt 	t0, t1, target
	
	la		a1, yay
	jr      a1
	target:
	la		a1, handle_exception
	csrr	a0, stval
	addi	sp,	sp,	-4
	jr      a1
	

interrupt_handler:
	# li		a0, SIP_STIP
	# csrc	sip, a0


	# PRINT_REGS


	li      a0, 0
	la		t0, schedule
	jalr    t0




.text

FEXPORT(ret_from_exception)
	RESTORE_SOME
	ld      sp, TF_REG2(sp) /* Deallocate stack */
	# csrrw	a0, sscratch, a0
	# li		a0, SATP_MODE_SV39
	# csrs	satp, a0
	# csrrw	a0, sscratch, a0
	sret

# FEXPORT(handle_int)
# // NESTED(handle_int, TF_SIZE, zero)
# 	mfc0    t0, CP0_CAUSE
# 	mfc0    t2, CP0_STATUS
# 	and     t0, t2
# 	andi    t1, t0, STATUS_IM4
# 	bnez    t1, timer_irq
# 	TODO: handle other irqs
# timer_irq:
# 	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
# 	li      a0, 0
# 	j       schedule
# END(handle_int)

# BUILD_HANDLER tlb do_tlb_refill

# #if !defined(LAB) || LAB >= 4
# BUILD_HANDLER mod do_tlb_mod
# BUILD_HANDLER sys do_syscall
# #endif

# BUILD_HANDLER reserved do_reserved
