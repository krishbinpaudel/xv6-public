#include "types.h"
#include "user.h"

int stdout = 1;
void
hello_world()
{
    printf(stdout,"Hello Xv6!\n");
}

int
main(int argc, char *argv[])
{
    hello_world();
    exit();
}
