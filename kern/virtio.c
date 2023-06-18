#include <virtio.h>
#include <asm/asm.h>
#include <env.h>
#include <kclock.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include <sbi.h>

struct virtq vq __attribute__((aligned(PAGE_SIZE)));
struct virtq_desc desc[QUEUE_SIZE] __attribute__((aligned(PAGE_SIZE)));
struct virtq_avail avail[1] __attribute__((aligned(PAGE_SIZE)));
struct virtq_used used[1] __attribute__((aligned(PAGE_SIZE)));
struct virtio_blk_req read_buffer;
struct virtio_blk_req write_buffer;
struct virtio_blk_req flush_buffer;

#ifdef RISCV32
typedef le32 le;
#else
typedef le64 le;
#endif

void virtio_init() {
	for (u_long diskva = 0xb0001000; diskva < 0xb0009000; diskva += 0x1000) {
		struct Virtio *disk = (struct Virtio *)diskva;
		printk("virtio %d: ", (diskva - 0xb0001000) >> 12);
		if (disk->device_id == 0) {
			printk("Unavailable");
		} else if (disk->device_id == 1) {
			printk("Network Device");
		} else if (disk->device_id == 2) {
			printk("Block Device");
		} else if (disk->device_id == 3) {
			printk("Console Device");
		} else if (disk->device_id == 4) {
			printk("Entropy Device");
		} else if (disk->device_id == 5) {
			printk("Traditional Memory Balloon Device");
		} else if (disk->device_id == 6) {
			printk("SCSI Host Device");
		} else {
			printk("Unknown");
		}
		printk("\n");
	}

	u_long diskva = 0xb0008000; // 映射到了这个虚拟地址
	struct Virtio *disk = (struct Virtio *)diskva;
	printk("queue num max    : %08x\n", disk->queue_num_max);

	u_int magic_value = disk->magic_value;
	u_int version = disk->version;
	u_int device_id = disk->device_id;
	printk("magic value      : %08x\n", magic_value);
	printk("version          : %08x\n", version);
	printk("device id        : %08x\n", device_id);
	assert(device_id == 2);

	// The driver MUST follow this sequence to initialize a device:

	// Reset the device.
	disk->status = 0;

	// Set the ACKNOWLEDGE status bit: the guest OS has notice the device.
	// Set the DRIVER status bit: the guest OS knows how to drive the device.
	disk->status |= ACKNOWLEDGE | DRIVER;

	/**
	 * Read device feature bits, and write the subset of feature bits understood by the OS and driver to the
	 * device. During this step the driver MAY read (but MUST NOT write) the device-specific configuration
	 * fields to check that it can support the device before accepting it.
	*/
	u_int device_features = disk->device_features;
	printk("feature bits     : %016lx\n", device_features);
	disk->driver_features = device_features;

	// Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
	disk->status |= FEATURES_OK;

	/**
	 * Re-read device status to ensure the FEATURES_OK bit is still set: otherwise, the device does not
	 * support our subset of features and the device is unusable.
	*/
	printk("status           : %016lx\n", disk->status);

	/**
	 * Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup,
	 * reading and possibly writing the device’s virtio configuration space, and population of virtqueues.
	*/

	// Set the DRIVER_OK status bit. At this point the device is “live”.
	disk->status |= DRIVER_OK;

	/**
	 * If any of these steps go irrecoverably wrong, the driver SHOULD set the FAILED status bit to indicate that it
	 * has given up on the device (it can reset the device later to restart if desired). The driver MUST NOT continue
	 * initialization in that case.
	 * 
	 * The driver MUST NOT notify the device before setting DRIVER_OK.
	*/


	/**
	 * When the driver wants to send a read_buffer to the device, it fills in a slot in the descriptor table (or chains several
	 * together), and writes the descriptor index into the available ring. It then notifies the device. When the device
	 * has finished a read_buffer, it writes the descriptor index into the used ring, and sends an interrupt.
	*/

	disk->queue_num = QUEUE_SIZE;

	disk->queue_desc = (le)desc;
	disk->queue_avail = (le)avail;
	disk->queue_used = (le)used;

	assert(disk->queue_ready == 0);
	disk->queue_ready = 1;
	assert(disk->queue_ready == 1);
	
	read_buffer.type = VIRTIO_BLK_T_IN; // 读

	desc[0].addr = (le)&read_buffer;
	desc[0].len = 16;
	desc[0].flags = VIRTQ_DESC_F_NEXT;
	desc[0].next = 1;

	desc[1].addr = (le)&read_buffer.data;
	desc[1].len = 512;
	desc[1].flags = VIRTQ_DESC_F_WRITE | VIRTQ_DESC_F_NEXT; // 设备可写
	desc[1].next = 2;

	desc[2].addr = (le)&read_buffer.status;
	desc[2].len = 1;
	desc[2].flags = VIRTQ_DESC_F_WRITE;

    write_buffer.type = VIRTIO_BLK_T_OUT; // 写

    desc[3].addr = (le)&write_buffer;
	desc[3].len = 16;
	desc[3].flags = VIRTQ_DESC_F_NEXT;
	desc[3].next = 4;

	desc[4].addr = (le)&write_buffer.data;
	desc[4].len = 512;
	desc[4].flags = VIRTQ_DESC_F_NEXT; // 设备可读
	desc[4].next = 5;

	desc[5].addr = (le)&write_buffer.status;
	desc[5].len = 1;
	desc[5].flags = VIRTQ_DESC_F_WRITE;

	flush_buffer.type = VIRTIO_BLK_T_FLUSH; // flush

    desc[6].addr = (le)&flush_buffer;
	desc[6].len = 16;
	desc[6].flags = VIRTQ_DESC_F_NEXT;
	desc[6].next = 7;

	desc[7].addr = (le)&flush_buffer.data;
	desc[7].len = 512;
	desc[7].flags = VIRTQ_DESC_F_NEXT; // 设备可读
	desc[7].next = 8;

	desc[8].addr = (le)&flush_buffer.status;
	desc[8].len = 1;
	desc[8].flags = VIRTQ_DESC_F_WRITE;

	// for (int i = 0; i < 3; i++) {
	// 	printk("%lx-%lx\n", desc[i].addr, desc[i].addr + desc[i].len);
	// }
	printk("device size      : %lx\n", (u_long)disk->config.capacity); // 6.17 printk 尚未支持 long long，因此不加 (u_long) 可能导致异常
	printk("config generation: %d\n", disk->config_generation);
	// printk("size=%d\n", (le)sizeof(read_buffer));



	// 测试读写功能

    // read_sector(disk, 8);
    // memcpy((void *)&write_buffer.data, (void *)&read_buffer.data, 512);
    // ((u_long *)write_buffer.data)[0] = 0x12345678;
    // write_sector(disk, 8);
    // read_sector(disk, 8);

	// printk("sector = %d\n", read_buffer.sector);
    // debug_sector(write_buffer);
    // debug_sector(read_buffer);

	// halt();



	// disk->queue_sel = 0;
	// printk("0 %016lx\n", disk->queue_num_max);
	// disk->queue_sel = 1;
	// printk("1 %016lx\n", disk->queue_num_max);
	// disk->queue_sel = 2;
	// printk("2 %016lx\n", disk->queue_num_max);
	
	// halt();
}

void read_sector(struct Virtio *disk, le64 sector) {
    read_buffer.sector = sector;
	read_buffer.status = -1;

    avail->ring[avail->idx & (QUEUE_SIZE - 1)] = 0;
	avail->idx++;

    disk->queue_notify = 0;

    while (used->idx < avail->idx) {
		// printk("-");
	}

	// printk("%x\n", sector);
	// debug_sector(read_buffer);

	// printk("read %d %x\n", used->idx, read_buffer.status);
}

void write_sector(struct Virtio *disk, le64 sector) {
    write_buffer.sector = sector;
	write_buffer.status = -1;

    avail->ring[avail->idx & (QUEUE_SIZE - 1)] = 3;
	avail->idx++;

    disk->queue_notify = 0;

    while (used->idx < avail->idx) {
		// printk("-");
	}

	// printk("%x\n", sector);
	// debug_sector(write_buffer);

	// printk("write %d %x\n", used->idx, write_buffer.status);
}

void flush(struct Virtio *disk) {
    flush_buffer.sector = 0;
	flush_buffer.status = -1;

    avail->ring[avail->idx & (QUEUE_SIZE - 1)] = 6;
	avail->idx++;

    disk->queue_notify = 0;

    while (used->idx < avail->idx) {
		// printk("-");
	}

	// printk("flush %d %x\n", used->idx, flush_buffer.status);
}

void debug_sector(struct virtio_blk_req buffer) {
    printk("%016lx\n", ((u_long *)buffer.data)[0]);
	printk("%016lx\n", ((u_long *)buffer.data)[1]);
	printk("%016lx\n", ((u_long *)buffer.data)[2]);
	printk("%016lx\n", ((u_long *)buffer.data)[3]);
	printk("%016lx\n", ((u_long *)buffer.data)[4]);
	printk("%016lx\n", ((u_long *)buffer.data)[5]);
	printk("%016lx\n", ((u_long *)buffer.data)[6]);
	printk("%016lx\n", ((u_long *)buffer.data)[7]);
	printk("%016lx\n", ((u_long *)buffer.data)[8]);
}
