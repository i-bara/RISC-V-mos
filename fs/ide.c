/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	// if ((u_long)dst % BY2SECT != 0) {
	// 	user_panic("Read dst not aligned");
	// }
	for (u_int i = 0; i < nsecs; i++) {
		syscall_read_sector((u_long)dst + i * BY2SECT, secno + i);
	}

	// u_int begin = secno * BY2SECT;
	// u_int end = begin + nsecs * BY2SECT;

	// for (u_int off = 0; off < nsecs * BY2SECT; off += BY2SECT) {
	// 	uint32_t temp = diskno;
	// 	/* Exercise 5.3: Your code here. (1/2) */
	// 	temp = begin + off;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, sizeof(uint32_t));
	// 	temp = 0;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET_HIGH32, sizeof(uint32_t));
	// 	temp = diskno;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, sizeof(uint32_t));
	// 	temp = DEV_DISK_OPERATION_READ;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, sizeof(uint32_t));
	// 	syscall_read_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_STATUS, sizeof(uint32_t));
	// 	if (!temp) {
	// 		user_panic("Read error: %08x(%08x+%08x)", diskno, begin, off);
	// 	}
	// 	syscall_read_dev(dst + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, DEV_DISK_BUFFER_LEN);

	// }
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	// if ((u_long)src % BY2SECT != 0) {
	// 	user_panic("Write src not aligned");
	// }
	for (u_int i = 0; i < nsecs; i++) {
		syscall_write_sector((u_long)src + i * BY2SECT, secno + i);
	}

	// u_int begin = secno * BY2SECT;
	// u_int end = begin + nsecs * BY2SECT;

	// for (u_int off = 0; off < nsecs * BY2SECT; off += BY2SECT) {
	// 	uint32_t temp = diskno;
	// 	/* Exercise 5.3: Your code here. (2/2) */
	// 	temp = begin + off;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, sizeof(uint32_t));
	// 	temp = 0;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET_HIGH32, sizeof(uint32_t));
	// 	temp = diskno;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, sizeof(uint32_t));
	// 	syscall_write_dev(src + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, DEV_DISK_BUFFER_LEN);
	// 	temp = DEV_DISK_OPERATION_WRITE;
	// 	syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, sizeof(uint32_t));
	// 	syscall_read_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_STATUS, sizeof(uint32_t));
	// 	if (!temp) {
	// 		user_panic("Write error: %08x(%08x+%08x)", diskno, begin, off);
	// 	}

	// }
}
