#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

int
main(void)
{
  struct uproc processes[NPROC];
  const char *states[] = {"unused", "used", "sleeping", "runnable", "running", "zombie"};
  int n = getprocs(processes, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for (int i = 0; i < n; i++) {
    struct uproc *p = &processes[i];
    printf("%d\t%d\t%s\t%lu\t%s\n", p->pid, p->ppid, states[p->state], p->sz, p->name);
  }
  exit(0);
}
