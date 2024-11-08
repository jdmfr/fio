/*
 * nvme structure declarations and helper functions for the
 * io_uring_cmd engine.
 */

#ifndef FIO_VIRTBLK_H
#define FIO_VIRTBLK_H

#include "../fio.h"

/*
 * If the uapi headers installed on the system lacks nvme uring command
 * support, use the local version to prevent compilation issues.
 */
#ifndef CONFIG_VIRTBLK_URING_CMD

struct virtio_blk_outhdr {
	/* VIRTIO_BLK_T* */
	uint32_t type;
	/* io priority. */
	uint32_t ioprio;
	/* Sector (ie. 512 byte offset) */
	uint64_t sector;
};

struct virtblk_req {
	struct virtio_blk_outhdr out_hdr;
	uint8_t status;
};

/* Virtio ring descriptors: 16 bytes.  These can chain together via "next". */
struct vring_desc {
	/* Address (guest-physical). */
	uint64_t addr;
	/* Length. */
	uint32_t len;
	/* The flags as indicated above. */
	uint16_t flags;
	/* We chain unused descriptors via this, too */
	uint16_t next;
};

struct virtblk_uring_cmd {
	/* VIRTIO_BLK_T* */
	uint32_t type;
	/* io priority. */
	uint32_t ioprio;
	/* Sector (ie. 512 byte offset) */
	uint64_t sector;

	uint64_t data;
	uint32_t data_len;

	uint32_t flag;
	uint32_t extra1; //used for save write/read counts.

};

#endif /* CONFIG_NVME_URING_CMD */

#define VIRTBLK_URING_CMD_IO 1
#define VIRTBLK_URING_CMD_IO_VEC 2

enum virtblk_io_opcode {
	VIRTIO_BLK_T_IN			= 0x0,
	VIRTIO_BLK_T_OUT		= 0x1,
	VIRTIO_BLK_T_SCSI_CMD		= 0x2,
	VIRTIO_BLK_T_FLUSH		= 0x4,
	VIRTIO_BLK_T_DISCARD		= 0xb,
	VIRTIO_BLK_T_WRITE_ZEROES	= 0xd,
	VIRTIO_BLK_T_BID		= 0x10,
};

struct virtblk_data {
	__u32 lba_shift;
};

int fio_virtblk_uring_cmd_prep(struct virtblk_uring_cmd *cmd, struct io_u *io_u,
			    struct iovec *iov);
int fio_virtblk_get_info(struct fio_file *f, __u64 *bytes);
int fio_virtblk_get_lba_shift(char* f_name);
#endif
