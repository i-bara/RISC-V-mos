#include <drivers/dev_cons.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <sched.h>
#include <syscall.h>

extern struct Env *curenv;

/* Overview:
 * 	This function is used to print a character on screen.
 *
 * Pre-Condition:
 * 	`c` is the character you want to print.
 */
void sys_putchar(int c) {
	printcharc((char)c);
	return;
}

/* Overview:
 * 	This function is used to print a string of bytes on screen.
 *
 * Pre-Condition:
 * 	`s` is base address of the string, and `num` is length of the string.
 */
int sys_print_cons(const void *s, u_long num) {
	u_long pa = get_pa(&cur_pgdir, (u_long)s);

	if (/* ((u_long)pa + num) > UTOP || ((u_long)pa) >= UTOP || */ (pa > pa + num)) {
		return -E_INVAL;
	}

	if (ROUNDDOWN((u_long)s, PAGE_SIZE) == ROUNDDOWN((u_long)s + num, PAGE_SIZE)) {
		u_long i;
		for (i = 0; i < num; i++) {
			printcharc(((char *)pa)[i]);
		}
		return 0;
	}
	
	u_long remain = (u_long)s % PAGE_SIZE;
	u_long i;
	for (i = 0; i < remain; i++) {
		printcharc(((char *)pa)[i]);
	}
	num -= remain;
	s += remain;
	
	while (num >= PAGE_SIZE) {
		pa = get_pa(&cur_pgdir, (u_long)s);
		for (i = 0; i < PAGE_SIZE; i++) {
			printcharc(((char *)pa)[i]);
		}
		num -= PAGE_SIZE;
		s += PAGE_SIZE;
	}

	if (num) {
		pa = get_pa(&cur_pgdir, (u_long)s);
		for (i = 0; i < num; i++) {
			printcharc(((char *)pa)[i]);
		}
	}

	return 0;
}

/* Overview:
 *	This function provides the environment id of current process.
 *
 * Post-Condition:
 * 	return the current environment id
 */
u_long sys_getenvid(void) {
	return curenv->env_id;
}

/* Overview:
 *   Give up remaining CPU time slice for 'curenv'.
 *
 * Post-Condition:
 *   Another env is scheduled.
 *
 * Hint:
 *   This function will never return.
 */
// void sys_yield(void);
void __attribute__((noreturn)) sys_yield(void) {
	// Hint: Just use 'schedule' with 'yield' set.
	/* Exercise 4.7: Your code here. */
	schedule(1);

}

/* Overview:
 * 	This function is used to destroy the current environment.
 *
 * Pre-Condition:
 * 	The parameter `envid` must be the environment id of a
 * process, which is either a child of the caller of this function
 * or the caller itself.
 *
 * Post-Condition:
 *  Returns 0 on success.
 *  Returns the original error if underlying calls fail.
 */
int sys_env_destroy(u_long envid) {
	struct Env *e;
	try(envid2env(envid, &e, 1));

	#ifdef DEBUG
	#if (DEBUG >= 1)
	printk("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	#endif
	#endif
	env_destroy(e);
	return 0;
}

/* Overview:
 *   Register the entry of user space TLB Mod handler of 'envid'.
 *
 * Post-Condition:
 *   The 'envid''s TLB Mod exception handler entry will be set to 'func'.
 *   Returns 0 on success.
 *   Returns the original error if underlying calls fail.
 */
int sys_set_tlb_mod_entry(u_long envid, u_long func) {
	struct Env *env;

	/* Step 1: Convert the envid to its corresponding 'struct Env *' using 'envid2env'. */
	/* Exercise 4.12: Your code here. (1/2) */
	try(envid2env(envid, &env, curenv->env_id));

	/* Step 2: Set its 'env_user_tlb_mod_entry' to 'func'. */
	/* Exercise 4.12: Your code here. (2/2) */
	env->env_user_tlb_mod_entry = func;

	return 0;
}

/* Overview:
 *   Check 'va' is illegal or not, according to include/mmu.h
 */
static inline int is_illegal_va(u_long va) {
	return va < UTEMP || va >= UTOP;
}

static inline int is_illegal_va_range(u_long va, u_long len) {
	if (len == 0) {
		return 0;
	}
	return va + len < va || va < UTEMP || va + len > UTOP;
}

/* Overview:
 *   Allocate a physical page and map 'va' to it with 'perm' in the address space of 'envid'.
 *   If 'va' is already mapped, that original page is sliently unmapped.
 *   'envid2env' should be used with 'checkperm' set, like in most syscalls, to ensure the target is
 * either the caller or its child.
 *
 * Post-Condition:
 *   Return 0 on success.
 *   Return -E_BAD_ENV: 'checkperm' of 'envid2env' fails for 'envid'.
 *   Return -E_INVAL:   'va' is illegal (should be checked using 'is_illegal_va').
 *   Return the original error: underlying calls fail (you can use 'try' macro).
 *
 * Hint:
 *   You may want to use the following functions:
 *   'envid2env', 'page_alloc', 'page_insert', 'try' (macro)
 */
int sys_mem_alloc(u_long envid, u_long va, u_long perm) {

	struct Env *env;
	struct Page *pp;

	/* Step 1: Check if 'va' is a legal user virtual address using 'is_illegal_va'. */
	/* Exercise 4.4: Your code here. (1/3) */
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}

	/* Step 2: Convert the envid to its corresponding 'struct Env *' using 'envid2env'. */
	/* Hint: **Always** validate the permission in syscalls! */
	/* Exercise 4.4: Your code here. (2/3) */
	try(envid2env(envid, &env, curenv->env_id));

	// /* Step 3: Allocate a physical page using 'page_alloc'. */
	// /* Exercise 4.4: Your code here. (3/3) */
	// try(page_alloc(&pp));

	// /* Step 4: Map the allocated page at 'va' with permission 'perm' using 'page_insert'. */
	// return page_insert(env->env_pgdir, env->env_asid, pp, va, perm);
	return alloc_page_user(&env->env_pgdir, env->env_asid, va, perm);

}

/* Overview:
 *   Find the physical page mapped at 'srcva' in the address space of env 'srcid', and map 'dstid''s
 *   'dstva' to it with 'perm'.
 *
 * Post-Condition:
 *   Return 0 on success.
 *   Return -E_BAD_ENV: 'checkperm' of 'envid2env' fails for 'srcid' or 'dstid'.
 *   Return -E_INVAL: 'srcva' or 'dstva' is illegal, or 'srcva' is unmapped in 'srcid'.
 *   Return the original error: underlying calls fail.
 *
 * Hint:
 *   You may want to use the following functions:
 *   'envid2env', 'page_lookup', 'page_insert'
 */
int sys_mem_map(u_long srcid, u_long srcva, u_long dstid, u_long dstva, u_long perm) {
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *pp;

	/* Step 1: Check if 'srcva' and 'dstva' are legal user virtual addresses using
	 * 'is_illegal_va'. */
	/* Exercise 4.5: Your code here. (1/4) */
	if (is_illegal_va(srcva) || is_illegal_va(dstva)) {
		return -E_INVAL;
	}

	/* Step 2: Convert the 'srcid' to its corresponding 'struct Env *' using 'envid2env'. */
	/* Exercise 4.5: Your code here. (2/4) */
	try(envid2env(srcid, &srcenv, curenv->env_id));

	/* Step 3: Convert the 'dstid' to its corresponding 'struct Env *' using 'envid2env'. */
	/* Exercise 4.5: Your code here. (3/4) */
	try(envid2env(dstid, &dstenv, curenv->env_id));

	/* Step 4: Find the physical page mapped at 'srcva' in the address space of 'srcid'. */
	/* Return -E_INVAL if 'srcva' is not mapped. */
	/* Exercise 4.5: Your code here. (4/4) */
	// if ((pp = page_lookup(srcenv->env_pgdir, srcva, 0)) == NULL) {
	// 	return -E_INVAL;
	// }

	// debug_page_user(&srcenv->env_pgdir);
	if (is_mapped_page(&srcenv->env_pgdir, srcva) == 0) {
		return -E_INVAL;
	}

	u_long pa = get_pa(&srcenv->env_pgdir, srcva);
	// static int iii = 0;
	// if (iii == 1) {
	// 	printk("%016lx\n", dstenv->env_pgdir);
	// 	debug_page_user(&dstenv->env_pgdir);
	// }
	// iii = 1;

	/* Step 5: Map the physical page at 'dstva' in the address space of 'dstid'. */
	// return page_insert(dstenv->env_pgdir, dstenv->env_asid, pp, dstva, perm);
	return map_page_user(&dstenv->env_pgdir, dstenv->env_asid, dstva, pa, perm);
}

/* Overview:
 *   Unmap the physical page mapped at 'va' in the address space of 'envid'.
 *   If no physical page is mapped there, this function silently succeeds.
 *
 * Post-Condition:
 *   Return 0 on success.
 *   Return -E_BAD_ENV: 'checkperm' of 'envid2env' fails for 'envid'.
 *   Return -E_INVAL:   'va' is illegal.
 *   Return the original error when underlying calls fail.
 */
int sys_mem_unmap(u_long envid, u_long va) {
	struct Env *e;

	/* Step 1: Check if 'va' is a legal user virtual address using 'is_illegal_va'. */
	/* Exercise 4.6: Your code here. (1/2) */
	if (is_illegal_va(va)) {
		return -E_INVAL;
	}

	/* Step 2: Convert the envid to its corresponding 'struct Env *' using 'envid2env'. */
	/* Exercise 4.6: Your code here. (2/2) */
	try(envid2env(envid, &e, curenv->env_id));

	/* Step 3: Unmap the physical page at 'va' in the address space of 'envid'. */
	// page_remove(e->env_pgdir, e->env_asid, va);
	// return 0;

	return unmap_page(&e->env_pgdir, e->env_asid, va);
}

/* Overview:
 *   Allocate a new env as a child of 'curenv'.
 *
 * Post-Condition:
 *   Returns the child's envid on success, and
 *   - The new env's 'env_tf' is copied from the kernel stack, except for $v0 set to 0 to indicate
 *     the return value in child.
 *   - The new env's 'env_status' is set to 'ENV_NOT_RUNNABLE'.
 *   - The new env's 'env_pri' is copied from 'curenv'.
 *   Returns the original error if underlying calls fail.
 *
 * Hint:
 *   This syscall works as an essential step in user-space 'fork' and 'spawn'.
 */
int sys_exofork(void) {
	struct Env *e;

	/* Step 1: Allocate a new env using 'env_alloc'. */
	/* Exercise 4.9: Your code here. (1/4) */
	try(env_alloc(&e, curenv->env_id));

	/* Step 2: Copy the current Trapframe below 'KSTACKTOP' to the new env's 'env_tf'. */
	/* Exercise 4.9: Your code here. (2/4) */
	e->env_tf = *((struct Trapframe *)KSTACKTOP - 1);

	/* Step 3: Set the new env's 'env_tf.regs[10]' to 0 to indicate the return value in child. */
	/* Exercise 4.9: Your code here. (3/4) */
	e->env_tf.regs[10] = 0;

	/* Step 4: Set up the new env's 'env_status' and 'env_pri'.  */
	/* Exercise 4.9: Your code here. (4/4) */
	extern u_long base_pgdir;
	if (!e->env_pgdir) {
		struct Page *pp;
		try(page_alloc(&pp));
		e->env_pgdir = page2pa(pp);
	}
	((u_long *)e->env_pgdir)[2] = ((u_long *)base_pgdir)[2]; // 快速的映射！
	((u_long *)e->env_pgdir)[PENVS] = ((u_long *)base_pgdir)[PENVS] | PTE_V;

	// int debug_i = 0; // 测试页表复制 (1/3)
	for (u_long va = PAGE_TABLE; va < 0x100000000L; va += PAGE_SIZE) {
		if (is_mapped_page(&cur_pgdir, va)) {
			// printk("%016lx ", va); // 测试页表复制 (2/3)
			// if ((++debug_i) % 4 == 0) {
			// 	printk("\n");
			// }
			alloc_page(&e->env_pgdir, e->env_asid, va, PTE_R | PTE_W | PTE_U); // 不 user，直接复制
			u_long pa = get_pa(&e->env_pgdir, va);
			u_long curpa = get_pa(&cur_pgdir, va);
			memcpy((void *)pa, (void *)curpa, PAGE_SIZE);
		}
	}
	// printk("\n"); // 测试页表复制 (3/3)
	#ifdef DEBUG
	#if (DEBUG >= 2)
	printk("%x: exofork %lx with epc=%016lx\n", curenv->env_id, e->env_id, e->env_tf.sepc);
	#endif
	#endif

	e->env_status = ENV_NOT_RUNNABLE;
	e->env_pri = curenv->env_pri;

	return e->env_id;
}

/* Overview:
 *   Set 'envid''s 'env_status' to 'status' and update 'env_sched_list'.
 *
 * Post-Condition:
 *   Returns 0 on success.
 *   Returns -E_INVAL if 'status' is neither 'ENV_RUNNABLE' nor 'ENV_NOT_RUNNABLE'.
 *   Returns the original error if underlying calls fail.
 *
 * Hint:
 *   The invariant that 'env_sched_list' contains and only contains all runnable envs should be
 *   maintained.
 */
int sys_set_env_status(u_long envid, u_long status) {
	struct Env *env;

	/* Step 1: Check if 'status' is valid. */
	/* Exercise 4.14: Your code here. (1/3) */
	if (!(status == ENV_RUNNABLE || status == ENV_NOT_RUNNABLE)) {
		return -E_INVAL;
	}

	/* Step 2: Convert the envid to its corresponding 'struct Env *' using 'envid2env'. */
	/* Exercise 4.14: Your code here. (2/3) */
	try(envid2env(envid, &env, curenv->env_id));

	/* Step 3: Update 'env_sched_list' if the 'env_status' of 'env' is being changed. */
	/* Exercise 4.14: Your code here. (3/3) */
	if (env->env_status == ENV_RUNNABLE && status != ENV_RUNNABLE) {
		TAILQ_REMOVE(&env_sched_list, env, env_sched_link);
	} else if (env->env_status != ENV_RUNNABLE && status == ENV_RUNNABLE) {
		TAILQ_INSERT_TAIL(&env_sched_list, env, env_sched_link);
	}

	/* Step 4: Set the 'env_status' of 'env'. */
	env->env_status = status;
	return 0;
}

/* Overview:
 *  Set envid's trap frame to 'tf'.
 *
 * Post-Condition:
 *  The target env's context is set to 'tf'.
 *  Returns 0 on success (except when the 'envid' is the current env, so no value could be
 * returned).
 *  Returns -E_INVAL if the environment cannot be manipulated or 'tf' is invalid.
 *  Returns the original error if other underlying calls fail.
 */
int sys_set_trapframe(u_long envid, struct Trapframe *tf) {
	// if (envid == 0) { // 测试 set_trapframe：当且仅当 cow 或者 spawn 时会调用这个，所以删掉了
	// 	printk("%x: set trapframe\n", curenv->env_id);
	// } else {
	// 	printk("%x: set trapframe\n", envid);
	// }

	if (is_illegal_va_range((u_long)tf, sizeof *tf)) {
		return -E_INVAL;
	}

	u_long pa = get_pa(&cur_pgdir, (u_long)tf); // 需要翻译
	tf = (struct Trapframe *)pa;
	
	struct Env *env;
	try(envid2env(envid, &env, curenv->env_id)); // lab 6: 给父进程赋予权限

	if (env == curenv) {
		*((struct Trapframe *)KSTACKTOP - 1) = *tf;
		// return `tf->regs[10]` instead of 0, because return value overrides regs[10] on
		// current trapframe.
		return tf->regs[10];
	} else {
		env->env_tf = *tf;
		return 0;
	}
}

/* Overview:
 * 	Kernel panic with message `msg`.
 *
 * Post-Condition:
 * 	This function will halt the system.
 */
void sys_panic(char *msg) {
	u_long pa = get_pa(&cur_pgdir, (u_long)msg);
	panic("%s", TRUP(pa)); // lab 4_6 找到的漏洞：在 RISC-V 中无法访问用户页面
}

/* Overview:
 *   Wait for a message (a value, together with a page if 'dstva' is not 0) from other envs.
 *   'curenv' is blocked until a message is sent.
 *
 * Post-Condition:
 *   Return 0 on success.
 *   Return -E_INVAL: 'dstva' is neither 0 nor a legal address.
 */
int sys_ipc_recv(u_long dstva) {
	/* Step 1: Check if 'dstva' is either zero or a legal address. */
	if (dstva != 0 && is_illegal_va(dstva)) {
		return -E_INVAL;
	}

	/* Step 2: Set 'curenv->env_ipc_recving' to 1. */
	/* Exercise 4.8: Your code here. (1/8) */
	curenv->env_ipc_recving = 1;

	/* Step 3: Set the value of 'curenv->env_ipc_dstva'. */
	/* Exercise 4.8: Your code here. (2/8) */
	curenv->env_ipc_dstva = dstva;

	/* Step 4: Set the status of 'curenv' to 'ENV_NOT_RUNNABLE' and remove it from
	 * 'env_sched_list'. */
	/* Exercise 4.8: Your code here. (3/8) */
	curenv->env_status = ENV_NOT_RUNNABLE;
	TAILQ_REMOVE(&env_sched_list, curenv, env_sched_link);

	/* Step 5: Give up the CPU and block until a message is received. */
	((struct Trapframe *)KSTACKTOP - 1)->regs[10] = 0;
	schedule(1);
}

/* Overview:
 *   Try to send a 'value' (together with a page if 'srcva' is not 0) to the target env 'envid'.
 *
 * Post-Condition:
 *   Return 0 on success, and the target env is updated as follows:
 *   - 'env_ipc_recving' is set to 0 to block future sends.
 *   - 'env_ipc_from' is set to the sender's envid.
 *   - 'env_ipc_value' is set to the 'value'.
 *   - 'env_status' is set to 'ENV_RUNNABLE' again to recover from 'ipc_recv'.
 *   - if 'srcva' is not NULL, map 'env_ipc_dstva' to the same page mapped at 'srcva' in 'curenv'
 *     with 'perm'.
 *
 *   Return -E_IPC_NOT_RECV if the target has not been waiting for an IPC message with
 *   'sys_ipc_recv'.
 *   Return the original error when underlying calls fail.
 */
int sys_ipc_try_send(u_long envid, u_long value, u_long srcva, u_long perm) {
	struct Env *e;
	struct Page *p;

	/* Step 1: Check if 'srcva' is either zero or a legal address. */
	/* Exercise 4.8: Your code here. (4/8) */
	if (srcva != 0 && is_illegal_va(srcva)) {
		return -E_INVAL;
	}

	/* Step 2: Convert 'envid' to 'struct Env *e'. */
	/* This is the only syscall where the 'envid2env' should be used with 'checkperm' UNSET,
	 * because the target env is not restricted to 'curenv''s children. */
	/* Exercise 4.8: Your code here. (5/8) */
	try(envid2env(envid, &e, 0));

	/* Step 3: Check if the target is waiting for a message. */
	/* Exercise 4.8: Your code here. (6/8) */
	if (!e->env_ipc_recving) {
		return -E_IPC_NOT_RECV;
	}

	/* Step 4: Set the target's ipc fields. */
	e->env_ipc_value = value;
	e->env_ipc_from = curenv->env_id;
	e->env_ipc_perm = PTE_V | perm;
	e->env_ipc_recving = 0;

	/* Step 5: Set the target's status to 'ENV_RUNNABLE' again and insert it to the tail of
	 * 'env_sched_list'. */
	/* Exercise 4.8: Your code here. (7/8) */
	e->env_status = ENV_RUNNABLE;
	TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);

	/* Step 6: If 'srcva' is not zero, map the page at 'srcva' in 'curenv' to 'e->env_ipc_dstva'
	 * in 'e'. */
	/* Return -E_INVAL if 'srcva' is not zero and not mapped in 'curenv'. */
	if (srcva != 0) {
		/* Exercise 4.8: Your code here. (8/8) */	
		// if ((p = page_lookup(cur_pgdir, srcva, 0)) == NULL) {
		// 	return -E_INVAL;
		// }
		// page_insert(e->env_pgdir, e->env_asid, p, e->env_ipc_dstva, perm);

		if (is_mapped_page(&cur_pgdir, srcva) == 0) {
			return -E_INVAL;
		}

		u_long pa = get_pa(&cur_pgdir, srcva);

		/* Step 5: Map the physical page at 'dstva' in the address space of 'dstid'. */
		// return page_insert(dstenv->env_pgdir, dstenv->env_asid, pp, dstva, perm);
		// printk("ipc: %x: %016lx->%x: %016lx(%016lx)\n", e->env_id, e->env_ipc_dstva, curenv->env_id, srcva, pa);
		return map_page_user(&e->env_pgdir, e->env_asid, e->env_ipc_dstva, pa, perm);

	}
	return 0;
}

// XXX: kernel does busy waiting here, blocking all envs
int sys_cgetc(void) {
	int ch;
	while ((ch = scancharc()) == 255) { // 把 0 改成 255，这是因为 sbi 规定没有输入返回 255
	}
	return ch;
}

/* Overview:
 *  This function is used to write data at 'va' with length 'len' to a device physical address
 *  'pa'. Remember to check the validity of 'va' and 'pa' (see Hint below);
 *
 *  'va' is the starting address of source data, 'len' is the
 *  length of data (in bytes), 'pa' is the physical address of
 *  the device (maybe with a offset).
 *
 * Post-Condition:
 *  Data within [va, va+len) is copied to the physical address 'pa'.
 *  Return 0 on success.
 *  Return -E_INVAL on bad address.
 *
 * Hint: Use the unmapped and uncached segment in kernel address space (KSEG1) to perform MMIO.
 * Hint: You can use 'is_illegal_va_range' to validate 'va'.
 * Hint: You MUST use 'memcpy' to copy data after checking the validity.
 *
 *  All valid device and their physical address ranges:
 *	* ---------------------------------*
 *	|   device   | start addr | length |
 *	* -----------+------------+--------*
 *	|  console   | 0x10000000 | 0x20   | (dev_cons.h)
 *	|  IDE disk  | 0x13000000 | 0x4200 | (dev_disk.h)
 *	|    rtc     | 0x15000000 | 0x200  | (dev_rtc.h)
 *	* ---------------------------------*
 */
int sys_write_dev(u_long va, u_long pa, u_long len) {
	/* Exercise 5.1: Your code here. (1/2) */

	return 0;
}

/* Overview:
 *  This function is used to read data from a device physical address.
 *  Remember to check the validity of addresses (same as in 'sys_write_dev').
 *
 * Post-Condition:
 *  Data at 'pa' is copied from device to [va, va+len).
 *  Return 0 on success.
 *  Return -E_INVAL on bad address.
 *
 * Hint: You MUST use 'memcpy' to copy data after checking the validity.
 */
int sys_read_dev(u_long va, u_long pa, u_long len) {
	/* Exercise 5.1: Your code here. (2/2) */

	return 0;
}

#include <virtio.h>

int sys_read_sector(u_long va, le64 sector) {
	if (!is_mapped_page(&cur_pgdir, va)) {
		alloc_page(&cur_pgdir, curenv->env_asid, va, PTE_R | PTE_W | PTE_U);
	}
	u_long pa = get_pa(&cur_pgdir, va);
	
	if (va >> VPN0_SHIFT != (va + SECTOR_SIZE - 1) >> VPN0_SHIFT) {
		u_long offset = (va &~ (PAGE_SIZE - 1)) + PAGE_SIZE - va;
		if (!is_mapped_page(&cur_pgdir, va + SECTOR_SIZE)) {
			alloc_page(&cur_pgdir, curenv->env_asid, va + SECTOR_SIZE, PTE_R | PTE_W | PTE_U);
		}
		u_long pa_end = get_pa(&cur_pgdir, va + SECTOR_SIZE);

		u_long diskva = 0xb0008000;
		struct Virtio *disk = (struct Virtio *)diskva;
		read_sector(disk, sector);

		memcpy((void *)pa, (void *)&read_buffer.data, offset);
		memcpy((void *)pa_end - SECTOR_SIZE + offset, (void *)((u_long)&read_buffer.data + offset), SECTOR_SIZE - offset);

		if (read_buffer.status != 0) {
			panic("Write sector failed!");
		}
		return 0;
	}

	u_long diskva = 0xb0008000;
	struct Virtio *disk = (struct Virtio *)diskva;
	read_sector(disk, sector);

	memcpy((void *)pa, (void *)&read_buffer.data, SECTOR_SIZE);

	if (read_buffer.status != 0) {
		panic("Write sector failed!");
	}
	return 0;
}

int sys_write_sector(u_long va, le64 sector) {
	if (!is_mapped_page(&cur_pgdir, va)) {
		alloc_page(&cur_pgdir, curenv->env_asid, va, PTE_R | PTE_W | PTE_U);
	}
	u_long pa = get_pa(&cur_pgdir, va);

	if (va >> VPN0_SHIFT != (va + SECTOR_SIZE - 1) >> VPN0_SHIFT) {
		u_long offset = (va &~ (PAGE_SIZE - 1)) + PAGE_SIZE - va;
		if (!is_mapped_page(&cur_pgdir, va + SECTOR_SIZE)) {
			alloc_page(&cur_pgdir, curenv->env_asid, va + SECTOR_SIZE, PTE_R | PTE_W | PTE_U);
		}
		u_long pa_end = get_pa(&cur_pgdir, va + SECTOR_SIZE);

		memcpy((void *)&write_buffer.data, (void *)pa, offset);
		memcpy((void *)((u_long)&write_buffer.data + offset), (void *)pa_end - SECTOR_SIZE + offset, SECTOR_SIZE - offset);

		u_long diskva = 0xb0008000;
		struct Virtio *disk = (struct Virtio *)diskva;
		write_sector(disk, sector);

		if (write_buffer.status != 0) {
			panic("Write sector failed!");
		}
		return 0;
	}

	memcpy((void *)&write_buffer.data, (void *)pa, SECTOR_SIZE);

	u_long diskva = 0xb0008000;
	struct Virtio *disk = (struct Virtio *)diskva;
	write_sector(disk, sector);

	if (write_buffer.status != 0) {
		panic("Write sector failed!");
	}
	return 0;
}

int sys_flush() {
	u_long diskva = 0xb0008000;
	struct Virtio *disk = (struct Virtio *)diskva;
	flush(disk);

	if (flush_buffer.status != 0) {
		panic("Flush sector failed!");
	}
	return 0;
}

void *syscall_table[MAX_SYSNO] = {
    [SYS_putchar] = sys_putchar,
    [SYS_print_cons] = sys_print_cons,
    [SYS_getenvid] = sys_getenvid,
    [SYS_yield] = sys_yield,
    [SYS_env_destroy] = sys_env_destroy,
    [SYS_set_tlb_mod_entry] = sys_set_tlb_mod_entry,
    [SYS_mem_alloc] = sys_mem_alloc,
    [SYS_mem_map] = sys_mem_map,
    [SYS_mem_unmap] = sys_mem_unmap,
    [SYS_exofork] = sys_exofork,
    [SYS_set_env_status] = sys_set_env_status,
    [SYS_set_trapframe] = sys_set_trapframe,
    [SYS_panic] = sys_panic,
    [SYS_ipc_try_send] = sys_ipc_try_send,
    [SYS_ipc_recv] = sys_ipc_recv,
    [SYS_cgetc] = sys_cgetc,
    [SYS_write_dev] = sys_write_dev,
    [SYS_read_dev] = sys_read_dev,
	[SYS_read_sector] = sys_read_sector,
	[SYS_write_sector] = sys_write_sector,
	[SYS_flush] = sys_flush,
};

/* Overview:
 *   Call the function in 'syscall_table' indexed at 'sysno' with arguments from user context and
 * stack.
 *
 * Hint:
 *   Use sysno from $a0 to dispatch the syscall.
 *   The possible arguments are stored at $a1, $a2, $a3, [$sp + 16 bytes], [$sp + 20 bytes] in
 *   order.
 *   Number of arguments cannot exceed 5.
 */
void do_syscall(struct Trapframe *tf) {
	int (*func)(u_long, u_long, u_long, u_long, u_long);
	int sysno = tf->regs[10];
	if (sysno < 0 || sysno >= MAX_SYSNO) {
		tf->regs[10] = -E_NO_SYS;
		return;
	}

	// printk("syscall sysno=%d\n", sysno);

	/* Step 1: Add the EPC in 'tf' by a word (size of an instruction). */
	/* Exercise 4.2: Your code here. (1/4) */
	tf->sepc += 4;

	/* Step 2: Use 'sysno' to get 'func' from 'syscall_table'. */
	/* Exercise 4.2: Your code here. (2/4) */
	func = syscall_table[sysno];

	/* Step 3: First 3 args are stored in $a1, $a2, $a3. */
	// print_tf(tf);
	u_long arg1 = tf->regs[11];
	u_long arg2 = tf->regs[12];
	u_long arg3 = tf->regs[13];
	u_long arg4 = tf->regs[14];
	u_long arg5 = tf->regs[15];

	u_long sip;
	asm volatile("csrr %0, sip" : "=r"(sip));
	// printk("sysno=%d\n", sysno);

	// /* Step 4: Last 2 args are stored in stack at [$sp + 16 bytes], [$sp + 20 bytes]. */
	// u_long arg4, arg5;
	// /* Exercise 4.2: Your code here. (3/4) */
	// u_long sp = tf->sscratch; // sp 改为 sscratch
	// debug_page_user(&cur_pgdir);
	// arg4 = get(&cur_pgdir, sp + 16);
	// arg5 = get(&cur_pgdir, sp + 20);

	/* Step 5: Invoke 'func' with retrieved arguments and store its return value to $v0 in 'tf'.
	 */
	/* Exercise 4.2: Your code here. (4/4) */
	tf->regs[10] = func(arg1, arg2, arg3, arg4, arg5);

}
