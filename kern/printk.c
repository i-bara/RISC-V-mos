#include <drivers/dev_cons.h>
#include <print.h>
#include <printk.h>
#include <trap.h>

void outputk(void *data, const char *buf, size_t len) {
	for (int i = 0; i < len; i++) {
		printcharc(buf[i]);
	}
}

void printk(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintfmt(outputk, NULL, fmt, ap);
	va_end(ap);
}

void print_tf(struct Trapframe *tf) {
	printk("------trapframe at %016lx------\n"
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
	printk("sie: %016lx    sepc:  %016lx    scause: %016lx    sstatus:  %016lx\n", tf->sie, tf->sepc, tf->scause, tf->sstatus);
	printk("sip: %016lx    stvec: %016lx    stval:  %016lx    sscratch: %016lx\n", tf->sip, tf->stvec, tf->stval, tf->sscratch);
}
