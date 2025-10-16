// Simple grep with substring matching and flags: -i, -n, -v

#include "types.h"
#include "stat.h"
#include "user.h"

char buf[1024];
int match(char*, char*);
int match_case_insensitive(char*, char*);

int flag_ignore_case = 0;
int flag_line_number = 0;
int flag_invert_match = 0;

char to_lower(char c) {
  if (c >= 'A' && c <= 'Z')
    return c + ('a' - 'A');
  return c;
}

void
grep(char *pattern, int fd, char *filename)
{
  int n, m;
  char *p, *q;
  int line_num = 0;

  m = 0;
  while((n = read(fd, buf+m, sizeof(buf)-m-1)) > 0){
    m += n;
    buf[m] = '\0';
    p = buf;
    while((q = strchr(p, '\n')) != 0){
      *q = 0;
      line_num++;
      
      int matches;
      if (flag_ignore_case)
        matches = match_case_insensitive(pattern, p);
      else
        matches = match(pattern, p);
      
      // Invert match if -v flag is set
      if (flag_invert_match)
        matches = !matches;
      
      if(matches){
        // Print filename if multiple files (not implemented here)
        // Print line number if -n flag
        if (flag_line_number)
          printf(1, "%d:", line_num);
        
        *q = '\n';
        write(1, p, q+1 - p);
      }
      p = q+1;
    }
    if(p == buf)
      m = 0;
    if(m > 0){
      m -= p - buf;
      memmove(buf, p, m);
    }
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;
  char *pattern;
  int argi = 1;

  if(argc <= 1){
    printf(2, "usage: grep [-inv] pattern [file ...]\n");
    exit();
  }

  while (argi < argc && argv[argi][0] == '-') {
    char *flags = argv[argi] + 1;
    if (*flags == '\0') {
      printf(2, "usage: grep [-inv] pattern [file ...]\n");
      exit();
    }
    
    while (*flags) {
      if (*flags == 'i')
        flag_ignore_case = 1;
      else if (*flags == 'n')
        flag_line_number = 1;
      else if (*flags == 'v')
        flag_invert_match = 1;
      else {
        printf(2, "grep: unknown flag -%c\n", *flags);
        exit();
      }
      flags++;
    }
    argi++;
  }

  if (argi >= argc) {
    printf(2, "usage: grep [-inv] pattern [file ...]\n");
    exit();
  }

  pattern = argv[argi++];

  if(argi >= argc){
    grep(pattern, 0, "");
    exit();
  }

  for(i = argi; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "grep: cannot open %s\n", argv[i]);
      exit();
    }
    // Print filename if multiple files
    if (argc - argi > 1)
      printf(1, "%s:\n", argv[i]);
    grep(pattern, fd, argv[i]);
    close(fd);
  }
  exit();
}

// Case-sensitive substring match
int match(char *pattern, char *text)
{
  int plen = strlen(pattern);
  int tlen = strlen(text);
  int i, j;
  
  if (plen == 0)
    return 1;
  
  // Try matching at each position in text
  for (i = 0; i <= tlen - plen; i++) {
    for (j = 0; j < plen; j++) {
      if (text[i + j] != pattern[j])
        break;
    }
    if (j == plen)
      return 1;  // Found match
  }
  return 0;
}

// Case-insensitive substring match
int match_case_insensitive(char *pattern, char *text)
{
  int plen = strlen(pattern);
  int tlen = strlen(text);
  int i, j;
  
  if (plen == 0)
    return 1;
  
  // Try matching at each position in text
  for (i = 0; i <= tlen - plen; i++) {
    for (j = 0; j < plen; j++) {
      if (to_lower(text[i + j]) != to_lower(pattern[j]))
        break;
    }
    if (j == plen)
      return 1;  // Found match
  }
  return 0;
}

