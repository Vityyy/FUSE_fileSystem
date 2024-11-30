
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <libgen.h>
#include <errno.h>


#define MAX_FILES 1024
#define MAX_DENTRIES 16
#define MAX_FILE_SIZE 4096
#define MAX_FILE_NAME 248



// STRUCTURES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	char data[MAX_FILE_SIZE];       // if regular file
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



// USEFUL FUNCTIONS /////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		block_t *block = &blocks[block_idx];
		for (i = 0; i < MAX_DENTRIES; i++) {
			dentry_t *dentry = &block->dentries[i];
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

ssize_t get_free_inode_idx() {
	for (size_t i = 0; i < MAX_FILES; i++)
		if (!inodes_bitmap[i])
			return i;

	return -1;
}

ssize_t get_free_block_idx() {
	for (size_t i = 0; i < MAX_FILES; i++)
		if (!data_bitmap[i])
			return i;

	return -1;
}

void
initialize_inode(ssize_t inode_index, ssize_t block_idx, ssize_t parent_inode_idx)
{
	memset(&inodes[inode_index], 0, sizeof(inode_t));

	inodes_bitmap[inode_index] = 1;

	inode_t *inode = &inodes[inode_index];

	inode->block_idx = block_idx;
	inode->parent_idx = parent_inode_idx;

	// This should not stay like this. The type (dir/file) may be an argument?
	if (inode_index == 0) {
		inode->st.st_uid = 1717;
		inode->st.st_mode = __S_IFDIR | 0755;
		inode->st.st_nlink = 2;  // ?
	} else {
		inode->st.st_uid = 1818;
		inode->st.st_mode = __S_IFREG | 0644;
		inode->st.st_nlink = 1;
	}
}

void
initialize_directory_block(ssize_t block_idx,
                           ssize_t self_inode_idx,
                           ssize_t parent_inode_idx)
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
initialize_file_block(ssize_t block_idx)
{
	data_bitmap[block_idx] = 1;
	memset(blocks[block_idx].data, 0, MAX_FILE_SIZE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// OPERATIONS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	        (inode_t){ .st = { .st_mode = __S_IFDIR | 0777, .st_nlink = 1 },
		           .block_idx = free_block_idx,
		           .parent_idx = parent_idx };

	blocks[free_block_idx] = (block_t){
		.dentries = { { .name = ".", .inode_idx = free_inode_idx },
		              { .name = "..", .inode_idx = parent_idx } }
	};

	for (ssize_t i = 2; i < MAX_DENTRIES; i++)
		blocks[free_block_idx].dentries[i] = (dentry_t){ .inode_idx = -1 };

	blocks[inodes[parent_idx].block_idx].dentries[free_dentry_idx] =
	        (dentry_t){ .inode_idx = free_inode_idx };

	strcpy(blocks[inodes[parent_idx].block_idx].dentries[free_dentry_idx].name,
	       new_dir_name);

	free(path_copy);
	free(new_dir);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dentries_ordered_deletion(dentry_t *dentries, size_t beggining) {
	size_t i;
	for (i = beggining; dentries[i].inode_idx != -1 && i < MAX_DENTRIES; i++) {
		strcpy(dentries[i - 1].name, dentries[i].name);
		dentries[i - 1].inode_idx = dentries[i].inode_idx;
	}
	dentries[i - 1].inode_idx = -1; // Free the last one that was not free
}


/* Remove a file */
int
fs_unlink(const char *path)
{
	ssize_t inode_idx = get_inode_idx_from_path(path);
	if (inode_idx == -1)
		return -1;

	inode_t *inode = &inodes[inode_idx];

	if (!es_de_tipo_regular())
		return -1;

	inodes_bitmap[inode_idx] = 0;
	data_bitmap[inode->block_idx] = 0;

	inode_t *parent_inode = &inodes[inode->parent_idx];
	dentry_t *dentries = blocks[inode->block_idx].dentries;
	char *file_name = basename(path);

	for (size_t i = 2; dentries[i].inode_idx != -1 && i < MAX_DENTRIES; i++) {
		if (strcmp(dentries[i].name, file_name) == 0) {
			dentries_ordered_deletion(dentries, i);
			break;
		}
	}
 
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Remove a directory */
int
fs_rmdir(const char *path)
{
	// Completar

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Change the size of a file */
int
fs_truncate(const char *path, off_t size)
{
	ssize_t inode_index = get_inode_idx_from_path(path);
	if (inode_index == -1)
		return -1;

	// Should we verify the size?

	inode_t *inode = &inodes[inode_index];
	char *data = blocks[inode->block_idx].data;

	// Change size
	inode->st.st_size = size;

	// Maybe unnecessary but:
	memset(data + size, 0, MAX_FILE_SIZE - size);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Read data from an open file */
int
fs_read(const char *path, char *buffer, size_t size, off_t offset)
{
	ssize_t inode_index = get_inode_idx_from_path(path);
	if (inode_index == -1)
		return -1;

	inode_t *inode = &inodes[inode_index];
	off_t file_size = inode->st.st_size;
	char *data = blocks[inode->block_idx].data;

	if (!es_de_tipo_regular())
		return -1; 

	if (offset + size > file_size)
		size = file_size - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, data + offset, size);

	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Write data to an open file */
int
fs_write(const char *path, const char *buffer, size_t size, off_t offset)
{
	ssize_t inode_index = get_inode_idx_from_path(path);
	if (inode_index == -1)
		return -1; // Si no existe quizá podemos crearlo

	inode_t *inode = &inodes[inode_index];
	char *data = blocks[inode->block_idx].data;

	if (!es_de_tipo_regular())
		return -1; 

	if (offset + size > MAX_FILE_SIZE)
		size = MAX_FILE_SIZE - offset;

	size = size > 0 ? size : 0;

	memcpy(data + offset, buffer, size);

	inode->st.st_size += size;

	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		if (blocks[inodes[inode_idx].block_idx].dentries[last_idx].inode_idx ==
		    -1) {
			++last_idx;
			continue;
		}

		strcpy(buffer,
		       blocks[inodes[inode_idx].block_idx].dentries[last_idx].name);
		last_idx = last_idx + 1;
		return 1;
	}

	last_idx = 2;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * New file system
 */
void
new_file_system(void)
{
	memset(inodes, 0, sizeof(inodes_bitmap));
	memset(blocks, 0, sizeof(data_bitmap));
	memset(inodes, 0, sizeof(inodes));
	memset(blocks, 0, sizeof(blocks));

	initialize_inode(0, 0, -1);

	initialize_directory_block(0, 0, -2);

	for (ssize_t i = 1; i < MAX_FILES; i++) {
		inodes[i].block_idx = -1;
		inodes[i].parent_idx = -1;
	}
}

/* Initialize filesystem */
void *
fs_init(const char *const restrict filepath)
{
	const int fd = open(filepath, O_RDONLY);
	if (fd == -1) {
		new_file_system();
		return NULL;
	}

	ssize_t err = read(fd, &fisopfs, sizeof(fisopfs));
	if (err == -1)
		new_file_system();

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Clean up filesystem */
void
fs_destroy(const char *const restrict filepath)
{
	const int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
		return;

	write(fd, &fisopfs, sizeof(fisopfs));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Create and open a file */
int
fs_create(const char *path, mode_t mode)
{
	char *parent_path = dirname(path);
	char *dentry_name = basename(path);

	ssize_t parent_inode_index = get_inode_idx_from_path(parent_path);
	if (parent_inode_index == -1)
		return -1;

	block_t *block = &blocks[inodes[parent_inode_index].block_idx];
	ssize_t i, dentry_index;
	for (i = 0; i < MAX_DENTRIES; i++) {
		dentry_t *dentry = &block->dentries[i];
		if (dentry->inode_idx == -1) {
			dentry_index = dentry->inode_idx;
			break;
		}
	}
	
	if (i >= MAX_DENTRIES)
		return -1;

	ssize_t new_inode_index = get_free_inode_idx();
	if (new_inode_index == -1)
		return -1;

	ssize_t new_block_index = get_free_block_idx();
	if (new_block_index == -1)
		return -1;

	strcpy(block->dentries[dentry_index].name, dentry_name);
	block->dentries[dentry_index].inode_idx = new_inode_index;

	initialize_inode(new_inode_index, new_block_index, parent_inode_index);
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////