#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{
    struct timespec deadline;

    if (clock_gettime(CLOCK_MONOTONIC, &deadline) == -1) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    for (int tick = 1; tick <= 3; ++tick) {
        int error;

        deadline.tv_nsec += 50000000L;
        if (deadline.tv_nsec >= 1000000000L) {
            deadline.tv_nsec -= 1000000000L;
            ++deadline.tv_sec;
        }

        do {
            error = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                    &deadline, NULL);
        } while (error == EINTR);

        if (error != 0) {
            fprintf(stderr, "clock_nanosleep: %s\n", strerror(error));
            return EXIT_FAILURE;
        }

        if (printf("tick %d\n", tick) < 0) {
            perror("stdout");
            return EXIT_FAILURE;
        }
    }

    if (fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
