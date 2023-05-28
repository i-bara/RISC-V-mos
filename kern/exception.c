#include <asm/asm.h>
// #include <drivers/dev_rtc.h>
// #include <stackframe.h>

#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

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

void yay() {
    printk("Yayayayay!\n");
    // printk("ccccccccccccc!\n");
    u_long r;
    asm volatile("csrr %0, scause " : "=r"(r));
    printk("cause=%016lx\n", r);
    asm volatile("csrr %0, sepc " : "=r"(r));
    printk("epc=%016lx\n", r);
    asm volatile("csrr %0, sstatus " : "=r"(r));
    printk("status=%016lx\n", r);
    // asm volatile("sret");
    schedule(1);
    printk("dddddddddddd!\n");
}

void handle_exception(u_long err) {
	u_long r;
    asm volatile("csrr %0, scause " : "=r"(r));
    printk("Exception: cause=%ld\n", r);
    asm volatile("csrr %0, sepc " : "=r"(r));
    printk("epc=%016lx\n", r);
    asm volatile("csrr %0, sstatus " : "=r"(r));
    printk("status=%016lx\n", r);
    asm volatile("csrr %0, stval " : "=r"(r));
    printk("tval=%016lx\n", r);
    halt();
    asm volatile("sret");
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