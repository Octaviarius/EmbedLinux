#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t stop_requested;

static void request_stop(int signum)
{
    (void)signum;
    stop_requested = 1;
}

int main(void)
{
    sigset_t blocked;
    sigset_t previous;
    sigset_t waiting;
    struct sigaction action = {0};

    if (sigemptyset(&blocked) == -1 ||
        sigaddset(&blocked, SIGINT) == -1 ||
        sigaddset(&blocked, SIGTERM) == -1 ||
        sigprocmask(SIG_BLOCK, &blocked, &previous) == -1) {
        perror("block signals");
        return EXIT_FAILURE;
    }

    action.sa_handler = request_stop;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    waiting = previous;
    if (sigdelset(&waiting, SIGINT) == -1 ||
        sigdelset(&waiting, SIGTERM) == -1) {
        perror("wait mask");
        return EXIT_FAILURE;
    }

    if (puts("ready") == EOF || fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }

    while (!stop_requested) {
        if (sigsuspend(&waiting) == -1 && errno != EINTR) {
            perror("sigsuspend");
            return EXIT_FAILURE;
        }
    }

    if (puts("stopped") == EOF || fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }

    if (sigprocmask(SIG_SETMASK, &previous, NULL) == -1) {
        perror("restore mask");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
