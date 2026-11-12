#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

int main(void)
{
    struct itimerspec setting = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 20000000L},
        .it_value = {.tv_sec = 0, .tv_nsec = 20000000L}
    };
    unsigned int remaining = 3;
    int status = EXIT_SUCCESS;
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);

    if (fd == -1) {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }

    if (timerfd_settime(fd, 0, &setting, NULL) == -1) {
        perror("timerfd_settime");
        status = EXIT_FAILURE;
    }

    while (status == EXIT_SUCCESS && remaining > 0) {
        uint64_t expirations;
        ssize_t size;

        do {
            size = read(fd, &expirations, sizeof(expirations));
        } while (size == -1 && errno == EINTR);

        if (size != (ssize_t)sizeof(expirations)) {
            if (size == -1) {
                perror("read timerfd");
            } else {
                fputs("unexpected timerfd record size\n", stderr);
            }
            status = EXIT_FAILURE;
        } else if (printf("expirations=%" PRIu64 "\n", expirations) < 0) {
            perror("stdout");
            status = EXIT_FAILURE;
        } else if (expirations >= remaining) {
            remaining = 0;
        } else {
            remaining -= (unsigned int)expirations;
        }
    }

    if (close(fd) == -1) {
        perror("close");
        status = EXIT_FAILURE;
    }

    if (fflush(stdout) == EOF) {
        perror("stdout");
        status = EXIT_FAILURE;
    }

    return status;
}
