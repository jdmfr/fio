/*
 * nvme structure declarations and helper functions for the
 * io_uring_cmd engine.
 */

#include "virtblk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fio_virtblk_uring_cmd_prep(struct virtblk_uring_cmd *cmd, struct io_u *io_u,
			    struct iovec *iov)
{
	struct virtblk_data *data = FILE_ENG_DATA(io_u->file);
	__u64 slba;

	memset(cmd, 0, sizeof(struct virtblk_uring_cmd));

	if (io_u->ddir == DDIR_READ)
		cmd->type = VIRTIO_BLK_T_IN;
	else if (io_u->ddir == DDIR_WRITE)
		cmd->type = VIRTIO_BLK_T_OUT;
	slba = io_u->offset >> data->lba_shift;

	cmd->sector = slba;

	if (iov) {
		iov->iov_base = io_u->xfer_buf;
		iov->iov_len = io_u->xfer_buflen;
		cmd->data = (__u64)(uintptr_t)iov;
		cmd->data_len = 1;
	} else {
		cmd->data = (__u64)(uintptr_t)io_u->xfer_buf;
		cmd->data_len = io_u->xfer_buflen;
	}

	cmd->extra1 = 0;
	cmd->flag = 0;

	return 0;
}

/*
 * Following helpers just used for get block device 
 * size.
 */
char* extract_basename(char *path) {
    char *last_slash = strrchr(path, '/');

    if (last_slash != NULL) {
        return last_slash + 1;
    } else {
        return path;
    }
}

char* strip_suffix(const char* input, const char* suffix) {
    size_t input_len = strlen(input);
    size_t suffix_len = strlen(suffix);

    if (input_len >= suffix_len) {
        if (strcmp(input + input_len - suffix_len, suffix) == 0) {
            size_t new_len = input_len - suffix_len;
            char* new_str = (char*)malloc(new_len + 1);
            if (new_str == NULL) {
                fprintf(stderr, "Memory allocation failed!\n");
                exit(EXIT_FAILURE);
            }
            strncpy(new_str, input, new_len);
            new_str[new_len] = '\0';
            return new_str;
        }
    }
    
    return strdup(input);
}

int fio_virtblk_get_lba_shift(char* f_name)
{
	FILE *file;
	int block_size;
	char path[256];
	char* new_filename = strip_suffix(f_name, "c0");

	snprintf(path, sizeof(path), "/sys/block/%s/queue/physical_block_size", extract_basename(new_filename));

	file = fopen(path, "r");
	if (file == NULL) {
		return 0;
	}

	if (fscanf(file, "%d", &block_size) != 1) {
		fclose(file);
		return 0;
	}
	printf("pbs is 0x%llx\n",block_size);
	fclose(file);
	return block_size;
}

int fio_virtblk_get_info(struct fio_file *f, __u64 *bytes)
{
	int fd, err;
	char* new_filename = strip_suffix(f->file_name, "c0");

	if (f->filetype != FIO_TYPE_CHAR) {
		log_err("ioengine io_uring_cmd only works with nvme ns "
			"generic char devices (/dev/vdXc0)\n");
		return 1;
	}

	fd = open(new_filename, O_RDONLY);
	if (fd < 0)
		return -errno;

	err = ioctl(fd, BLKGETSIZE64, bytes);
	if (err < 0)
		return err;
	printf("blksize is 0x%llx\n",*bytes);
	close(fd);
	return 0;
}
