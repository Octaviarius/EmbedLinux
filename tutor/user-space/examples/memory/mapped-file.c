#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 3 || (strcmp(argv[2], "private") != 0 &&
                      strcmp(argv[2], "shared") != 0)) {
        fprintf(stderr, "usage: %s file private|shared\n", argv[0]);
        return 1;
    }
    const int shared = strcmp(argv[2], "shared") == 0;
    int fd = open(argv[1], (shared ? O_RDWR : O_RDONLY) | O_CLOEXEC);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    struct stat info;
    int result = 0;
    if (fstat(fd, &info) == -1) {
        perror("fstat");
        result = 1;
    } else if (!S_ISREG(info.st_mode) || info.st_size < 0 ||
               (uintmax_t)info.st_size > SIZE_MAX) {
        fprintf(stderr, "expected a regular file of representable size\n");
        result = 1;
    }
    if (result != 0 || info.st_size == 0) {
        if (result == 0) {
            puts("empty file: no mapping");
        }
        if (close(fd) == -1) {
            perror("close");
            result = 1;
        }
        return result;
    }

    size_t length = (size_t)info.st_size;
    unsigned char *data = mmap(NULL, length, PROT_READ | PROT_WRITE,
                               shared ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        if (close(fd) == -1) {
            perror("close");
        }
        return 1;
    }
    if (close(fd) == -1) {
        perror("close");
        result = 1;
    }

    if (result == 0) {
        unsigned int before = data[0];
        data[0] = 'X';
        printf("mode=%s before=%u after=%u\n", argv[2], before,
               (unsigned int)data[0]);
        if (shared && msync(data, length, MS_SYNC) == -1) {
            perror("msync");
            result = 1;
        }
    }
    if (munmap(data, length) == -1) {
        perror("munmap");
        result = 1;
    }
    return result;
}
