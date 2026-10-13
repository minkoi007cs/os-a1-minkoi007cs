#include "kernel/types.h"
#include "user/user.h"

static int
maskvalue(const char *s, int *mask)
{
  int negative = *s == '-';
  if (negative)
    s++;
  if (*s == 0)
    return -1;
  uint value = 0, limit = negative ? 2147483648U : 2147483647U;
  for (; *s; s++) {
    if (*s < '0' || *s > '9' || value > (limit - (*s - '0')) / 10)
      return -1;
    value = value * 10 + (*s - '0');
  }
  *mask = negative && value ? -((int)(value - 1)) - 1 : (int)value;
  return 0;
}

int
main(int argc, char **argv)
{
  int mask;
  if (argc < 3 || maskvalue(argv[1], &mask) < 0) {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }
  if (trace(mask) < 0) {
    fprintf(2, "trace: system call failed\n");
    exit(1);
  }
  exec(argv[2], &argv[2]);
  fprintf(2, "trace: cannot exec %s\n", argv[2]);
  exit(1);
}
