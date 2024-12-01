#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

char *filedisk = DEFAULT_FILE_DISK;

extern int fs_getattr(const char *path, struct stat *st);
/* Get file attributes */
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	return fs_getattr(path, st);
}

extern int fs_mkdir(const char *path);
/* Create a directory */
int
fisopfs_mkdir(const char *path, const mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	return fs_mkdir(path);
}

extern int fs_unlink(const char *path);
/* Remove a file */
int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	return fs_unlink(path);
}

extern int fs_rmdir(const char *path);
/* Remove a directory */
int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	return fs_rmdir(path);
}

extern int fs_truncate(const char *path, off_t size);
/* Change the size of a file */
int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	return fs_truncate(path, size);
}

extern int fs_read(const char *path, char *buffer, size_t size, off_t offset);

/* Read data from an open file */
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	return fs_read(path, buffer, size, offset);
}

extern int fs_write(const char *path, const char *buf, size_t size, off_t offset);
/* Write data to an open file */
int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s\n", path);
	return fs_write(path, buf, size, offset);
}

extern int fs_readdir(const char *path, void *buffer, off_t offset);
/* Read directory */
static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s - offset: %li\n", path, offset);

	off_t i = 0;
	char entry_name[248] = { 0 };
	while (fs_readdir(path, entry_name, i++) >= 0)
		filler(buffer, entry_name, NULL, 0);

	return 0;
}

extern void *fs_init(const char *filepath);
/* Initialize filesystem */
void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");
	return fs_init(filedisk);
}

extern void *fs_destroy(const char *filepath);
/* Clean up filesystem */
void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy\n");
	fs_destroy(filedisk);
}

extern int fs_create(const char *path, mode_t mode);
/* Create and open a file */
int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	return fs_create(path, mode);
}

extern int fs_utimens(const char *path, const struct timespec tv[2]);
/* Change the access and modification times of a file with
        nanosecond resolution */
int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	return fs_utimens(path, tv);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.init = fisopfs_init,
	.write = fisopfs_write,
	.destroy = fisopfs_destroy,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.truncate = fisopfs_truncate,
};


int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];
			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}
			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}