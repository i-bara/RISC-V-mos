// #include <env.h>
// #include <pmap.h>

// static void passive_alloc(u_int va, Pde *pgdir, u_int asid) {
// 	struct Page *p = NULL;

// 	if (va < UTEMP) {
// 		panic("address too low");
// 	}

// 	if (va >= USTACKTOP && va < USTACKTOP + BY2PG) {
// 		panic("invalid memory");
// 	}

// 	if (va >= UENVS && va < UPAGES) {
// 		panic("envs zone");
// 	}

// 	if (va >= UPAGES && va < UVPT) {
// 		panic("pages zone");
// 	}

// 	if (va >= ULIM) {
// 		panic("kernel address");
// 	}

// 	panic_on(page_alloc(&p));
// 	panic_on(page_insert(pgdir, asid, p, PTE_ADDR(va), PTE_D));
// }

// /* Overview:
//  *  Refill TLB.
//  */
// Pte _do_tlb_refill(u_long va, u_int asid) {
// 	Pte *pte;
// 	/* Hints:
// 	 *  Invoke 'page_lookup' repeatedly in a loop to find the page table entry 'pte' associated
// 	 *  with the virtual address 'va' in the current address space 'cur_pgdir'.
// 	 *
// 	 *  **While** 'page_lookup' returns 'NULL', indicating that the 'pte' could not be found,
// 	 *  allocate a new page using 'passive_alloc' until 'page_lookup' succeeds.
// 	 */

// 	/* Exercise 2.9: Your code here. */
// 	// printk("[tlb] refilling %08x asid=%d\n", va, asid);
// 	while (1) {
// 		struct Page *pp = page_lookup(cur_pgdir, va, &pte);
// 		if (pp) {
// 			break;
// 		}
// 		passive_alloc(va, cur_pgdir, asid);
// 		// printk("[tlb] creating %08x->%08x\n", va, get_pa(cur_pgdir, va));
// 	}

// 	return *pte;
// }

// #if !defined(LAB) || LAB >= 4
// /* Overview:
//  *   This is the TLB Mod exception handler in kernel.
//  *   Our kernel allows user programs to handle TLB Mod exception in user mode, so we copy its
//  *   context 'tf' into UXSTACK and modify the EPC to the registered user exception entry.
//  *
//  * Hints:
//  *   'env_user_tlb_mod_entry' is the user space entry registered using
//  *   'sys_set_user_tlb_mod_entry'.
//  *
//  *   The user entry should handle this TLB Mod exception and restore the context.
//  */
// void do_tlb_mod(struct Trapframe *tf) {
// 	struct Trapframe tmp_tf = *tf;

// 	if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
// 		tf->regs[29] = UXSTACKTOP;
// 	}
// 	tf->regs[29] -= sizeof(struct Trapframe);
// 	*(struct Trapframe *)tf->regs[29] = tmp_tf;

// 	if (curenv->env_user_tlb_mod_entry) {
// 		tf->regs[4] = tf->regs[29];
// 		tf->regs[29] -= sizeof(tf->regs[4]);
// 		// Hint: Set 'cp0_epc' in the context 'tf' to 'curenv->env_user_tlb_mod_entry'.
// 		/* Exercise 4.11: Your code here. */
// 		tf->cp0_epc = curenv->env_user_tlb_mod_entry;

// 	} else {
// 		panic("TLB Mod but no user handler registered");
// 	}
// }
// #endif

// void print_tlb(u_long index, u_long hi, u_long lo) {
// 	printk("index:  %08x  hi:     %08x  lo:     %08x\n", index, hi, lo);
// }

// void debug_tlb() {
// 	for (u_long i = 0; i < 64; i++) {
// 		_debug_tlb(0);
// 	}
// }

// void print_reg_inline(u_long reg) {
// 	printk("%08x ", reg);
// }

// void print_reg_zero(u_long reg) {
// 	printk("zero:%08x    ", reg);
// }

// void print_reg_at(u_long reg) {
// 	printk("at:  %08x    ", reg);
// }

// void print_reg_v0(u_long reg) {
// 	printk("v0:  %08x    ", reg);
// }

// void print_reg_v1(u_long reg) {
// 	printk("v1:  %08x    ", reg);
// }

// void print_reg_a0(u_long reg) {
// 	printk("a0:  %08x    ", reg);
// }

// void print_reg_a1(u_long reg) {
// 	printk("a1:  %08x    ", reg);
// }

// void print_reg_a2(u_long reg) {
// 	printk("a2:  %08x    ", reg);
// }

// void print_reg_a3(u_long reg) {
// 	printk("a3:  %08x    ", reg);
// }

// void print_reg_t0(u_long reg) {
// 	printk("t0:  %08x    ", reg);
// }

// void print_reg_t1(u_long reg) {
// 	printk("t1:  %08x    ", reg);
// }

// void print_reg_t2(u_long reg) {
// 	printk("t2:  %08x    ", reg);
// }

// void print_reg_t3(u_long reg) {
// 	printk("t3:  %08x    ", reg);
// }

// void print_reg_t4(u_long reg) {
// 	printk("t4:  %08x    ", reg);
// }

// void print_reg_t5(u_long reg) {
// 	printk("t5:  %08x    ", reg);
// }

// void print_reg_t6(u_long reg) {
// 	printk("t6:  %08x    ", reg);
// }

// void print_reg_t7(u_long reg) {
// 	printk("t7:  %08x    ", reg);
// }

// void print_reg_s0(u_long reg) {
// 	printk("s0:  %08x    ", reg);
// }

// void print_reg_s1(u_long reg) {
// 	printk("s1:  %08x    ", reg);
// }

// void print_reg_s2(u_long reg) {
// 	printk("s2:  %08x    ", reg);
// }

// void print_reg_s3(u_long reg) {
// 	printk("s3:  %08x    ", reg);
// }

// void print_reg_s4(u_long reg) {
// 	printk("s4:  %08x    ", reg);
// }

// void print_reg_s5(u_long reg) {
// 	printk("s5:  %08x    ", reg);
// }

// void print_reg_s6(u_long reg) {
// 	printk("s6:  %08x    ", reg);
// }

// void print_reg_s7(u_long reg) {
// 	printk("s7:  %08x    ", reg);
// }

// void print_reg_t8(u_long reg) {
// 	printk("t8:  %08x    ", reg);
// }

// void print_reg_t9(u_long reg) {
// 	printk("t9:  %08x    ", reg);
// }

// void print_reg_k0(u_long reg) {
// 	printk("k0:  %08x    ", reg);
// }

// void print_reg_k1(u_long reg) {
// 	printk("k1:  %08x    ", reg);
// }

// void print_reg_gp(u_long reg) {
// 	printk("gp:  %08x    ", reg);
// }

// void print_reg_sp(u_long reg) {
// 	printk("sp:  %08x    ", reg);
// }

// void print_reg_fp(u_long reg) {
// 	printk("fp:  %08x    ", reg);
// }

// void print_reg_ra(u_long reg) {
// 	printk("ra:  %08x    ", reg);
// }

// void print_reg_status(u_long reg) {
// 	printk("status:  %08x    ", reg);
// }

// void print_reg_hi(u_long reg) {
// 	printk("hi:      %08x    ", reg);
// }

// void print_reg_lo(u_long reg) {
// 	printk("lo:      %08x    ", reg);
// }

// void print_reg_badvaddr(u_long reg) {
// 	printk("badvaddr:%08x    ", reg);
// }

// void print_reg_cause(u_long reg) {
// 	printk("cause:   %08x    ", reg);
// }

// void print_reg_epc(u_long reg) {
// 	printk("epc:     %08x    ", reg);
// }

// void print_endl() {
// 	printk("\n");
// }

// void print_reg(u_long reg) {
// 	printk("%08x\n", reg);
// }

// void print_stack(u_long sp) {
// 	printk("at %08x: \n", sp);
// 	printk("reg0: %08x\n", *(u_long *)(sp + TF_REG0));
// 	printk("reg1: %08x\n", *(u_long *)(sp + TF_REG1));
// 	printk("reg2: %08x\n", *(u_long *)(sp + TF_REG2));
// 	printk("reg3: %08x\n", *(u_long *)(sp + TF_REG3));
// 	printk("reg4: %08x\n", *(u_long *)(sp + TF_REG4));
// }