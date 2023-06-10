#include <elf.h>
#include <env.h>
#include <lib.h>
#include <mmu.h>

#define debug 0

int init_stack(u_int child, char **argv, u_int *init_sp) {
	int argc, i, r, tot;
	char *strings;
	u_long *args; // 需要符合字长

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (tot)
	tot = 0;
	for (argc = 0; argv[argc]; argc++) {
		tot += strlen(argv[argc]) + 1; // 修改 argv
	}

	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, sizeof(u_long)) + sizeof(u_long) * (argc + 2) + sizeof(int) > BY2PG) { // 需要符合字长
		return -E_NO_MEM;
	}

	// Determine where to place the strings and the args array
	strings = (char *)(UTEMP + BY2PG) - tot;
	args = (u_int *)(UTEMP + BY2PG - ROUND(tot, sizeof(u_long)) - sizeof(u_long) * (argc + 1)); // 需要符合字长

	if ((r = syscall_mem_alloc(0, (void *)UTEMP, PTE_R | PTE_W | PTE_U)) < 0) {
		return r;
	}

	// Copy the argument strings into the stack page at 'strings'
	char *ctemp, *argv_temp;
	u_int j;
	ctemp = strings;
	for (i = 0; i < argc; i++) {
		argv_temp = argv[i];
		for (j = 0; j < strlen(argv[i]); j++) {
			*ctemp = *argv_temp;
			ctemp++;
			argv_temp++;
		}
		*ctemp = 0;
		ctemp++;
	}

	// Initialize args[0..argc-1] to be pointers to these strings
	// that will be valid addresses for the child environment
	// (for whom this page will be at USTACKTOP-BY2PG!).
	ctemp = (char *)(USTACKTOP - UTEMP - BY2PG + (u_int)strings);
	for (i = 0; i < argc; i++) {
		args[i] = (u_long)ctemp; // 改成字长
		ctemp += strlen(argv[i]) + 1;
	}

	// Set args[argc] to 0 to null-terminate the args array.
	ctemp--;
	args[argc] = (u_long)ctemp; // 改成字长

	// Push two more words onto the child's stack below 'args',
	// containing the argc and argv parameters to be passed
	// to the child's main() function.
	u_long *pargv_ptr; // 改成字长
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - UTEMP - BY2PG + (u_long)args; // 改成字长

	int *argc_ptr;
	argc_ptr = (int *)pargv_ptr - 1;
	*argc_ptr = argc;

	// Set *init_sp to the initial stack pointer for the child
	*init_sp = USTACKTOP - UTEMP - BY2PG + (u_long)argc_ptr; // 改成字长

	if ((r = syscall_mem_map(0, (void *)UTEMP, child, (void *)(USTACKTOP - BY2PG), PTE_R | PTE_W | PTE_U)) <
	    0) {
		goto error;
	}
	if ((r = syscall_mem_unmap(0, (void *)UTEMP)) < 0) {
		goto error;
	}

	return 0;

error:
	syscall_mem_unmap(0, (void *)UTEMP);
	return r;
}

static int spawn_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			size_t len) {
	u_int child_id = *(u_int *)data;
	try(syscall_mem_alloc(child_id, (void *)va, perm));
	if (src != NULL) {
		int r = syscall_mem_map(child_id, (void *)va, 0, (void *)UTEMP, perm | PTE_R | PTE_W | PTE_U);
		if (r) {
			syscall_mem_unmap(child_id, (void *)va);
			return r;
		}
		memcpy((void *)(UTEMP + offset), src, len);
		return syscall_mem_unmap(0, (void *)UTEMP);
	}
	return 0;
}

void debug_Ehdr(char *elfbuf) {
	for (int i = 0; i < sizeof(Elf64_Ehdr); i++) {
		debugf("%02x ", (u_char)elfbuf[i]);
		if (i % 4 == 3) {
			debugf("\n");
		}
	}
	debugf("\n");
}

int spawn(char *prog, char **argv) {
	// Step 1: Open the file 'prog' (the path of the program).
	// Return the error if 'open' fails.
	int fd;
	if ((fd = open(prog, O_RDONLY)) < 0) {
		return fd;
	}

	// Step 2: Read the ELF header (of type 'Elf64_Ehdr') from the file into 'elfbuf' using
	// 'readn()'.
	// If that fails (where 'readn' returns a different size than expected),
	// set 'r' and 'goto err' to close the file and return the error.
	int r;
	u_char elfbuf[512];
	u_char elfbuf_ph[512];
	/* Exercise 6.4: Your code here. (1/6) */
	if ((r = readn(fd, elfbuf, sizeof(Elf64_Ehdr))) < sizeof(Elf64_Ehdr)) {
		goto err;
	}

	const Elf64_Ehdr *ehdr = elf_from_64(elfbuf, sizeof(Elf64_Ehdr));
	if (!ehdr) {
		r = -E_NOT_EXEC;
		goto err;
	}
	u_long entrypoint = ehdr->e_entry;

	// Step 3: Create a child using 'syscall_exofork()' and store its envid in 'child'.
	// If the syscall fails, set 'r' and 'goto err'.
	u_int child;
	/* Exercise 6.4: Your code here. (2/6) */
	if ((r = syscall_exofork()) < 0) {
		goto err;
	}
	child = r;

	// Step 4: Use 'init_stack(child, argv, &sp)' to initialize the stack of the child.
	// 'goto err1' if that fails.
	u_int sp;
	/* Exercise 6.4: Your code here. (3/6) */
	if ((r = init_stack(child, argv, &sp)) < 0) {
		goto err1;
	}

	// Step 5: Load the ELF segments in the file into the child's memory.
	// This is similar to 'load_icode()' in the kernel.
	size_t ph_off;
	// debugf("%016lx\n", ((u_long *)ehdr)[0]);
	// debugf("%016lx\n", ((u_long *)ehdr)[1]);
	// debugf("%016lx\n", ((u_long *)ehdr)[2]);
	// debugf("%016lx\n", ((u_long *)ehdr)[3]);
	// debugf("%016lx\n", ((u_long *)ehdr)[4]);
	// debugf("%016lx\n", ((u_long *)ehdr)[5]);
	// debugf("%016lx\n", ((u_long *)ehdr)[6]);
	// debugf("%016lx\n", ((u_long *)ehdr)[7]);
	ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
		// Read the program header in the file with offset 'ph_off' and length
		// 'ehdr->e_phentsize' into 'elfbuf'.
		// 'goto err1' on failure.
		// You may want to use 'seek' and 'readn'.
		/* Exercise 6.4: Your code here. (4/6) */
		if ((r = seek(fd, ph_off)) < 0) {
			goto err1;
		}

		user_assert(ehdr->e_phentsize == sizeof(Elf64_Phdr));
		if ((r = readn(fd, elfbuf_ph, sizeof(Elf64_Phdr))) < sizeof(Elf64_Phdr)) { // 全都要改成 64 位，否则无法导入
			goto err1;
		}

		Elf64_Phdr *ph = (Elf64_Phdr *)elfbuf_ph; // 需要用两个缓存，否则会覆盖掉 ehdr
		// debugf("ph_off: %d\n", ph_off);
		// debugf("%016lx\n", ((u_long *)ph)[0]);
		// debugf("%016lx\n", ((u_long *)ph)[1]);
		// debugf("%016lx\n", ((u_long *)ph)[2]);
		// debugf("%016lx\n", ((u_long *)ph)[3]);
		// debugf("%016lx\n", ((u_long *)ph)[4]);
		// debugf("%016lx\n", ((u_long *)ph)[5]);
		// debugf("%016lx\n", ((u_long *)ph)[6]);
		// debugf("%016lx\n", ((u_long *)ph)[7]);
		if (ph->p_type == PT_LOAD) {
			void *bin;
			// Read and map the ELF data in the file at 'ph->p_offset' into our memory
			// using 'read_map()'.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (5/6) */
			if ((r = read_map(fd, ph->p_offset, &bin)) < 0) {
				goto err1;
			}

			// Load the segment 'ph' into the child's memory using 'elf_load_seg()'.
			// Use 'spawn_mapper' as the callback, and '&child' as its data.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (6/6) */
			if ((r = elf_load_seg_64(ph, bin, spawn_mapper, &child)) < 0) {
				goto err1;
			}

		}
	}
	close(fd);

	struct Trapframe tf = envs[ENVX(child)].env_tf;
	tf.sie = SIE_UTIE | SIE_STIE;
	tf.sstatus = SSTATUS_UIE | SSTATUS_SPIE;
	tf.sepc = entrypoint;
	tf.sscratch = sp;
	if ((r = syscall_set_trapframe(child, &tf)) != 0) {
		goto err2;
	}

	// Pages with 'PTE_LIBRARY' set are shared between the parent and the child.
	for (int va = 0; va < USTACKTOP; va += PAGE_SIZE) {
		if (pt2[va >> VPN2_SHIFT] & PTE_V) {
			if (pt1[va >> VPN1_SHIFT] & PTE_V) {
				if (pt0[va >> VPN0_SHIFT] & PTE_V) {
					if ((pt0[va >> VPN0_SHIFT] & PTE_LIBRARY)) { // 仅 duppage 共享页面
						if ((r = syscall_mem_map(0, va, child, va, pt0[va >> VPN0_SHIFT] & PTE_PERM)) < 0) {
							debugf("spawn: syscall_mem_map %x %x: %d\n", va, child, r);
							goto err2;
						}
					}
				}
			}
		}
	}

	// for (u_int pdeno = 0; pdeno <= PDX(USTACKTOP); pdeno++) {
	// 	if (!(vpd[pdeno] & PTE_V)) {
	// 		continue;
	// 	}
	// 	for (u_int pteno = 0; pteno <= PTX(~0); pteno++) {
	// 		u_int pn = (pdeno << 10) + pteno;
	// 		u_int perm = vpt[pn] & ((1 << PGSHIFT) - 1);
	// 		if ((perm & PTE_V) && (perm & PTE_LIBRARY)) {
	// 			void *va = (void *)(pn << PGSHIFT);

	// 			if ((r = syscall_mem_map(0, va, child, va, perm)) < 0) {
	// 				debugf("spawn: syscall_mem_map %x %x: %d\n", va, child, r);
	// 				goto err2;
	// 			}
	// 		}
	// 	}
	// }

	if ((r = syscall_set_env_status(child, ENV_RUNNABLE)) < 0) {
		debugf("spawn: syscall_set_env_status %x: %d\n", child, r);
		goto err2;
	}
	return child;

err2:
	syscall_env_destroy(child);
	return r;
err1:
	syscall_env_destroy(child);
err:
	close(fd);
	return r;
}

char *args_1[512]; // 不能放在 spawn 里面，否则会成为局部变量

#include <stdarg.h>

int spawnl(char *prog, char *args, ...) {
	// Thanks to MIPS calling convention, the layout of arguments on the stack
	// are straightforward.
	// 但是 RISC-V 在第二个参数之后会紧接着 局部变量、fp 和 ra，并不是连续的，因此需要有所修改，具体修改不能在 init_stack 中实现，否则不经过 spawnl 的 spawn 会出问题
	
	#ifdef DEBUG_ELF
	debug_hex(args, 8);
	debug_hex(&args, 16);
	#endif
		
	va_list ap;
	va_start(ap, args);
	char* s;
	int i = 0;
	args_1[i++] = args;
	while ((s = (char *)va_arg(ap, char *)) != NULL) {
		args_1[i++] = s;
	}
	args_1[i++] = s;
	va_end(ap);
	

	// int i;
	// for (i = 0; (&args)[i > 0 ? i + 5 : i]; i++) { // 与编译器有关，太糟糕了！
	// 	args_1[i] = (&args)[i > 0 ? i + 5 : i];
	// }
	// args_1[i] = 0;
	
	return spawn(prog, args_1);
}

void debug_hex(void *args, int n) {
	for (int i = 0; i < n; i++) {
		debugf("%016lx: %016lx    ", &((u_long *)args)[i], ((u_long *)args)[i]);
		for (int j = 0; j < 8; j++) {
			if (((u_char *)args)[i * 8 + j] == '\n') {
				debugf("\\n");
			} else {
				debugf(" %c", ((u_char *)args)[i * 8 + j]);
			}
		}
		debugf("\n");
	}
}
