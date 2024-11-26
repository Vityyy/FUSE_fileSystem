#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <fuse/fuse_common.h>
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

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path, "/fisop") == 0) {
		st->st_uid = 1818;
		st->st_mode = __S_IFREG | 0644;
		st->st_size = 2048;
		st->st_nlink = 1;
	} else {
		return -ENOENT;
	}

	return 0;
}

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

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

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

/**
 * Check file access permissions
 */
int
fisopfs_access(const char *pathname, const int mode)
{
	printf("fisopfs_access\n");
	return 0;
}

/** File open operation **/
int
fisopfs_open(const char *pathname, struct fuse_file_info *mode)
{
	printf("fisopfs_open\n");
	return 0;
}

/** Write data to an open file */
int
fisopfs_write(const char *path,
              const char *buff,
              const size_t n_bytes,
              const off_t offset,
              struct fuse_file_info *file_info)
{
	printf("fisopfs_write");
	return 0;
}

/**
 * Create and open a file
 */
int
fisopfs_create(const char *pathname,
               const mode_t mode,
               struct fuse_file_info *file_info)
{
	printf("fisopfs_create\n");
	return 0;
}

/** Remove a file */
int
fisopfs_unlink(const char *pathname)
{
	printf("fisopfs_unlink");
	return 0;
}

/** Create a directory */
int
fisopfs_mkdir(const char *pathname, const mode_t mode)
{
	printf("fisopfs_mkdir\n");
	return 0;
}

/** Remove a directory */
int
fisopfs_rmdir(const char *pathname)
{
	printf("fisopfs_rmdir\n");
	return 0;
}

/** Rename a file */
int
fisopfs_rename(const char *oldpath, const char *newpath)
{
	printf("fisopfs_rename");
	return 0;
}

/** Change the size of a file */
int
fisopfs_truncate(const char *path, off_t length)
{
	printf("fisopfs_truncate");
	return 0;
}

/** Change the permission bits of a file */
int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("fisopfs_chmod");
	return 0;
}

/** Change the owner and group of a file */
int
fisopfs_chown(const char *pathname, uid_t owner, gid_t group)
{
	printf("fisopfs_chown");
	return 0;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 */
int
fisopfs_statfs(const char *path, struct statvfs *buf)
{
	printf("fisopfs_statfs");
	return 0;
}

/** Synchronize file contents
 */
int
fisopfs_fsync(const char *path, int isdatasync, struct fuse_file_info *info)
{
	printf("fisopfs_fsync");
	return 0;
}

/** Set extended attributes */
int
fisopfs_setxattr(const char *path,
                 const char *name,
                 const char *value,
                 size_t size,
                 int flags)
{
	printf("fisopfs_setxattr");
	return 0;
}

/** Get extended attributes */
int
fisopfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	printf("[debug] fisopfs_getxattr");
	return 0;
}

/** List extended attributes */
int
fisopfs_listxattr(const char *path, char *list, size_t size)
{
	printf("[debug] fisopfs_listxattr");
	return 0;
}

/** Remove extended attributes */
int
fisopfs_removexattr(const char *path, const char *name)
{
	printf("[debug] fisopfs_removexattr");
	return 0;
}

/** Open directory */
int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_opendir");
	return 0;
}

/** Release directory */
int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_releasedir");
	return 0;
}

/** Synchronize directory contents */
int
fisopfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fsyncdir");
	return 0;
}

/** Change the size of an open file */
int
fisopfs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_ftruncate");
	return 0;
}

/** Get attributes from an open file */
int
fisopfs_fgetattr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fgetattr");
	return 0;
}

/** Perform POSIX file locking operation */
int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock)
{
	printf("[debug] fisopfs_lock");
	return 0;
}

/** Change the access and modification times of a file with nanosecond resolution */
int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_timens");
	return 0;
}

/** Map block index within file to block index within device */
int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *idx)
{
	printf("[debug] fisopfs_lock");
	return 0;
}

/** Ioctl */
int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *info,
              unsigned int flags,
              void *data)
{
	printf("[debug] fisopfs_ioctl");
	return 0;
}

/** Poll for IO readiness events */
int
fisopfs_poll(const char *path,
             struct fuse_file_info *info,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	printf("[debug] fisopfs_poll");
	return 0;
}

int
fisopfs_write_buf(const char *path,
                  struct fuse_bufvec *buf,
                  off_t offset,
                  struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write_buf\n");
	return 0;
}

int
fisopfs_read_buf(const char *path,
                 struct fuse_bufvec **bufp,
                 size_t size,
                 off_t offset,
                 struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read_buf\n");
	return 0;
}


int
fisopfs_flock(const char *path, struct fuse_file_info *fi, int operation)
{
	printf("[debug] fisopfs_flock\n");
	return 0;
}

int
fisopfs_fallocate(const char *path,
                  int mode,
                  off_t offset,
                  off_t length,
                  struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fallocate\n");
	return 0;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy\n");
}

int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush\n");
	return 0;
}

/**
 * Initialize filesystem
 */
void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");
	return NULL;
}

int
fisopfs_link(const char *oldpath, const char *newpath)
{
	printf("[debug] fisopfs_link\n");
	return 0;
}

int
fisopfs_mknod(const char *path, mode_t mode, dev_t dev)
{
	printf("[debug] fisopfs_mknod\n");
	return 0;
}

int
fisopfs_readlink(const char *path, char *buf, size_t size)
{
	printf("[debug] fisopfs_readlink\n");
	return 0;
}

int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_release\n");
	return 0;
}

int
fisopfs_symlink(const char *target, const char *linkpath)
{
	printf("[debug] fisopfs_symlink\n");
	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,        //
	.readdir = fisopfs_readdir,        //
	.read = fisopfs_read,              // ?
	.access = fisopfs_access,          //
	.open = fisopfs_open,              //
	.write = fisopfs_write,            // ?
	.create = fisopfs_create,          //
	.mkdir = fisopfs_mkdir,            //
	.rmdir = fisopfs_rmdir,            // ?
	.truncate = fisopfs_truncate,      //
	.statfs = fisopfs_statfs,          // ?
	.setxattr = fisopfs_setxattr,      // ?
	.getxattr = fisopfs_getxattr,      //
	.opendir = fisopfs_opendir,        //
	.releasedir = fisopfs_releasedir,  //
	.ftruncate = fisopfs_ftruncate,    // ?
	.fgetattr = fisopfs_fgetattr,      //
	.lock = fisopfs_lock,              //
	.write_buf = fisopfs_write_buf,    //
	.read_buf = fisopfs_read_buf,      //
	.flock = fisopfs_flock,            // ?
	.fallocate = fisopfs_fallocate,    // ?
	.destroy = fisopfs_destroy,        //
	.flush = fisopfs_flush,            //
	.init = fisopfs_init,              //
	.release = fisopfs_release         //
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
