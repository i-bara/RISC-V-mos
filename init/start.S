#include <asm/asm.h>
#include <mmu.h>

.text
EXPORT(_start)
# .set at
# .set reorder
	/* disable interrupts */
	# li		t0, 0x00000008
	# csrw    mstatus, t0

	/* hint: you can reference the memory layout in include/mmu.h */
	/* set up the kernel stack */
	/* Exercise 1.3: Your code here. (1/2) */
	li		x2, 	0x80800000
	

	/* jump to mips_init */
	/* Exercise 1.3: Your code here. (2/2) */
	jal mips_init
