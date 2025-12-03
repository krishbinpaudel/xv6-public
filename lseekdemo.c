#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd;
  char buf[100];
  
  printf(1, "lseek demo starting...\n");

  // Clean up old file
  unlink("lseek_test.txt");

  fd = open("lseek_test.txt", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "error opening file\n");
    exit();
  }

  printf(1, "Writing 'Hello'...\n");
  if(write(fd, "Hello", 5) != 5){
    printf(1, "error writing Hello\n");
    exit();
  }

  printf(1, "Seeking forward by 10 bytes...\n");
  // Current offset is 5. Adding 10 makes it 15.
  // Hole from 5 to 15 (10 bytes).
  int new_off = lseek(fd, 10);
  if(new_off < 0){
    printf(1, "error lseeking\n");
    exit();
  }
  printf(1, "New offset: %d (expected 15)\n", new_off);

  printf(1, "Writing 'World'...\n");
  if(write(fd, "World", 5) != 5){
    printf(1, "error writing World\n");
    exit();
  }

  close(fd);

  printf(1, "Closed and reopening file to verify...\n");
  fd = open("lseek_test.txt", O_RDONLY);
  if(fd < 0){
    printf(1, "error opening file\n");
    exit();
  }

  int n = read(fd, buf, sizeof(buf));
  if(n < 0){
    printf(1, "error reading\n");
    exit();
  }

  printf(1, "Read %d bytes:\n", n);
  for(int i=0; i<n; i++){
    if(buf[i] == 0)
      printf(1, "."); // Print dot for zero
    else
      printf(1, "%c", buf[i]);
  }
  printf(1, "\n");

  // Test seeking backwards
  printf(1, "Seeking back to start...\n");
  // We are at n. lseek(fd, -n) should go to 0.
  new_off = lseek(fd, -n);
  if(new_off < 0){
     printf(1, "error seeking back\n");
     exit();
  }
  printf(1, "New offset: %d (expected 0)\n", new_off);
  
  char buf2[6];
  read(fd, buf2, 5);
  buf2[5] = 0;
  printf(1, "Read from start: %s\n", buf2);

  close(fd);
  exit();
}
