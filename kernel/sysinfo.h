#ifndef XV6_SYSINFO_H
#define XV6_SYSINFO_H
struct sysinfo {
  uint64 freemem;
  uint64 nproc;
  uint64 nopenfile;
};
#endif
