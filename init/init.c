#include <asm/asm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <sbi.h>

// When build with 'make test lab=?_?', we will replace your 'mips_init' with a generated one from
// 'tests/lab?_?'.
#ifdef MOS_INIT_OVERRIDDEN
#include <generated/init_override.h>
#else

#define MOS_SCHED_MAX_TICKS 100
#define MOS_SCHED_END_PC 0x500000

void mips_init() {
	printk("\n\ninit.c:\tmips_init() is called\n");

	// lab2:
	// mips_detect_memory();
	// mips_vm_init();
	page_init();


	extern char exc_gen_entry[];
	asm volatile("csrw stvec, %0" : : "r"(exc_gen_entry));

	// // lab3:
	env_init();

	// // lab3:
	ENV_CREATE_PRIORITY(user_bare_loop, 1);
	ENV_CREATE_PRIORITY(user_bare_loop, 2);

	// ENV_CREATE_PRIORITY(user_bare_overflow, 3);
	// ENV_CREATE_PRIORITY(user_bare_overflow, 4);

	// ENV_CREATE_PRIORITY(test_loop, 1); ENV_CREATE_PRIORITY(test_loop, 2);
	// ENV_CREATE_PRIORITY(test_quick_sort, 1); ENV_CREATE_PRIORITY(test_quick_sort, 2); ENV_CREATE_PRIORITY(test_quick_sort, 3);

	// lab4:
	// ENV_CREATE(user_tltest);
	// ENV_CREATE(user_fktest);
	// ENV_CREATE(user_pingpong);

	// lab6:
	// ENV_CREATE(user_icode);  // This must be the first env!

	// lab5:
	// ENV_CREATE(user_fstest);
	// ENV_CREATE(fs_serv);  // This must be the second env!
	// ENV_CREATE(user_devtst);

	// lab3:
	// kclock_init();
	// enable_irq();
	
	u_long sie;
	asm volatile("csrr %0, sie" : "=r"(sie));
	printk("%016lx\n", sie);

	
	int r = sbi_set_timer(50000000L);
	asm volatile("csrs sie, %0" : : "r"(SIE_STIE));
	asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SIE));

	// page_check();

	while (1) {
	}
}

void page_check() {
	printk("nyan!!!\n");
	u_long pgdir = 0;
	map_page(&pgdir, 0, 0x10000000, 0x80000000, 0);
	map_page(&pgdir, 0, 0x10000001, 0x80000002, 0);
	map_page(&pgdir, 0, 0x10100000, 0x80100000, 0);
	map_page(&pgdir, 0, 0x00000000, 0x0f0ff0000000L, 0);
	map_page(&pgdir, 0, 0x111111fff0000000L, 0x80000000, 0);
	map_page(&pgdir, 0, 0x10000000, 0x80000000, 0);
	alloc_page(&pgdir, 0, 0x10030000, 0);
	debug_page(&pgdir);
	alloc_page(&pgdir, 0, 0x10030000, 0);
	debug_page(&pgdir);
	alloc_page(&pgdir, 0, 0x10030000, 0);
	debug_page(&pgdir);
	map_page(&pgdir, 0, 0x10030000, 0x83003031, 0);
	debug_page(&pgdir);
	unmap_page(&pgdir, 0, 0x10030000);
	debug_page(&pgdir);
	
	printk("nyan\n");
	printk("to pa %016lx\n", get_pa(&pgdir, 0x10000000));

	
	printk("to pa %016lx\n", get_pa(&pgdir, 0x10000000));
}

#endif
