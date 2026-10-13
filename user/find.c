#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

struct entry {
  struct entry *next;
  char name[DIRSIZ + 1];
};

// Remember the most recent star and retry it with one more character.
// This avoids exponential recursive matching for patterns with many stars.
static int
matches(const char *name, const char *pattern)
{
  const char *star = 0, *retry = 0;
  while (*name) {
    if (*pattern == '*') {
      star = pattern++;
      retry = name;
    } else if (*pattern == *name) {
      pattern++;
      name++;
    } else if (star) {
      pattern = star + 1;
      name = ++retry;
    } else {
      return 0;
    }
  }
  while (*pattern == '*')
    pattern++;
  return *pattern == 0;
}

static void
discard(struct entry *list)
{
  while (list) {
    struct entry *next = list->next;
    free(list);
    list = next;
  }
}

static int
read_entries(const char *dir, struct entry **entries)
{
  struct stat st;
  struct dirent de;
  struct entry *head = 0, **tail = &head;
  int fd, n;

  if ((fd = open(dir, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", dir);
    return 1;
  }
  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    fprintf(2, "find: not a directory %s\n", dir);
    close(fd);
    return 1;
  }

  // Copy directory names, then close the fd before descending.
  while ((n = read(fd, &de, sizeof(de))) == sizeof(de)) {
    char name[DIRSIZ + 1];
    if (!de.inum)
      continue;
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;
    if (!strcmp(name, ".") || !strcmp(name, ".."))
      continue;
    struct entry *e = malloc(sizeof(*e));
    if (e == 0) {
      fprintf(2, "find: out of memory\n");
      close(fd);
      discard(head);
      return 1;
    }
    strcpy(e->name, name);
    e->next = 0;
    *tail = e;
    tail = &e->next;
  }
  close(fd);
  if (n != 0) {
    fprintf(2, "find: cannot read %s\n", dir);
    discard(head);
    return 1;
  }

  *entries = head;
  return 0;
}

struct traversal {
  struct traversal *parent;
  struct entry *remaining;
  char *dir;
};

static int
search(const char *dir, const char *pattern, int type)
{
  struct traversal *frame = malloc(sizeof(*frame));
  if (frame == 0) {
    fprintf(2, "find: out of memory\n");
    return 1;
  }
  frame->dir = malloc(strlen(dir) + 1);
  if (frame->dir == 0) {
    fprintf(2, "find: out of memory\n");
    free(frame);
    return 1;
  }
  strcpy(frame->dir, dir);
  frame->parent = 0;
  if (read_entries(dir, &frame->remaining)) {
    free(frame->dir);
    free(frame);
    return 1;
  }

  // Explicit heap frames perform depth-first recursive traversal without
  // consuming another user-stack frame or open fd for every directory.
  int failed = 0;
  while (frame) {
    if (frame->remaining == 0) {
      struct traversal *parent = frame->parent;
      free(frame->dir);
      free(frame);
      frame = parent;
      continue;
    }
    struct entry *e = frame->remaining;
    frame->remaining = e->next;
    uint len = strlen(frame->dir);
    char *path = malloc(len + strlen(e->name) + 2);
    if (path == 0) {
      fprintf(2, "find: out of memory\n");
      free(e);
      failed = 1;
      break;
    }
    strcpy(path, frame->dir);
    uint offset = len;
    if (len == 0 || frame->dir[len - 1] != '/')
      path[offset++] = '/';
    strcpy(path + offset, e->name);
    struct stat st;
    if (stat(path, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", path);
      failed = 1;
    } else {
      int allowed = type == 0 || (type == T_DIR && st.type == T_DIR) ||
                    (type == T_FILE && (st.type == T_FILE || st.type == T_DEVICE));
      if (allowed && matches(e->name, pattern))
        printf("%s\n", path);
      if (st.type == T_DIR) {
        struct traversal *child = malloc(sizeof(*child));
        if (child == 0) {
          fprintf(2, "find: out of memory\n");
          failed = 1;
        } else if (read_entries(path, &child->remaining)) {
          failed = 1;
          free(child);
        } else {
          child->dir = path;
          child->parent = frame;
          frame = child;
          path = 0; // Ownership moved to the new frame.
        }
      }
    }
    if (path)
      free(path);
    free(e);
  }
  while (frame) {
    struct traversal *parent = frame->parent;
    discard(frame->remaining);
    free(frame->dir);
    free(frame);
    frame = parent;
  }
  return failed;
}

int
main(int argc, char **argv)
{
  int type = 0;
  if (argc != 3 && argc != 5)
    goto usage;
  if (argc == 5) {
    if (strcmp(argv[3], "-type"))
      goto usage;
    if (!strcmp(argv[4], "f"))
      type = T_FILE;
    else if (!strcmp(argv[4], "d"))
      type = T_DIR;
    else
      goto usage;
  }
  exit(search(argv[1], argv[2], type));
usage:
  fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
  exit(1);
}
