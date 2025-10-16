#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid = getpid();
  int i;
  
  printf(1, "Parent PID: %d, Job Length: %d\n\n", pid, sjf_job_length(pid));
  
  // Create 3 child processes
  for(i = 0; i < 3; i++){
    int child_pid = fork();
    
    if(child_pid == 0){
      // Child process
      int my_pid = getpid();
      printf(1, "Child %d: PID=%d, Job Length=%d\n", 
             i, my_pid, sjf_job_length(my_pid));
      
      volatile int dummy = 0;
      int j;
      for(j = 0; j < 50000000; j++){
        dummy = dummy + j;
      }

      printf(1, "Child %d: done (ticks=%d) Uptime: %d\n", i, ticks_running(my_pid), uptime());
      exit();
    }
  }
  
  for(i = 0; i < 3; i++){
    wait();
  }
  
  // Test invalid PID
  printf(1, "\nTesting invalid PID: %d\n", sjf_job_length(99999));
  
  exit();
}
