#ifndef _TRAP_H_
#define _TRAP_H_

#ifndef __ASSEMBLER__

#include <types.h>

struct Trapframe {
	/* Saved general purpose registers. */
	u_long regs[32];

	/* Saved control status registers. */
	u_long sie;
	u_long sip;
	u_long sepc;
	u_long stvec;
	u_long scause;
	u_long stval;
	u_long sstatus;
	u_long sscratch;
};

void print_tf(struct Trapframe *tf);

#endif /* !__ASSEMBLER__ */

#ifdef RISCV32
#define _TRAP_LEN_ 4
#else // riscv64
#define _TRAP_LEN_ 8
#endif

#define TF_REG0 0
#define TF_REG1 ((TF_REG0) + _TRAP_LEN_)
#define TF_REG2 ((TF_REG1) + _TRAP_LEN_)
#define TF_REG3 ((TF_REG2) + _TRAP_LEN_)
#define TF_REG4 ((TF_REG3) + _TRAP_LEN_)
#define TF_REG5 ((TF_REG4) + _TRAP_LEN_)
#define TF_REG6 ((TF_REG5) + _TRAP_LEN_)
#define TF_REG7 ((TF_REG6) + _TRAP_LEN_)
#define TF_REG8 ((TF_REG7) + _TRAP_LEN_)
#define TF_REG9 ((TF_REG8) + _TRAP_LEN_)
#define TF_REG10 ((TF_REG9) + _TRAP_LEN_)
#define TF_REG11 ((TF_REG10) + _TRAP_LEN_)
#define TF_REG12 ((TF_REG11) + _TRAP_LEN_)
#define TF_REG13 ((TF_REG12) + _TRAP_LEN_)
#define TF_REG14 ((TF_REG13) + _TRAP_LEN_)
#define TF_REG15 ((TF_REG14) + _TRAP_LEN_)
#define TF_REG16 ((TF_REG15) + _TRAP_LEN_)
#define TF_REG17 ((TF_REG16) + _TRAP_LEN_)
#define TF_REG18 ((TF_REG17) + _TRAP_LEN_)
#define TF_REG19 ((TF_REG18) + _TRAP_LEN_)
#define TF_REG20 ((TF_REG19) + _TRAP_LEN_)
#define TF_REG21 ((TF_REG20) + _TRAP_LEN_)
#define TF_REG22 ((TF_REG21) + _TRAP_LEN_)
#define TF_REG23 ((TF_REG22) + _TRAP_LEN_)
#define TF_REG24 ((TF_REG23) + _TRAP_LEN_)
#define TF_REG25 ((TF_REG24) + _TRAP_LEN_)
#define TF_REG26 ((TF_REG25) + _TRAP_LEN_)
#define TF_REG27 ((TF_REG26) + _TRAP_LEN_)
#define TF_REG28 ((TF_REG27) + _TRAP_LEN_)
#define TF_REG29 ((TF_REG28) + _TRAP_LEN_)
#define TF_REG30 ((TF_REG29) + _TRAP_LEN_)
#define TF_REG31 ((TF_REG30) + _TRAP_LEN_)

#define TF_SIE ((TF_REG31) + _TRAP_LEN_)
#define TF_SIP ((TF_SIE) + _TRAP_LEN_)
#define TF_SEPC ((TF_SIP) + _TRAP_LEN_)
#define TF_STVEC ((TF_SEPC) + _TRAP_LEN_)
#define TF_SCAUSE ((TF_STVEC) + _TRAP_LEN_)
#define TF_STVAL ((TF_SCAUSE) + _TRAP_LEN_)
#define TF_SSTATUS ((TF_STVAL) + _TRAP_LEN_)
#define TF_SSCRATCH ((TF_SSTATUS) + _TRAP_LEN_)

#define TF_SIZE ((TF_SSCRATCH) + _TRAP_LEN_)

#endif /* _TRAP_H_ */
