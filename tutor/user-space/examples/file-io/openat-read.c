#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int print_file(int fd)
{
    unsigned char buffer[4096];
    for (;;) {
        ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count == 0) {
            return 0;
        }
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            return 1;
        }
        size_t offset = 0;
        while (offset < (size_t)count) {
            ssize_t written = write(STDOUT_FILENO, buffer + offset,
                                    (size_t)count - offset);
            if (written < 0 && errno == EINTR) {
                continue;
            }
            if (written <= 0) {
                if (written < 0) {
                    perror("write");
                } else {
                    fprintf(stderr, "write made no progress\n");
                }
                return 1;
            }
            offset += (size_t)written;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s directory path\n", argv[0]);
        return 1;
    }

    int directory = open(argv[1], O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (directory == -1) {
        perror("open directory");
        return 1;
    }

    /* Only this demonstration process changes its working directory. */
    if (chdir("/") == -1) {
        perror("chdir");
        if (close(directory) == -1) {
            perror("close directory");
        }
        return 1;
    }

    int input = openat(directory, argv[2], O_RDONLY | O_CLOEXEC);
    int result = 0;
    if (input == -1) {
        perror("openat");
        result = 1;
    }
    if (close(directory) == -1) {
        perror("close directory");
        result = 1;
    }
    if (input != -1) {
        if (result == 0) {
            result = print_file(input);
        }
        if (close(input) == -1) {
            perror("close file");
            result = 1;
        }
    }
    return result;
}
