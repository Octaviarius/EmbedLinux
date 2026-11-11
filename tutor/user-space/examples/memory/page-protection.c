#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        fprintf(stderr, "cannot determine page size\n");
        return 1;
    }
    size_t length = (size_t)page_size;
    char *page = mmap(NULL, length, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (page == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    int result = 0;
    memcpy(page, "ready", sizeof("ready"));
    if (mprotect(page, length, PROT_READ) == -1) {
        perror("mprotect read");
        result = 1;
    } else {
        puts(page);
        if (mprotect(page, length, PROT_NONE) == -1) {
            perror("mprotect none");
            result = 1;
        }
    }
    if (munmap(page, length) == -1) {
        perror("munmap");
        result = 1;
    }
    return result;
}
