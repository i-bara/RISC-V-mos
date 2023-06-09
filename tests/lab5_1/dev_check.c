#include <lib.h>

int main() {
	char buf[16 * 512];

	for (int i = 8; i < 32; i++) {
		for (int j = 0; j < 16; j++) {
			ide_read(0, i, (buf + j * 512), 1);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[0]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[1]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[2]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[3]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[4]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[5]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[6]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[7]);

			((u_long *)(buf + j * 512))[0] = 0x12345678 + 133 * i + 1333 * j;
			((u_long *)(buf + j * 512))[63] = 0x1234abcd + 133 * i + 1333 * j;

			ide_write(0, i, (buf + j * 512), 1);

			ide_read(0, i, (buf + j * 512), 1);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[0]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[1]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[2]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[3]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[4]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[5]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[6]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[7]);
			// debugf("%016lx\n", ((u_long *)(buf + j * 512))[8]);

			if (((u_long *)(buf + j * 512))[0] != 0x0000000012345678 + 133 * i + 1333 * j) {
				user_panic("Error in disk");
			}

			if (((u_long *)(buf + j * 512))[63] != 0x000000001234abcd + 133 * i + 1333 * j) {
				user_panic("Error in disk");
			}
		}
	}

	debugf("Disk test passed!\n");
	// debugf("devtst begin\n");
	// int i = 0;
	// int r;
	// char buf[32] __attribute__((aligned(4))) = {0};
	// char c __attribute__((aligned(4))) = 0;
	// u_int cons = 0x10000000;
	// while (1) {
	// 	if ((r = syscall_read_dev(&c, cons, 1)) != 0) {
	// 		debugf("syscall_read_dev is bad\n");
	// 	}
	// 	if (c == '\r') {
	// 		break;
	// 	}
	// 	if (c != 0) {
	// 		buf[i++] = c;
	// 	}
	// }
	// if (i == 14) {
	// 	debugf("syscall_read_dev is good\n");
	// }
	// buf[i++] = '\n';
	// for (int j = 0; j < i; j++) {
	// 	int ch = buf[j];
	// 	if ((r = syscall_write_dev(&ch, cons, 4)) != 0) {
	// 		debugf("syscall_write_dev is bad\n");
	// 	}
	// }
	// debugf("end of devtst\n");

	// if (syscall_read_dev(&c, 0x0fffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x10000020, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x1000001f, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// if (syscall_read_dev(&c, 0x12ffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x13004200, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x130041ff, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// if (syscall_read_dev(&c, 0x14ffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x15000200, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_read_dev(&c, 0x150001ff, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// if (syscall_write_dev(&c, 0x0fffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x10000020, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x1000001f, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// if (syscall_write_dev(&c, 0x12ffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x13004200, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x130041ff, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// if (syscall_write_dev(&c, 0x14ffffff, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x15000200, 1) != -3) {
	// 	user_panic("failed dev address test");
	// }
	// if (syscall_write_dev(&c, 0x150001ff, 8) != -3) {
	// 	user_panic("failed dev address test");
	// }

	// debugf("dev address is ok\n");

	// syscall_read_dev(&c, 0x10000010, 4);
	return 0;
}
