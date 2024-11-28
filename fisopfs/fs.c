#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

typedef union BlockData BlockData;
typedef struct Inode Inode;
typedef struct Dentry Dentry;
typedef struct Block Block;

struct Inode {
	struct stat st;
	ssize_t block;
	ssize_t parent;
};

struct Dentry {
	char name[248];
	ssize_t inode;
};

union BlockData {
	Dentry dentries[16];  // if directory
	char data[4096];      // if regular file
};

struct Block {
	ssize_t nextFree;
	BlockData data;
};

static ssize_t firstFree = 0;
static Inode inodes[1024] = { 0 };
static Block blocks[1024] = { 0 };

/*
 * New file system
 */
void
newFileSystem(void)
{
	firstFree = 0;
	memset(inodes, 0, sizeof(inodes));
	memset(blocks, 0, sizeof(blocks));

	inodes[0] = (Inode){ .st = { .st_mode = __S_IFDIR | 0755, .st_nlink = 1 },
		             .block = 0,
		             .parent = -1 };

	blocks[0] =
	        (Block){ .nextFree = -1,
		         .data = { .dentries = { { .name = ".", .inode = 0 },
		                                 { .name = "..", .inode = -1 } } } };

	firstFree = 1;

	for (ssize_t i = 1; i < 1023; i++) {
		inodes[i] = (Inode){ .block = -1, .parent = -1 };
		blocks[i] = (Block){ .nextFree = i + 1 };
	}

	inodes[1023] = (Inode){ .block = -1, .parent = -1 };
	blocks[1023] = (Block){ .nextFree = 1 };
}

void *
fs_init(const char *const restrict filepath)
{
	const int fd = open(filepath, O_RDONLY);
	if (fd == -1) {
		newFileSystem();
		return NULL;
	}

	ssize_t err = read(fd, &firstFree, sizeof(firstFree));
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

void
fisopfs_destroy(const char *const restrict filepath)
{
	const int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
		return;

	ssize_t err = write(fd, &firstFree, sizeof(firstFree));
	if (err == -1)
		return;

	err = write(fd, inodes, sizeof(inodes));
	if (err == -1)
		return;

	write(fd, blocks, sizeof(blocks));
}

static int
fs_getattr(const char *path, struct stat *st)
{
	st->st_mode = __S_IFDIR | 0755;
}