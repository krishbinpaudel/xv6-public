#include "types.h"
#include "stat.h"
#include "user.h"

// Volatile to prevent compiler optimization
volatile int dummy = 0;

int
main(int argc, char *argv[])
{
  int pid = getpid();
  int i;
  
  printf(1, "Testing ticks_running syscall\n");
  printf(1, "Current PID: %d\n", pid);
  
  // Test 1: Check initial ticks (should be > 0 since we're running)
  int ticks1 = ticks_running(pid);
  printf(1, "Initial ticks for PID %d: %d\n", pid, ticks1);
  
  // Test 2: Do some real work that won't be optimized away
  printf(1, "Doing CPU-intensive work...\n");
  for(i = 0; i < 10000000; i++) {
    dummy = dummy + i;
    if(i % 1000000 == 0) {
      // Periodically do something to prevent optimization
      dummy = dummy % 100;
    }
  }
  
  int ticks2 = ticks_running(pid);
  printf(1, "Ticks after busy work: %d\n", ticks2);
  printf(1, "Ticks increased by: %d\n", ticks2 - ticks1);
  
  if(ticks2 - ticks1 < 5) {
    printf(1, "WARNING: Expected more ticks! CPU work may have been optimized away.\n");
  }
  
  // Test 3: Test non-existent PID (should return -1)
  int invalid_ticks = ticks_running(99999);
  printf(1, "Ticks for invalid PID 99999: %d (should be -1)\n", invalid_ticks);
  
  // Test 4: Fork a child and check its ticks
  int child_pid = fork();
  if(child_pid == 0) {
    // Child process - do some work
    for(i = 0; i < 5000000; i++) {
      dummy = dummy + i;
    }
    int child_ticks = ticks_running(getpid());
    printf(1, "Child PID %d ticks: %d (should be > 0)\n", getpid(), child_ticks);
    exit();
  } else {
    // Parent waits for child
    wait();
    printf(1, "Child process completed\n");
  }
  
  // Test 5: Check final ticks for parent
  int ticks3 = ticks_running(pid);
  printf(1, "Final parent ticks: %d (total increase: %d)\n", ticks3, ticks3 - ticks1);
  
  printf(1, "\nAll tests completed!\n");
  exit();
}
