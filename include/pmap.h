#ifndef _PMAP_H_
#define _PMAP_H_

#include <asm/sv39.h>
#include <mmu.h>
#include <printk.h>
#include <queue.h>
#include <types.h>

extern Pde *cur_pgdir;

LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

struct Page {
	Page_LIST_entry_t pp_link; /* free list link */

	// Ref is the count of pointers (usually in page table entries)
	// to this page.  This only holds for pages allocated using
	// page_alloc.  Pages allocated at boot time using pmap.c's "alloc"
	// do not have valid reference count fields.

	u_short pp_ref;
};

extern struct Page *pages;
extern struct Page_list page_free_list;

#define pa2page(pa) (&pages[(pa - 0x80000000) >> VPN0_SHIFT])
#define page2pa(pp) (((((u_long)pp - (u_long)pages) / sizeof(struct Page)) << VPN0_SHIFT) + 0x80000000)
#define page2kva(pp) page2pa(pp)

// static inline u_long page2ppn(struct Page *pp) {
// 	return ((u_long)pp - (u_long)pages) / sizeof(struct Page);
// }

// static inline u_long page2pa(struct Page *pp) {
// 	return page2ppn(pp) << PGSHIFT;
// }

// static inline struct Page *pa2page(u_long pa) {
// 	if (PPN(pa) >= npage) {
// 		panic("pa2page called with invalid pa: %x", pa);
// 	}
// 	return &pages[PPN(pa)];
// }

// static inline u_long page2kva(struct Page *pp) {
// 	return KADDR(page2pa(pp));
// }

// static inline u_long va2pa(Pde *pgdir, u_long va) {
// 	Pte *p;

// 	pgdir = &pgdir[PDX(va)];
// 	if (!(*pgdir & PTE_V)) {
// 		return ~0;
// 	}
// 	p = (Pte *)KADDR(PTE_ADDR(*pgdir));
// 	if (!(p[PTX(va)] & PTE_V)) {
// 		return ~0;
// 	}
// 	return PTE_ADDR(p[PTX(va)]);
// }

void mips_detect_memory(void);
void mips_vm_init(void);
void mips_init(void);
void page_init(void);
void *alloc(u_int n, u_int align, int clear);

int page_alloc(struct Page **pp);
void page_free(struct Page *pp);
void page_decref(struct Page *pp);
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm);
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte);
void page_remove(Pde *pgdir, u_int asid, u_long va);
void tlb_invalidate(u_int asid, u_long va);

extern struct Page *pages;

void physical_memory_manage_check(void);
void page_check(void);

#define PA(pte) pte & ~0xfff;
#define PERM(pte) pte & 0xfff;
void debug_page(u_long *pgdir);
void debug_page_va(u_long *pgdir, u_long va);
u_long get_pa(u_long *pgdir, u_long va);
u_long get_perm(u_long *pgdir, u_long va);
void set_pa(u_long *pgdir, u_long va, u_long pa);
void set_perm(u_long *pgdir, u_long va, u_long perm);

int malloc_page(u_long *pgdir, u_int asid, u_long va, u_int perm);
int map_page(u_long *pgdir, u_int asid, u_long va, u_long pa, u_int perm);
int unmap_page(u_long *pgdir, u_int asid, u_long va);
int is_mapped_page(u_long *pgdir, u_long va);

int destroy_pgdir(u_long *pgdir, u_int asid);

int create_pd(Pde **pgdir);
int create_pt(Pde *pgdir, u_long pdx);
int destroy_pd(Pde *pgdir, u_int asid);

void set(Pde *pgdir, u_long va, u_long value);
u_long get(Pde *pgdir, u_long va);

#endif /* _PMAP_H_ */
