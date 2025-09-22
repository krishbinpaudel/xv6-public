#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define HIDDEN 1
#define SORT 2

struct list_content {
    char name[DIRSIZ];
    struct stat st;
    struct list_content *next;
    struct list_content *prev;
};

void store_in_list(struct list_content **head, struct list_content **tail, char *name, struct stat st, int sort) {
    struct list_content *data = malloc(sizeof(struct list_content));
    if (!data) {
        printf(2, "ls: cannot allocate memory\n");
        return;
    }
    strcpy(data->name, name);
    data->st = st;
    if (*head == 0) {
        data->next = 0;
        data->prev = 0;
        *head = data;
        *tail = data;
        return;
    }
    if (!sort) {
        // insert at the end
        (*tail)->next = data;
        data->prev = *tail;
        data->next = 0;
        *tail = data;
        return;
    }
    // sort by size (descending)
    struct list_content *current = *head;
    while (current) {
        if (data->st.size > current->st.size) {
            // insert before current
            data->next = current;
            data->prev = current->prev;
            if (current->prev) {
                current->prev->next = data;
            } else {
                *head = data;
            }
            current->prev = data;
            return;
        }
        if (current->next == 0) {
            // insert at the end
            current->next = data;
            data->prev = current;
            data->next = 0;
            *tail = data;
            return;
        }
        current = current->next;
    }
}

void deallocate_list(struct list_content *head) {
    struct list_content *current = head;
    struct list_content *next;
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
}

void print_list(struct list_content *head) {
    struct list_content *current = head;
    while (current) {
        printf(1, "%s %d %d %d\n", current->name, current->st.type, current->st.ino, current->st.size);
        current = current->next;
    }
}

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
  struct list_content *head = 0;
  struct list_content *tail = 0;
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
  char *fmted;
  switch(st.type){
  case T_FILE:
    // hide files starting with '.' unless show_hidden is set
    fmted = fmtname(path);
    if(!show_hidden && fmted[0] == '.')
      break;
    store_in_list(&head, &tail, fmted, st, sort);
    //printf(1, "%s %d %d %d\n", fmted, st.type, st.ino, st.size);
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
      // print '/' after directory names
      fmted = fmtname(buf);
      if (st.type == T_DIR){
          int j = 0;
          for (; j < DIRSIZ; j++) {
              if (fmted[j] == ' ') {
                  fmted[j] = '/';
                  break;
              }
          }
        }
      store_in_list(&head, &tail, fmted, st, sort);
      //printf(1, "%s %d %d %d\n", fmted, st.type, st.ino, st.size);
    }
    break;
  }
  print_list(head);
  deallocate_list(head);
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
