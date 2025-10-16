#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  // same priority test
  for(i = 0; i < 2; i++){
    if(fork() == 0){
      // Child process
      set_sched_priority(1);  // Both children with same priority
      
      volatile long dummy = 0;
      long j;
      for(j = 0; j < 500000000; j++){
        dummy = dummy + j;
      }
      
      // Print only at end
      printf(1, "Child %d (Pri %d): ticks=%d, Uptime: %d\n", 
             i, 1, ticks_running(getpid()), uptime());
      exit();
    }
  }

  wait();
  wait();

  // different priority test
  printf(1, "\nDifferent priority test:\n");
  
  int pid0, pid1;
  
  // Fork BOTH children first
  pid0 = fork();
  if(pid0 == 0){
    // Child 0 - LOW priority
    set_sched_priority(1);
    printf(1, "[Child 0] Priority set to 1\n");
    sleep(1);  // Force yield
    
    volatile long dummy = 0;
    long j;
    for(j = 0; j < 500000000; j++){
      dummy = dummy + j;
      // Print less frequently to reduce console collisions
      if(j % 200000000 == 0 && j > 0){
        printf(1, "[Child 0] Progress: %dM iterations, ticks=%d\n", j/1000000, ticks_running(getpid()));
      }
    }

    printf(1, "[Child 0] DONE (Pri 1): ticks=%d\n", ticks_running(getpid()));
    exit();
  }
  
  pid1 = fork();
  if(pid1 == 0){
    // Child 1 - HIGH priority
    set_sched_priority(0);
    printf(1, "[Child 1] Priority set to 0 (should preempt Child 0!)\n");
    sleep(1);  // Force yield
    
    volatile long dummy = 0;
    long j;
    for(j = 0; j < 500000000; j++){
      dummy = dummy + j;
      // Print less frequently to reduce console collisions
      if(j % 200000000 == 0 && j > 0){
        printf(1, "[Child 1] Progress: %dM iterations, ticks=%d\n", j/1000000, ticks_running(getpid()));
      }
      // set priority at 80% of iteration to low
        if (j == 200000000) {
            set_sched_priority(2);
            printf(1, "[Child 1] Priority changed to 2 \n");
        }
    }
    
    printf(1, "[Child 1] DONE (Pri 0): ticks=%d\n", ticks_running(getpid()));
    exit();
  }
  
  wait();
  wait();
  
  exit();
}
