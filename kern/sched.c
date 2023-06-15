#include <env.h>
#include <pmap.h>
#include <printk.h>

/* Overview:
 *   Implement a round-robin scheduling to select a runnable env and schedule it using 'env_run'.
 *
 * Post-Condition:
 *   If 'yield' is set (non-zero), 'curenv' should not be scheduled again unless it is the only
 *   runnable env.
 *
 * Hints:
 *   1. The variable 'count' used for counting slices should be defined as 'static'.
 *   2. Use variable 'env_sched_list', which contains and only contains all runnable envs.
 *   3. You shouldn't use any 'return' statement because this function is 'noreturn'.
 */
// 00000000 80020000 8001e0dc 80055004
// 00000000 83fff7f4 00000008 803fff74
// 800148a0 80000000 00000046 00000000
// 00000046 00000000 00000000 00000000
// 00000000 00000000 00000000 00000000
// 00000000 00000000 00000000 00000000
// 00000000 00000000 00000001 00000000
// 00000000 803fff50 803fffe8 80011a60
// 10001004 00000013 00000001 00000000
// 00001000 80011a60
// ------regs at pc 80018da0, sched.c:20(schedule)------
// zero:00000000  at:  b5000000  v0:  b5000000  v1:  80055004
// a0:  00000000  a1:  83fff7f4  a2:  00000008  a3:  803fff74
// t0:  00001000  t1:  00001000  t2   10001004  t3:  00000000
// t4:  00000046  t5:  00000000  t6   00000000  t7:  00000000
// s0:  00000000  s1:  00000000  s2:  00000000  s3:  00000000
// s4:  00000000  s5:  00000000  s6:  00000000  s7:  00000000
// s8:  00000000  s9:  00000000  k0:  00000001  k1:  00000000
// gp:  00000000  sp:  803ffe88  fp:  803ffe88  ra:  80011a60
// index:   80000000  entryhi: 00000000  entrylo: 00000000
// BadAddr: 00000000  status:  10001004  cause:   00000000  epc: 80011a60

void schedule(int yield) {
	static int count = 0; // remaining time slices of current env
	struct Env *e = curenv;

	/* We always decrease the 'count' by 1.
	 *
	 * If 'yield' is set, or 'count' has been decreased to 0, or 'e' (previous 'curenv') is
	 * 'NULL', or 'e' is not runnable, then we pick up a new env from 'env_sched_list' (list of
	 * all runnable envs), set 'count' to its priority, and schedule it with 'env_run'. **Panic
	 * if that list is empty**.
	 *
	 * (Note that if 'e' is still a runnable env, we should move it to the tail of
	 * 'env_sched_list' before picking up another env from its head, or we will schedule the
	 * head env repeatedly.)
	 *
	 * Otherwise, we simply schedule 'e' again.
	 *
	 * You may want to use macros below:
	 *   'TAILQ_FIRST', 'TAILQ_REMOVE', 'TAILQ_INSERT_TAIL'
	 */
	/* Exercise 3.12: Your code here. */
	count--;
	// printk("count=%d\n", count);
	if (yield || !count || !e) {
		if (e) {
			TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
			if (e->env_status == ENV_RUNNABLE) {
				TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
			}
		}
		e = TAILQ_FIRST(&env_sched_list);
		// printk("%08x: pc=%08x\n", e->env_id, e->env_tf.cp0_epc);
		if (!e) {
			panic("schedule: no runnable envs");
		}
		count = e->env_pri;
	} else if (e->env_status != ENV_RUNNABLE) {
		TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
		if (e->env_status == ENV_RUNNABLE) {
			TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
		}
		e = TAILQ_FIRST(&env_sched_list);
		if (!e) {
			panic("schedule: no runnable envs");
		}
		count = e->env_pri;
	}
	env_run(e);

}
