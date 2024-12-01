#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <libgen.h>
#include <errno.h>
#include <stdio.h>

#define MAX_FILES 1024
#define MAX_DENTRIES 16
#define MAX_FILE_SIZE 4096
#define MAX_FILE_NAME 248

// STRUCTURES
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct inode inode_t;
typedef struct dentry dentry_t;
typedef union block block_t;
typedef struct filesystem filesystem_t;

struct inode {
	struct stat st;
	ssize_t block_idx;   // in blocks table
	ssize_t parent_idx;  // in inodes table
};

struct dentry {
	char name[MAX_FILE_NAME];
	ssize_t inode_idx;
};

union block {
	dentry_t dentries[MAX_DENTRIES];  // if directory
	char data[MAX_FILE_SIZE];         // if regular file
};

struct filesystem {
	int inodes_bitmap[MAX_FILES];  // 0 if free, 1 if used
	int data_bitmap[MAX_FILES];    // same
	inode_t inodes[MAX_FILES];
	block_t blocks[MAX_FILES];
};

/**
 * Our filesystem, all in one big block
 */
filesystem_t fisopfs;

// To simplify access
int *inodes_bitmap = fisopfs.inodes_bitmap;
int *data_bitmap = fisopfs.data_bitmap;
inode_t *inodes = fisopfs.inodes;
block_t *blocks = fisopfs.blocks;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// USEFUL FUNCTIONS
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *
strdup(const char *s)
{
	char *p = calloc(strlen(s) + 1, 1);
	if (p)
		strcpy(p, s);
	return p;
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

		size_t i = 0;
		block_t *block = &blocks[block_idx];
		for (i = 0; i < MAX_DENTRIES; i++) {
			dentry_t *dentry = &block->dentries[i];
			if (strcmp(dentry->name, s) == 0) {
				inode_idx = dentry->inode_idx;
				break;
			}
		}

		if (i >= MAX_DENTRIES) {
			free(path_copy);
			return -1;  // Dentry not found
		}
	}

	free(path_copy);
	return inode_idx;
}

ssize_t
get_free_inode_idx()
{
	for (size_t i = 1; i < MAX_FILES; i++)
		if (!inodes_bitmap[i])
			return i;

	return -1;
}

ssize_t
get_free_block_idx()
{
	for (size_t i = 1; i < MAX_FILES; i++)
		if (!data_bitmap[i])
			return i;

	return -1;
}

ssize_t
get_free_dentry_idx(dentry_t *dentries)
{
	for (ssize_t i = 2; i < MAX_DENTRIES; i++)
		if (dentries[i].inode_idx == -1)
			return i;
	return -1;
}

void
dentries_ordered_deletion(dentry_t *dentries, size_t beggining)
{
	size_t i;
	for (i = beggining + 1; dentries[i].inode_idx != -1 && i < MAX_DENTRIES;
	     i++) {
		strcpy(dentries[i - 1].name, dentries[i].name);
		dentries[i - 1].inode_idx = dentries[i].inode_idx;
	}
	dentries[i - 1].inode_idx = -1;  // Free the last one that was not free
}

void
remove_dentry_from_parent_directory(dentry_t *parent_dentries, char *dentry_name)
{
	for (size_t i = 2; parent_dentries[i].inode_idx != -1 && i < MAX_DENTRIES;
	     i++) {
		if (strcmp(parent_dentries[i].name, dentry_name) == 0) {
			dentries_ordered_deletion(parent_dentries, i);
			break;
		}
	}
}

// The inode must be of type directory.
dentry_t *
get_dentries_from_inode_index(ssize_t inode_idx)
{
	return blocks[inodes[inode_idx].block_idx].dentries;
}

void
initialize_inode(const ssize_t inode_idx,
                 const ssize_t block_idx,
                 const ssize_t parent_inode_idx,
                 const struct stat st)
{
	memset(&inodes[inode_idx], 0, sizeof(inode_t));
	inodes_bitmap[inode_idx] = 1;

	inode_t *inode = &inodes[inode_idx];
	inode->block_idx = block_idx;
	inode->parent_idx = parent_inode_idx;
	inode->st = st;
}

void
initialize_directory_block(const ssize_t block_idx,
                           const ssize_t self_inode_idx,
                           const ssize_t parent_inode_idx)
{
	data_bitmap[block_idx] = 1;
	memset(blocks[block_idx].dentries, 0, MAX_DENTRIES * sizeof(dentry_t));

	dentry_t *d = blocks[block_idx].dentries;

	strcpy(d[0].name, ".");
	d[0].inode_idx = self_inode_idx;  // itself

	strcpy(d[1].name, "..");
	d[1].inode_idx = parent_inode_idx;

	for (size_t i = 2; i < MAX_DENTRIES; i++)
		d[i].inode_idx = -1;  // not used
}

void
initialize_root(void)
{
	struct stat st = { 0 };
	st.st_mode = __S_IFDIR | 0755;
	st.st_nlink = 2;
	st.st_uid = getuid();
	st.st_gid = getgid();

	initialize_inode(0, 0, 0, st);
	initialize_directory_block(0, 0, 0);
}

void
initialize_file_block(const ssize_t block_idx)
{
	data_bitmap[block_idx] = 1;
	memset(blocks[block_idx].data, 0, MAX_FILE_SIZE);
}

void
initialize_file(const ssize_t inode, const ssize_t block, const ssize_t parent)
{
	struct stat st = { 0 };
	st.st_mode = __S_IFREG | 0644;
	st.st_nlink = 1;
	st.st_uid = getuid();
	st.st_gid = getgid();

	initialize_inode(inode, block, parent, st);
	initialize_file_block(block);
}

void
initialize_directory(const ssize_t inode, const ssize_t block, const ssize_t parent)
{
	struct stat st = { 0 };
	st.st_mode = __S_IFDIR | 0755;
	st.st_nlink = 2;
	st.st_uid = getuid();
	st.st_gid = getgid();

	initialize_inode(inode, block, parent, st);
	initialize_directory_block(block, inode, parent);
}

/*
 * New file system
 */
void
new_file_system(void)
{
	memset(inodes_bitmap, 0, sizeof(fisopfs.inodes_bitmap));
	memset(data_bitmap, 0, sizeof(fisopfs.data_bitmap));
	memset(inodes, 0, sizeof(fisopfs.inodes));
	memset(blocks, 0, sizeof(fisopfs.blocks));

	initialize_root();

	for (ssize_t i = 1; i < MAX_FILES; i++) {
		inodes[i].block_idx = -1;
		inodes[i].parent_idx = -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// OPERATIONS
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Get file attributes */
int
fs_getattr(const char *path, struct stat *st)
{
	const ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1)
		return -ENOENT;

	*st = inodes[inode_idx].st;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Create a directory */
int
fs_mkdir(const char *path)
{
	char *path_copy_1 = strdup(path);
	char *path_copy_2 = strdup(path);

	char *parent_path = dirname(path_copy_1);
	char *dentry_name = basename(path_copy_2);

	ssize_t parent_inode_idx = get_inode_idx_from_path(parent_path);
	if (parent_inode_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}
	if (!S_ISDIR(inodes[parent_inode_idx].st.st_mode)) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}


	dentry_t *parent_dentries =
	        get_dentries_from_inode_index(parent_inode_idx);

	ssize_t dentry_idx = get_free_dentry_idx(parent_dentries);
	if (dentry_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	ssize_t new_inode_idx = get_free_inode_idx();
	if (new_inode_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	ssize_t new_block_index = get_free_block_idx();
	if (new_block_index == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	strcpy(parent_dentries[dentry_idx].name, dentry_name);
	parent_dentries[dentry_idx].inode_idx = new_inode_idx;

	initialize_directory(new_inode_idx, new_block_index, parent_inode_idx);

	free(path_copy_1);
	free(path_copy_2);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Remove a file */
int
fs_unlink(const char *path)
{
	ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1)
		return -1;

	inode_t *inode = &inodes[inode_idx];

	if (!S_ISREG(inode->st.st_mode))
		return -1;

	inodes_bitmap[inode_idx] = 0;
	data_bitmap[inode->block_idx] = 0;

	dentry_t *parent_dentries =
	        get_dentries_from_inode_index(inode->parent_idx);

	char *path_copy = strdup(path);
	char *dentry_name = basename(path_copy);
	remove_dentry_from_parent_directory(parent_dentries, dentry_name);
	free(path_copy);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Remove a directory */
int
fs_rmdir(const char *path)
{
	ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1)
		return -1;

	inode_t *inode = &inodes[inode_idx];

	if (!S_ISDIR(inode->st.st_mode))
		return -1;

	dentry_t *dentries = get_dentries_from_inode_index(inode_idx);

	if (dentries[2].inode_idx != -1)
		return -1;  // Directory is not empty

	inodes_bitmap[inode_idx] = 0;
	data_bitmap[inode->block_idx] = 0;

	dentry_t *parent_dentries =
	        get_dentries_from_inode_index(inode->parent_idx);

	char *path_copy = strdup(path);
	char *dentry_name = basename(path_copy);
	remove_dentry_from_parent_directory(parent_dentries, dentry_name);
	free(path_copy);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Change the size of a file */
int
fs_truncate(const char *path, off_t size)
{
	if (size > MAX_FILE_SIZE) {
		errno = EINVAL;
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		return -EINVAL;
	}

	ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1) {
		errno = ENOENT;
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		return -ENOENT;
	}

	inode_t *inode = &inodes[inode_idx];
	// Change size
	inode->st.st_size = size;
	// Change modification time
	inode->st.st_mtime = time(NULL);

	// Maybe unnecessary but:
	char *data = blocks[inode->block_idx].data;
	memset(data + size, 0, MAX_FILE_SIZE - size);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Read data from an open file */
int
fs_read(const char *path, char *buffer, size_t size, off_t offset)
{
	if (offset < 0 || size < 0) {
		errno = EINVAL;
		return -EINVAL;
	}

	const ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1) {
		errno = ENOENT;
		return -ENOENT;
	}

	inode_t *inode = &inodes[inode_idx];

	if (!S_ISREG(inode->st.st_mode)) {
		errno = EISDIR;
		return -EISDIR;
	}
	const char *data = blocks[inode->block_idx].data;
	const off_t file_size = inode->st.st_size;
	if (offset > file_size) {
		errno = EINVAL;
		return -EINVAL;
	}


	if (file_size - offset < size)
		size = file_size - offset;


	strncpy(buffer, data + offset, size);
	inode->st.st_atime = time(NULL);
	return size;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Create and open a file */
int
fs_create(const char *path, mode_t mode)
{
	char *path_copy_1 = strdup(path);
	char *path_copy_2 = strdup(path);

	char *parent_path = dirname(path_copy_1);
	char *dentry_name = basename(path_copy_2);

	ssize_t parent_inode_idx = get_inode_idx_from_path(parent_path);
	if (parent_inode_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	if (!S_ISDIR(inodes[parent_inode_idx].st.st_mode)) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	dentry_t *parent_dentries =
	        get_dentries_from_inode_index(parent_inode_idx);

	ssize_t dentry_idx = get_free_dentry_idx(parent_dentries);
	if (dentry_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	ssize_t new_inode_idx = get_free_inode_idx();
	if (new_inode_idx == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	ssize_t new_block_index = get_free_block_idx();
	if (new_block_index == -1) {
		free(path_copy_1);
		free(path_copy_2);
		return -1;
	}

	strcpy(parent_dentries[dentry_idx].name, dentry_name);
	parent_dentries[dentry_idx].inode_idx = new_inode_idx;

	initialize_file(new_inode_idx, new_block_index, parent_inode_idx);

	free(path_copy_1);
	free(path_copy_2);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Write data to an open file */
int
fs_write(const char *path, const char *buffer, size_t size, off_t offset)
{
	const size_t sum = offset + size;
	if (sum > MAX_FILE_SIZE) {
		errno = EFBIG;
		return -EFBIG;
	}

	ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1) {
		const size_t err = fs_create(path, 0644);
		if (err < -1)
			return err;
		inode_idx = get_inode_idx_from_path(path);
	}

	if (inode_idx < 0) {
		errno = ENOENT;
		return -ENOENT;
	}
	inode_t *inode = &inodes[inode_idx];

	if (inode->st.st_size < offset) {
		errno = EINVAL;
		return -EINVAL;
	}

	if (!S_ISREG(inode->st.st_mode)) {
		errno = EACCES;
		return -EACCES;
	}

	char *data = blocks[inode->block_idx].data;
	strncpy(data + offset, buffer, size);
	inode->st.st_size = (off_t) strlen(data);
	inode->st.st_mtime = time(NULL);
	inode->st.st_atime = time(NULL);
	data[inode->st.st_size] = '\0';

	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Read directory */
int
fs_readdir(const char *path, void *buffer, off_t offset)
{
	const ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1)
		return -1;

	if (!S_ISDIR(inodes[inode_idx].st.st_mode))
		return -1;

	dentry_t *dentries = get_dentries_from_inode_index(inode_idx);

	if (offset >= MAX_DENTRIES || dentries[offset].inode_idx == -1)
		return -1;

	strcpy(buffer, dentries[offset].name);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Initialize filesystem */
void *
fs_init(const char *const restrict filepath)
{
	const int fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr,
		        "[debug] Error loading filesystem: %s\n",
		        strerror(errno));
		new_file_system();
		return NULL;
	}

	if ((read(fd, &fisopfs, sizeof(fisopfs))) < 0) {
		fprintf(stderr,
		        "[debug] Error loading filesystem: %s\n",
		        strerror(errno));
		new_file_system();
	}

	if (close(fd) < 0)
		fprintf(stderr,
		        "[debug] Error loading filesystem: %s\n",
		        strerror(errno));

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Clean up filesystem */
void
fs_destroy(const char *const restrict filepath)
{
	const int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0) {
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
		return;
	}

	if (write(fd, &fisopfs, sizeof(fisopfs)) < 0) {
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
		return;
	}

	if (close(fd) < 0)
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Change the access and modification times of a file with
        nanosecond resolution */
int
fs_utimens(const char *path, const struct timespec tv[2])
{
	ssize_t inode_idx = get_inode_idx_from_path(path);

	if (inode_idx == -1)
		return -1;

	struct stat *st = &inodes[inode_idx].st;

	st->st_atime = tv[0].tv_sec;
	st->st_mtime = tv[1].tv_sec;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////