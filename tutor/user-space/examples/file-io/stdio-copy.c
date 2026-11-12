#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s source destination\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("fopen source");
        return 1;
    }
    FILE *output = fopen(argv[2], "wbx");
    if (output == NULL) {
        perror("fopen destination");
        if (fclose(input) == EOF) {
            perror("fclose source");
        }
        return 1;
    }

    unsigned char buffer[4096];
    size_t count;
    int result = 0;
    while ((count = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, count, output) != count) {
            perror("fwrite");
            result = 1;
            break;
        }
    }
    if (ferror(input)) {
        perror("fread");
        result = 1;
    }
    if (fclose(output) == EOF) {
        perror("fclose destination");
        result = 1;
    }
    if (fclose(input) == EOF) {
        perror("fclose source");
        result = 1;
    }
    return result;
}
