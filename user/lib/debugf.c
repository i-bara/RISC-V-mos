#include <lib.h>
#include <print.h>

#define BUF_LEN 1024

struct debug_ctx {
	char buf[BUF_LEN];
	size_t pos;
};

static void debug_flush(struct debug_ctx *ctx) {
	if (ctx->pos == 0) {
		return;
	}
	int r;
	if ((r = syscall_print_cons(ctx->buf, ctx->pos)) != 0) {
		user_panic("syscall_print_cons: %d", r);
	}
	ctx->pos = 0;
}

static void debug_output(void *data, const char *s, size_t l) {
	struct debug_ctx *ctx = (struct debug_ctx *)data;

	while (ctx->pos + l > BUF_LEN) {
		size_t n = BUF_LEN - ctx->pos;
		memcpy(ctx->buf + ctx->pos, s, n);
		s += n;
		l -= n;
		ctx->pos = BUF_LEN;
		debug_flush(ctx);
	}
	memcpy(ctx->buf + ctx->pos, s, l);
	ctx->pos += l;
}

static void vdebugf(const char *fmt, va_list ap) {
	struct debug_ctx ctx;
	ctx.pos = 0;
	vprintfmt(debug_output, &ctx, fmt, ap);
	debug_flush(&ctx);
}

void debugf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
}

void _user_panic(const char *file, int line, const char *fmt, ...) {
	debugf("panic at %s:%d: ", file, line);
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
	debugf("\n");
	exit();
}

void _user_halt(const char *file, int line, const char *fmt, ...) {
	debugf("halt at %s:%d: ", file, line);
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
	debugf("\n");
	syscall_panic("user halt");
}

void user_page_debug() {
	debugf("------------page------------\n");
	for (int va = 0; va < USTACKTOP; va += BY2PG) {
		if (vpd[va >> PDSHIFT] & PTE_V) {
			if (vpt[va >> PGSHIFT] & PTE_V) {
				u_long pa = PTE_ADDR(vpt[va >> PGSHIFT]);
				debugf("|%08x->%08x  ", va, pa);
				if (vpt[va >> PGSHIFT] & PTE_G) {
					debugf("G");
				} else {
					debugf(" ");
				}
				if (vpt[va >> PGSHIFT] & PTE_V) {
					debugf("V");
				} else {
					debugf(" ");
				}
				if (vpt[va >> PGSHIFT] & PTE_D) {
					debugf("D");
				} else {
					debugf(" ");
				}
				if (vpt[va >> PGSHIFT] & PTE_N) {
					debugf("N");
				} else {
					debugf(" ");
				}
				if (vpt[va >> PGSHIFT] & PTE_COW) {
					debugf("C");
				} else {
					debugf(" ");
				}
				if (vpt[va >> PGSHIFT] & PTE_LIBRARY) {
					debugf("L");
				} else {
					debugf(" ");
				}
				debugf("|\n");
			}
		}
	}
	debugf("----------------------------\n");
}

void user_page_debug_va(u_long va) {
	if (vpd[va >> PDSHIFT] & PTE_V) {
		if (vpt[va >> PGSHIFT] & PTE_V) {
			u_long pa = PTE_ADDR(vpt[va >> PGSHIFT]);
			debugf("----------------------------\n");
			debugf("|%08x->%08x  ", va, pa);
			if (vpt[va >> PGSHIFT] & PTE_G) {
				debugf("G");
			} else {
				debugf(" ");
			}
			if (vpt[va >> PGSHIFT] & PTE_V) {
				debugf("V");
			} else {
				debugf(" ");
			}
			if (vpt[va >> PGSHIFT] & PTE_D) {
				debugf("D");
			} else {
				debugf(" ");
			}
			if (vpt[va >> PGSHIFT] & PTE_N) {
				debugf("N");
			} else {
				debugf(" ");
			}
			if (vpt[va >> PGSHIFT] & PTE_COW) {
				debugf("C");
			} else {
				debugf(" ");
			}
			if (vpt[va >> PGSHIFT] & PTE_LIBRARY) {
				debugf("L");
			} else {
				debugf(" ");
			}
			debugf("|\n");
			debugf("----------------------------\n");
		} else {
			debugf("%08x invalid\n", va);
		}
	} else {
		debugf("%08x invalid\n", va);
	}
}

void debug_env() {
	debugf("-----------------------------------env------------------------------------\n");
	debugf("| id        status       parent    asid      pgdir     priority  index   |\n");
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
				debugf("%08x  ", e->env_pgdir);
			} else {
				debugf("          ");
			}
			debugf("%-8x  ", e->env_pri);
			debugf("%-8x|\n", e - envs);
		}
	}
	debugf("--------------------------------------------------------------------------\n");
}

void print_env(struct Env* e) {
	debugf("--------------------------------print env---------------------------------\n");
	if (!e) {
		debugf("|                                no env!                                 |\n");
		return;
	}
	if (e->env_id) {
		debugf("| id        status       parent    asid      pgdir     priority  index   |\n");
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
				debugf("%08x  ", e->env_pgdir);
			} else {
				debugf("          ");
			}
			debugf("%-8x  ", e->env_pri);
			debugf("%-8x|\n", e - envs);
	}
	debugf("--------------------------------------------------------------------------\n");
}

void print_tff(struct Trapframe *tf) {
	u_long zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, 
	s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra;
	u_long badva, sr, cause, epc;
	zero = tf->regs[0];
	at = tf->regs[1];
	v0 = tf->regs[2];
	v1 = tf->regs[3];
	a0 = tf->regs[4];
	a1 = tf->regs[5];
	a2 = tf->regs[6];
	a3 = tf->regs[7];
	t0 = tf->regs[8];
	t1 = tf->regs[9];
	t2 = tf->regs[10];
	t3 = tf->regs[11];
	t4 = tf->regs[12];
	t5 = tf->regs[13];
	t6 = tf->regs[14];
	t7 = tf->regs[15];
	s0 = tf->regs[16];
	s1 = tf->regs[17];
	s2 = tf->regs[18];
	s3 = tf->regs[19];
	s4 = tf->regs[20];
	s5 = tf->regs[21];
	s6 = tf->regs[22];
	s7 = tf->regs[23];
	t8 = tf->regs[24];
	t9 = tf->regs[25];
	k0 = tf->regs[26];
	k1 = tf->regs[27];
	gp = tf->regs[28];
	sp = tf->regs[29];
	fp = tf->regs[30];
	ra = tf->regs[31];
	badva = tf->cp0_badvaddr;
	sr = tf->cp0_status;
	cause = tf->cp0_cause;
	epc = tf->cp0_epc;

	debugf("------trapframe at %08x------\n"
		   "zero:%08x  at:  %08x  v0:  %08x  v1:  %08x\n"
	       "a0:  %08x  a1:  %08x  a2:  %08x  a3:  %08x\n"
	       "t0:  %08x  t1:  %08x  t2   %08x  t3:  %08x\n"
		   "t4:  %08x  t5:  %08x  t6   %08x  t7:  %08x\n"
	       "s0:  %08x  s1:  %08x  s2:  %08x  s3:  %08x\n"
		   "s4:  %08x  s5:  %08x  s6:  %08x  s7:  %08x\n"
		   "s8:  %08x  s9:  %08x  k0:  %08x  k1:  %08x\n"
		   "gp:  %08x  sp:  %08x  fp:  %08x  ra:  %08x\n"
	       "BadAddr: %08x  status:  %08x  cause:   %08x  epc: %08x\n",
		   tf, 
	       zero, at, v0, v1, a0, a1, a2, a3, t0, t1, t2, t3, t4, t5, t6, t7, 
		   s0, s1, s2, s3, s4, s5, s6, s7, t8, t9, k0, k1, gp, sp, fp, ra, 
		   badva, sr, cause, epc);
}