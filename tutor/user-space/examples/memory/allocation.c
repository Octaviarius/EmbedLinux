#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const size_t initial_count = 4;
    const size_t final_count = 8;
    int *values = calloc(initial_count, sizeof(*values));
    if (values == NULL) {
        perror("calloc");
        return 1;
    }

    for (size_t i = 0; i < initial_count; ++i) {
        values[i] = (int)(i + 1);
    }

    if (final_count > SIZE_MAX / sizeof(*values)) {
        fprintf(stderr, "allocation size overflow\n");
        free(values);
        return 1;
    }
    int *resized = realloc(values, final_count * sizeof(*values));
    if (resized == NULL) {
        perror("realloc");
        free(values);
        return 1;
    }
    values = resized;
    for (size_t i = initial_count; i < final_count; ++i) {
        values[i] = (int)(i + 1);
    }

    int sum = 0;
    for (size_t i = 0; i < final_count; ++i) {
        sum += values[i];
    }
    printf("count=%zu sum=%d\n", final_count, sum);

    free(values);
    values = NULL;
    return 0;
}
