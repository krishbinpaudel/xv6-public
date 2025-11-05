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
      int job_len = sjf_job_length(my_pid);
      printf(1, "Child %d: PID=%d, Job Length=%d\n", i, my_pid, job_len);
      
      // Do work proportional to job length to simulate different workloads
      // Each unit of job length = 5 million iterations
      volatile long dummy = 0;
      long j;
      long total_iterations = job_len * 5000000L;
      for(j = 0; j < total_iterations; j++){
        dummy = dummy + j;
      }

      printf(1, "Child %d: DONE - ticks=%d, uptime=%d\n", i, ticks_running(my_pid), uptime());
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
