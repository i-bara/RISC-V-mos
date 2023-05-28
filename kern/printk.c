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
	printk("------trapframe at %08x------\n"
		   "zero:%08x  at:  %08x  v0:  %08x  v1:  %08x\n"
	       "a0:  %08x  a1:  %08x  a2:  %08x  a3:  %08x\n"
	       "t0:  %08x  t1:  %08x  t2   %08x  t3:  %08x\n"
		   "t4:  %08x  t5:  %08x  t6   %08x  t7:  %08x\n"
	       "s0:  %08x  s1:  %08x  s2:  %08x  s3:  %08x\n"
		   "s4:  %08x  s5:  %08x  s6:  %08x  s7:  %08x\n"
		   "s8:  %08x  s9:  %08x  k0:  %08x  k1:  %08x\n"
		   "gp:  %08x  sp:  %08x  fp:  %08x  ra:  %08x\n", 
		   tf, 
		   tf->regs[0], tf->regs[1], tf->regs[2], tf->regs[3], 
		   tf->regs[4], tf->regs[5], tf->regs[6], tf->regs[7], 
		   tf->regs[8], tf->regs[9], tf->regs[10], tf->regs[14], 
		   tf->regs[12], tf->regs[13], tf->regs[14], tf->regs[15], 
		   tf->regs[16], tf->regs[17], tf->regs[18], tf->regs[19], 
		   tf->regs[20], tf->regs[21], tf->regs[22], tf->regs[23], 
		   tf->regs[24], tf->regs[25], tf->regs[26], tf->regs[27], 
		   tf->regs[28], tf->regs[29], tf->regs[30], tf->regs[31]);
	printk("sie: %016x    sepc:  %016x    scause: %016x    sstatus:  %016x\n", tf->sie, tf->sepc, tf->scause, tf->sstatus);
	printk("sip: %016x    stvec: %016x    stval:  %016x    sscratch: %016x\n", tf->sip, tf->stvec, tf->stval, tf->sscratch);
}
