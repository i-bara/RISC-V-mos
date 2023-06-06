#include <drivers/dev_mp.h>
#include <env.h>
#include <asm/sv39.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>

/* These variables are set by mips_detect_memory() */
u_long npage;	       /* Amount of memory(in pages) */

u_long cur_pgdir;

struct Page *pages;
static u_long freemem;

struct Page_list page_free_list; /* Free list of physical pages */

void mips_detect_memory() {

}

/* Overview:
 *   Read memory size from DEV_MP to initialize 'memsize' and calculate the corresponding 'npage'
 *   value.
 */
void page_init() {
	u_long r;
	asm volatile("mv %0, ra" : "=r"(r));
	printk("return %08x\n", r);

	npage = MEMORY_SIZE >> VPN0_SHIFT;

	printk("Memory size: %lu KiB, number of pages: %lu\n", MEMORY_SIZE / 1024, npage);

	pages = (struct Page *)alloc(npage * sizeof(struct Page), PAGE_SIZE, 1);
	
	printk("to memory %lx for struct Pages.\n", freemem);
	printk("pmap.c:\t mips vm init success\n");

	LIST_INIT(&page_free_list);

	freemem = ROUND(freemem, PAGE_SIZE);

	int i;
	for (i = 0; i < npage && i << VPN0_SHIFT < PADDR(freemem); i++) {
		pages[i].pp_ref = 1;
	}
	for (; i < npage; i++) {
		pages[i].pp_ref = 0;
		// printk("aaa %08lx %08lx->%016lx\n", pages, &pages[i], ((u_long)&pages[i] - (u_long)pages) / sizeof(struct Page));
		LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
	}
	
	
}

/* Overview:
    Allocate `n` bytes physical memory with alignment `align`, if `clear` is set, clear the
    allocated memory.
    This allocator is used only while setting up virtual memory system.
   Post-Condition:
    If we're out of memory, should panic, else return this address of memory we have allocated.*/
void *alloc(u_int n, u_int align, int clear) {
	extern char end[];
	u_long alloced_mem;

	/* Initialize `freemem` if this is the first time. The first virtual address that the
	 * linker did *not* assign to any kernel code or global variables. */
	if (freemem == 0) {
		freemem = (u_long)end + 0x00100000; // end
	}

	/* Step 1: Round up `freemem` up to be aligned properly */
	freemem = ROUND(freemem, align);

	/* Step 2: Save current value of `freemem` as allocated chunk. */
	alloced_mem = freemem;

	/* Step 3: Increase `freemem` to record allocation. */
	freemem = freemem + n;

	// Panic if we're out of memory.
	panic_on(PADDR(freemem) >= MEMORY_SIZE);

	/* Step 4: Clear allocated chunk if parameter `clear` is set. */
	if (clear) {
		memset((void *)alloced_mem, 0, n);
	}

	/* Step 5: return allocated chunk. */
	return (void *)alloced_mem;
}

/* Overview:
    Set up two-level page table.
   Hint:
    You can get more details about `UPAGES` and `UENVS` in include/mmu.h. */
void mips_vm_init() {
	/* Allocate proper size of physical memory for global array `pages`,
	 * for physical memory management. Then, map virtual address `UPAGES` to
	 * physical address `pages` allocated before. For consideration of alignment,
	 * you should round up the memory size before map. */
	
}

/* Overview:
 *   Allocate a physical page from free memory, and fill this page with zero.
 *
 * Post-Condition:
 *   If failed to allocate a new page (out of memory, there's no free page), return -E_NO_MEM.
 *   Otherwise, set the address of the allocated 'Page' to *pp, and return 0.
 *
 * Note:
 *   This does NOT increase the reference count 'pp_ref' of the page - the caller must do these if
 *   necessary (either explicitly or via page_insert).
 *
 * Hint: Use LIST_FIRST and LIST_REMOVE defined in include/queue.h.
 */
int page_alloc(struct Page **new) {
	/* Step 1: Get a page from free memory. If fails, return the error code.*/
	struct Page *pp;
	/* Exercise 2.4: Your code here. (1/2) */
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	pp = LIST_FIRST(&page_free_list);

	LIST_REMOVE(pp, pp_link);

	/* Step 2: Initialize this page with zero.
	 * Hint: use `memset`. */
	/* Exercise 2.4: Your code here. (2/2) */
	memset((void *)page2kva(pp), 0, PAGE_SIZE);

	*new = pp;
	return 0;
}

/* Overview:
 *   Release a page 'pp', mark it as free.
 *
 * Pre-Condition:
 *   'pp->pp_ref' is '0'.
 */
void page_free(struct Page *pp) {
	assert(pp->pp_ref == 0);
	/* Just insert it into 'page_free_list'. */
	/* Exercise 2.5: Your code here. */
	LIST_INSERT_HEAD(&page_free_list, pp, pp_link);

}

// /* Overview:
//  *   Given 'pgdir', a pointer to a page directory, 'pgdir_walk' returns a pointer to the page table
//  *   entry (with permission PTE_D|PTE_V) for virtual address 'va'.
//  *
//  * Pre-Condition:
//  *   'pgdir' is a two-level page table structure.
//  *
//  * Post-Condition:
//  *   If we're out of memory, return -E_NO_MEM.
//  *   Otherwise, we get the page table entry, store
//  *   the value of page table entry to *ppte, and return 0, indicating success.
//  *
//  * Hint:
//  *   We use a two-level pointer to store page table entry and return a state code to indicate
//  *   whether this function succeeds or not.
//  */
// static int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
// 	Pde *pgdir_entryp;
// 	struct Page *pp;

// 	/* Step 1: Get the corresponding page directory entry. */
// 	/* Exercise 2.6: Your code here. (1/3) */
// 	pgdir_entryp = pgdir + PDX(va);

// 	/* Step 2: If the corresponding page table is not existent (valid) and parameter `create`
// 	 * is set, create one. Set the permission bits 'PTE_D | PTE_V' for this new page in the
// 	 * page directory.
// 	 * If failed to allocate a new page (out of memory), return the error. */
// 	/* Exercise 2.6: Your code here. (2/3) */
// 	assert(ppte);
// 	Pte *pte;
// 	if (!(*pgdir_entryp & PTE_V)) {
// 		if (create) {
// 			try(page_alloc(&pp));
// 			*pgdir_entryp = page2pa(pp) | PTE_D | PTE_V;
// 			pp->pp_ref++;
// 			pte = (Pte *)page2kva(pp) + PTX(va);
// 		} else {
// 			if (ppte) {
// 				*ppte = NULL;
// 			}
// 			return 0;
// 		}
// 	} else {
// 		pte = (Pte *)KADDR(PTE2PA(*pgdir_entryp)) + PTX(va);
// 	}

// 	/* Step 3: Assign the kernel virtual address of the page table entry to '*ppte'. */
// 	/* Exercise 2.6: Your code here. (3/3) */
// 	if (ppte) {
// 		*ppte = pte;
// 	}

// 	return 0;
// }

// /* Overview:
//  *   Map the physical page 'pp' at virtual address 'va'. The permission (the low 12 bits) of the
//  *   page table entry should be set to 'perm|PTE_V'.
//  *
//  * Post-Condition:
//  *   Return 0 on success
//  *   Return -E_NO_MEM, if page table couldn't be allocated
//  *
//  * Hint:
//  *   If there is already a page mapped at `va`, call page_remove() to release this mapping.
//  *   The `pp_ref` should be incremented if the insertion succeeds.
//  */
// int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm) {
// 	Pte *pte;

// 	/* Step 1: Get corresponding page table entry. */
// 	pgdir_walk(pgdir, va, 0, &pte);

// 	if (pte && (*pte & PTE_V)) {
// 		if (pa2page(*pte) != pp) {
// 			page_remove(pgdir, asid, va);
// 		} else {
// 			tlb_invalidate(asid, va);
// 			*pte = page2pa(pp) | perm | PTE_V;
// 			return 0;
// 		}
// 	}

// 	/* Step 2: Flush TLB with 'tlb_invalidate'. */
// 	/* Exercise 2.7: Your code here. (1/3) */
//     tlb_invalidate(asid, va);

// 	/* Step 3: Re-get or create the page table entry. */
// 	/* If failed to create, return the error. */
// 	/* Exercise 2.7: Your code here. (2/3) */
// 	try(pgdir_walk(pgdir, va, 1, &pte));

// 	/* Step 4: Insert the page to the page table entry with 'perm | PTE_V' and increase its
// 	 * 'pp_ref'. */
// 	/* Exercise 2.7: Your code here. (3/3) */
// 	*pte = page2pa(pp) | perm | PTE_V;
// 	pp->pp_ref++;

// 	return 0;
// }

// /*Overview:
//     Look up the Page that virtual address `va` map to.
//   Post-Condition:
//     Return a pointer to corresponding Page, and store it's page table entry to *ppte.
//     If `va` doesn't mapped to any Page, return NULL.*/
// struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte) {
// 	struct Page *pp;
// 	Pte *pte;

// 	/* Step 1: Get the page table entry. */
// 	pgdir_walk(pgdir, va, 0, &pte);

// 	/* Hint: Check if the page table entry doesn't exist or is not valid. */
// 	if (pte == NULL || (*pte & PTE_V) == 0) {
// 		return NULL;
// 	}

// 	/* Step 2: Get the corresponding Page struct. */
// 	/* Hint: Use function `pa2page`, defined in include/pmap.h . */
// 	pp = pa2page(*pte);
// 	if (ppte) {
// 		*ppte = pte;
// 	}

// 	return pp;
// }

// /* Overview:
//  *   Decrease the 'pp_ref' value of Page 'pp'.
//  *   When there's no references (mapped virtual address) to this page, release it.
//  */
// void page_decref(struct Page *pp) {
// 	assert(pp->pp_ref > 0);

// 	/* If 'pp_ref' reaches to 0, free this page. */
// 	if (--pp->pp_ref == 0) {
// 		page_free(pp);
// 	}
// }

// // Overview:
// //   Unmap the physical page at virtual address 'va'.
// void page_remove(Pde *pgdir, u_int asid, u_long va) {
// 	Pte *pte;

// 	/* Step 1: Get the page table entry, and check if the page table entry is valid. */
// 	struct Page *pp = page_lookup(pgdir, va, &pte);
// 	if (pp == NULL) {
// 		return;
// 	}

// 	/* Step 2: Decrease reference count on 'pp'. */
// 	page_decref(pp);

// 	/* Step 3: Flush TLB. */
// 	*pte = 0;
// 	tlb_invalidate(asid, va);
// 	return;
// }

/* Overview:
 *   Invalidate the TLB entry with specified 'asid' and virtual address 'va'.

 *	 S 模式添加了另一条指令来解决这个问题。这条 sfence.vma 会通知处理器，软件可能已经修改了页表，
 *	 于是处理器可以相应地刷新转换缓存。它需要两个可选的参数，这样可以缩小缓存刷新的范围。
 *	 一个位于 rs1，它指示了页表哪个虚址对应的转换被修改了；
 *	 另一个位于 rs2，它给出了被修改页表的进程的地址空间标识符（ASID）。
 *	 如果两者都是 x0，便会刷新整个转换缓存。

 * Hint:
 *   Construct a new Entry HI and call 'tlb_out' to flush TLB.
 *   'tlb_out' is defined in mm/tlb_asm.S
 */
void tlb_invalidate(u_int asid, u_long va) {
    asm volatile("sfence.vma %0, %1" : : "r"(va), "r"(asid));
}

// void physical_memory_manage_check(void) {
// 	struct Page *pp, *pp0, *pp1, *pp2;
// 	struct Page_list fl;
// 	int *temp;

// 	// should be able to allocate three pages
// 	pp0 = pp1 = pp2 = 0;
// 	assert(page_alloc(&pp0) == 0);
// 	assert(page_alloc(&pp1) == 0);
// 	assert(page_alloc(&pp2) == 0);

// 	assert(pp0);
// 	assert(pp1 && pp1 != pp0);
// 	assert(pp2 && pp2 != pp1 && pp2 != pp0);

// 	// temporarily steal the rest of the free pages
// 	fl = page_free_list;
// 	// now this page_free list must be empty!!!!
// 	LIST_INIT(&page_free_list);
// 	// should be no free memory
// 	assert(page_alloc(&pp) == -E_NO_MEM);

// 	temp = (int *)page2kva(pp0);
// 	// write 1000 to pp0
// 	*temp = 1000;
// 	// free pp0
// 	page_free(pp0);
// 	printk("The number in address temp is %d\n", *temp);

// 	// alloc again
// 	assert(page_alloc(&pp0) == 0);
// 	assert(pp0);

// 	// pp0 should not change
// 	assert(temp == (int *)page2kva(pp0));
// 	// pp0 should be zero
// 	assert(*temp == 0);

// 	page_free_list = fl;
// 	page_free(pp0);
// 	page_free(pp1);
// 	page_free(pp2);
// 	struct Page_list test_free;
// 	struct Page *test_pages;
// 	test_pages = (struct Page *)alloc(10 * sizeof(struct Page), PAGE_SIZE, 1);
// 	LIST_INIT(&test_free);
// 	// LIST_FIRST(&test_free) = &test_pages[0];
// 	int i, j = 0;
// 	struct Page *p, *q;
// 	for (i = 9; i >= 0; i--) {
// 		test_pages[i].pp_ref = i;
// 		// test_pages[i].pp_link=NULL;
// 		// printk("0x%x  0x%x\n",&test_pages[i], test_pages[i].pp_link.le_next);
// 		LIST_INSERT_HEAD(&test_free, &test_pages[i], pp_link);
// 		// printk("0x%x  0x%x\n",&test_pages[i], test_pages[i].pp_link.le_next);
// 	}
// 	p = LIST_FIRST(&test_free);
// 	int answer1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
// 	assert(p != NULL);
// 	while (p != NULL) {
// 		// printk("%d %d\n",p->pp_ref,answer1[j]);
// 		assert(p->pp_ref == answer1[j++]);
// 		// printk("ptr: 0x%x v: %d\n",(p->pp_link).le_next,((p->pp_link).le_next)->pp_ref);
// 		p = LIST_NEXT(p, pp_link);
// 	}
// 	// insert_after test
// 	int answer2[] = {0, 1, 2, 3, 4, 20, 5, 6, 7, 8, 9};
// 	q = (struct Page *)alloc(sizeof(struct Page), PAGE_SIZE, 1);
// 	q->pp_ref = 20;

// 	// printk("---%d\n",test_pages[4].pp_ref);
// 	LIST_INSERT_AFTER(&test_pages[4], q, pp_link);
// 	// printk("---%d\n",LIST_NEXT(&test_pages[4],pp_link)->pp_ref);
// 	p = LIST_FIRST(&test_free);
// 	j = 0;
// 	// printk("into test\n");
// 	while (p != NULL) {
// 		//      printk("%d %d\n",p->pp_ref,answer2[j]);
// 		assert(p->pp_ref == answer2[j++]);
// 		p = LIST_NEXT(p, pp_link);
// 	}

// 	printk("physical_memory_manage_check() succeeded\n");
// }

// void page_check(void) {
// 	Pde *boot_pgdir = alloc(PAGE_SIZE, PAGE_SIZE, 1);
// 	struct Page *pp, *pp0, *pp1, *pp2;
// 	struct Page_list fl;

// 	// should be able to allocate three pages
// 	pp0 = pp1 = pp2 = 0;
// 	assert(page_alloc(&pp0) == 0);
// 	assert(page_alloc(&pp1) == 0);
// 	assert(page_alloc(&pp2) == 0);

// 	assert(pp0);
// 	assert(pp1 && pp1 != pp0);
// 	assert(pp2 && pp2 != pp1 && pp2 != pp0);

// 	// temporarily steal the rest of the free pages
// 	fl = page_free_list;
// 	// now this page_free list must be empty!!!!
// 	LIST_INIT(&page_free_list);

// 	// should be no free memory
// 	assert(page_alloc(&pp) == -E_NO_MEM);

// 	// there is no free memory, so we can't allocate a page table
// 	assert(page_insert(boot_pgdir, 0, pp1, 0x0, 0) < 0);

// 	// free pp0 and try again: pp0 should be used for page table
// 	page_free(pp0);
// 	assert(page_insert(boot_pgdir, 0, pp1, 0x0, 0) == 0);
// 	assert(PTE2PA(boot_pgdir[0]) == page2pa(pp0));

// 	printk("va2pa(boot_pgdir, 0x0) is %x\n", va2pa(boot_pgdir, 0x0));
// 	printk("page2pa(pp1) is %x\n", page2pa(pp1));
// 	//	printk("pp1->pp_ref is %d\n",pp1->pp_ref);
// 	assert(va2pa(boot_pgdir, 0x0) == page2pa(pp1));
// 	assert(pp1->pp_ref == 1);

// 	// should be able to map pp2 at PAGE_SIZE because pp0 is already allocated for page table
// 	assert(page_insert(boot_pgdir, 0, pp2, PAGE_SIZE, 0) == 0);
// 	assert(va2pa(boot_pgdir, PAGE_SIZE) == page2pa(pp2));
// 	assert(pp2->pp_ref == 1);

// 	// should be no free memory
// 	assert(page_alloc(&pp) == -E_NO_MEM);

// 	printk("start page_insert\n");
// 	// should be able to map pp2 at PAGE_SIZE because it's already there
// 	assert(page_insert(boot_pgdir, 0, pp2, PAGE_SIZE, 0) == 0);
// 	assert(va2pa(boot_pgdir, PAGE_SIZE) == page2pa(pp2));
// 	assert(pp2->pp_ref == 1);

// 	// pp2 should NOT be on the free list
// 	// could happen in ref counts are handled sloppily in page_insert
// 	assert(page_alloc(&pp) == -E_NO_MEM);

// 	// should not be able to map at PDMAP because need free page for page table
// 	assert(page_insert(boot_pgdir, 0, pp0, PDMAP, 0) < 0);

// 	// insert pp1 at PAGE_SIZE (replacing pp2)
// 	assert(page_insert(boot_pgdir, 0, pp1, PAGE_SIZE, 0) == 0);

// 	// should have pp1 at both 0 and PAGE_SIZE, pp2 nowhere, ...
// 	assert(va2pa(boot_pgdir, 0x0) == page2pa(pp1));
// 	assert(va2pa(boot_pgdir, PAGE_SIZE) == page2pa(pp1));
// 	// ... and ref counts should reflect this
// 	assert(pp1->pp_ref == 2);
// 	printk("pp2->pp_ref %d\n", pp2->pp_ref);
// 	assert(pp2->pp_ref == 0);
// 	printk("end page_insert\n");

// 	// pp2 should be returned by page_alloc
// 	assert(page_alloc(&pp) == 0 && pp == pp2);

// 	// unmapping pp1 at 0 should keep pp1 at PAGE_SIZE
// 	page_remove(boot_pgdir, 0, 0x0);
// 	assert(va2pa(boot_pgdir, 0x0) == ~0);
// 	assert(va2pa(boot_pgdir, PAGE_SIZE) == page2pa(pp1));
// 	assert(pp1->pp_ref == 1);
// 	assert(pp2->pp_ref == 0);

// 	// unmapping pp1 at PAGE_SIZE should free it
// 	page_remove(boot_pgdir, 0, PAGE_SIZE);
// 	assert(va2pa(boot_pgdir, 0x0) == ~0);
// 	assert(va2pa(boot_pgdir, PAGE_SIZE) == ~0);
// 	assert(pp1->pp_ref == 0);
// 	assert(pp2->pp_ref == 0);

// 	// so it should be returned by page_alloc
// 	assert(page_alloc(&pp) == 0 && pp == pp1);

// 	// should be no free memory
// 	assert(page_alloc(&pp) == -E_NO_MEM);

// 	// forcibly take pp0 back
// 	assert(PTE2PA(boot_pgdir[0]) == page2pa(pp0));
// 	boot_pgdir[0] = 0;
// 	assert(pp0->pp_ref == 1);
// 	pp0->pp_ref = 0;

// 	// give free list back
// 	page_free_list = fl;

// 	// free the pages we took
// 	page_free(pp0);
// 	page_free(pp1);
// 	page_free(pp2);

// 	printk("page_check() succeeded!\n");
// }

static void _debug_page(u_long va, u_long pte) {
	printk("|%016lx->%016lx  ", va, PTE2PA(pte));
	if (pte & PTE_R) {
		printk("R");
	} else {
		printk(" ");
	}
	if (pte & PTE_W) {
		printk("W");
	} else {
		printk(" ");
	}
	if (pte & PTE_X) {
		printk("X");
	} else {
		printk(" ");
	}
	if (pte & PTE_U) {
		printk("U");
	} else {
		printk(" ");
	}
	if (pte & PTE_G) {
		printk("G");
	} else {
		printk(" ");
	}
	if (pte & PTE_A) {
		printk("A");
	} else {
		printk(" ");
	}
	if (pte & PTE_D) {
		printk("D");
	} else {
		printk(" ");
	}
	if (pte & PTE_RSW1) {
		printk("C");
	} else {
		printk(" ");
	}
	if (pte & PTE_RSW2) {
		printk("L");
	} else {
		printk(" ");
	}
	printk("|\n");
}

void debug_page(u_long *pgdir) {
	printk("---------------------page----------------------\n");
	for (u_long vpn2 = 0; vpn2 < PAGE_SIZE / sizeof(u_long); vpn2++) {
		u_long *pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			for (u_long vpn1 = 0; vpn1 < PAGE_SIZE / sizeof(u_long); vpn1++) {
				u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

				if (*pte1 & PTE_V) {
					for (u_long vpn0 = 0; vpn0 < PAGE_SIZE / sizeof(u_long); vpn0++) {
						u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

						if (*pte0 & PTE_V) {
							u_long va = vpn2 << VPN2_SHIFT | vpn1 << VPN1_SHIFT | vpn0 << VPN0_SHIFT;
							_debug_page(va, *pte0);

						}
					}
				}
			}
		}
	}
	printk("-----------------------------------------------\n");
}

void debug_page_user(u_long *pgdir) {
	printk("---------------------page----------------------\n");
	for (u_long vpn2 = 0; vpn2 < PAGE_SIZE / sizeof(u_long); vpn2++) {
		u_long *pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			for (u_long vpn1 = 0; vpn1 < PAGE_SIZE / sizeof(u_long); vpn1++) {
				u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

				if (*pte1 & PTE_V) {
					for (u_long vpn0 = 0; vpn0 < PAGE_SIZE / sizeof(u_long); vpn0++) {
						u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

						if ((*pte0 & PTE_V) && (*pte0 & PTE_U)) {
							u_long va = vpn2 << VPN2_SHIFT | vpn1 << VPN1_SHIFT | vpn0 << VPN0_SHIFT;
							_debug_page(va, *pte0);

						}
					}
				}
			}
		}
	}
	printk("-----------------------------------------------\n");
}

void debug_page_va(u_long *pgdir, u_long va) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

			if (*pte0 & PTE_V) {
				_debug_page(va, *pte0);

			} else {
				printk("%016lx invalid\n", va);
			}
		} else {
			printk("%016lx invalid\n", va);
		}
	} else {
		printk("%016lx invalid\n", va);
	}
}

u_long debug_pte(u_long *pgdir, u_long va) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);
	printk("%3lx: %3lx: %3lx\n", vpn2, vpn1, vpn0);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];
	printk("pte2 = %016lx: %016lx\n", pte2, *pte2);

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];
		printk("pte1 = %016lx: %016lx\n", pte1, *pte1);

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];
			printk("pte0 = %016lx: %016lx\n", pte0, *pte0);

			if (*pte0 & PTE_V) {
				return *pte0;

			} else {
				printk("invalid\n");
			}
		} else {
			printk("invalid\n");
		}
	} else {
		printk("invalid\n");
	}
}

u_long get_pa(u_long *pgdir, u_long va) {
	// u_long *pte;
	// u_long pgdir2;
	// pte = &((u_long *)*pgdir)[VPN2(va)];
	// printk("kkk pte = %016lx\n", *pte);
	// if (*pte & PTE_V) {
	// 	pgdir2 = (u_long)PTE2PA(*pte);
	// }
	// pte = &((u_long *)pgdir2)[VPN1(va)];
	// printk("kkk pte = %016lx\n", *pte);
	// if (*pte & PTE_V) {
	// 	pgdir2 = (u_long)PTE2PA(*pte);
	// }
	// pte = &((u_long *)pgdir2)[VPN0(va)];
	// printk("kkk pte = %016lx\n", *pte);

	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);
	// printk("%016lx: %016lx: %016lx\n", vpn2, vpn1, vpn0);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];
	if (va==0x3ff000) printk("pte2 = %016lx: %016lx\n", pte2, *pte2);

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];
		if (va==0x3ff000) printk("pte1 = %016lx: %016lx\n", pte1, *pte1);

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];
			if (va==0x3ff000) printk("pte0 = %016lx: %016lx\n", pte0, *pte0);

			if (*pte0 & PTE_V) {
				return PTE2PA(*pte0) | va & PTE_OFFSET;

			} else {
				return -1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

u_long get_perm(u_long *pgdir, u_long va) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

			if (*pte0 & PTE_V) {
				return PTE2PERM(*pte0);

			} else {
				return -1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

void set_pa(u_long *pgdir, u_long va, u_long pa) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

			if (*pte0 & PTE_V) {
				u_long perm = PTE2PERM(*pte0);
				*pte0 = PA2PTE(pa) | perm;

			}
		}
	}
}

void set_perm(u_long *pgdir, u_long va, u_long perm) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2 = &((u_long *)*pgdir)[vpn2];

	if (*pte2 & PTE_V) {
		u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

		if (*pte1 & PTE_V) {
			u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

			if (*pte0 & PTE_V) {
				u_long pa = PTE2PA(*pte0);
				*pte0 = PA2PTE(pa) | perm;

			}
		}
	}
}

int alloc_page(u_long *pgdir, u_int asid, u_long va, u_int perm) {
	if (perm >= 0x400) {
		panic("invalid perm");
	}
	
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					u_long original_pa = PTE2PA(*pte0);
					// clear
					if (--pa2page(original_pa)->pp_ref == 0) {
						page_free(pa2page(original_pa));
					}
					goto map;
				} else {
					goto map;
				}
			} else {
				goto create_pte0;
			}
		} else {
			goto create_pte1;
		}
	} else {
		goto create_pte2;
	}

	struct Page *pp;

create_pte2:
	try(page_alloc(&pp));
	*pgdir = page2pa(pp);
	pp->pp_ref++;

	pte2 = &((u_long *)*pgdir)[vpn2];

create_pte1:
	try(page_alloc(&pp));
	*pte2 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;

	pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

create_pte0:
	try(page_alloc(&pp));
	*pte1 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;
	
	pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

map:
	tlb_invalidate(asid, va);
	try(page_alloc(&pp));
	*pte0 = PA2PTE(page2pa(pp)) | perm | PTE_V;
	pp->pp_ref++;

	// printk("alloc %x\n", va, curenv->env_id);
	// debug_page_va(pgdir, va);

	return 0;
}

int map_page(u_long *pgdir, u_int asid, u_long va, u_long pa, u_int perm) {
	if (perm >= 0x400) {
		panic("invalid perm");
	}

	if (pa < KERNBASE || pa >= KERNBASE + MEMORY_SIZE) { 
		return -E_INVAL;
	}
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					u_long original_pa = PTE2PA(*pte0);
					if (original_pa == PTE2PA(PA2PTE(pa))) {
						// add perm
						tlb_invalidate(asid, va);
						*pte0 = PA2PTE(pa) | perm | PTE_V;
						return 0;
					} else {
						// clear
						if (--pa2page(original_pa)->pp_ref == 0) {
							page_free(pa2page(original_pa));
						}
						goto map;
					}
				} else {
					goto map;
				}
			} else {
				goto create_pte0;
			}
		} else {
			goto create_pte1;
		}
	} else {
		goto create_pte2;
	}

	struct Page *pp;

create_pte2:
	try(page_alloc(&pp));
	*pgdir = page2pa(pp);
	pp->pp_ref++;

	pte2 = &((u_long *)*pgdir)[vpn2];

create_pte1:
	try(page_alloc(&pp));
	*pte2 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;

	pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

create_pte0:
	try(page_alloc(&pp));
	*pte1 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;
	
	pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

map:
	tlb_invalidate(asid, va);
	*pte0 = PA2PTE(pa) | perm | PTE_V;
	pa2page(pa)->pp_ref++;

	return 0;
}

int alloc_page_user(u_long *pgdir, u_int asid, u_long va, u_int perm) {
	if (perm >= 0x400) {
		panic("invalid perm");
	}

	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					u_long original_pa = PTE2PA(*pte0);
					// clear
					if (--pa2page(original_pa)->pp_ref == 0) {
						page_free(pa2page(original_pa));
					}
					goto map;
				} else {
					goto map;
				}
			} else {
				goto create_pte0;
			}
		} else {
			goto create_pte1;
		}
	} else {
		goto create_pte2;
	}

	struct Page *pp;

create_pte2:
	try(page_alloc(&pp));
	*pgdir = page2pa(pp);
	pp->pp_ref++;

	pte2 = &((u_long *)*pgdir)[vpn2];

	map_page(pgdir, asid, PAGE_TABLE + (PAGE_TABLE >> 9) + (PAGE_TABLE >> 18) + (va >> 27), (u_long)pte2, PTE_R | PTE_U);

create_pte1:
	try(page_alloc(&pp));
	*pte2 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;

	pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

	map_page(pgdir, asid, PAGE_TABLE + (PAGE_TABLE >> 9) + (va >> 18), (u_long)pte1, PTE_R | PTE_U);

create_pte0:
	try(page_alloc(&pp));
	*pte1 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;
	
	pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

	// 建立自映射
	// printk("self mapping %016lx->%016lx\n", PAGE_TABLE + (va >> 9), (u_long)pte0);
	map_page(pgdir, asid, PAGE_TABLE + (va >> 9), (u_long)pte0, PTE_R | PTE_U);
	get_pa(pgdir, va);
	// debug_page(pgdir);
	// printk("%016lx: %016lx->%016lx\n", *pgdir, PAGE_TABLE + (va >> 9), get_pa(pgdir, PAGE_TABLE + (va >> 9)));

map:
	tlb_invalidate(asid, va);
	try(page_alloc(&pp));
	
	*pte0 = PA2PTE(page2pa(pp)) | perm | PTE_V;
	pp->pp_ref++;

	// if (curenv) {
	// 	printk("alloc in env %x\n", curenv->env_id);
	// } else {
	// 	printk("alloc in kernel\n");
	// }
	
	debug_page_va(pgdir, va);

	return 0;
}

int map_page_user(u_long *pgdir, u_int asid, u_long va, u_long pa, u_int perm) {
	if (perm >= 0x400) {
		panic("invalid perm");
	}

	if (pa < KERNBASE || pa >= KERNBASE + MEMORY_SIZE) { 
		return -E_INVAL;
	}
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					u_long original_pa = PTE2PA(*pte0);
					if (original_pa == PTE2PA(PA2PTE(pa))) {
						// add perm
						tlb_invalidate(asid, va);
						*pte0 = PA2PTE(pa) | perm | PTE_V;
						return 0;
					} else {
						// clear
						if (--pa2page(original_pa)->pp_ref == 0) {
							page_free(pa2page(original_pa));
						}
						goto map;
					}
				} else {
					goto map;
				}
			} else {
				goto create_pte0;
			}
		} else {
			goto create_pte1;
		}
	} else {
		goto create_pte2;
	}

	struct Page *pp;

create_pte2:
	try(page_alloc(&pp));
	*pgdir = page2pa(pp);
	pp->pp_ref++;

	pte2 = &((u_long *)*pgdir)[vpn2];

	map_page(pgdir, asid, PAGE_TABLE + (PAGE_TABLE >> 9) + (PAGE_TABLE >> 18) + (va >> 27), (u_long)pte2, PTE_R | PTE_U);

create_pte1:
	try(page_alloc(&pp));
	*pte2 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;

	pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

	map_page(pgdir, asid, PAGE_TABLE + (PAGE_TABLE >> 9) + (va >> 18), (u_long)pte1, PTE_R | PTE_U);

create_pte0:
	try(page_alloc(&pp));
	*pte1 = PA2PTE(page2pa(pp)) | PTE_V;
	pp->pp_ref++;
	
	pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

	// 建立自映射
	// printk("%016lx->%016lx\n", PAGE_TABLE + (va >> 9), (u_long)pte0);
	map_page(pgdir, asid, PAGE_TABLE + (va >> 9), (u_long)pte0, PTE_R | PTE_U);
	// debug_page(pgdir);
	// printk("%016lx: %016lx->%016lx\n", *pgdir, PAGE_TABLE + (va >> 9), get_pa(pgdir, PAGE_TABLE + (va >> 9)));

map:
	tlb_invalidate(asid, va);
	*pte0 = PA2PTE(pa) | perm | PTE_V;
	pa2page(pa)->pp_ref++;

	return 0;
}

int unmap_page(Pde *pgdir, u_int asid, u_long va) {
	if (va >= KERNBASE && va < KERNBASE + MEMORY_SIZE) {
		panic("nyan");
	}
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					u_long original_pa = PTE2PA(*pte0);

					// clear
					if (--pa2page(original_pa)->pp_ref == 0) {
						page_free(pa2page(original_pa));
					}

					// unmap(map at 0L)
					tlb_invalidate(asid, va);
					*pte0 = 0L;
				}
			}
		}
	}

	return 0;
}

int is_mapped_page(Pde *pgdir, u_long va) {
	u_long vpn0 = VPN0(va);
	u_long vpn1 = VPN1(va);
	u_long vpn2 = VPN2(va);

	u_long *pte2;
	u_long *pte1;
	u_long *pte0;

	if (*pgdir) {
		pte2 = &((u_long *)*pgdir)[vpn2];

		if (*pte2 & PTE_V) {
			pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

			if (*pte1 & PTE_V) {
				pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

				if (*pte0 & PTE_V) {
					return 1;
				} else {
					return 0;
				}
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

// int create_pd(Pde **pgdir) {
// 	struct Page *pp;
// 	try(page_alloc(&pp));
// 	*pgdir = (Pde *)page2kva(pp);
// 	pp->pp_ref++;
// 	return 0;
// }

// int create_pt(Pde *pgdir, u_long pdx) {
// 	struct Page *pp;
// 	try(page_alloc(&pp));
// 	pgdir[pdx] = page2pa(pp) | PTE_D | PTE_V;
// 	pp->pp_ref++;
// 	return 0;
// }

int alloc_pgdir(u_long *pgdir) {
	struct Page *p;
	try(page_alloc(&p));
	p->pp_ref++;
	*pgdir = page2pa(p);
}

int destroy_pgdir(u_long *pgdir, u_int asid) {
	if (*pgdir) {
		for (u_long vpn2 = 0; vpn2 < PAGE_SIZE / sizeof(u_long); vpn2++) {
			u_long *pte2 = &((u_long *)*pgdir)[vpn2];
			if (vpn2 == 2 || vpn2 == PENVS) {
				continue; // 两种映射形式！巨页映射应该巨页销毁！
			} 

			if (*pte2 & PTE_V) {
				for (u_long vpn1 = 0; vpn1 < PAGE_SIZE / sizeof(u_long); vpn1++) {
					u_long *pte1 = &((u_long *)PTE2PA(*pte2))[vpn1];

					if (*pte1 & PTE_V) {
						for (u_long vpn0 = 0; vpn0 < PAGE_SIZE / sizeof(u_long); vpn0++) {
							u_long *pte0 = &((u_long *)PTE2PA(*pte1))[vpn0];

							if (*pte0 & PTE_V) {
								u_long va = vpn2 << VPN2_SHIFT | vpn1 << VPN1_SHIFT | vpn0 << VPN0_SHIFT;
								u_long pa = PTE2PA(*pte0);
								u_long perm = *pte0 & PTE_PERM;

								// clear
								if (--pa2page(pa)->pp_ref == 0) {
									page_free(pa2page(pa));
								}

								// unmap
								tlb_invalidate(asid, va);
								*pte0 = 0L;
							}
						}
						u_long va = PPT << VPN2_SHIFT | vpn2 << VPN1_SHIFT | vpn1 << VPN0_SHIFT;
						u_long pa = PTE2PA(*pte1);

						// clear
						if (--pa2page(pa)->pp_ref == 0) {
							page_free(pa2page(pa));
						}

						// unmap
						tlb_invalidate(asid, va);
						*pte1 = 0L;
					}
				}
				u_long va = PPT << VPN2_SHIFT | PPT << VPN1_SHIFT | vpn2 << VPN0_SHIFT;
				u_long pa = PTE2PA(*pte2);

				// clear
				if (--pa2page(pa)->pp_ref == 0) {
					page_free(pa2page(pa));
				}

				// unmap
				tlb_invalidate(asid, va);
				*pte2 = 0L;
			}
		}
		u_long va = PPT << VPN2_SHIFT | PPT << VPN1_SHIFT | PPT << VPN0_SHIFT;
		u_long pa = *pgdir;

		// clear
		if (--pa2page(pa)->pp_ref == 0) {
			page_free(pa2page(pa));
		}

		// unmap
		tlb_invalidate(asid, va);
		*pgdir = 0L;
	}
	return 0;
}

// void set(Pde *pgdir, u_long va, u_long value) {
// 	u_long pa = get_pa(pgdir, va);
// 	pa = pa | va & 0x00000fff;
// 	u_long kva = KADDR(pa);
// 	*(u_long *)kva = value;
// }

u_long get(u_long *pgdir, u_long va) {
	u_long pa = get_pa(pgdir, va);
	return *(u_long *)pa;
}