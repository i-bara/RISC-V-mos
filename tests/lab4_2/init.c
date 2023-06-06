void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	page_init();

	extern char exc_gen_entry[];
	asm volatile("csrw stvec, %0" : : "r"(exc_gen_entry));

	env_init();

	struct Env *ppb = ENV_CREATE_PRIORITY(test_ppb, 5);
	struct Env *ppc = ENV_CREATE_PRIORITY(test_ppc, 5);
	ppc->env_parent_id = ppb->env_id;

	u_long sie;
	asm volatile("csrr %0, sie" : "=r"(sie));
	printk("%016lx\n", sie);

	int r = sbi_set_timer(10000000);
	asm volatile("csrs sie, %0" : : "r"(SIE_STIE));
	asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SIE));

	while (1) {
	}
	panic("init.c:\tend of mips_init() reached!");
}
