#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include "user/a1parse.h"

static void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

static int
run(char **args, int count)
{
  args[count] = 0;
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    return 1;
  }
  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "xargs: cannot exec %s\n", args[0]);
    exit(1);
  }
  int status;
  if (wait(&status) != pid) {
    fprintf(2, "xargs: wait failed\n");
    return 1;
  }
  return status != 0;
}

static void
clearwords(char **args, int base, int *count)
{
  for (int i = base; i < *count; i++)
    free(args[i]);
  *count = base;
}

int
main(int argc, char **argv)
{
  int first = 1, batch = 0, limited = 0;
  if (argc > 1 && !strcmp(argv[1], "-n")) {
    if (argc < 4 || a1number(argv[2], &batch) < 0 || batch == 0)
      usage();
    first = 3;
    limited = 1;
  }
  int base = argc - first;
  if (base < 1 || base > MAXARG)
    usage();
  char *args[MAXARG + 1];
  for (int i = 0; i < base; i++)
    args[i] = argv[first + i];
  int limit = MAXARG - base;
  if (limited && batch < limit)
    limit = batch;

  char *word = 0;
  uint length = 0, capacity = 0;
  int count = base, failed = 0;
  char c;
  int n;
  while ((n = read(0, &c, 1)) > 0) {
    if (c != ' ' && c != '\t' && c != '\n') {
      if (length + 1 >= capacity) {
        uint next = capacity ? capacity * 2 : 32;
        if (next <= capacity) {
          fprintf(2, "xargs: word too long\n");
          exit(1);
        }
        char *grown = malloc(next);
        if (grown == 0) {
          fprintf(2, "xargs: out of memory\n");
          exit(1);
        }
        if (word) {
          memmove(grown, word, length);
          free(word);
        }
        word = grown;
        capacity = next;
      }
      word[length++] = c;
    } else {
      if (length) {
        if (count == MAXARG) {
          fprintf(2, "xargs: too many arguments\n");
          exit(1);
        }
        word[length] = 0;
        args[count++] = word;
        word = 0;
        length = capacity = 0;
        if (limited && count - base == limit) {
          failed |= run(args, count);
          clearwords(args, base, &count);
        }
      }
      if (!limited && c == '\n' && count > base) {
        failed |= run(args, count);
        clearwords(args, base, &count);
      }
    }
  }
  if (n < 0) {
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }
  if (length) {
    if (count == MAXARG) {
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    word[length] = 0;
    args[count++] = word;
  }
  if (count > base)
    failed |= run(args, count);
  clearwords(args, base, &count);
  exit(failed ? 1 : 0);
}
