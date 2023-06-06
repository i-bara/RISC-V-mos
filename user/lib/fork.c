#include <env.h>
#include <lib.h>
#include <mmu.h>

/* Overview:
 *   Map the faulting page to a private writable copy.
 *
 * Pre-Condition:
 * 	'va' is the address which led to the TLB Mod exception.
 *
 * Post-Condition:
 *  - Launch a 'user_panic' if 'va' is not a copy-on-write page.
 *  - Otherwise, this handler should map a private writable copy of
 *    the faulting page at the same address.
 */
static void __attribute__((noreturn)) cow_entry(struct Trapframe *tf) {
	u_long sp;
	// print_tf(tf);
	// debug_page_user();
	asm volatile("move %0, sp" : "=r"(sp));
	// debugf("cow! sp=%016lx\n", sp);
	// debugf("cow! tf=%016lx\n", tf);
	// debugf("%016lx\n", *(u_long *)tf);
	// debugf("%016lx\n", ((u_long *)tf)[1]);
	// debugf("%016lx\n", ((u_long *)tf)[2]);
	// debugf("%016lx\n", ((u_long *)tf)[3]);
	// debugf("%016lx\n", ((u_long *)tf)[4]);
	// debugf("%016lx\n", ((u_long *)tf)[5]);
	// debugf("%016lx\n", ((u_long *)tf)[6]);
	u_long va = tf->stval;
	u_int perm;

	/* Step 1: Find the 'perm' in which the faulting address 'va' is mapped. */
	/* Hint: Use 'vpt' and 'VPN' to find the page table entry. If the 'perm' doesn't have
	 * 'PTE_COW', launch a 'user_panic'. */
	/* Exercise 4.13: Your code here. (1/6) */
	perm = pt0[va >> VPN0_SHIFT] & PTE_PERM;
	if (!(perm & PTE_COW)) {
		user_panic("wwwwwwwwwww");
	}

	/* Step 2: Remove 'PTE_COW' from the 'perm', and add 'PTE_D' to it. */
	/* Exercise 4.13: Your code here. (2/6) */
	perm &= ~PTE_COW;
	perm |= PTE_R | PTE_W | PTE_U;

	/* Step 3: Allocate a new page at 'UCOW'. */
	/* Exercise 4.13: Your code here. (3/6) */
	int r;
	if ((r = syscall_mem_alloc(0, UCOW, perm)) < 0) {
		user_panic("can not alloc: %d", r);
	}

	/* Step 4: Copy the content of the faulting page at 'va' to 'UCOW'. */
	/* Hint: 'va' may not be aligned to a page! */
	/* Exercise 4.13: Your code here. (4/6) */
	memcpy(UCOW, ROUNDDOWN(va, PAGE_SIZE), PAGE_SIZE);

	// Step 5: Map the page at 'UCOW' to 'va' with the new 'perm'.
	/* Exercise 4.13: Your code here. (5/6) */
	if ((r = syscall_mem_map(0, UCOW, 0, va, perm)) < 0) {
		user_panic("can not map: %d", r);
	}

	// Step 6: Unmap the page at 'UCOW'.
	/* Exercise 4.13: Your code here. (6/6) */
	syscall_mem_unmap(0, UCOW);

	// Step 7: Return to the faulting routine.
	r = syscall_set_trapframe(0, tf);
	user_panic("syscall_set_trapframe returned %d", r);
}

/* Overview:
 *   Grant our child 'envid' access to the virtual page 'vpn' (with address 'vpn' * 'PAGE_SIZE') in our
 *   (current env's) address space.
 *   'PTE_COW' should be used to isolate the modifications on unshared memory from a parent and its
 *   children.
 *
 * Post-Condition:
 *   If the virtual page 'vpn' has 'PTE_D' and doesn't has 'PTE_LIBRARY', both our original virtual
 *   page and 'envid''s newly-mapped virtual page should be marked 'PTE_COW' and without 'PTE_D',
 *   while the other permission bits are kept.
 *
 *   If not, the newly-mapped virtual page in 'envid' should have the exact same permission as our
 *   original virtual page.
 *
 * Hint:
 *   - 'PTE_LIBRARY' indicates that the page should be shared among a parent and its children.
 *   - A page with 'PTE_LIBRARY' may have 'PTE_D' at the same time, you should handle it correctly.
 *   - You can pass '0' as an 'envid' in arguments of 'syscall_*' to indicate current env because
 *     kernel 'envid2env' converts '0' to 'curenv').
 *   - You should use 'syscall_mem_map', the user space wrapper around 'msyscall' to invoke
 *     'sys_mem_map' in kernel.
 */
static void duppage(u_int envid, u_int vpn) {
	int r;
	u_int addr;
	u_int perm;

	/* Step 1: Get the permission of the page. */
	/* Hint: Use 'vpt' to find the page table entry. */
	/* Exercise 4.10: Your code here. (1/2) */
	addr = pt0[vpn] & PTE_PPN;
	perm = pt0[vpn] & PTE_PERM;

	/* Step 2: If the page is writable, and not shared with children, and not marked as COW yet,
	 * then map it as copy-on-write, both in the parent (0) and the child (envid). */
	/* Hint: The page should be first mapped to the child before remapped in the parent. (Why?)
	 */
	/* Exercise 4.10: Your code here. (2/2) */

	// debugf("duppage %016lx %x->%x", vpn << VPN0_SHIFT, envid, env->env_id);
	// debugf("%b\n", (perm | PTE_COW) & ~PTE_W);
	if (!(perm & PTE_U)) {
		syscall_mem_map(0, vpn << VPN0_SHIFT, envid, vpn << VPN0_SHIFT, perm);
	} else if ((perm & PTE_W) && !(perm & PTE_LIBRARY)) {
		// debugf("yeah!\n");
		syscall_mem_map(0, vpn << VPN0_SHIFT, envid, vpn << VPN0_SHIFT, (perm | PTE_COW) & ~PTE_W);
		syscall_mem_map(0, vpn << VPN0_SHIFT, 0, vpn << VPN0_SHIFT, (perm | PTE_COW) & ~PTE_W);
	} else {
		syscall_mem_map(0, vpn << VPN0_SHIFT, envid, vpn << VPN0_SHIFT, perm);
	}
	// debugf("end\n");

}

/* Overview:
 *   User-level 'fork'. Create a child and then copy our address space.
 *   Set up ours and its TLB Mod user exception entry to 'cow_entry'.
 *
 * Post-Conditon:
 *   Child's 'env' is properly set.
 *
 * Hint:
 *   Use global symbols 'env', 'vpt' and 'vpd'.
 *   Use 'syscall_set_tlb_mod_entry', 'syscall_getenvid', 'syscall_exofork',  and 'duppage'.
 */
int fork(void) {
	u_int child;
	u_int i;
	extern volatile struct Env *env;

	/* Step 1: Set our TLB Mod user exception entry to 'cow_entry' if not done yet. */
	if (env->env_user_tlb_mod_entry != (u_int)cow_entry) {
		try(syscall_set_tlb_mod_entry(0, cow_entry));
	}

	/* Step 2: Create a child env that's not ready to be scheduled. */
	// Hint: 'env' should always point to the current env itself, so we should fix it to the
	// correct value.
	
	child = syscall_exofork();
	
	if (child == 0) {
		env = envs + ENVX(syscall_getenvid());
		debugf("fork! %016lx\n", env->env_id);
		return 0;
	}

	/* Step 3: Map all mapped pages below 'USTACKTOP' into the child's address space. */
	// Hint: You should use 'duppage'.
	/* Exercise 4.15: Your code here. (1/2) */
	for (int va = 0; va < USTACKTOP; va += PAGE_SIZE) { // 用户异常栈不 duppage，否则无法正确进入 cow_entry：
		if (pt2[va >> VPN2_SHIFT] & PTE_V) {			// 一旦进入必须写时复制，但是必须进入才能写时复制，矛盾
			if (pt1[va >> VPN1_SHIFT] & PTE_V) {
				if (pt0[va >> VPN0_SHIFT] & PTE_V) {	// 内核在 exofork 内 duppage，这样能够实现快速的映射
					duppage(child, va >> VPN0_SHIFT);	// 页表和 env，pages 都是在 exofork 内映射
				}
			}
		}
	}

	/* Step 4: Set up the child's tlb mod handler and set child's 'env_status' to
	 * 'ENV_RUNNABLE'. */
	/* Hint:
	 *   You may use 'syscall_set_tlb_mod_entry' and 'syscall_set_env_status'
	 *   Child's TLB Mod user exception entry should handle COW, so set it to 'cow_entry'
	 */
	/* Exercise 4.15: Your code here. (2/2) */
	if (child) {
		syscall_set_tlb_mod_entry(child, cow_entry);
		syscall_set_env_status(child, ENV_RUNNABLE);
	}

	return child;
}

static void _debug_page(u_long va, u_long pte) {
	debugf("|%016lx->%016lx  ", va, PTE2PA(pte));
	if (pte & PTE_R) {
		debugf("R");
	} else {
		debugf(" ");
	}
	if (pte & PTE_W) {
		debugf("W");
	} else {
		debugf(" ");
	}
	if (pte & PTE_X) {
		debugf("X");
	} else {
		debugf(" ");
	}
	if (pte & PTE_U) {
		debugf("U");
	} else {
		debugf(" ");
	}
	if (pte & PTE_G) {
		debugf("G");
	} else {
		debugf(" ");
	}
	if (pte & PTE_A) {
		debugf("A");
	} else {
		debugf(" ");
	}
	if (pte & PTE_D) {
		debugf("D");
	} else {
		debugf(" ");
	}
	if (pte & PTE_COW) {
		debugf("C");
	} else {
		debugf(" ");
	}
	if (pte & PTE_LIBRARY) {
		debugf("L");
	} else {
		debugf(" ");
	}
	debugf("|\n");
}

void debug_page_user() {
	debugf("---------------------page----------------------\n");
	for (int va = 0; va < USTACKTOP; va += PAGE_SIZE) {
		if (pt2[va >> VPN2_SHIFT] & PTE_V) {
			if (pt1[va >> VPN1_SHIFT] & PTE_V) {
				if (pt0[va >> VPN0_SHIFT] & PTE_V) {
					_debug_page(va, pt0[va >> VPN0_SHIFT]);
				}
			}
		}
	}
	// for (int va = PAGE_TABLE; va < 0x100000000L; va += PAGE_SIZE) {
	// 	if (pt2[va >> VPN2_SHIFT] & PTE_V) {
	// 		if (pt1[va >> VPN1_SHIFT] & PTE_V) {
	// 			if (pt0[va >> VPN0_SHIFT] & PTE_V) {
	// 				_debug_page(va, pt0[va >> VPN0_SHIFT]);
	// 			}
	// 		}
	// 	}
	// }
	debugf("-----------------------------------------------\n");
}

void print_tf(struct Trapframe *tf) {
	debugf("------trapframe at %016lx------\n"
		   "zero:%016lx  at:  %016lx  v0:  %016lx  v1:  %016lx\n"
	       "a0:  %016lx  a1:  %016lx  a2:  %016lx  a3:  %016lx\n"
	       "t0:  %016lx  t1:  %016lx  t2   %016lx  t3:  %016lx\n"
		   "t4:  %016lx  t5:  %016lx  t6   %016lx  t7:  %016lx\n"
	       "s0:  %016lx  s1:  %016lx  s2:  %016lx  s3:  %016lx\n"
		   "s4:  %016lx  s5:  %016lx  s6:  %016lx  s7:  %016lx\n"
		   "s8:  %016lx  s9:  %016lx  k0:  %016lx  k1:  %016lx\n"
		   "gp:  %016lx  sp:  %016lx  fp:  %016lx  ra:  %016lx\n", 
		   tf, 
		   tf->regs[0], tf->regs[1], tf->regs[2], tf->regs[3], 
		   tf->regs[4], tf->regs[5], tf->regs[6], tf->regs[7], 
		   tf->regs[8], tf->regs[9], tf->regs[10], tf->regs[14], 
		   tf->regs[12], tf->regs[13], tf->regs[14], tf->regs[15], 
		   tf->regs[16], tf->regs[17], tf->regs[18], tf->regs[19], 
		   tf->regs[20], tf->regs[21], tf->regs[22], tf->regs[23], 
		   tf->regs[24], tf->regs[25], tf->regs[26], tf->regs[27], 
		   tf->regs[28], tf->regs[29], tf->regs[30], tf->regs[31]);
	debugf("sie: %016lx    sepc:  %016lx    scause: %016lx    sstatus:  %016lx\n", tf->sie, tf->sepc, tf->scause, tf->sstatus);
	debugf("sip: %016lx    stvec: %016lx    stval:  %016lx    sscratch: %016lx\n", tf->sip, tf->stvec, tf->stval, tf->sscratch);
}

void debug_env() {
	debugf("---------------------------------------env----------------------------------------\n");
	debugf("| id        status       parent    asid      pgdir             priority  index   |\n");
	for (int i = 0; i < NENV; i++) {
		struct Env *e = &envs[i];
		if (e->env_id) {
			if (e == env) {
				debugf("|*");
			} else {
				debugf("| ");
			}
			debugf("%08x  ", e->env_id, e->env_pri);
			if (e->env_status == ENV_FREE) {
				debugf("free         ");
			} else if (e->env_status == ENV_RUNNABLE) {
				debugf("runnable     ");
			} else if (e->env_status == ENV_NOT_RUNNABLE) {
				debugf("not runnable ");
			}
			if (e->env_parent_id) {
				debugf("%08x  ", e->env_parent_id);
			} else {
				debugf("          ");
			}
			debugf("%08x  ", e->env_asid);
			if (e->env_pgdir) {
				debugf("%016lx  ", e->env_pgdir);
			} else {
				debugf("          ");
			}
			debugf("%-8x  ", e->env_pri);
			debugf("%-8x|\n", e - envs);
		}
	}
	debugf("----------------------------------------------------------------------------------\n");
}
