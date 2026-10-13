#ifndef XV6_UPROC_H
#define XV6_UPROC_H
struct uproc {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
};
#endif
