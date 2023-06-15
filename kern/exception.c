#include <asm/asm.h>
// #include <drivers/dev_rtc.h>
// #include <stackframe.h>

#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <sched.h>
#include <syscall.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);

void (*exception_handlers[32])(void) = {
//     [0 ... 31] = handle_reserved,
//     [0] = handle_int,
//     [2 ... 3] = handle_tlb,
// #if !defined(LAB) || LAB >= 4
//     [1] = handle_mod,
//     [8] = handle_sys,
// #endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
// void do_reserved(struct Trapframe *tf) {
// 	print_tf(tf);
// 	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
// }


// .macro BUILD_HANDLER exception handler
// NESTED(handle_\exception, TF_SIZE + 8, zero)
// 	move    a0, sp
// 	addiu   sp, sp, -8
// 	jal     \handler
// 	addiu   sp, sp, 8
// 	j       ret_from_exception
// END(handle_\exception)
// .endm

void handle_interrupt() {
	u_long sip;
	asm volatile("csrr %0, sip " : "=r"(sip));
	if (sip & 0x0000000000000020) {
		schedule(0);
	}
	printk("sip=%016lx\n", sip);
	halt();
}

void handle_exception(u_long err) {
	struct Trapframe *tf = (struct Trapframe *)KSTACKTOP - 1;
	// print_tf(tf);

	u_long cause, epc, status, tval;

    asm volatile("csrr %0, scause " : "=r"(cause));
    asm volatile("csrr %0, sepc " : "=r"(epc));
    asm volatile("csrr %0, sstatus " : "=r"(status));
    asm volatile("csrr %0, stval " : "=r"(tval));

	// printk("Exception: cause=%ld\n", cause);
	// printk("epc=%016lx\n", epc);
	// printk("status=%016lx\n", status);
	// printk("tval=%016lx\n", tval);

	if (cause == 8) {
		do_syscall(tf);
		// print_tf(tf);
		asm volatile("add sp, %0, zero" : : "r"(tf));
		asm volatile("j ret_from_exception");
	} else if (cause == 12 || cause == 13 || cause == 15) {
		if (tval < 0x3000 || tval >= 0x200000000L) {
			panic("virtual memory out of range");
		}

		// print_tf(tf);
		if (is_mapped_page(&cur_pgdir, tval)) {
			u_long pa = get_pa(&cur_pgdir, tval);
			u_long perm = get_perm(&cur_pgdir, tval);

			// printk("%d: ", cause);
			// debug_page_va(&cur_pgdir, tval);

			if (cause == 12) {
				map_page(&cur_pgdir, curenv->env_asid, tval, pa, perm | PTE_X);
				asm volatile("add sp, %0, zero" : : "r"(tf));
				asm volatile("j ret_from_exception");
			}

			// printk("cow=%d\n", (get_perm(&cur_pgdir, tval) & PTE_COW) != 0);
			// printk("entry=%016lx of %x\n", curenv->env_user_tlb_mod_entry, curenv->env_id);

			if (get_perm(&cur_pgdir, tval) & PTE_COW) {
				
				if (!is_mapped_page(&cur_pgdir, UXSTACKTOP - sizeof(struct Trapframe) - sizeof(u_long))) {
					#ifdef DEBUG
					#if (DEBUG >= 3)
					printk("%x: alloc uxstacktop\n", curenv->env_id);
					#endif
					#endif
					alloc_page_user(&cur_pgdir, curenv->env_asid, UXSTACKTOP - sizeof(struct Trapframe) - sizeof(u_long), PTE_R | PTE_W | PTE_U);
				}
				#ifdef DEBUG
				#if (DEBUG >= 3)
				printk("%x: cow %016lx\n", curenv->env_id, tval);
				#endif
				#endif

				u_long pa = get_pa(&cur_pgdir, UXSTACKTOP - sizeof(struct Trapframe) - sizeof(u_long));
				*(struct Trapframe *)(pa + sizeof(u_long)) = *tf;
				*(u_long *)pa = (u_long)tf;
				
				tf->sepc = curenv->env_user_tlb_mod_entry;
				tf->sscratch = UXSTACKTOP - sizeof(struct Trapframe) - sizeof(u_long);
				tf->regs[10] = UXSTACKTOP - sizeof(struct Trapframe);
				
				asm volatile("add sp, %0, zero" : : "r"(tf));
				asm volatile("j ret_from_exception");
			} else if (cause == 15) {
				map_page(&cur_pgdir, curenv->env_asid, tval, pa, perm | PTE_W);
				asm volatile("add sp, %0, zero" : : "r"(tf));
				asm volatile("j ret_from_exception");
			}
		}

		alloc_page_user(&cur_pgdir, curenv->env_asid, tval, PTE_R | PTE_W | PTE_U);
		printk("cause=%d      page fault in %016lx->%016lx        env=%x at pc=%016lx\n", cause, tval, get_pa(&cur_pgdir, tval), curenv->env_id, epc);
		// printk("%016lx\n", tf);
		// debug_page_user(&cur_pgdir);
		
		asm volatile("add sp, %0, zero" : : "r"(tf));
		asm volatile("j ret_from_exception");
	} else {
		printk("Exception: cause=%ld (", cause);
		if (cause == 2) {
			printk("Illegal instruction");
		} else {
			printk("Unknown");
		}
		printk(")\n");
		printk("epc=%016lx\n", epc);
		printk("status=%016lx\n", status);
		printk("tval=%016lx\n", tval);
		printk("env=%08x\n", curenv->env_id);
		halt();
	}
    
}

void print_reg_zero(u_long reg) {
	printk("zero:%016lx    ", reg);
}

void print_reg_at(u_long reg) {
	printk("at:  %016lx    ", reg);
}

void print_reg_v0(u_long reg) {
	printk("v0:  %016lx    ", reg);
}

void print_reg_v1(u_long reg) {
	printk("v1:  %016lx    ", reg);
}

void print_reg_a0(u_long reg) {
	printk("a0:  %016lx    ", reg);
}

void print_reg_a1(u_long reg) {
	printk("a1:  %016lx    ", reg);
}

void print_reg_a2(u_long reg) {
	printk("a2:  %016lx    ", reg);
}

void print_reg_a3(u_long reg) {
	printk("a3:  %016lx    ", reg);
}

void print_reg_t0(u_long reg) {
	printk("t0:  %016lx    ", reg);
}

void print_reg_t1(u_long reg) {
	printk("t1:  %016lx    ", reg);
}

void print_reg_t2(u_long reg) {
	printk("t2:  %016lx    ", reg);
}

void print_reg_t3(u_long reg) {
	printk("t3:  %016lx    ", reg);
}

void print_reg_t4(u_long reg) {
	printk("t4:  %016lx    ", reg);
}

void print_reg_t5(u_long reg) {
	printk("t5:  %016lx    ", reg);
}

void print_reg_t6(u_long reg) {
	printk("t6:  %016lx    ", reg);
}

void print_reg_t7(u_long reg) {
	printk("t7:  %016lx    ", reg);
}

void print_reg_s0(u_long reg) {
	printk("s0:  %016lx    ", reg);
}

void print_reg_s1(u_long reg) {
	printk("s1:  %016lx    ", reg);
}

void print_reg_s2(u_long reg) {
	printk("s2:  %016lx    ", reg);
}

void print_reg_s3(u_long reg) {
	printk("s3:  %016lx    ", reg);
}

void print_reg_s4(u_long reg) {
	printk("s4:  %016lx    ", reg);
}

void print_reg_s5(u_long reg) {
	printk("s5:  %016lx    ", reg);
}

void print_reg_s6(u_long reg) {
	printk("s6:  %016lx    ", reg);
}

void print_reg_s7(u_long reg) {
	printk("s7:  %016lx    ", reg);
}

void print_reg_t8(u_long reg) {
	printk("t8:  %016lx    ", reg);
}

void print_reg_t9(u_long reg) {
	printk("t9:  %016lx    ", reg);
}

void print_reg_k0(u_long reg) {
	printk("k0:  %016lx    ", reg);
}

void print_reg_k1(u_long reg) {
	printk("k1:  %016lx    ", reg);
}

void print_reg_gp(u_long reg) {
	printk("gp:  %016lx    ", reg);
}

void print_reg_sp(u_long reg) {
	printk("sp:  %016lx    ", reg);
}

void print_reg_fp(u_long reg) {
	printk("fp:  %016lx    ", reg);
}

void print_reg_ra(u_long reg) {
	printk("ra:  %016lx    ", reg);
}

void print_reg_status(u_long reg) {
	printk("status:  %016lx    ", reg);
}

void print_reg_hi(u_long reg) {
	printk("hi:      %016lx    ", reg);
}

void print_reg_lo(u_long reg) {
	printk("lo:      %016lx    ", reg);
}

void print_reg_badvaddr(u_long reg) {
	printk("badvaddr:%016lx    ", reg);
}

void print_reg_cause(u_long reg) {
	printk("cause:   %016lx    ", reg);
}

void print_reg_epc(u_long reg) {
	printk("epc:     %016lx    ", reg);
}

void print_endl() {
	printk("\n");
}

void print_reg(u_long reg) {
	printk("%016lx\n", reg);
}

void dodo() {
    printk("dodo!\n");
}