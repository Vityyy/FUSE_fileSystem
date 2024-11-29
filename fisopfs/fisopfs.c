
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


// IMPLEMENTED
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern int fs_getattr(const char *path, struct stat *st);
/* Get file attributes */
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	return fs_getattr(path, st);

	// if (strcmp(path, "/") == 0) {
	// 	st->st_uid = 1717;
	// 	st->st_mode = __S_IFDIR | 0755;
	// 	st->st_nlink = 2;
	// } else if (strcmp(path, "/fisop") == 0) {
	// 	st->st_uid = 1818;
	// 	st->st_mode = __S_IFREG | 0644;
	// 	st->st_size = 2048;
	// 	st->st_nlink = 1;
	// } else {
	// 	return -ENOENT;
	// }

	// return 0;
}

extern int fs_mkdir(const char *path);
/* Create a directory */
int
fisopfs_mkdir(const char *path, const mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	if (!S_ISDIR(mode))
		return -1;

	return fs_mkdir(path);
}

extern int fs_unlink(const char *path);
/* Remove a file */
int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	return 0;
}

extern int fs_rmdir(const char *path);
/* Remove a directory */
int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	return 0;
}

extern int fs_truncate(const char *path, off_t size);
/* Change the size of a file */
int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	return -1;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

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

	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fisop_file_contenidos + offset, size);

	return size;
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
	return 0;
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
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		filler(buffer, "fisop", NULL, 0);
		return 0;
	}

	return -ENOENT;
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
	return 0;
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


// UNIMPLEMENTED
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Read the target of a symbolic link */
int
fisopfs_readlink(const char *path, char *buf, size_t size)
{
	printf("[debug] fisopfs_readlink - path: %s\n", path);
	return -1;
}

/* Create a file node */
int
fisopfs_mknod(const char *path, mode_t mode, dev_t dev)
{
	printf("[debug] fisopfs_mknod - path: %s\n", path);
	return -1;
}

/* Create a symbolic link */
int
fisopfs_symlink(const char *target, const char *linkpath)
{
	printf("[debug] fisopfs_symlink\n");
	return -1;
}

/* Rename a file */
int
fisopfs_rename(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_rename\n");
	return -1;
}

/* Create a hard link to a file */
int
fisopfs_link(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_link\n");
	return -1;
}

/* Change the permission bits of a file */
int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_chmod - path: %s\n", path);
	return -1;
}

/* Change the owner and group of a file */
int
fisopfs_chown(const char *path, uid_t owner, gid_t group)
{
	printf("[debug] fisopfs_chown - path: %s\n", path);
	return -1;
}

/* File open operation */
int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_open - path: %s\n", path);
	return -1;
}

/* Get file system statistics */
int
fisopfs_statfs(const char *path, struct statvfs *stbuf)
{
	printf("[debug] fisopfs_statfs - path: %s\n", path);
	return -1;
}

/* Possibly flush cached data */
int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);
	return -1;
}

/* Release an open file */
int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_release - path: %s\n", path);
	return -1;
}

/* Synchronize file contents */
int
fisopfs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fsync - path: %s\n", path);
	return -1;
}

/* Set extended attributes */
int
fisopfs_setxattr(const char *path,
                 const char *name,
                 const char *value,
                 size_t size,
                 int flags)
{
	printf("[debug] fisopfs_setxattr - path: %s\n", path);
	return -1;
}

/* Get extended attributes */
int
fisopfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	printf("[debug] fisopfs_getxattr - path: %s\n", path);
	return -1;
}

/* List extended attributes */
int
fisopfs_listxattr(const char *path, char *list, size_t size)
{
	printf("[debug] fisopfs_listxattr - path: %s\n", path);
	return -1;
}

/* Remove extended attributes */
int
fisopfs_removexattr(const char *path, const char *name)
{
	printf("[debug] fisopfs_removexattr - path: %s\n", path);
	return -1;
}

/* Open directory */
int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_opendir - path: %s\n", path);
	return -1;
}

/* Release directory */
int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_releasedir - path: %s\n", path);
	return -1;
}

/* Synchronize directory contents */
int
fisopfs_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fsyncdir - path: %s\n", path);
	return -1;
}

/* Check file access permissions */
int
fisopfs_access(const char *path, int mask)
{
	printf("[debug] fisopfs_access - path: %s\n", path);
	return -1;
}

/* Change the size of an open file */
int
fisopfs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_ftruncate - path: %s\n", path);
	return -1;
}

/* Get attributes from an open file */
int
fisopfs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fgetattr - path: %s\n", path);
	return -1;
}

/* Perform POSIX file locking operation */
int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock)
{
	printf("[debug] fisopfs_lock - path: %s\n", path);
	return -1;
}

/* Map block index within file to block index within device */
int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *idx)
{
	printf("[debug] fisopfs_bmap - path: %s\n", path);
	return -1;
}

/* Ioctl */
int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *fi,
              unsigned int flags,
              void *data)
{
	printf("[debug] fisopfs_ioctl - path: %s\n", path);
	return -1;
}

/* Poll for IO readiness events */
int
fisopfs_poll(const char *path,
             struct fuse_file_info *fi,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	printf("[debug] fisopfs_poll - path: %s\n", path);
	return -1;
}

/* Write contents of buffer to an open file */
int
fisopfs_write_buf(const char *path,
                  struct fuse_bufvec *buf,
                  off_t off,
                  struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write_buf - path: %s\n", path);
	return -1;
}

/* Store data from an open file in a buffer */
int
fisopfs_read_buf(const char *path,
                 struct fuse_bufvec **bufp,
                 size_t size,
                 off_t off,
                 struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read_buf - path: %s\n", path);
	return -1;
}

/* Perform BSD file locking operation */
int
fisopfs_flock(const char *path, struct fuse_file_info *fi, int op)
{
	printf("[debug] fisopfs_flock - path: %s\n", path);
	return -1;
}

/* Allocates space for an open file */
int
fisopfs_fallocate(const char *path,
                  int mode,
                  off_t offset,
                  off_t length,
                  struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fallocate - path: %s\n", path);
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static struct fuse_operations operations = { .getattr = fisopfs_getattr,
	                                     .readlink = fisopfs_readlink,
	                                     .mknod = fisopfs_mknod,
	                                     .mkdir = fisopfs_mkdir,
	                                     .unlink = fisopfs_unlink,
	                                     .rmdir = fisopfs_rmdir,
	                                     .symlink = fisopfs_symlink,
	                                     .rename = fisopfs_rename,
	                                     .link = fisopfs_link,
	                                     .chmod = fisopfs_chmod,
	                                     .chown = fisopfs_chown,
	                                     .truncate = fisopfs_truncate,
	                                     .open = fisopfs_open,
	                                     .read = fisopfs_read,
	                                     .write = fisopfs_write,
	                                     .statfs = fisopfs_statfs,
	                                     .flush = fisopfs_flush,  // ?
	                                     .release = fisopfs_release,
	                                     .fsync = fisopfs_fsync,
	                                     .setxattr = fisopfs_setxattr,
	                                     .getxattr = fisopfs_getxattr,
	                                     .listxattr = fisopfs_listxattr,
	                                     .removexattr = fisopfs_removexattr,
	                                     .opendir = fisopfs_opendir,
	                                     .readdir = fisopfs_readdir,
	                                     .releasedir = fisopfs_releasedir,
	                                     .fsyncdir = fisopfs_fsyncdir,
	                                     .init = fisopfs_init,
	                                     .destroy = fisopfs_destroy,
	                                     .access = fisopfs_access,
	                                     .create = fisopfs_create,
	                                     .ftruncate = fisopfs_ftruncate,
	                                     .fgetattr = fisopfs_fgetattr,
	                                     .lock = fisopfs_lock,
	                                     .utimens = fisopfs_utimens,
	                                     .bmap = fisopfs_bmap,
	                                     .ioctl = fisopfs_ioctl,
	                                     .poll = fisopfs_poll,
	                                     .write_buf = fisopfs_write_buf,
	                                     .read_buf = fisopfs_read_buf,
	                                     .flock = fisopfs_flock,
	                                     .fallocate = fisopfs_fallocate };


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