#ifndef __ASM_MIPS_REGDEF_H
#define __ASM_MIPS_REGDEF_H

/*
 * Symbolic register names for 32 bit ABI
 */
#define zero x0 /* wired zero */
#define AT x1	/* assembler temp  - uppercase because of ".set at" */
#define v0 x2	/* return value */
#define v1 x3
#define a0 x4 /* argument registers */
#define a1 x5
#define a2 x6
#define a3 x7
#define t0 x8 /* caller saved */
#define t1 x9
#define t2 x10
#define t3 x11
#define t4 x12
#define t5 x13
#define t6 x14
#define t7 x15
#define s0 x16 /* callee saved */
#define s1 x17
#define s2 x18
#define s3 x19
#define s4 x20
#define s5 x21
#define s6 x22
#define s7 x23
#define t8 x24 /* caller saved */
#define t9 x25
#define jp x25 /* PIC jump register */
#define k0 x26 /* kernel scratch */
#define k1 x27
#define gp x28 /* global pointer */
#define sp x29 /* stack pointer */
#define fp x30 /* frame pointer */
#define s8 x30 /* same like fp! */
#define ra x31 /* return address */

#endif /* __ASM_MIPS_REGDEF_H */
