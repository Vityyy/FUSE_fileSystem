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

int fisopfs_write_buf(const char *path, struct fuse_bufvec *buf, off_t offset, struct fuse_file_info *fi) {
    printf("[debug] fisopfs_write_buf\n");
    return 0;
}

int fisopfs_read_buf(const char *path, struct fuse_bufvec **bufp, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("[debug] fisopfs_read_buf\n");
    return 0;
}

int fisopfs_flock(const char *path, struct fuse_file_info *fi, int operation) {
    printf("[debug] fisopfs_flock\n");
    return 0;
}

int fisopfs_fallocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *fi) {
    printf("[debug] fisopfs_fallocate\n");
    return 0;
}

void fisopfs_destroy(void *private_data) {
    printf("[debug] fisopfs_destroy\n");
}

int fisopfs_flush(const char *path, struct fuse_file_info *fi) {
    printf("[debug] fisopfs_flush\n");
    return 0;
}

void *fisopfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    printf("[debug] fisopfs_init\n");
    return NULL;
}

int fisopfs_link(const char *oldpath, const char *newpath) {
    printf("[debug] fisopfs_link\n");
    return 0;    
}

int fisopfs_mknod(const char *path, mode_t mode, dev_t dev) {
    printf("[debug] fisopfs_mknod\n");
    return 0;
}

int fisopfs_readlink(const char *path, char *buf, size_t size) {
    printf("[debug] fisopfs_readlink\n");
    return 0;
}

int fisopfs_release(const char *path, struct fuse_file_info *fi) {
    printf("[debug] fisopfs_release\n");
    return 0;
}

int fisopfs_symlink(const char *target, const char *linkpath) {
    printf("[debug] fisopfs_symlink\n");
    return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.write_buf = fisopfs_write_buf,
    	.read_buf = fisopfs_read_buf,
    	.flock = fisopfs_flock,
    	.fallocate = fisopfs_fallocate,
    	.destroy = fisopfs_destroy,
    	.flush = fisopfs_flush,
    	.init = fisopfs_init,
    	.link = fisopfs_link,
    	.mknod = fisopfs_mknod,
    	.readlink = fisopfs_readlink,
    	.release = fisopfs_release,
   	.symlink = fisopfs_symlink
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
