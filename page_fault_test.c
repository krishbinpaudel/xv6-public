#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "\n=== Page Fault Test Program ===\n");
    printf(1, "PID: %d\n\n", getpid());
    
    // Test 1: Single page allocation
    // printf(1, "Test 1: Allocating 1 page (4096 bytes)\n");
    // char *p1 = sbrk(4096);
    // printf(1, "sbrk returned: 0x%x\n", p1);
    // printf(1, "Writing to first byte of allocated page...\n");
    // p1[0] = 'A';
    // printf(1, "Successfully wrote: %c\n\n", p1[0]);
    
    // Test 2: Multiple pages allocation
    // for locality-aware allocation, these pages should be allocated together, no page faults after first
    printf(1, "Test 2: Allocating 3 pages (12288 bytes) and accessing them sequentially\n");
    char *p2 = sbrk(12288);
    printf(1, "sbrk returned: 0x%x\n", p2);
    printf(1, "Writing to first byte of page 1...\n");
    p2[0] = 'B';
    printf(1, "Successfully wrote: %c\n", p2[0]);
    printf(1, "Writing to first byte of page 2 (offset 4096)...\n");
    p2[4096] = 'C';
    printf(1, "Successfully wrote: %c\n", p2[4096]);
    printf(1, "Writing to first byte of page 3 (offset 8192)...\n");
    p2[8192] = 'D';
    printf(1, "Successfully wrote: %c\n\n", p2[8192]);
    
    // Test 3: Non-sequential access pattern
    printf(1, "Test 3: Allocating 5 pages and accessing non-sequentially\n");
    char *p3 = sbrk(20480);
    printf(1, "sbrk returned: 0x%x\n", p3);
    printf(1, "Accessing page 3 first (offset 12288)...\n");
    p3[12288] = 'X';
    printf(1, "Successfully wrote: %c\n", p3[12288]);
    printf(1, "Now accessing page 1 (offset 4096)...\n");
    p3[4096] = 'Y';
    printf(1, "Successfully wrote: %c\n", p3[4096]);
    printf(1, "Now accessing page 0 (offset 0)...\n");
    p3[0] = 'Z';
    printf(1, "Successfully wrote: %c\n\n", p3[0]);
    
    printf(1, "=== All Tests Passed! ===\n\n");

    printf(1, "Test 4: Allocating 5 pages and accessing sequentially\n");
    char *p4 = sbrk(20480);
    printf(1, "sbrk returned: 0x%x\n", p4);
    for(int i = 0; i < 5; i++) {
        printf(1, "Accessing page %d (offset %d)...\n", i, i * 4096);
        p4[i * 4096] = 'A' + i;
        printf(1, "Successfully wrote: %c\n", p4[i * 4096]);
    }
    exit();
}
