#include "stdio.h"
#include "sys/stat.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s directory_name ...\n", argv[0]);
        return -1;
    }
    return mkdir(argv[1], S_IRWXO);
}
