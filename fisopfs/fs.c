#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define MAX_FILES 1024
#define MAX_DENTRIES 16
#define MAX_FILE_SIZE 4096
#define MAX_FILE_NAME 248

typedef struct inode inode_t;
typedef struct dentry dentry_t;
typedef union block_data block_data_t;
typedef struct block block_t;

struct inode {
	struct stat st;
	ssize_t block_idx;   // in blocks table
	ssize_t parent_idx;  // in inodes table
};

struct dentry {
	char name[MAX_FILE_NAME];
	ssize_t inode_idx;
};

union block_data {
	dentry_t dentries[MAX_DENTRIES];  // if directory
	char buffer[MAX_FILE_SIZE];       // if regular file
};

struct block {
	ssize_t next_free;
	block_data_t data;
};


static ssize_t first_free = 0;
static inode_t inodes[MAX_FILES] = { 0 };
static block_t blocks[MAX_FILES] = { 0 };


// PRE: path should be a valid path (in terms of format).
// Maybe the file or directory does not exist, but the path
// must be a valid path. (Eg.: /dir/subdir/subsubdir/file)
ssize_t
get_inode_idx_from_path(const char *path)
{
	if (strcmp(path, "/") == 0)
		return 0;

	size_t inode_idx = 0;

	for (char *s = strtok(path, "/"); s != NULL; s = strtok(NULL, "/")) {
		ssize_t block_idx = inodes[inode_idx].block_idx;
		if (block_idx == -1) {
			// error
		}
		for (size_t i = 0; i < MAX_DENTRIES; i++) {
			dentry_t dentry = blocks[block_idx].data.dentries[i];
			if (strcmp(dentry.name, s) == 0) {
				inode_idx = dentry.inode_idx;
				break;
			}
		}
		return -1;  // Dentry not found
	}

	return inode_idx;
}

/* Get file attributes */
int
fs_getattr(const char *path, struct stat *st)
{
	ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1) {
		// error
	}

	struct stat *src = &inodes[inode_idx].st;
	struct stat *dst = st;

	memcpy(dst, src, sizeof(struct stat));

	return 0;
}

/* Create a directory */
int
fs_mkdir(const char *path, mode_t mode)
{
	// Completar

	return 0;
}

/* Remove a file */
int
fs_unlink(const char *path)
{
	// Completar

	return 0;
}

/* Remove a directory */
int
fs_rmdir(const char *path)
{
	// Completar

	return 0;
}

/* Change the size of a file */
int
fs_truncate(const char *path, off_t size)
{
	// Completar

	return 0;
}

/* Read data from an open file */
int
fs_read(const char *path, char *buffer, size_t size, off_t offset)
{
	// Completar

	return 0;
}

/* Write data to an open file */
int
fs_write(const char *path, const char *buf, size_t size, off_t offset)
{
	// Completar

	return 0;
}

/* Read directory */
int
fs_readdir(const char *path, void *buffer, off_t offset)
{
	// Completar

	return 0;
}

/*
 * New file system
 */
void
newFileSystem(void)
{
	first_free = 0;
	memset(inodes, 0, sizeof(inodes));
	memset(blocks, 0, sizeof(blocks));

	inodes[0] =
	        (inode_t){ .st = { .st_mode = __S_IFDIR | 0755, .st_nlink = 1 },
		           .block_idx = 0,
		           .parent_idx = -1 };

	blocks[0] = (block_t){
		.next_free = -1,
		.data = { .dentries = { { .name = ".", .inode_idx = 0 },
		                        { .name = "..", .inode_idx = -1 } } }
	};

	first_free = 1;

	for (ssize_t i = 1; i < 1023; i++) {
		inodes[i] = (inode_t){ .block_idx = -1, .parent_idx = -1 };
		blocks[i] = (block_t){ .next_free = i + 1 };
	}

	inodes[1023] = (inode_t){ .block_idx = -1, .parent_idx = -1 };
	blocks[1023] = (block_t){ .next_free = 1 };
}

/* Initialize filesystem */
void *
fs_init(const char *const restrict filepath)
{
	const int fd = open(filepath, O_RDONLY);
	if (fd == -1) {
		newFileSystem();
		return NULL;
	}

	ssize_t err = read(fd, &first_free, sizeof(first_free));
	if (err == -1) {
		newFileSystem();
		return NULL;
	}

	err = read(fd, inodes, sizeof(inodes));
	if (err == -1) {
		newFileSystem();
		return NULL;
	}

	err = read(fd, blocks, sizeof(blocks));
	if (err == -1) {
		newFileSystem();
		return NULL;
	}

	return NULL;
}

/* Clean up filesystem */
void
fs_destroy(const char *const restrict filepath)
{
	const int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
		return;

	ssize_t err = write(fd, &first_free, sizeof(first_free));
	if (err == -1)
		return;

	err = write(fd, inodes, sizeof(inodes));
	if (err == -1)
		return;

	err = write(fd, blocks, sizeof(blocks));
}

/* Create and open a file */
int
fs_create(const char *path, mode_t mode)
{
	// Completar

	return 0;
}

/* Change the access and modification times of a file with
        nanosecond resolution */
int
fs_utimens(const char *path, const struct timespec tv[2])
{
	ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1) {
		// error
	}

	struct stat *st = &inodes[inode_idx].st;

	st->st_atime = tv[0].tv_sec;
	st->st_mtime = tv[1].tv_sec;

	return 0;
}
