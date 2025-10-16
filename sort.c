#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXLEN 512

struct line_node {
  char line[MAXLEN];
  struct line_node *next;
  struct line_node *prev;
};

int read_line(int fd, char *buf, int max) {
  int i = 0, n;
  char c;
  while (i < max - 1) {
    n = read(fd, &c, 1);
    if (n < 1) break;
    if (c == '\n') break;
    buf[i++] = c;
  }
  buf[i] = 0;
  return i;
}

int compare_lines(char *a, char *b, int numerical) {
  if (numerical) {
    int n1 = atoi(a);
    int n2 = atoi(b);
    if (n1 < n2) return -1;
    if (n1 > n2) return 1;
    return 0;
  }
  return strcmp(a, b);
}

void insert_sorted(struct line_node **head, char *line, int reverse, int numerical, int unique) {
  struct line_node *new_node = malloc(sizeof(struct line_node));
  if (!new_node) {
    printf(2, "sort: cannot allocate memory\n");
    return;
  }
  strcpy(new_node->line, line);
  new_node->next = 0;
  new_node->prev = 0;

  if (*head == 0) {
    *head = new_node;
    return;
  }

  if (unique) {
    struct line_node *current = *head;
    while (current) {
      if (compare_lines(current->line, line, numerical) == 0) {
        free(new_node);
        return;
      }
      current = current->next;
    }
  }

  struct line_node *current = *head;

  while (current) {
    int cmp = compare_lines(new_node->line, current->line, numerical);
    if (reverse) cmp = -cmp; // Reverse the comparison
    
    if (cmp < 0) {
      // Insert before current
      new_node->next = current;
      new_node->prev = current->prev;
      
      if (current->prev) {
        current->prev->next = new_node;
      } else {
        *head = new_node;
      }
      current->prev = new_node;
      return;
    }
    
    if (current->next == 0) {
      // Insert at end
      current->next = new_node;
      new_node->prev = current;
      new_node->next = 0;
      return;
    }
    
    current = current->next;
  }
}

void print_and_free(struct line_node *head) {
  struct line_node *current = head;
  struct line_node *next;
  while (current) {
    printf(1, "%s\n", current->line);
    next = current->next;
    free(current);
    current = next;
  }
}

int main(int argc, char *argv[]) {
  int fd = 0;
  int reverse = 0, numerical = 0, unique = 0;
  int argi = 1;
  char buf[MAXLEN];
  struct line_node *head = 0;

  for (; argi < argc; argi++) {
    if (argv[argi][0] != '-') break;
    if (strcmp(argv[argi], "-r") == 0) reverse = 1;
    else if (strcmp(argv[argi], "-n") == 0) numerical = 1;
    else if (strcmp(argv[argi], "-u") == 0) unique = 1;
    else {
      printf(2, "sort: unknown option %s\n", argv[argi]);
      exit();
    }
  }

  if (argi < argc) {
    fd = open(argv[argi], 0);
    if (fd < 0) {
      printf(2, "sort: cannot open %s\n", argv[argi]);
      exit();
    }
  }

  while (read_line(fd, buf, MAXLEN) > 0) {
    insert_sorted(&head, buf, reverse, numerical, unique);
  }

  if (fd > 0) close(fd);

  print_and_free(head);
  exit();
}
