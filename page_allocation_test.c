#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "Test: Lazy page allocation\n");
    char *p = sbrk(4096);
    printf(1, "sbrk returned: 0x%x\n", p);
    
    p[0] = 'H';
    p[1] = 'i';
    p[2] = '\0';
    printf(1, "Wrote: %s\n", p);
    
    printf(1, "All tests passed!\n");
    char *null_ptr = 0;
    *null_ptr = 'x';  // Write to address 0x0000
    exit();
}