#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf(2, "Usage: sleep <number_of_ticks>\n");
        exit();
    }
    int ticks = atoi(argv[1]);
    if (ticks < 0) {
        printf(2, "Error: Number of ticks must be non-negative\n");
        exit();
    }
    sleep(ticks);
    exit();
}