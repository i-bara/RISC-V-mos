#ifndef _ENV_H_
#define _ENV_H_

#include <mmu.h>
#include <queue.h>
#include <trap.h>
#include <types.h>

#define LOG2NENV 10
#define NENV (1 << LOG2NENV)
#define ENVX(envid) ((envid) & (NENV - 1))

// Values of env_status in struct Env
#define ENV_FREE 0
#define ENV_RUNNABLE 1
#define ENV_NOT_RUNNABLE 2

struct Env {
	struct Trapframe env_tf;  // Saved registers
	LIST_ENTRY(Env) env_link; // Free list
	u_int env_id;		  // Unique environment identifier
	u_int env_asid;		  // ASID
	u_int env_parent_id;	  // env_id of this env's parent
	u_int env_status;	  // Status of the environment
	u_long env_pgdir;		  // Kernel virtual address of page dir
	TAILQ_ENTRY(Env) env_sched_link;
	u_int env_pri;
	// Lab 4 IPC
	u_long env_ipc_value;   // data value sent to us 改为了 64 位
	u_int env_ipc_from;    // envid of the sender
	u_int env_ipc_recving; // env is blocked receiving
	u_long env_ipc_dstva;   // va at which to map received page 改为了 64 位
	u_int env_ipc_perm;    // perm of page mapping received

	// Lab 4 fault handling
	u_long env_user_tlb_mod_entry; // user tlb mod handler 改为了 64 位

	// Lab 6 scheduler counts
	u_int env_runs; // number of times been env_run'ed
};

LIST_HEAD(Env_list, Env);
TAILQ_HEAD(Env_sched_list, Env);
extern struct Env *curenv;		     // the current env
extern struct Env_sched_list env_sched_list; // runnable env list

void env_init(void);
int env_alloc(struct Env **e, u_int parent_id);
void env_free(struct Env *);
struct Env *env_create(const void *binary, size_t size, int priority);
void env_destroy(struct Env *e);

int envid2env(u_int envid, struct Env **penv, int checkperm);
void env_run(struct Env *e) __attribute__((noreturn));
void enable_irq(void);

void env_check(void);
void envid2env_check(void);

#define ENV_CREATE_PRIORITY(x, y)                                                                  \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, y);                       \
	})

#define ENV_CREATE(x)                                                                              \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, 1);                       \
	})

void debug_env(void);
void print_env(struct Env* e);
void debug_sched(void);
void print_tff(struct Trapframe *tf);
void debug_elf(const void *binary, size_t size);
void *get_pc(void);
void print_code(void *pc1, void *pc2);
#define print_pc() printk("pc: %08x\n", get_pc())

#include <asm/asm.h>

#define print_regs()\
	do {\
		printk("------regs at %s:%d(%s)------\n", __FILE__, __LINE__, __func__);\
		asm(".set noat");\
		asm("move  $k0, $sp");\
		asm("subu  $sp, $sp, 152");\
		asm("sw   $k0, 116($sp)");\
		asm("mfc0  $k0, $12");\
		asm("sw   $k0, 128($sp)");\
		asm("mfc0  $k0, $13");\
		asm("sw   $k0, 144($sp)");\
		asm("mfc0  $k0, $14");\
		asm("sw   $k0, 148($sp)");\
		asm("mfc0  $k0, $8");\
		asm("sw   $k0, 140($sp)");\
		asm("mfhi  $k0");\
		asm("sw   $k0, 132($sp)");\
		asm("mflo  $k0");\
		asm("sw   $k0, 136($sp)");\
		asm("sw   $0, 0($sp)");\
		asm("sw   $1, 4($sp)");\
		asm("sw   $2, 8($sp)");\
		asm("sw   $3, 12($sp)");\
		asm("sw   $4, 16($sp)");\
		asm("sw   $5, 20($sp)");\
		asm("sw   $6, 24($sp)");\
		asm("sw   $7, 28($sp)");\
		asm("sw   $8, 32($sp)");\
		asm("sw   $9, 36($sp)");\
		asm("sw   $10, 40($sp)");\
		asm("sw   $11, 44($sp)");\
		asm("sw   $12, 48($sp)");\
		asm("sw   $13, 52($sp)");\
		asm("sw   $14, 56($sp)");\
		asm("sw   $15, 60($sp)");\
		asm("sw   $16, 64($sp)");\
		asm("sw   $17, 68($sp)");\
		asm("sw   $18, 72($sp)");\
		asm("sw   $19, 76($sp)");\
		asm("sw   $20, 80($sp)");\
		asm("sw   $21, 84($sp)");\
		asm("sw   $22, 88($sp)");\
		asm("sw   $23, 92($sp)");\
		asm("sw   $24, 96($sp)");\
		asm("sw   $25, 100($sp)");\
		asm("sw   $26, 104($sp)");\
		asm("sw   $27, 108($sp)");\
		asm("sw   $28, 112($sp)");\
		asm("sw   $30, 120($sp)");\
		asm("sw   $31, 124($sp)");\
		asm("lw  $a0, 0($sp)");\
		asm("jal  print_reg_zero");\
		asm("lw  $a0, 4($sp)");\
		asm("jal  print_reg_at");\
		asm("lw  $a0, 8($sp)");\
		asm("jal  print_reg_v0");\
		asm("lw  $a0, 12($sp)");\
		asm("jal  print_reg_v1");\
		asm("jal  print_endl");\
		asm("lw  $a0, 16($sp)");\
		asm("jal  print_reg_a0");\
		asm("lw  $a0, 20($sp)");\
		asm("jal  print_reg_a1");\
		asm("lw  $a0, 24($sp)");\
		asm("jal  print_reg_a2");\
		asm("lw  $a0, 28($sp)");\
		asm("jal  print_reg_a3");\
		asm("jal  print_endl");\
		asm("lw  $a0, 32($sp)");\
		asm("jal  print_reg_t0");\
		asm("lw  $a0, 36($sp)");\
		asm("jal  print_reg_t1");\
		asm("lw  $a0, 40($sp)");\
		asm("jal  print_reg_t2");\
		asm("lw  $a0, 44($sp)");\
		asm("jal  print_reg_t3");\
		asm("jal  print_endl");\
		asm("lw  $a0, 48($sp)");\
		asm("jal  print_reg_t4");\
		asm("lw  $a0, 52($sp)");\
		asm("jal  print_reg_t5");\
		asm("lw  $a0, 56($sp)");\
		asm("jal  print_reg_t6");\
		asm("lw  $a0, 60($sp)");\
		asm("jal  print_reg_t7");\
		asm("jal  print_endl");\
		asm("lw  $a0, 64($sp)");\
		asm("jal  print_reg_s0");\
		asm("lw  $a0, 68($sp)");\
		asm("jal  print_reg_s1");\
		asm("lw  $a0, 72($sp)");\
		asm("jal  print_reg_s2");\
		asm("lw  $a0, 76($sp)");\
		asm("jal  print_reg_s3");\
		asm("jal  print_endl");\
		asm("lw  $a0, 80($sp)");\
		asm("jal  print_reg_s4");\
		asm("lw  $a0, 84($sp)");\
		asm("jal  print_reg_s5");\
		asm("lw  $a0, 88($sp)");\
		asm("jal  print_reg_s6");\
		asm("lw  $a0, 92($sp)");\
		asm("jal  print_reg_s7");\
		asm("jal  print_endl");\
		asm("lw  $a0, 96($sp)");\
		asm("jal  print_reg_t8");\
		asm("lw  $a0, 100($sp)");\
		asm("jal  print_reg_t9");\
		asm("lw  $a0, 104($sp)");\
		asm("jal  print_reg_k0");\
		asm("lw  $a0, 108($sp)");\
		asm("jal  print_reg_k1");\
		asm("jal  print_endl");\
		asm("lw  $a0, 112($sp)");\
		asm("jal  print_reg_gp");\
		asm("lw  $a0, 116($sp)");\
		asm("jal  print_reg_sp");\
		asm("lw  $a0, 120($sp)");\
		asm("jal  print_reg_fp");\
		asm("lw  $a0, 124($sp)");\
		asm("jal  print_reg_ra");\
		asm("jal  print_endl");\
		asm("lw  $a0, 128($sp)");\
		asm("jal  print_reg_status");\
		asm("lw  $a0, 132($sp)");\
		asm("jal  print_reg_hi");\
		asm("lw  $a0, 136($sp)");\
		asm("jal  print_reg_lo");\
		asm("jal  print_endl");\
		asm("lw  $a0, 140($sp)");\
		asm("jal  print_reg_badvaddr");\
		asm("lw  $a0, 144($sp)");\
		asm("jal  print_reg_cause");\
		asm("lw  $a0, 148($sp)");\
		asm("jal  print_reg_epc");\
		asm("jal  print_endl");\
		asm("lw   $v0, 128($sp)");\
		asm("mtc0  $v0, $12");\
		asm("lw   $v1, 136($sp)");\
		asm("mtlo  $v1");\
		asm("lw   $v0, 132($sp)");\
		asm("lw   $v1, 148($sp)");\
		asm("mthi  $v0");\
		asm("mtc0  $v1, $14");\
		asm("lw   $31, 124($sp)");\
		asm("lw   $30, 120($sp)");\
		asm("lw   $28, 112($sp)");\
		asm("lw   $25, 100($sp)");\
		asm("lw   $24, 96($sp)");\
		asm("lw   $23, 92($sp)");\
		asm("lw   $22, 88($sp)");\
		asm("lw   $21, 84($sp)");\
		asm("lw   $20, 80($sp)");\
		asm("lw   $19, 76($sp)");\
		asm("lw   $18, 72($sp)");\
		asm("lw   $17, 68($sp)");\
		asm("lw   $16, 64($sp)");\
		asm("lw   $15, 60($sp)");\
		asm("lw   $14, 56($sp)");\
		asm("lw   $13, 52($sp)");\
		asm("lw   $12, 48($sp)");\
		asm("lw   $11, 44($sp)");\
		asm("lw   $10, 40($sp)");\
		asm("lw   $9, 36($sp)");\
		asm("lw   $8, 32($sp)");\
		asm("lw   $7, 28($sp)");\
		asm("lw   $6, 24($sp)");\
		asm("lw   $5, 20($sp)");\
		asm("lw   $4, 16($sp)");\
		asm("lw   $3, 12($sp)");\
		asm("lw   $2, 8($sp)");\
		asm("lw   $1, 4($sp)");\
		asm("lw  $sp, 116($sp)");\
		asm(".set at");\
		printk("----------------------------------------\n");\
	} while (/*CONSTCOND*/ 0)

#define print_stackframe(n)\
	do {\
		u_long fp;\
		asm("move %0, $30" : "=r"(fp) :);\
		printk("------stackframe at %08x, %s:%d(%s)------", fp, __FILE__, __LINE__, __func__);\
		for (int i = 0; i < n; i++) {\
			if (i % 4 == 0) {\
				printk("\n");\
			}\
			printk("%2d: %08x ", i, ((u_long *)fp)[i]);\
		}\
		printk("\n");\
	} while (/*CONSTCOND*/ 0)

#endif // !_ENV_H_
