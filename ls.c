#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define HIDDEN 1
#define SORT 2

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;


  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path, uint show_hidden, uint sort)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
  switch(st.type){
  case T_FILE:
    // hide files starting with '.' unless show_hidden is set
    if(!show_hidden && fmtname(path)[0] == '.')
      break;
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      // hide files/folders starting with '.' unless show_hidden is set
      if(!show_hidden && de.name[0] == '.')
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

// parsing options passed into the command line
int check_options(char *argv[], int argc) {
    int show_hidden = 0;
    int sort = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            show_hidden = HIDDEN;
        } else if (strcmp(argv[i], "-s") == 0) {
            sort = SORT;
        }
    }
    // we use bitwise OR to combine options
    return show_hidden | sort;
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".",0,0);
    exit();
  }
  int options = check_options(argv, argc);
  if (options){
    //number of options
    int option_count = 0, buf = options;
    for (; buf != 0; buf >>= 1){if (buf & 1) option_count++;};
    int folder_count = argc - option_count - 1;
    int offset = option_count + 1;
    printf(1, "Number of folders: %d\n", folder_count);
    if (folder_count > 0){
      for(i=offset; i<argc; i++)
        ls(argv[i], (options & HIDDEN) ? 1 : 0, (options & SORT) ? 1 : 0);  
      exit();  
    } else {
      ls(".", (options & HIDDEN) ? 1 : 0, (options & SORT) ? 1 : 0);
      exit();
    }
  }
  for(i=1; i<argc; i++)
    ls(argv[i],0,0);
  exit();
}
