#include "serv.h"
#include <mmu.h>

struct Super *super;

uint32_t *bitmap;

void file_flush(struct File *);
int block_is_free(u_int);

// Overview:
//  Return the virtual address of this disk block in cache.
// Hint: Use 'DISKMAP' and 'BY2BLK' to calculate the address.
u_long diskaddr(u_int blockno) {
	/* Exercise 5.6: Your code here. */
	return (u_long)(DISKMAP + (blockno * BY2BLK));

}

// Overview:
//  Check if this virtual address is mapped to a block. (check PTE_V bit)
int va_is_mapped(u_long va) {
	return is_mapped(va);
}

// Overview:
//  Check if this disk block is mapped in cache.
//  Returns the virtual address of the cache page if mapped, 0 otherwise.
u_long block_is_mapped(u_int blockno) {
	u_long va = diskaddr(blockno);
	if (va_is_mapped(va)) {
		return va;
	}
	return 0;
}

// Overview:
//  Check if this virtual address is dirty. (check PTE_DIRTY bit)
int va_is_dirty(u_long va) {
	return (pt0[va >> VPN0_SHIFT] & PTE_D); // [VPN(va)] & PTE_DIRTY;
}

// Overview:
//  Check if this block is dirty. (check corresponding `va`)
int block_is_dirty(u_int blockno) {
	u_long va = diskaddr(blockno);
	return va_is_mapped(va) && va_is_dirty(va);
}

// Overview:
//  Mark this block as dirty (cache page has changed and needs to be written back to disk).
int dirty_block(u_int blockno) {
	u_long va = diskaddr(blockno);

	if (!va_is_mapped(va)) {
		return -E_NOT_FOUND;
	}

	if (va_is_dirty(va)) {
		return 0;
	}

	return syscall_mem_map(0, va, 0, va, PTE_R | PTE_W | PTE_U | PTE_DIRTY); // 页面标记
}

// Overview:
//  Write the current contents of the block out to disk.
void write_block(u_int blockno) {
	// Step 1: detect is this block is mapped, if not, can't write it's data to disk.
	if (!block_is_mapped(blockno)) {
		user_panic("write unmapped block %08x", blockno);
	}

	// Step2: write data to IDE disk. (using ide_write, and the diskno is 0)
	u_long va = diskaddr(blockno);
	ide_write(0, blockno * SECT2BLK, va, SECT2BLK);
	// debugf("[ide] Write: %08x->%d(%08x)\n", va, 0, blockno * BY2BLK);
}

// Overview:
//  Make sure a particular disk block is loaded into memory.
//
// Post-Condition:
//  Return 0 on success, or a negative error code on error.
//
//  If blk!=0, set *blk to the address of the block in memory.
//
//  If isnew!=0, set *isnew to 0 if the block was already in memory, or
//  to 1 if the block was loaded off disk to satisfy this request. (Isnew
//  lets callers like file_get_block clear any memory-only fields
//  from the disk blocks when they come in off disk.)
//
// Hint:
//  use diskaddr, block_is_mapped, syscall_mem_alloc, and ide_read.
int read_block(u_int blockno, void **blk, u_int *isnew) {
	// Step 1: validate blockno. Make file the block to read is within the disk.
	if (super && blockno >= super->s_nblocks) {
		user_panic("reading non-existent block %08x\n", blockno);
	}

	// Step 2: validate this block is used, not free.
	// Hint:
	//  If the bitmap is NULL, indicate that we haven't read bitmap from disk to memory
	//  until now. So, before we check if a block is free using `block_is_free`, we must
	//  ensure that the bitmap blocks are already read from the disk to memory.
	if (bitmap && block_is_free(blockno)) {
		user_panic("reading free block %08x\n", blockno);
	}

	// Step 3: transform block number to corresponding virtual address.
	u_long va = diskaddr(blockno);

	// Step 4: read disk and set *isnew.
	// Hint:
	//  If this block is already mapped, just set *isnew, else alloc memory and
	//  read data from IDE disk (use `syscall_mem_alloc` and `ide_read`).
	//  We have only one IDE disk, so the diskno of ide_read should be 0.
	if (block_is_mapped(blockno)) { // the block is in memory
		if (isnew) {
			*isnew = 0;
		}
	} else { // the block is not in memory
		if (isnew) {
			*isnew = 1;
		}
		syscall_mem_alloc(0, va, PTE_R | PTE_W | PTE_U); // 页面标记
		ide_read(0, blockno * SECT2BLK, va, SECT2BLK);
		// debugf("[ide] Read: %d(%d, %08x)->%08x\n", 0, blockno, blockno * BY2BLK, va);
		// for (int i = 0; i < 32; i++) {
		// 	if (block_is_mapped(i)) {
		// 		debugf("%d\n", i);
		// 	}
		// }
		// user_panic("stop");
	}

	// Step 5: if blk != NULL, assign 'va' to '*blk'.
	if (blk) {
		*blk = (void *)va;
	}
	return 0;
}

// Overview:
//  Allocate a page to cache the disk block.
int map_block(u_int blockno) {
	// Step 1: If the block is already mapped in cache, return 0.
	// Hint: Use 'block_is_mapped'.
	/* Exercise 5.7: Your code here. (1/5) */
	if (block_is_mapped(blockno)) {
		return 0;
	}

	// Step 2: Alloc a page in permission 'PTE_D' via syscall.
	// Hint: Use 'diskaddr' for the virtual address.
	/* Exercise 5.7: Your code here. (2/5) */
	u_long va = diskaddr(blockno);
	return syscall_mem_alloc(0, va, PTE_R | PTE_W | PTE_U); // 页面标记

}

// Overview:
//  Unmap a disk block in cache.
void unmap_block(u_int blockno) {
	// Step 1: Get the mapped address of the cache page of this block using 'block_is_mapped'.
	u_long va;
	/* Exercise 5.7: Your code here. (3/5) */
	va = block_is_mapped(blockno);

	// Step 2: If this block is used (not free) and dirty in cache, write it back to the disk
	// first.
	// Hint: Use 'block_is_free', 'block_is_dirty' to check, and 'write_block' to sync.
	/* Exercise 5.7: Your code here. (4/5) */
	if (!block_is_free(blockno) && block_is_dirty(blockno)) {
		write_block(blockno);
	}

	// Step 3: Unmap the virtual address via syscall.
	/* Exercise 5.7: Your code here. (5/5) */
	syscall_mem_unmap(0, va);

	user_assert(!block_is_mapped(blockno));
}

// Overview:
//  Check if the block 'blockno' is free via bitmap.
//
// Post-Condition:
//  Return 1 if the block is free, else 0.
int block_is_free(u_int blockno) {
	if (super == 0 || blockno >= super->s_nblocks) {
		return 0;
	}

	if (bitmap[blockno / 32] & (1 << (blockno % 32))) {
		return 1;
	}

	return 0;
}

// Overview:
//  Mark a block as free in the bitmap.
void free_block(u_int blockno) {
	// You can refer to the function 'block_is_free' above.
	// Step 1: If 'blockno' is invalid (0 or >= the number of blocks in 'super'), return.
	/* Exercise 5.4: Your code here. (1/2) */
	if (blockno == 0 || blockno >= super->s_nblocks) {
		return;
	}

	// Step 2: Set the flag bit of 'blockno' in 'bitmap'.
	// Hint: Use bit operations to update the bitmap, such as b[n / W] |= 1 << (n % W).
	/* Exercise 5.4: Your code here. (2/2) */
	bitmap[blockno / 32] |= (1 << (blockno % 32));

}

// Overview:
//  Search in the bitmap for a free block and allocate it.
//
// Post-Condition:
//  Return block number allocated on success,
//  Return -E_NO_DISK if we are out of blocks.
int alloc_block_num(void) {
	int blockno;
	// walk through this bitmap, find a free one and mark it as used, then sync
	// this block to IDE disk (using `write_block`) from memory.
	for (blockno = 3; blockno < super->s_nblocks; blockno++) {
		if (bitmap[blockno / 32] & (1 << (blockno % 32))) { // the block is free
			bitmap[blockno / 32] &= ~(1 << (blockno % 32));
			write_block(blockno / BIT2BLK + 2); // write to disk.
			return blockno;
		}
	}
	// no free blocks.
	return -E_NO_DISK;
}

// Overview:
//  Allocate a block -- first find a free block in the bitmap, then map it into memory.
int alloc_block(void) {
	int r, bno;
	// Step 1: find a free block.
	if ((r = alloc_block_num()) < 0) { // failed.
		return r;
	}
	bno = r;

	// Step 2: map this block into memory.
	if ((r = map_block(bno)) < 0) {
		free_block(bno);
		return r;
	}

	// Step 3: return block number.
	return bno;
}

// Overview:
//  Read and validate the file system super-block.
//
// Post-condition:
//  If error occurred during read super block or validate failed, panic.
void read_super(void) {
	int r;
	void *blk;

	// Step 1: read super block.
	if ((r = read_block(1, &blk, 0)) < 0) {
		user_panic("cannot read superblock: %e", r);
	}

	super = blk;

	// Step 2: Check fs magic nunber.
	if (super->s_magic != FS_MAGIC) {
		user_panic("bad file system magic number %x %x", super->s_magic, FS_MAGIC);
	}

	// Step 3: validate disk size.
	if (super->s_nblocks > DISKMAX / BY2BLK) {
		user_panic("file system is too large");
	}

	debugf("superblock is good\n");
}

// Overview:
//  Read and validate the file system bitmap.
//
// Hint:
//  Read all the bitmap blocks into memory.
//  Set the 'bitmap' to point to the first bitmap block.
//  For each block i, user_assert(!block_is_free(i))) to check that they're all marked as in use.
void read_bitmap(void) {
	u_int i;
	void *blk = NULL;

	// Step 1: Calculate the number of the bitmap blocks, and read them into memory.
	u_int nbitmap = super->s_nblocks / BIT2BLK + 1;
	for (i = 0; i < nbitmap; i++) {
		read_block(i + 2, blk, 0);
	}

	bitmap = (uint32_t *)diskaddr(2);

	// Step 2: Make sure the reserved and root blocks are marked in-use.
	// Hint: use `block_is_free`
	user_assert(!block_is_free(0));
	user_assert(!block_is_free(1));

	// Step 3: Make sure all bitmap blocks are marked in-use.
	for (i = 0; i < nbitmap; i++) {
		user_assert(!block_is_free(i + 2));
	}

	debugf("read_bitmap is good\n");
}

// Overview:
//  Test that write_block works, by smashing the superblock and reading it back.
void check_write_block(void) {
	super = 0;

	// backup the super block.
	// copy the data in super block to the first block on the disk.
	read_block(0, 0, 0);
	memcpy((char *)diskaddr(0), (char *)diskaddr(1), BY2PG);

	// smash it
	strcpy((char *)diskaddr(1), "OOPS!\n");
	write_block(1);
	user_assert(block_is_mapped(1));

	// clear it out
	syscall_mem_unmap(0, diskaddr(1));
	user_assert(!block_is_mapped(1));

	// validate the data read from the disk.
	read_block(1, 0, 0);
	user_assert(strcmp((char *)diskaddr(1), "OOPS!\n") == 0);

	// restore the super block.
	memcpy((char *)diskaddr(1), (char *)diskaddr(0), BY2PG);
	write_block(1);
	super = (struct Super *)diskaddr(1);
}

// Overview:
//  Initialize the file system.
// Hint:
//  1. read super block.
//  2. check if the disk can work.
//  3. read bitmap blocks from disk to memory.
void fs_init(void) {
	read_super();
	check_write_block();
	read_bitmap();
}

// Overview:
//  Like pgdir_walk but for files.
//  Find the disk block number slot for the 'filebno'th block in file 'f'. Then, set
//  '*ppdiskbno' to point to that slot. The slot will be one of the f->f_direct[] entries,
//  or an entry in the indirect block.
//  When 'alloc' is set, this function will allocate an indirect block if necessary.
//
// Post-Condition:
//  Return 0 on success, and set *ppdiskbno to the pointer to the target block.
//  Return -E_NOT_FOUND if the function needed to allocate an indirect block, but alloc was 0.
//  Return -E_NO_DISK if there's no space on the disk for an indirect block.
//  Return -E_NO_MEM if there's not enough memory for an indirect block.
//  Return -E_INVAL if filebno is out of range (>= NINDIRECT).
int file_block_walk(struct File *f, u_int filebno, uint32_t **ppdiskbno, u_int alloc) {
	int r;
	uint32_t *ptr;
	uint32_t *blk;

	if (filebno < NDIRECT) {
		// Step 1: if the target block is corresponded to a direct pointer, just return the
		// disk block number.
		ptr = &f->f_direct[filebno];
	} else if (filebno < NINDIRECT) {
		// Step 2: if the target block is corresponded to the indirect block, but there's no
		//  indirect block and `alloc` is set, create the indirect block.
		if (f->f_indirect == 0) {
			if (alloc == 0) {
				return -E_NOT_FOUND;
			}

			if ((r = alloc_block()) < 0) {
				return r;
			}
			f->f_indirect = r;
		}

		// Step 3: read the new indirect block to memory.
		if ((r = read_block(f->f_indirect, (void **)&blk, 0)) < 0) {
			return r;
		}
		ptr = blk + filebno;
	} else {
		return -E_INVAL;
	}

	// Step 4: store the result into *ppdiskbno, and return 0.
	*ppdiskbno = ptr;
	return 0;
}

// OVerview:
//  Set *diskbno to the disk block number for the filebno'th block in file f.
//  If alloc is set and the block does not exist, allocate it.
//
// Post-Condition:
//  Returns 0: success, < 0 on error.
//  Errors are:
//   -E_NOT_FOUND: alloc was 0 but the block did not exist.
//   -E_NO_DISK: if a block needed to be allocated but the disk is full.
//   -E_NO_MEM: if we're out of memory.
//   -E_INVAL: if filebno is out of range.
int file_map_block(struct File *f, u_int filebno, u_int *diskbno, u_int alloc) {
	int r;
	uint32_t *ptr;

	// Step 1: find the pointer for the target block.
	if ((r = file_block_walk(f, filebno, &ptr, alloc)) < 0) {
		return r;
	}

	// Step 2: if the block not exists, and create is set, alloc one.
	if (*ptr == 0) {
		if (alloc == 0) {
			return -E_NOT_FOUND;
		}

		if ((r = alloc_block()) < 0) {
			return r;
		}
		*ptr = r;
	}

	// Step 3: set the pointer to the block in *diskbno and return 0.
	*diskbno = *ptr;
	return 0;
}

// Overview:
//  Remove a block from file f. If it's not there, just silently succeed.
int file_clear_block(struct File *f, u_int filebno) {
	int r;
	uint32_t *ptr;

	if ((r = file_block_walk(f, filebno, &ptr, 0)) < 0) {
		return r;
	}

	if (*ptr) {
		free_block(*ptr);
		*ptr = 0;
	}

	return 0;
}

// Overview:
//  Set *blk to point at the filebno'th block in file f.
//
// Hint: use file_map_block and read_block.
//
// Post-Condition:
//  return 0 on success, and read the data to `blk`, return <0 on error.
int file_get_block(struct File *f, u_int filebno, void **blk) {
	int r;
	u_int diskbno;
	u_int isnew;

	// Step 1: find the disk block number is `f` using `file_map_block`.
	if ((r = file_map_block(f, filebno, &diskbno, 1)) < 0) {
		return r;
	}

	// Step 2: read the data in this disk to blk.
	if ((r = read_block(diskbno, blk, &isnew)) < 0) {
		return r;
	}
	return 0;
}

// Overview:
//  Mark the offset/BY2BLK'th block dirty in file f.
int file_dirty(struct File *f, u_int offset) {
	int r;
	u_int diskbno;

	if ((r = file_map_block(f, offset / BY2BLK, &diskbno, 0)) < 0) {
		return r;
	}

	return dirty_block(diskbno);
}

// Overview:
//  Find a file named 'name' in the directory 'dir'. If found, set *file to it.
//
// Post-Condition:
//  Return 0 on success, and set the pointer to the target file in `*file`.
//  Return the underlying error if an error occurs.
int dir_lookup(struct File *dir, char *name, struct File **file) {
	int r;
	// Step 1: Calculate the number of blocks in 'dir' via its size.
	u_int nblock;
	/* Exercise 5.8: Your code here. (1/3) */
	nblock = dir->f_size / BY2BLK;

	// Step 2: Iterate through all blocks in the directory.
	for (int i = 0; i < nblock; i++) {
		// Read the i'th block of 'dir' and get its address in 'blk' using 'file_get_block'.
		void *blk;
		/* Exercise 5.8: Your code here. (2/3) */
		if ((r = file_get_block(dir, i, &blk)) < 0) {
			return r;
		}

		struct File *files = (struct File *)blk;

		// Find the target among all 'File's in this block.
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			// Compare the file name against 'name' using 'strcmp'.
			// If we find the target file, set '*file' to it and set up its 'f_dir'
			// field.
			/* Exercise 5.8: Your code here. (3/3) */
			if (strcmp(f->f_name, name) == 0) {
				*file = f;
				f->f_dir = dir;
				return 0;
			}

		}
	}

	return -E_NOT_FOUND;
}

// Overview:
//  Alloc a new File structure under specified directory. Set *file
//  to point at a free File structure in dir.
int dir_alloc_file(struct File *dir, struct File **file) {
	int r;
	u_int nblock, i, j;
	void *blk;
	struct File *f;

	nblock = dir->f_size / BY2BLK;

	for (i = 0; i < nblock; i++) {
		// read the block.
		if ((r = file_get_block(dir, i, &blk)) < 0) {
			return r;
		}

		f = blk;

		for (j = 0; j < FILE2BLK; j++) {
			if (f[j].f_name[0] == '\0') { // found free File structure.
				*file = &f[j];
				return 0;
			}
		}
	}

	// no free File structure in exists data block.
	// new data block need to be created.
	dir->f_size += BY2BLK;
	if ((r = file_get_block(dir, i, &blk)) < 0) {
		return r;
	}
	f = blk;
	*file = &f[0];

	return 0;
}

// Overview:
//  Skip over slashes.
char *skip_slash(char *p) {
	while (*p == '/') {
		p++;
	}
	return p;
}

// Overview:
//  Evaluate a path name, starting at the root.
//
// Post-Condition:
//  On success, set *pfile to the file we found and set *pdir to the directory
//  the file is in.
//  If we cannot find the file but find the directory it should be in, set
//  *pdir and copy the final path element into lastelem.
int walk_path(char *path, struct File **pdir, struct File **pfile, char *lastelem) {
	char *p;
	char name[MAXNAMELEN];
	struct File *dir, *file;
	int r;

	// start at the root.
	path = skip_slash(path);
	file = &super->s_root;
	dir = 0;
	name[0] = 0;

	if (pdir) {
		*pdir = 0;
	}

	*pfile = 0;

	// find the target file by name recursively.
	while (*path != '\0') {
		dir = file;
		p = path;

		while (*path != '/' && *path != '\0') {
			path++;
		}

		if (path - p >= MAXNAMELEN) {
			return -E_BAD_PATH;
		}

		memcpy(name, p, path - p);
		name[path - p] = '\0';
		path = skip_slash(path);
		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}

		if ((r = dir_lookup(dir, name, &file)) < 0) {
			if (r == -E_NOT_FOUND && *path == '\0') {
				if (pdir) {
					*pdir = dir;
				}

				if (lastelem) {
					strcpy(lastelem, name);
				}

				*pfile = 0;
			}

			return r;
		}
	}

	if (pdir) {
		*pdir = dir;
	}

	*pfile = file;
	return 0;
}

// Overview:
//  Open "path".
//
// Post-Condition:
//  On success set *pfile to point at the file and return 0.
//  On error return < 0.
int file_open(char *path, struct File **file) {
	return walk_path(path, 0, file, 0);
}

// Overview:
//  Create "path".
//
// Post-Condition:
//  On success set *file to point at the file and return 0.
//  On error return < 0.
int file_create(char *path, struct File **file) {
	char name[MAXNAMELEN];
	int r;
	struct File *dir, *f;

	if ((r = walk_path(path, &dir, &f, name)) == 0) {
		return -E_FILE_EXISTS;
	}

	if (r != -E_NOT_FOUND || dir == 0) {
		return r;
	}

	if (dir_alloc_file(dir, &f) < 0) {
		return r;
	}

	strcpy(f->f_name, name);
	*file = f;
	return 0;
}

// Overview:
//  Truncate file down to newsize bytes.
//
//  Since the file is shorter, we can free the blocks that were used by the old
//  bigger version but not by our new smaller self. For both the old and new sizes,
//  figure out the number of blocks required, and then clear the blocks from
//  new_nblocks to old_nblocks.
//
//  If the new_nblocks is no more than NDIRECT, free the indirect block too.
//  (Remember to clear the f->f_indirect pointer so you'll know whether it's valid!)
//
// Hint: use file_clear_block.
void file_truncate(struct File *f, u_int newsize) {
	u_int bno, old_nblocks, new_nblocks;

	old_nblocks = f->f_size / BY2BLK + 1;
	new_nblocks = newsize / BY2BLK + 1;

	if (newsize == 0) {
		new_nblocks = 0;
	}

	if (new_nblocks <= NDIRECT) {
		for (bno = new_nblocks; bno < old_nblocks; bno++) {
			file_clear_block(f, bno);
		}
		if (f->f_indirect) {
			free_block(f->f_indirect);
			f->f_indirect = 0;
		}
	} else {
		for (bno = new_nblocks; bno < old_nblocks; bno++) {
			file_clear_block(f, bno);
		}
	}

	f->f_size = newsize;
}

// Overview:
//  Set file size to newsize.
int file_set_size(struct File *f, u_int newsize) {
	if (f->f_size > newsize) {
		file_truncate(f, newsize);
	}

	f->f_size = newsize;

	if (f->f_dir) {
		file_flush(f->f_dir);
	}

	return 0;
}

// Overview:
//  Flush the contents of file f out to disk.
//  Loop over all the blocks in file.
//  Translate the file block number into a disk block number and then
//  check whether that disk block is dirty. If so, write it out.
//
// Hint: use file_map_block, block_is_dirty, and write_block.
void file_flush(struct File *f) {
	// Your code here
	u_int nblocks;
	u_int bno;
	u_int diskno;
	int r;

	nblocks = f->f_size / BY2BLK + 1;

	for (bno = 0; bno < nblocks; bno++) {
		if ((r = file_map_block(f, bno, &diskno, 0)) < 0) {
			continue;
		}
		if (block_is_dirty(diskno)) {
			write_block(diskno);
		}
	}
}

// Overview:
//  Sync the entire file system.  A big hammer.
void fs_sync(void) {
	int i;
	for (i = 0; i < super->s_nblocks; i++) {
		if (block_is_dirty(i)) {
			write_block(i);
		}
	}
}

// Overview:
//  Close a file.
void file_close(struct File *f) {
	// Flush the file itself, if f's f_dir is set, flush it's f_dir.
	file_flush(f);
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
}

// Overview:
//  Remove a file by truncating it and then zeroing the name.
int file_remove(char *path) {
	int r;
	struct File *f;

	// Step 1: find the file on the disk.
	if ((r = walk_path(path, 0, &f, 0)) < 0) {
		return r;
	}

	// Step 2: truncate it's size to zero.
	file_truncate(f, 0);

	// Step 3: clear it's name.
	f->f_name[0] = '\0';

	// Step 4: flush the file.
	file_flush(f);
	if (f->f_dir) {
		file_flush(f->f_dir);
	}

	return 0;
}

int alloc_block1(void) {
	int r;

	for (int bno = 3; bno < super->s_nblocks; bno++) {
		if (bitmap[bno / 32] & (1 << (bno % 32))) {
			bitmap[bno / 32] &= ~(1 << (bno % 32));
			write_block(bno / BIT2BLK + 2);

			if (block_is_mapped(bno)) {
				return bno;
			}
			u_long va = diskaddr(bno);
			if ((r = syscall_mem_alloc(0, va, PTE_R | PTE_W | PTE_U)) < 0) { // 页面标记
				bitmap[bno / 32] |= (1 << (bno % 32));
				return r;
			}
			return bno;
		}
	}
	return -E_NO_DISK;
}

int dir_lookup1(struct File *dir, char *name, struct File **file) {
	int r;
	u_int nblock = dir->f_size / BY2BLK;
	for (u_int bno = 0; bno < nblock; bno++) {
		u_int diskbno;

		if (bno < NDIRECT) {
			diskbno = dir->f_direct[bno];
		} else if (bno < NINDIRECT) {
			if (dir->f_indirect == 0) {
				if ((r = alloc_block()) < 0) {
					return r;
				}
				dir->f_indirect = r;
			}
			uint32_t *diskbnos;
			if ((r = read_block(dir->f_indirect, (void **)&diskbnos, NULL)) < 0) {
				return r;
			}
			diskbno = diskbnos[bno];
		} else {
			return -E_INVAL;
		}

		if (diskbno == 0) {
			if ((r = alloc_block()) < 0) {
				return r;
			}
			diskbno = r;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return r;
		}

		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (strcmp(f->f_name, name) == 0) {
				*file = f;
				f->f_dir = dir;
				return 0;
			}
		}
	}
	return -E_NOT_FOUND;
}

// 如果 path 为 ""，则与 file_open 行为不同
int file_open1(char *path, struct File **file) {
	char name[MAXNAMELEN];
	struct File *dir;
	int r;

	*file = &super->s_root;

	while (*path != '\0') {
		dir = *file;
		
		int len = 0;
		if (*path == '/') {
			path++;
		}
		while (*path != '/' && *path != '\0') {
			if (len == MAXNAMELEN) {
				return -E_BAD_PATH;
			}
			name[len++] = *path;
			path++;
		}
		name[len] = '\0';

		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}

		if ((r = dir_lookup1(dir, name, file)) < 0) {
			return r;
		}
	}
	return 0;
}

void file_flush1(struct File *f) {
	int r;

	u_int nblocks = f->f_size / BY2BLK + 1;

	for (u_int bno = 0; bno < nblocks; bno++) {
		u_int diskno;
		if (bno < NDIRECT) {
			diskno = f->f_direct[bno];
		} else if (bno < NINDIRECT) {
			if (f->f_indirect == 0) {
				continue;
			}
			uint32_t *disknos;
			if ((r = read_block(f->f_indirect, (void **)&disknos, 0)) < 0) {
				continue;
			}
			diskno = *(disknos + bno);
		} else {
			continue;
		}
		if (diskno == 0) {
			continue;
		}
		if (block_is_dirty(diskno)) {
			write_block(diskno);
		}
	}
}

void get_path(struct File *file, char *path) {
	struct File *f = file;
	char *p = path;
	char *n, *name;

	while (1) {
		n = name = f->f_name;
		while (*n) {
			n++;
		}
		while (n != name) {
			*p++ = *--n;
		}
		*p++ = '/';

		if (!f->f_dir) {
			break;
		}
		if (f->f_dir->f_name[0] == '/' && f->f_dir->f_name[1] == '\0') {
			break;
		}
		f = f->f_dir;
	}

	*p-- = '\0';
	while (path < p) {
		char tmp = *p;
		*p = *path;
		*path = tmp;
		p--;
		path++;
	}
}

void debug_file(struct File *file) {
	if (file->f_name[0] == '\0') {
		debugf("[debug] file does not exist at %08x\n", file);
		return;
	}

	debugf("[debug] in file %s at %08x\n", file->f_name, file);
	char path[MAXPATHLEN];
	get_path(file, path);
	debugf("path:   %s\n", path);
	debugf("size:   %d\n", file->f_size);
	debugf("block:  bno   disk\n");

	int r;
	u_int nblock = file->f_size / BY2BLK + 1;

	for (int bno = 0; bno < nblock; bno++) {
		u_int diskbno;

		if (bno < NDIRECT) {
			diskbno = file->f_direct[bno];
		} else if (bno < NINDIRECT) {
			if (file->f_indirect == 0) {
				continue;
			}
			uint32_t *diskbnos;
			if ((r = read_block(file->f_indirect, (void **)&diskbnos, NULL)) < 0) {
				return;
			}
			diskbno = diskbnos[bno];
		} else {
			return;
		}

		if (diskbno == 0) {
			continue;
		}
		debugf("        %-4d  %-4d\n", bno, diskbno);
	}

	debugf("content\n");
	int read = 0;
	for (int bno = 0; bno < nblock; bno++) {
		u_int diskbno;

		if (bno < NDIRECT) {
			diskbno = file->f_direct[bno];
		} else if (bno < NINDIRECT) {
			if (file->f_indirect == 0) {
				if ((r = alloc_block()) < 0) {
					return;
				}
				file->f_indirect = r;
			}
			uint32_t *diskbnos;
			if ((r = read_block(file->f_indirect, (void **)&diskbnos, NULL)) < 0) {
				return;
			}
			diskbno = diskbnos[bno];
		} else {
			return;
		}

		if (diskbno == 0) {
			if ((r = alloc_block()) < 0) {
				return;
			}
			diskbno = r;
		}
		char *text;
		if ((r = read_block(diskbno, (void **)&text, NULL)) < 0) {
			return;
		}

		for (int i = 0; i < BY2BLK; i++) {
			if (read == file->f_size) {
				return;
			}
			debugf("%c", *text++);
			read++;
		}
	}
}

void debug_dir(struct File *dir) {
	if (dir->f_name[0] == '\0') {
		debugf("[debug] dir does not exist at %08x\n", dir);
		return;
	}

	debugf("[debug] in dir %s at %08x\n", dir->f_name, dir);

	int r;
	u_int nblock = dir->f_size / BY2BLK;

	for (int bno = 0; bno < nblock; bno++) {
		u_int diskbno;

		if (bno < NDIRECT) {
			diskbno = dir->f_direct[bno];
		} else if (bno < NINDIRECT) {
			if (dir->f_indirect == 0) {
				continue;
			}
			uint32_t *diskbnos;
			if ((r = read_block(dir->f_indirect, (void **)&diskbnos, NULL)) < 0) {
				return;
			}
			diskbno = diskbnos[bno];
		} else {
			return;
		}

		if (diskbno == 0) {
			continue;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return;
		}

		debugf("addr      type  name        size      bno   disk\n");
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (*(f->f_name)) {
				if (f->f_type == 0) {
					debugf("%08x  file  %-10s  %-8d  %-4d  %-4d\n", f, f->f_name, f->f_size, bno, diskbno);
				} else {
					debugf("%08x  dir   %-10s  %-8d  %-4d  %-4d\n", f, f->f_name, f->f_size, bno, diskbno);
				}
			}
		}
	}
}

static void to_tree_dir(struct File *dir, int depth) {
	int r;
	u_int nblock = dir->f_size / BY2BLK;
	for (int i = 0; i < nblock; i++) {
		u_int diskbno;

		if (i < NDIRECT) {
			diskbno = dir->f_direct[i];
		} else if (i < NINDIRECT) {
			if (dir->f_indirect == 0) {
				if ((r = alloc_block()) < 0) {
					return;
				}
				dir->f_indirect = r;
			}
			uint32_t *diskbnos;
			if ((r = read_block(dir->f_indirect, (void **)&diskbnos, NULL)) < 0) {
				return;
			}
			diskbno = diskbnos[i];
		} else {
			return;
		}

		if (diskbno == 0) {
			if ((r = alloc_block()) < 0) {
				return;
			}
			diskbno = r;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return;
		}

		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (*(f->f_name)) {
				for (int i = 0; i < depth; i++) {
					debugf("│   ");
				}
				debugf("├── ");
				if (f->f_type == 0) {
					debugf("%s\n", f->f_name);
				} else {
					debugf("%s\n", f->f_name);
					to_tree_dir(f, depth + 1);
				}
			}
		}
	}
}

void tree_dir(struct File *dir) {
	debugf("[tree]\n");
	debugf(".\n");
	to_tree_dir(dir, 0);
}

struct File *get_root() {
	return &super->s_root;
}

void tree_root() {
	tree_dir(get_root());
}

struct File *get_file_relative(struct File *relative, char *path) {
	char name[MAXNAMELEN];
	struct File *dir;
	struct File *file;
	int r;

	file = relative;

	while (*path != '\0') {
		dir = file;
		
		int len = 0;
		if (*path == '/') {
			path++;
		}
		while (*path != '/' && *path != '\0') {
			if (len == MAXNAMELEN) {
				return NULL;
			}
			name[len++] = *path;
			path++;
		}
		name[len] = '\0';

		if (dir->f_type != FTYPE_DIR) {
			return NULL;
		}

		if ((r = dir_lookup1(dir, name, &file)) < 0) {
			return NULL;
		}
	}
	return file;
}

struct File *get_file_absolute(char *path) {
	return get_file_relative(get_root(), path);
}

// file_map_block: alloc == 0
int get_block_file(struct File *f, u_int filebno, u_int *pdiskbno) {
	int r;

	if (filebno < NDIRECT) {
		*pdiskbno = f->f_direct[filebno];
	} else if (filebno < NINDIRECT) {
		if (f->f_indirect == 0) {
			return -E_NOT_FOUND;
		}
		uint32_t *blk;
		if ((r = read_block(f->f_indirect, (void **)&blk, NULL)) < 0) {
			return r;
		}
		*pdiskbno = blk[filebno];
	} else {
		return -E_INVAL;
	}
	if (*pdiskbno == 0) {
		return -E_NOT_FOUND;
	}
	return 0;
}

// file_map_block: alloc == 1
int alloc_block_file(struct File *f, u_int filebno, u_int *pdiskbno) {
	int r;
	u_int *pdiskbno1;

	if (filebno < NDIRECT) {
		pdiskbno1 = &f->f_direct[filebno];
	} else if (filebno < NINDIRECT) {
		if (f->f_indirect == 0) {
			if ((r = alloc_block()) < 0) {
				return r;
			}
			f->f_indirect = r;
		}
		uint32_t *blk;
		if ((r = read_block(f->f_indirect, (void **)&blk, NULL)) < 0) {
			return r;
		}
		pdiskbno1 = &blk[filebno];
	} else {
		return -E_INVAL;
	}
	if (*pdiskbno1 == 0) {
		if ((r = alloc_block()) < 0) {
			return r;
		}
		*pdiskbno1 = r;
	}
	if (*pdiskbno) {
		*pdiskbno = *pdiskbno1;
	}
	return 0;
}

// file_clear_block
int free_block_file(struct File *f, u_int filebno) {
	int r;
	uint32_t *pdiskbno;

	if (filebno < NDIRECT) {
		pdiskbno = &f->f_direct[filebno];
	} else if (filebno < NINDIRECT) {
		if (f->f_indirect == 0) {
			return -E_NOT_FOUND;
		}
		uint32_t *blk;
		if ((r = read_block(f->f_indirect, (void **)&blk, NULL)) < 0) {
			return r;
		}
		pdiskbno = &blk[filebno];
	} else {
		return -E_INVAL;
	}
	if (*pdiskbno) {
		free_block(*pdiskbno);
		*pdiskbno = 0;
	}
	return 0;
}

// dir_lookup
int get_file_dir(struct File *dir, char *name, struct File **file) {
	int r;
	u_int nblock = dir->f_size / BY2BLK;

	for (u_int bno = 0; bno < nblock; bno++) {
		u_int diskbno;
		if ((r = get_block_file(dir, bno, &diskbno)) < 0) {
			return r;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return r;
		}
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (strcmp(f->f_name, name) == 0) {
				*file = f;
				f->f_dir = dir;
				return 0;
			}
		}
	}
	return -E_NOT_FOUND;
}

// dir_alloc_file 并且为写入名字和目录地址
int alloc_file_dir(struct File *dir, char *name, struct File **file) {
	int r;
	u_int nblock = dir->f_size / BY2BLK;

	u_int bno;
	for (bno = 0; bno < nblock; bno++) {
		u_int diskbno;
		if ((r = get_block_file(dir, bno, &diskbno)) < 0) {
			return r;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return r;
		}
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (*f->f_name == '\0') {
				*file = f;
				strcpy(f->f_name, name);
				f->f_dir = dir;
				return 0;
			}
		}
	}
	dir->f_size += BY2BLK;
	u_int diskbno;
	if ((r = alloc_block_file(dir, bno, &diskbno)) < 0) {
		return r;
	}
	struct File *files;
	if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
		return r;
	}
	*file = files;
	strcpy(files->f_name, name);
	files->f_dir = dir;
	return 0;
}

int free_file_dir(struct File *dir, char *name) {
	int r;
	u_int nblock = dir->f_size / BY2BLK;

	for (u_int bno = 0; bno < nblock; bno++) {
		u_int diskbno;
		if ((r = get_block_file(dir, bno, &diskbno)) < 0) {
			return r;
		}
		struct File *files;
		if ((r = read_block(diskbno, (void **)&files, NULL)) < 0) {
			return r;
		}
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
			if (strcmp(f->f_name, name) == 0) {
				f->f_size = 0;
				f->f_name[0] = '\0';
				return 0;
			}
		}
	}
	return -E_NOT_FOUND;
}

int get_file(char *path, struct File **file) {
	char name[MAXNAMELEN];
	struct File *dir;
	int r;

	*file = &super->s_root;

	while (*path != '\0') {
		dir = *file;
		
		int len = 0;
		if (*path == '/') {
			path++;
		}
		while (*path != '/' && *path != '\0') {
			if (len == MAXNAMELEN) {
				return -E_BAD_PATH;
			}
			name[len++] = *path;
			path++;
		}
		name[len] = '\0';

		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}

		if ((r = get_file_dir(dir, name, file)) < 0) {
			return r;
		}
	}
	return 0;
}

int alloc_file(char *path, struct File **file) {
	char name[MAXNAMELEN];
	struct File *dir;
	int r;

	*file = &super->s_root;

	while (*path != '\0') {
		dir = *file;
		
		int len = 0;
		if (*path == '/') {
			path++;
		}
		while (*path != '/' && *path != '\0') {
			if (len == MAXNAMELEN) {
				return -E_BAD_PATH;
			}
			name[len++] = *path;
			path++;
		}
		name[len] = '\0';

		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}

		if ((r = get_file_dir(dir, name, file)) < 0) {
			if (r == -E_NOT_FOUND && *path == '\0') {
				if (dir == 0) {
					return r;
				}
				if (alloc_file_dir(dir, name, file) < 0) {
					return r;
				}
				return 0;
 	 	 	}
		}
	}
	return -E_FILE_EXISTS;
}

int free_file(char *path) {
	char name[MAXNAMELEN];
	struct File *dir, *file;
	int r;

	file = &super->s_root;

	while (*path != '\0') {
		dir = file;
		
		int len = 0;
		if (*path == '/') {
			path++;
		}
		while (*path != '/' && *path != '\0') {
			if (len == MAXNAMELEN) {
				return -E_BAD_PATH;
			}
			name[len++] = *path;
			path++;
		}
		name[len] = '\0';

		if (dir->f_type != FTYPE_DIR) {
			return -E_NOT_FOUND;
		}

		if ((r = get_file_dir(dir, name, &file)) < 0) {
			return r;
		}
	}
	
	u_int nblock = file->f_size / BY2BLK + 1;

	for (u_int bno = 0; bno < nblock; bno++) {
		free_block_file(file, bno);
	}
	if (file->f_indirect) {
		free_block(file->f_indirect);
		file->f_indirect = 0;
	}

	file->f_size = 0;
	file->f_name[0] = '\0';
	file_flush(file);
	if (file->f_dir) {
		file_flush(file->f_dir);
	}

	return 0;
}

void flush_file(struct File *f) {
	int r;
	u_int nblocks = f->f_size / BY2BLK + 1;

	for (u_int bno = 0; bno < nblocks; bno++) {
		u_int diskno;
		if ((r = file_map_block(f, bno, &diskno, 0)) < 0) {
			continue;
		}
		if (block_is_dirty(diskno)) {
			write_block(diskno);
		}
	}
}

int size_file(struct File *f, u_int newsize) {
	if (f->f_size > newsize) {
		u_int old_nblock = f->f_size / BY2BLK + 1;
		u_int new_nblock = newsize / BY2BLK + 1;
		if (newsize == 0) {
			new_nblock = 0;
		}
		for (u_int bno = new_nblock; bno < old_nblock; bno++) {
			free_block_file(f, bno);
		}
		if (new_nblock <= NDIRECT) {
			if (f->f_indirect) {
				free_block(f->f_indirect);
				f->f_indirect = 0;
			}
		}
	}
	f->f_size = newsize;
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
	return 0;
}

void sync_fs(void) {
	int i;
	for (i = 0; i < super->s_nblocks; i++) {
		if (block_is_dirty(i)) {
			write_block(i);
		}
	}
}

void close_file(struct File *f) {
	file_flush(f);
	if (f->f_dir) {
		file_flush(f->f_dir);
	}
}

// void file_page_debug_all() {
// 	debugf("------------page------------\n");
// 	for (int va = 0; va < ULIM; va += BY2PG) {
// 		if (vpd[va >> PDSHIFT] & PTE_V) {
// 			if (vpt[va >> PGSHIFT] & PTE_V) {
// 				u_long pa = PTE_ADDR(vpt[va >> PGSHIFT]);
// 				debugf("|%08x->%08x  ", va, pa);
// 				if (vpt[va >> PGSHIFT] & PTE_G) {
// 					debugf("G");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_V) {
// 					debugf("V");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_D) {
// 					debugf("D");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_N) {
// 					debugf("N");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_COW) {
// 					debugf("C");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_LIBRARY) {
// 					debugf("L");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_DIRTY) {
// 					debugf(".");
// 				} else {
// 					debugf(" ");
// 				}
// 				debugf("|\n");
// 			}
// 		}
// 	}
// 	debugf("----------------------------\n");
// }

// void file_page_debug() {
// 	debugf("------------page------------\n");
// 	for (int va = 0; va < USTACKTOP; va += BY2PG) {
// 		if (vpd[va >> PDSHIFT] & PTE_V) {
// 			if (vpt[va >> PGSHIFT] & PTE_V) {
// 				u_long pa = PTE_ADDR(vpt[va >> PGSHIFT]);
// 				debugf("|%08x->%08x  ", va, pa);
// 				if (vpt[va >> PGSHIFT] & PTE_G) {
// 					debugf("G");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_V) {
// 					debugf("V");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_D) {
// 					debugf("D");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_N) {
// 					debugf("N");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_COW) {
// 					debugf("C");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_LIBRARY) {
// 					debugf("L");
// 				} else {
// 					debugf(" ");
// 				}
// 				if (vpt[va >> PGSHIFT] & PTE_DIRTY) {
// 					debugf(".");
// 				} else {
// 					debugf(" ");
// 				}
// 				debugf("|\n");
// 			}
// 		}
// 	}
// 	debugf("----------------------------\n");
// }

// void file_page_debug_va(u_long va) {
// 	if (vpd[va >> PDSHIFT] & PTE_V) {
// 		if (vpt[va >> PGSHIFT] & PTE_V) {
// 			u_long pa = PTE_ADDR(vpt[va >> PGSHIFT]);
// 			debugf("----------------------------\n");
// 			debugf("|%08x->%08x  ", va, pa);
// 			if (vpt[va >> PGSHIFT] & PTE_G) {
// 				debugf("G");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_V) {
// 				debugf("V");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_D) {
// 				debugf("D");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_N) {
// 				debugf("N");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_COW) {
// 				debugf("C");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_LIBRARY) {
// 				debugf("L");
// 			} else {
// 				debugf(" ");
// 			}
// 			if (vpt[va >> PGSHIFT] & PTE_DIRTY) {
// 				debugf(".");
// 			} else {
// 				debugf(" ");
// 			}
// 			debugf("|\n");
// 			debugf("----------------------------\n");
// 		} else {
// 			debugf("%08x invalid\n", va);
// 		}
// 	} else {
// 		debugf("%08x invalid\n", va);
// 	}
// }
