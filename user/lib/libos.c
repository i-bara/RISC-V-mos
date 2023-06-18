#include <env.h>
#include <lib.h>
#include <mmu.h>

void exit(void) {
	// After fs is ready (lab5), all our open files should be closed before dying.
#if !defined(LAB) || LAB >= 5
	close_all();
#endif

	syscall_env_destroy(0);
	user_panic("unreachable code");
}

volatile struct Env *env;
extern int main(int, char **);

void libmain(int argc, char **argv) {
	// #ifdef RISCV32
	// debugf("pte000 = %08x\n", pt1[0x5fc00000 >> VPN1_SHIFT]);
	// debugf("pte000 = %08x\n", pt0[0x5fc00000 >> VPN0_SHIFT]);
	// #else
	// debugf("pte000 = %08x\n", pt2[0x00400000 >> VPN2_SHIFT]);
	// debugf("pte000 = %08x\n", pt1[0x00400000 >> VPN1_SHIFT]);
	// debugf("pte000 = %08x\n", pt0[0x00400000 >> VPN0_SHIFT]);
	// #endif
	// debugf("pte000 = %08x(%08x)\n", ((u_long *)0xc0002000)[0], PTE2PA(((u_long *)0xc0002000)[0]));
	// debugf("pte000 = %08x(%08x)\n", ((u_long *)0xc0002000)[1], PTE2PA(((u_long *)0xc0002000)[1]));
	// debugf("pte000 = %08x(%08x)\n", ((u_long *)0xc0002000)[2], PTE2PA(((u_long *)0xc0002000)[2]));
	// debugf("pte000 = %08x(%08x)\n", ((u_long *)0xc0002000)[3], PTE2PA(((u_long *)0xc0002000)[3]));
	// debugf("pte000 = %08x\n", pt2[0x5fe00000 >> VPN2_SHIFT]);
	// debugf("pte000 = %08x\n", pt1[0x5fe00000 >> VPN1_SHIFT]);
	// debugf("pte000 = %08x\n", pt0[0x5fe00000 >> VPN0_SHIFT]);
	// set env to point at our env structure in envs[].
	env = &envs[ENVX(syscall_getenvid())];

	#ifdef DEBUG
	#if (DEBUG >= 2)
	debugf("%x: Hello, world!\n", env->env_id);
	user_debug_page_user();
	#endif
	#endif

	// call user main routine
	main(argc, argv);

	// exit gracefully
	exit();
}
