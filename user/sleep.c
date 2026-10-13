#include "kernel/types.h"
#include "user/user.h"
#include "user/a1parse.h"

int
main(int argc, char **argv)
{
  int ticks;
  if (argc != 2 || a1number(argv[1], &ticks) < 0) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  if (pause(ticks) < 0) {
    fprintf(2, "sleep: pause failed\n");
    exit(1);
  }
  exit(0);
}
