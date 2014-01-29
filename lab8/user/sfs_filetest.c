#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <stat.h>
#include <dirent.h>
#include <file.h>
#include <dir.h>
#include <unistd.h>

#define printf(...)                 fprintf(1, __VA_ARGS__)

static int safe_open(const char *path, int open_flags)
{
    int fd = open(path, open_flags);
    assert(fd >= 0);
    return fd;
}

static void safe_unlink(const char *path)
{
    int ret = unlink(path);
    assert(ret == 0);
}

static char getmode(uint32_t st_mode)
{
	char mode = '?';
	if (S_ISREG(st_mode))
		mode = '-';
	if (S_ISDIR(st_mode))
		mode = 'd';
	if (S_ISLNK(st_mode))
		mode = 'l';
	if (S_ISCHR(st_mode))
		mode = 'c';
	if (S_ISBLK(st_mode))
		mode = 'b';
	return mode;
}

static void safe_stat(const char *name)
{
	struct stat __stat, *stat = &__stat;
	int fd = open(name, O_RDONLY), ret = fstat(fd, stat);
	assert(fd >= 0 && ret == 0);
	printf("%c %3d   %4d %10d  ", getmode(stat->st_mode),
	       stat->st_nlinks, stat->st_blocks, stat->st_size);
    close(fd);
}

static void safe_getcwd(round)
{
	static char buffer[FS_MAX_FPATH_LEN + 1];
	int ret = getcwd(buffer, sizeof(buffer));
	assert(ret == 0);
	printf("%d: current: %s\n", round, buffer);
}

static void safe_lsdir(int round)
{
	DIR *dirp = opendir(".");
	assert(dirp != NULL);
	struct dirent *direntp;
	while ((direntp = readdir(dirp)) != NULL) {
		printf("%d: ", round);
		safe_stat(direntp->name);
		printf("%s\n", direntp->name);
	}
	closedir(dirp);
}

static void safe_read(int fd, void *data, size_t len)
{
    int ret = read(fd, data, len);
    assert(ret == len);
}

static void safe_write(int fd, void *data, size_t len)
{
    int ret = write(fd, data, len);
    assert(ret == len);
}

void dumpdir(const char *path)
{
	static int round = 0;
	printf("------------------------\n");
	safe_getcwd(round);
	safe_lsdir(round);
	round++;
}

static uint32_t buffer[1024];

int main(void)
{
    int i;
    for (i = 0; i < 1024; i ++)
        buffer[i] = i;

	dumpdir("/");

    int fd1 = safe_open("/test.txt", O_WRONLY | O_CREAT);
	dumpdir("/");

    safe_write(fd1, buffer, 32);
    dumpdir("/");

    int fd2 = safe_open("/test.txt", O_RDONLY);
    safe_read(fd2, buffer, 32);

    for (i = 0; i < 32; i ++)
        assert(buffer[i] == i);

    close(fd1);
    close(fd2);

    link("/test.txt", "/test2.txt");
    dumpdir("/");

    safe_unlink("/test2.txt");
    dumpdir("/");

    safe_unlink("/test.txt");
    dumpdir("/");

	printf("sfs_dirtest pass.\n");
	return 0;
}
