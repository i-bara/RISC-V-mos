#include <env.h>
#include <print.h>
#include <printk.h>

void outputk(void *data, const char *buf, size_t len);

void _panic(const char *file, int line, const char *func, const char *fmt, ...) {
	u_long sp, ra, badva, sr, cause, epc;
	asm("move %0, x29" : "=r"(sp) :);
	asm("move %0, x31" : "=r"(ra) :);
	asm("csrr %0, stval" : "=r"(badva) :);
	asm("csrr %0, sstatus" : "=r"(sr) :);
	asm("csrr %0, scause" : "=r"(cause) :);
	asm("csrr %0, sepc" : "=r"(epc) :);

	printk("panic at %s:%d (%s): ", file, line, func);

	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);

	printk("\n"
	       "ra:    %016lx  sp:  %016lx  status: %016lx\n"
	       "cause: %016lx  epc: %016lx  tval:   %016lx\n",
	       ra, sp, sr, cause, epc, badva);

#if !defined(LAB) || LAB >= 3
	extern struct Env envs[];
	extern struct Env *curenv;
	extern u_long cur_pgdir;

	if ((u_long)curenv >= KERNBASE) {
		printk("curenv:    %016lx (id = 0x%x, off = %d)\n", curenv, curenv->env_id,
		       curenv - envs);
	} else if (curenv) {
		printk("curenv:    %016lx (invalid)\n", curenv);
	} else {
		printk("curenv:    NULL\n");
	}

	if ((u_long)cur_pgdir >= KERNBASE) {
		printk("cur_pgdir: %016lx\n", cur_pgdir);
	} else if (cur_pgdir) {
		printk("cur_pgdir: %016lx (invalid)\n", cur_pgdir);
	} else {
		printk("cur_pgdir: NULL\n", cur_pgdir);
	}
#endif

#ifdef MOS_HANG_ON_PANIC
	while (1) {
	}
#else
	halt();
#endif
}
