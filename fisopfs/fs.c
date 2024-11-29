#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <libgen.h>

#define MAX_FILES 1024
#define MAX_DENTRIES 16
#define MAX_FILE_SIZE 4096
#define MAX_FILE_NAME 248

typedef struct inode inode_t;
typedef struct dentry dentry_t;
typedef union block_data block_data_t;
typedef struct block block_t;

static char *
strdup(const char *const restrict __s)
{
	const size_t len = strlen(__s) + 1;
	char *copy = malloc(len);
	if (copy)
		memcpy(copy, __s, len);

	return copy;
}

struct inode {
	struct stat st;
	ssize_t block_idx;   // in blocks table
	ssize_t parent_idx;  // in inodes table
	bool free;
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
	bool free;
	block_data_t data;
};


static inode_t inodes[MAX_FILES] = { 0 };
static block_t blocks[MAX_FILES] = { 0 };

ssize_t
find_free_dentry(const block_t *block)
{
	for (ssize_t i = 0; i < MAX_DENTRIES; i++)
		if (block->data.dentries[i].inode_idx == -1)
			return i;

	return -1;
}

ssize_t
find_free_inode(void)
{
	for (ssize_t i = 0; i < MAX_FILES; i++)
		if (inodes[i].free)
			return i;

	return -1;
}

ssize_t
find_free_block(void)
{
	for (ssize_t i = 0; i < MAX_FILES; i++)
		if (blocks[i].free)
			return i;

	return -1;
}

// PRE: path should be a valid path (in terms of format).
// Maybe the file or directory does not exist, but the path
// must be a valid path. (Eg.: /dir/subdir/subsubdir/file)
ssize_t
get_inode_idx_from_path(const char *path)
{
	if (path == NULL)
		return -1;

	if (strcmp(path, "/") == 0)
		return 0;

	size_t inode_idx = 0;

	char *path_copy = strdup(path);

	for (const char *s = strtok(path_copy, "/"); s != NULL;
	     s = strtok(NULL, "/")) {
		const ssize_t block_idx = inodes[inode_idx].block_idx;
		if (block_idx == -1) {
			// error
		}

		size_t i = 0;
		for (i = 0; i < MAX_DENTRIES; i++) {
			dentry_t *dentry = &blocks[block_idx].data.dentries[i];
			if (strcmp(dentry->name, s) == 0) {
				inode_idx = dentry->inode_idx;
				break;
			}
		}

		if (i >= MAX_DENTRIES)
			return -1;  // Dentry not found
	}

	free(path_copy);

	return inode_idx;
}

/* Get file attributes */
int
fs_getattr(const char *path, struct stat *st)
{
	const ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1)
		return -1;

	*st = inodes[inode_idx].st;
	return 0;
}

/* Create a directory */
int
fs_mkdir(const char *path)
{
	char *path_copy = strdup(path);
	char *new_dir = strdup(path);
	const char *dir = dirname(path_copy);
	const char *new_dir_name = basename(path_copy);

	if (strlen(new_dir_name) > MAX_FILE_NAME) {
		free(path_copy);
		free(new_dir);
		return -1;
	}

	const ssize_t parent_idx = get_inode_idx_from_path(dir);

	if (parent_idx == -1)
		return -1;

	if (!S_ISDIR(inodes[parent_idx].st.st_mode))
		return -1;

	const ssize_t free_dentry_idx =
	        find_free_dentry(&blocks[inodes[parent_idx].block_idx]);

	if (free_dentry_idx == -1)
		return -1;

	const ssize_t free_inode_idx = find_free_inode();

	if (free_inode_idx == -1)
		return -1;

	const ssize_t free_block_idx = find_free_block();

	if (free_block_idx == -1)
		return -1;

	inodes[free_inode_idx] =
	        (inode_t) { .free = false,
		            .st = { .st_mode = S_IFDIR | 0777, .st_nlink = 1 },
		            .block_idx = free_block_idx,
		            .parent_idx = parent_idx };

	blocks[free_block_idx] =
	        (block_t) { .free = false,
		            .data = (block_data_t) {
		                    .dentries = { { .name = ".",
		                                    .inode_idx = free_inode_idx },
		                                  { .name = "..",
		                                    .inode_idx = parent_idx } } } };

	for (ssize_t i = 2; i < MAX_DENTRIES; i++)
		blocks[free_block_idx].data.dentries[i] =
		        (dentry_t) { .inode_idx = -1 };

	blocks[inodes[parent_idx].block_idx].data.dentries[free_dentry_idx] =
	        (dentry_t) { .inode_idx = free_inode_idx };

	strcpy(blocks[inodes[parent_idx].block_idx]
	               .data.dentries[free_dentry_idx]
	               .name,
	       new_dir_name);

	free(path_copy);
	free(new_dir);
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

static char last_path[248];

/* Read directory */
int
fs_readdir(const char *path, void *buffer)
{
	static ssize_t last_idx = 2;

	const ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1)
		return -1;

	if (strcmp(path, last_path) != 0)
		last_idx = 2;

	strcpy(last_path, path);

	while (last_idx < MAX_DENTRIES) {
		if (blocks[inodes[inode_idx].block_idx].data.dentries[last_idx].inode_idx ==
		    -1) {
			++last_idx;
			continue;
		}

		strcpy(buffer,
		       blocks[inodes[inode_idx].block_idx]
		               .data.dentries[last_idx]
		               .name);
		last_idx = last_idx + 1;
		return 1;
	}

	last_idx = 2;
	return 0;
}

/*
 * New file system
 */
void
newFileSystem(void)
{
	memset(inodes, 0, sizeof(inodes));
	memset(blocks, 0, sizeof(blocks));
	inodes[0] =
	        (inode_t) { .free = false,
		            .st = { .st_mode = S_IFDIR | 0777, .st_nlink = 1 },
		            .block_idx = 0,
		            .parent_idx = -1 };

	blocks[0] = (block_t) {
		.free = false,
		.data = { .dentries = { { .name = ".", .inode_idx = 0 },
		                        { .name = "..", .inode_idx = -2 } } }
	};

	for (int i = 2; i < MAX_DENTRIES; i++)
		blocks[0].data.dentries[i] = (dentry_t) { .inode_idx = -1 };

	for (ssize_t i = 1; i < MAX_FILES; i++) {
		inodes[i] = (inode_t) { .free = true,
			                .block_idx = -1,
			                .parent_idx = -1 };
		blocks[i] = (block_t) { .free = true };
	}
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

	ssize_t err = read(fd, inodes, sizeof(inodes));
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

	const ssize_t err = write(fd, inodes, sizeof(inodes));
	if (err == -1)
		return;

	(void) write(fd, blocks, sizeof(blocks));
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
