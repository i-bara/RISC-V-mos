#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include <types.h>
#include <virtio_queue.h>

struct virtio_blk_config {
    le64 capacity;
    le32 size_max;
    le32 seg_max;
    struct virtio_blk_geometry {
        le16 cylinders;
        u8 heads;
        u8 sectors;
    } geometry;
    le32 blk_size;
    struct virtio_blk_topology {
        // # of logical blocks per physical block (log2)
        u8 physical_block_exp;
        // offset of first aligned logical block
        u8 alignment_offset;
        // suggested minimum I/O size in blocks
        le16 min_io_size;
        // optimal (suggested maximum) I/O size in blocks
        le32 opt_io_size;
    } topology;
    u8 writeback;
};

struct Virtio {
	volatile u_int magic_value;
    volatile u_int version;
    volatile u_int device_id;
    volatile u_int vendor_id;
    volatile u_int device_features;
    volatile u_int device_features_sel;
    volatile u_int device_features_reserved1;
    volatile u_int device_features_reserved2;
    volatile u_int driver_features;
    volatile u_int driver_features_sel;
    volatile u_int driver_features_reserved1;
    volatile u_int driver_features_reserved2;
    volatile u_int queue_sel;
    volatile u_int queue_num_max;
    volatile u_int queue_num;
    volatile u_int queue_reserved1;
    volatile u_int queue_reserved2;
    volatile u_int queue_ready;
    volatile u_int queue_reserved3;
    volatile u_int queue_reserved4;
    volatile u_int queue_notify; // 0x50
    volatile u_int queue_reserved5;
    volatile u_int queue_reserved6;
    volatile u_int queue_reserved7;
    volatile u_int interrupt_status;
    volatile u_int interrupt_ack;
    volatile u_int interrupt_reserved1;
    volatile u_int interrupt_reserved2;
    volatile u_int status; // 0x70
    volatile u_int status_reserved1;
    volatile u_int status_reserved2;
    volatile u_int status_reserved3;
    volatile u_long queue_desc;
    volatile u_long queue_desc_padding;
    volatile u_long queue_avail;
    volatile u_long queue_avail_padding;
    volatile u_long queue_used;
    volatile u_long queue_used_padding;
    volatile u_char padding[0x4c];
    volatile u_int config_generation;
    volatile struct virtio_blk_config config;
};

struct virtio_blk_req {
    le32 type;
    le32 reserved;
    le64 sector;
    u8 data[512];
    u8 status;
};

#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4

#define VIRTIO_BLK_S_OK 0
#define VIRTIO_BLK_S_IOERR 1
#define VIRTIO_BLK_S_UNSUPP 2

// Feature Bits
// 0 to 23 Feature bits for the specific device type
// 24 to 32 Feature bits reserved for extensions to the queue and feature negotiation mechanisms
// 33 and above Feature bits reserved for future extensions.

#define ACKNOWLEDGE (1)
#define DRIVER (2)
#define FAILED (128)
#define FEATURES_OK (8)
#define DRIVER_OK (4)
#define DEVICE_NEEDS_RESET (64)

extern struct virtio_blk_req read_buffer;
extern struct virtio_blk_req write_buffer;
extern struct virtio_blk_req flush_buffer;

void virtio_init();
void read_sector(struct Virtio *disk, le64 sector);
void write_sector(struct Virtio *disk, le64 sector);
void flush(struct Virtio *disk);
void debug_sector(struct virtio_blk_req buffer);

#endif /* !_VIRTIO_H_ */
