#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int sum;
    int product;
} Result;

static int parse_number(const char *text, int *value)
{
    char *end;
    errno = 0;
    long number = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || number < -1000 || number > 1000) {
        return 0;
    }

    *value = (int)number;
    return 1;
}

static int read_number(const char *name, int *value)
{
    char text[64];

    for (;;) {
        printf("%s (-1000..1000): ", name);
        fflush(stdout);
        if (fgets(text, sizeof(text), stdin) == NULL) {
            return 0;
        }

        text[strcspn(text, "\r\n")] = '\0';
        if (parse_number(text, value)) {
            return 1;
        }
        puts("Invalid number. Try again.");
    }
}

static int sum(int a, int b)
{
    int result = a + b;
    return result;
}

static Result calculate(int a, int b)
{
    Result result = {0};
    result.sum = sum(a, b);
    result.product = a * b;
    return result;
}

static void print_result(int a, int b, const Result *result)
{
    printf("a = %d, b = %d\n", a, b);
    printf("sum = %d, product = %d\n", result->sum, result->product);

    if (result->sum < 0) {
        puts("The sum is negative.");
    } else if (result->sum == 0) {
        puts("The sum is zero.");
    } else {
        puts("The sum is positive.");
    }
}

int main(void)
{
    int a = 0;
    int b = 0;

    puts("Enter two integers. Press Ctrl+D to exit.");
    while (read_number("a", &a) && read_number("b", &b)) {
        Result result = calculate(a, b);
        print_result(a, b, &result);
        putchar('\n');
    }
    puts("Goodbye.");
    return 0;
}
