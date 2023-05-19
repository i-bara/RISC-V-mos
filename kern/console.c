#include <drivers/dev_cons.h>
#include <asm/sbi.h>
#include <mmu.h>

void printcharc(char ch) {
	sbi_console_putchar(ch);
}

char scancharc(void) {
	return (char) sbi_console_getchar();
}

void halt(void) {
	sbi_shutdown();
}
