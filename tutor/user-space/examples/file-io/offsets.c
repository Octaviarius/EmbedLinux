#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int read_pair(int fd, const char *label)
{
    char bytes[2];
    size_t count = 0;
    while (count < sizeof(bytes)) {
        ssize_t received = read(fd, bytes + count, sizeof(bytes) - count);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            return 1;
        }
        if (received == 0) {
            fprintf(stderr, "expected two more bytes\n");
            return 1;
        }
        count += (size_t)received;
    }
    printf("%s=%.*s\n", label, (int)sizeof(bytes), bytes);
    return 0;
}

static int show_offsets(int first, int duplicate, int independent)
{
    struct stat info;
    if (fstat(first, &info) == -1) {
        perror("fstat");
        return 1;
    }
    if (!S_ISREG(info.st_mode)) {
        fprintf(stderr, "source must be a regular file\n");
        return 1;
    }
    printf("size=%jd\n", (intmax_t)info.st_size);

    if (read_pair(first, "first") != 0 ||
        read_pair(duplicate, "duplicate") != 0 ||
        read_pair(independent, "independent") != 0) {
        return 1;
    }

    char bytes[2];
    ssize_t count;
    do {
        count = pread(first, bytes, sizeof(bytes), 4);
    } while (count == -1 && errno == EINTR);

    if (count == -1) {
        perror("pread");
        return 1;
    }
    if (count != (ssize_t)sizeof(bytes)) {
        fprintf(stderr, "expected two bytes at offset 4\n");
        return 1;
    }
    printf("pread=%.*s\n", (int)sizeof(bytes), bytes);

    off_t offset = lseek(first, 0, SEEK_CUR);
    if (offset == (off_t)-1) {
        perror("lseek current");
        return 1;
    }
    printf("shared offset=%jd\n", (intmax_t)offset);

    if (lseek(first, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek start");
        return 1;
    }
    return read_pair(duplicate, "after reset");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s source\n", argv[0]);
        return 1;
    }

    int first = open(argv[1], O_RDONLY | O_CLOEXEC);
    if (first == -1) {
        perror("open first");
        return 1;
    }
    int duplicate = dup(first);
    if (duplicate == -1) {
        perror("dup");
        if (close(first) == -1) {
            perror("close first");
        }
        return 1;
    }
    int independent = open(argv[1], O_RDONLY | O_CLOEXEC);
    if (independent == -1) {
        perror("open independent");
        if (close(duplicate) == -1) {
            perror("close duplicate");
        }
        if (close(first) == -1) {
            perror("close first");
        }
        return 1;
    }

    int result = show_offsets(first, duplicate, independent);
    int descriptors[] = {independent, duplicate, first};
    for (size_t i = 0; i < sizeof(descriptors) / sizeof(descriptors[0]); ++i) {
        if (close(descriptors[i]) == -1) {
            perror("close");
            result = 1;
        }
    }
    return result;
}
