#include "types.h"
#include "fs.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
    // 16,523 blocks write
    int fd = open("largefile.txt", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "Error: cannot create largefile.txt\n");
        exit();
    }
    char buffer[512];
    printf(1, "index starting from 1\n");
    printf(1, "Writing upto block 140\n");
    for (int i = 0; i < 140; i++) {
        // Fill buffer with some data
        for (int j = 0; j < 512; j++) {
            buffer[j] = 'A' + (j % 26);
        }
        if (write(fd, buffer, 512) != 512) {
            printf(1, "Error: write failed at block %d\n", i);
            close(fd);
            exit();
        }
    }
    printf(1, "Writing upto block 16,524\n");
    for (int i = 140; i < 16523; i++) {
        // Fill buffer with some data
        for (int j = 0; j < 512; j++) {
            buffer[j] = 'A' + (j % 26);
        }
        if (write(fd, buffer, 512) != 512) {
            printf(1, "Error: write failed at block %d\n", i);
            close(fd);
            exit();
        }
    }
    printf(1, "Writing upto block 32,907\n");
    for (int i = 16523; i < 32907; i++) {
        // Fill buffer with some data
        for (int j = 0; j < 512; j++) {
            buffer[j] = 'A' + (j % 26);
        }
        if (write(fd, buffer, 512) != 512) {
            printf(1, "Error: write failed at block %d\n", i);
            close(fd);
            exit();
        }
    }
    close(fd);
    return 0;
}