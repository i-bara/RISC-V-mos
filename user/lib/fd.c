#include <env.h>
#include <fd.h>
#include <lib.h>
#include <mmu.h>

static struct Dev *devtab[] = {&devfile, &devcons,
#if !defined(LAB) || LAB >= 6
			       &devpipe,
#endif
			       0};

int dev_lookup(int dev_id, struct Dev **dev) {
	for (int i = 0; devtab[i]; i++) {
		if (devtab[i]->dev_id == dev_id) {
			*dev = devtab[i];
			return 0;
		}
	}

	debugf("[%08x] unknown device type %d\n", env->env_id, dev_id);
	return -E_INVAL;
}

// Overview:
//  Find the smallest i from 0 to MAXFD-1 that doesn't have its fd page mapped.
//
// Post-Condition:
//   Set *fd to the fd page virtual address.
//   (Do not allocate any pages: It is up to the caller to allocate
//    the page, meaning that if someone calls fd_alloc twice
//    in a row without allocating the first page we returned, we'll
//    return the same page at the second time.)
//   Return 0 on success, or an error code on error.
int fd_alloc(struct Fd **fd) {
	u_int va;
	u_int fdno;

	for (fdno = 0; fdno < MAXFD - 1; fdno++) {
		va = INDEX2FD(fdno);
		
		if (!(pt2[va >> VPN2_SHIFT] & PTE_V)) {
			*fd = (struct Fd *)va;
			return 0;
		}

		if (!(pt1[va >> VPN1_SHIFT] & PTE_V)) {
			*fd = (struct Fd *)va;
			return 0;
		}

		if (!(pt0[va >> VPN0_SHIFT] & PTE_V)) { // the fd is not used
			*fd = (struct Fd *)va;
			return 0;
		}

		// if ((vpd[va / PDMAP] & PTE_V) == 0) {
		// 	*fd = (struct Fd *)va;
		// 	return 0;
		// }

		// if ((vpt[va / BY2PG] & PTE_V) == 0) {
		// 	*fd = (struct Fd *)va;
		// 	return 0;
		// }
	}

	return -E_MAX_OPEN;
}

void fd_close(struct Fd *fd) {
	syscall_mem_unmap(0, fd);
}

// Overview:
//  Find the 'Fd' page for the given fd number.
//
// Post-Condition:
//  Return 0 and set *fd to the pointer to the 'Fd' page on success.
//  Return -E_INVAL if 'fdnum' is invalid or unmapped.
int fd_lookup(int fdnum, struct Fd **fd) {
	u_int va;

	if (fdnum >= MAXFD) {
		return -E_INVAL;
	}

	va = INDEX2FD(fdnum);

	if (pt0[va >> VPN0_SHIFT] & PTE_V) { // the fd is used
		*fd = (struct Fd *)va;
		return 0;
	}

	// if ((vpt[va / BY2PG] & PTE_V) != 0) { // the fd is used
	// 	*fd = (struct Fd *)va;
	// 	return 0;
	// }

	return -E_INVAL;
}

u_long fd2data(struct Fd *fd) {
	return (u_long)INDEX2DATA(fd2num(fd));
}

int fd2num(struct Fd *fd) {
	return ((u_int)fd - FDTABLE) / BY2PG;
}

int num2fd(int fd) {
	return fd * BY2PG + FDTABLE;
}

int close(int fdnum) {
	int r;
	struct Dev *dev = NULL;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0 || (r = dev_lookup(fd->fd_dev_id, &dev)) < 0) {
		return r;
	}

	r = (*dev->dev_close)(fd);
	fd_close(fd);
	return r;
}

void close_all(void) {
	int i;

	for (i = 0; i < MAXFD; i++) {
		close(i);
	}
}

int dup(int oldfdnum, int newfdnum) {
	int i, r;
	u_long ova, nva;
	u_int pte;
	struct Fd *oldfd, *newfd;

	if ((r = fd_lookup(oldfdnum, &oldfd)) < 0) {
		return r;
	}

	close(newfdnum);
	newfd = (struct Fd *)INDEX2FD(newfdnum);
	ova = fd2data(oldfd);
	nva = fd2data(newfd);

	if ((pt2[ova >> VPN2_SHIFT] & PTE_V) && (pt1[ova >> VPN1_SHIFT] & PTE_V)) { // 原来是 vpd[PDX(ova)]，不知道有什么意义，可能是笔误
		for (i = 0; i < LARGE_PAGE_SIZE; i += BY2PG) {
			pte = pt0[(ova + i) >> VPN0_SHIFT];

			if (pte & PTE_V) {
				// should be no error here -- pd is already allocated
				if ((r = syscall_mem_map(0, (void *)(ova + i), 0, (void *)(nva + i),
							 pte & (PTE_R | PTE_W | PTE_U | PTE_LIBRARY))) < 0) { // 页面标记：D 改为 RWU
					goto err;
				}
			}
		}
	}

	if ((r = syscall_mem_map(0, oldfd, 0, newfd, pt0[(u_long)oldfd >> VPN0_SHIFT] & (PTE_R | PTE_W | PTE_U | PTE_LIBRARY))) <
	    0) {
		goto err;
	}

	return newfdnum;

err:
	syscall_mem_unmap(0, newfd);

	for (i = 0; i < LARGE_PAGE_SIZE; i += BY2PG) {
		syscall_mem_unmap(0, (void *)(nva + i));
	}

	return r;
}

// Overview:
//  Read at most 'n' bytes from 'fd' at the current seek position into 'buf'.
//
// Post-Condition:
//  Update seek position.
//  Return the number of bytes read successfully.
//  Return < 0 on error.
int read(int fdnum, void *buf, u_int n) {
	int r;

	// Similar to the 'write' function below.
	// Step 1: Get 'fd' and 'dev' using 'fd_lookup' and 'dev_lookup'.
	struct Dev *dev;
	struct Fd *fd;
	/* Exercise 5.10: Your code here. (1/4) */
	if ((r = fd_lookup(fdnum, &fd)) < 0) {
		return r;
	}
	if ((r = dev_lookup(fd->fd_dev_id, &dev)) < 0) {
		return r;
	}

	// Step 2: Check the open mode in 'fd'.
	// Return -E_INVAL if the file is opened for writing only (O_WRONLY).
	/* Exercise 5.10: Your code here. (2/4) */
	if (fd->fd_omode == O_WRONLY) {
		return -E_INVAL;
	}

	// Step 3: Read from 'dev' into 'buf' at the seek position (offset in 'fd').
	/* Exercise 5.10: Your code here. (3/4) */
	r = dev->dev_read(fd, buf, n, fd->fd_offset);

	// Step 4: Update the offset in 'fd' if the read is successful.
	/* Hint: DO NOT add a null terminator to the end of the buffer!
	 *  A character buffer is not a C string. Only the memory within [buf, buf+n) is safe to
	 *  use. */
	/* Exercise 5.10: Your code here. (4/4) */
	if (r > 0) {
		fd->fd_offset += r;
	}

	return r;
}

int readn(int fdnum, void *buf, u_int n) {
	int m, tot;

	for (tot = 0; tot < n; tot += m) {
		m = read(fdnum, (char *)buf + tot, n - tot);

		if (m < 0) {
			return m;
		}

		if (m == 0) {
			break;
		}
	}

	return tot;
}

int write(int fdnum, const void *buf, u_int n) {
	int r;
	struct Dev *dev;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0 || (r = dev_lookup(fd->fd_dev_id, &dev)) < 0) {
		return r;
	}

	if ((fd->fd_omode & O_ACCMODE) == O_RDONLY) {
		return -E_INVAL;
	}

	r = dev->dev_write(fd, buf, n, fd->fd_offset);
	if (r > 0) {
		fd->fd_offset += r;
	}

	return r;
}

int seek(int fdnum, u_int offset) {
	int r;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0) {
		return r;
	}

	fd->fd_offset = offset;
	return 0;
}

int fstat(int fdnum, struct Stat *stat) {
	int r;
	struct Dev *dev = NULL;
	struct Fd *fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0 || (r = dev_lookup(fd->fd_dev_id, &dev)) < 0) {
		return r;
	}

	stat->st_name[0] = 0;
	stat->st_size = 0;
	stat->st_isdir = 0;
	stat->st_dev = dev;
	return (*dev->dev_stat)(fd, stat);
}

int stat(const char *path, struct Stat *stat) {
	int fd, r;

	if ((fd = open(path, O_RDONLY)) < 0) {
		return fd;
	}

	r = fstat(fd, stat);
	close(fd);
	return r;
}

void debug_fd() {
	debugf("[debug] fd\n");
	debugf("no    dev     name        omode         offset    fileid    type    size    ref         \n");
	for (u_int fdno = 0; fdno < MAXFD; fdno++) {
		u_int va = INDEX2FD(fdno);

		if ((pt2[va >> VPN2_SHIFT] & PTE_V) && (pt1[va >> VPN1_SHIFT] & PTE_V) && (pt0[va >> VPN0_SHIFT] & PTE_V)) {
			struct Fd *fd = (struct Fd *)va;
			debugf("%-4d  ", fdno);
			if (fd->fd_dev_id == devcons.dev_id) {
				debugf("cons    <cons>      ");
			} else if (fd->fd_dev_id == devfile.dev_id) {
				debugf("file    %-10s  ", ((struct Filefd *) va)->f_file.f_name);
			} else if (fd->fd_dev_id == devpipe.dev_id) {
				debugf("pipe    <pipe>      ");
			} else {
				debugf("                    ");
			}
			if (fd->fd_omode == O_RDONLY) {
				debugf("read only     ");
			} else if (fd->fd_omode == O_WRONLY) {
				debugf("write only    ");
			} else if (fd->fd_omode == O_RDWR) {
				debugf("read write    ");
			} else if (fd->fd_omode == O_ACCMODE) {
				debugf("acc mode      ");
			} else {
				debugf("              ");
			}
			debugf("%-8d  ", fd->fd_offset);

			if (fd->fd_dev_id == devfile.dev_id) {
				struct Filefd *ffd = (struct Filefd *)va;
				debugf("%-8d  ", ffd->f_fileid);
				if (ffd->f_file.f_type == FTYPE_DIR) {
					debugf("dir     ");
				} else if (ffd->f_file.f_type == FTYPE_REG) {
					debugf("file    ");
				} else {
					debugf("        ");
				}
				debugf("%-8d", ffd->f_file.f_size);
			} else {
				debugf("                          ");
			}

			debugf("%-4d  %-4d  ", pageref((void *)fd), pageref((void *)fd2data(fd)));

			debugf("\n");
		}
	}
}
