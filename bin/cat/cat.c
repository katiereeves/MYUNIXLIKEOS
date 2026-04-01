#include "stdio.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat <file>\n");
        return -1;
    }

    char buf[256];
    char **pptr = argv;

    for (++pptr; --argc; pptr++) {
        FILE* f = fopen(*pptr, "r");
        if (!f) {
            printf("cat: %s: No such file or directory\n", *pptr);
            return -1;
        }

        size_t n;
        while ((n = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
            buf[n] = '\0';
            printf("%s", buf);
        }
        fclose(f);
    }

    return 0;
}