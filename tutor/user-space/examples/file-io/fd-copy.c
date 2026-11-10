#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int copy_data(int input, int output)
{
    unsigned char buffer[4096];

    for (;;) {
        ssize_t count = read(input, buffer, sizeof(buffer));
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
            ssize_t written = write(output, buffer + offset,
                                    (size_t)count - offset);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("write");
                return 1;
            }
            if (written == 0) {
                fprintf(stderr, "write made no progress\n");
                return 1;
            }
            offset += (size_t)written;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s source destination\n", argv[0]);
        return 1;
    }

    int input = open(argv[1], O_RDONLY | O_CLOEXEC);
    if (input == -1) {
        perror("open source");
        return 1;
    }

    int output = open(argv[2], O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC,
                      0644);
    if (output == -1) {
        perror("open destination");
        if (close(input) == -1) {
            perror("close source");
        }
        return 1;
    }

    int result = copy_data(input, output);
    if (close(output) == -1) {
        perror("close destination");
        result = 1;
    }
    if (close(input) == -1) {
        perror("close source");
        result = 1;
    }
    return result;
}
