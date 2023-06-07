#include <elf.h>
#include <pmap.h>

const Elf32_Ehdr *elf_from(const void *binary, size_t size) {
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)binary;
	if (size >= sizeof(Elf32_Ehdr) && ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    ehdr->e_ident[EI_MAG3] == ELFMAG3 && ehdr->e_type == 2) {
		return ehdr;
	}
	return NULL;
}

const Elf64_Ehdr *elf_from_64(const void *binary, size_t size) {
	const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)binary;
	if (size >= sizeof(Elf64_Ehdr) && ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    ehdr->e_ident[EI_MAG3] == ELFMAG3 && ehdr->e_type == 2) {
		return ehdr;
	}
	return NULL;
}

/* Overview:
 *   load an elf format binary file. Map all section
 * at correct virtual address.
 *
 * Pre-Condition:
 *   `binary` can't be NULL and `size` is the size of binary.
 *
 * Post-Condition:
 *   Return 0 if success. Otherwise return < 0.
 *   If success, the entry point of `binary` will be stored in `start`
 */
int elf_load_seg(Elf32_Phdr *ph, const void *bin, elf_mapper_t mapper, void *data) {
	u_long va = ph->p_vaddr;
	size_t bin_size = ph->p_filesz;
	size_t sgsize = ph->p_memsz;
	u_int perm = PTE_V | PTE_U;

	if (ph->p_flags & PF_R) {
		perm |= PTE_R;
	}

	if (ph->p_flags & PF_W) {
		perm |= PTE_W;
	}

	if (ph->p_flags & PF_X) {
		perm |= PTE_X;
	}

	int r;
	size_t i;
	u_long offset = va - ROUNDDOWN(va, PAGE_SIZE);
	if (offset != 0) {
		if ((r = mapper(data, va, offset, perm, bin, MIN(bin_size, PAGE_SIZE - offset))) !=
		    0) {
			return r;
		}
	}

	/* Step 1: load all content of bin into memory. */
	for (i = offset ? MIN(bin_size, PAGE_SIZE - offset) : 0; i < bin_size; i += PAGE_SIZE) {
		if ((r = mapper(data, va + i, 0, perm, bin + i, MIN(bin_size - i, PAGE_SIZE))) != 0) {
			return r;
		}
	}

	/* Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`. */
	while (i < sgsize) {
		if ((r = mapper(data, va + i, 0, perm, NULL, MIN(bin_size - i, PAGE_SIZE))) != 0) {
			return r;
		}
		i += PAGE_SIZE;
	}
	return 0;
}

int elf_load_seg_64(Elf64_Phdr *ph, const void *bin, elf_mapper_t mapper, void *data) {
	u_long va = ph->p_vaddr;
	size_t bin_size = ph->p_filesz;
	size_t sgsize = ph->p_memsz;
	u_int perm = PTE_V | PTE_U;

	if (ph->p_flags & PF_R) {
		perm |= PTE_R;
	}

	if (ph->p_flags & PF_W) {
		perm |= PTE_W;
	}

	if (ph->p_flags & PF_X) {
		perm |= PTE_X;
	}

	int r;
	size_t i;
	u_long offset = va - ROUNDDOWN(va, PAGE_SIZE);
	if (offset != 0) {
		if ((r = mapper(data, va, offset, perm, bin, MIN(bin_size, PAGE_SIZE - offset))) !=
		    0) {
			return r;
		}
	}

	/* Step 1: load all content of bin into memory. */
	for (i = offset ? MIN(bin_size, PAGE_SIZE - offset) : 0; i < bin_size; i += PAGE_SIZE) {
		// printk("%016lx->%016lx\n", )
		if ((r = mapper(data, va + i, 0, perm, bin + i, MIN(bin_size - i, PAGE_SIZE))) != 0) {
			return r;
		}
	}
	/* Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`. */
	while (i < sgsize) {
		if ((r = mapper(data, va + i, 0, perm, NULL, MIN(bin_size - i, PAGE_SIZE))) != 0) {
			return r;
		}
		i += PAGE_SIZE;
	}
	
	return 0;
}
