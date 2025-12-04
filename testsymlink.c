#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void
test_symlink()
{
  int fd;
  char buf[16];

  printf(1, "Testing symlinks...\n");

  // 1. Create a regular file
  fd = open("target_file", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "Error: cannot create target_file\n");
    exit();
  }
  write(fd, "Hello Symlink", 13);
  close(fd);

  // 2. Create a symlink to it
  if(symlink("target_file", "sym_link") < 0){
    printf(1, "Error: cannot create sym_link\n");
    exit();
  }

  // 3. Open the symlink and read from it
  fd = open("sym_link", O_RDONLY);
  if(fd < 0){
    printf(1, "Error: cannot open sym_link\n");
    exit();
  }
  read(fd, buf, 13);
  buf[13] = 0;
  close(fd);

  if(strcmp(buf, "Hello Symlink") != 0){
    printf(1, "Error: symlink read failed. Got: %s\n", buf);
  } else {
    printf(1, "Success: symlink read correct data: %s\n", buf);
  }

  // 4. Test O_NOFOLLOW
  fd = open("sym_link", O_RDONLY | O_NOFOLLOW);
  if(fd < 0){
    printf(1, "Error: O_NOFOLLOW failed (open returned -1)\n");
  } else {
    memset(buf, 0, sizeof(buf));
    read(fd, buf, sizeof(buf));
    // We expect to read the symlink content (the path "target_file")
    // The file content is "Hello Symlink" (13 chars). "target_file" is 11 chars.
    if(strcmp(buf, "target_file") == 0){
      printf(1, "Success: O_NOFOLLOW opened symlink and read target path\n");
    } else {
      printf(1, "Error: O_NOFOLLOW opened something, but read '%s' instead of 'target_file'\n", buf);
    }
    close(fd);
  }

  // 5. Test Recursive Symlinks
  symlink("sym_link", "sym_link_2");
  fd = open("sym_link_2", O_RDONLY);
  if(fd < 0){
    printf(1, "Error: cannot open recursive symlink\n");
  } else {
    read(fd, buf, 13);
    buf[13] = 0;
    close(fd);
    if(strcmp(buf, "Hello Symlink") == 0){
      printf(1, "Success: recursive symlink read correct data\n");
    }
  }

  // 6. Test Cyclic Symlinks
  symlink("cycle_1", "cycle_2");
  symlink("cycle_2", "cycle_1");
  fd = open("cycle_1", O_RDONLY);
  if(fd < 0){
    printf(1, "Success: cyclic symlink detected (open failed)\n");
  } else {
    printf(1, "Error: cyclic symlink opened successfully\n");
    close(fd);
  }

  // Cleanup
  unlink("target_file");
  unlink("sym_link");
  unlink("sym_link_2");
  unlink("cycle_1");
  unlink("cycle_2");
}

int
main(int argc, char *argv[])
{
  test_symlink();
  exit();
}