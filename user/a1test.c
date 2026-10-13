// Assignment-specific regression tests, independent of the missing grade-a1.
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/sysinfo.h"
#include "kernel/uproc.h"
#include "user/user.h"

static char output[16384];
static struct uproc rows[NPROC];
static int checks;

static void
check(int condition, const char *label)
{
  if (!condition) {
    fprintf(2, "A1 FAIL: %s\n", label);
    exit(1);
  }
  checks++;
}

static int
contains(const char *s, const char *needle)
{
  uint n = strlen(needle);
  for (; *s; s++)
    if (strlen(s) >= n && memcmp(s, needle, n) == 0)
      return 1;
  return 0;
}

// Feed stdin in a separate process to avoid input/output pipe deadlocks.
static int
capture(char **args, const char *input)
{
  int in[2], out[2];
  check(pipe(in) == 0 && pipe(out) == 0, "capture pipes");
  int command = fork();
  check(command >= 0, "capture fork");
  if (command == 0) {
    close(0);
    dup(in[0]);
    close(1);
    dup(out[1]);
    close(2);
    dup(out[1]);
    close(in[0]); close(in[1]); close(out[0]); close(out[1]);
    exec(args[0], args);
    exit(127);
  }
  int writer = fork();
  check(writer >= 0, "input fork");
  if (writer == 0) {
    close(in[0]); close(out[0]); close(out[1]);
    int length = strlen(input), used = 0;
    while (used < length) {
      int n = write(in[1], input + used, length - used);
      if (n < 1)
        exit(1);
      used += n;
    }
    close(in[1]);
    exit(0);
  }
  close(in[0]); close(in[1]); close(out[1]);
  int used = 0, n;
  while ((n = read(out[0], output + used, sizeof(output) - used - 1)) > 0) {
    used += n;
    check(used < sizeof(output) - 1, "capture output fits");
  }
  check(n == 0, "capture read");
  output[used] = 0;
  close(out[0]);
  int result = -1;
  for (int i = 0; i < 2; i++) {
    int status, pid = wait(&status);
    check(pid == command || pid == writer, "capture wait");
    if (pid == command)
      result = status;
  }
  return result;
}

static void
utilities(void)
{
  char *s0[] = {"sleep", "0", 0};
  check(capture(s0, "") == 0 && output[0] == 0, "sleep zero");
  char *s2[] = {"sleep", "2", 0};
  int start = uptime();
  check(capture(s2, "") == 0 && uptime() - start >= 2, "sleep ticks");
  char *bad[] = {"sleep", 0, 0, 0};
  check(capture(bad, "") == 1 && !strcmp(output, "usage: sleep <ticks>\n"), "sleep missing");
  const char *invalid[] = {"-1", "a", "1a", "", "2147483648"};
  for (int i = 0; i < 5; i++) {
    bad[1] = (char *)invalid[i];
    check(capture(bad, "") == 1 && !strcmp(output, "usage: sleep <ticks>\n"), "sleep bad argument");
  }
  bad[1] = "1"; bad[2] = "2";
  check(capture(bad, "") == 1, "sleep extra argument");

  mkdir("a1tree"); mkdir("a1tree/a"); mkdir("a1tree/a/b");
  int fd = open("a1tree/a/b/axxbyy", O_CREATE | O_WRONLY);
  check(fd >= 0, "find fixture"); close(fd);
  fd = open("a1tree/ab", O_CREATE | O_WRONLY); check(fd >= 0, "find fixture 2"); close(fd);
  char *findargs[] = {"find", "a1tree", "a*b*", 0, 0, 0};
  check(capture(findargs, "") == 0 && contains(output, "a1tree/ab\n") &&
        contains(output, "a1tree/a/b/axxbyy\n"), "find wildcard middle and end");
  findargs[2] = "*byy";
  check(capture(findargs, "") == 0 && !strcmp(output, "a1tree/a/b/axxbyy\n"), "find leading wildcard");
  findargs[2] = "*"; findargs[3] = "-type"; findargs[4] = "d";
  check(capture(findargs, "") == 0 && contains(output, "a1tree/a\n") &&
        contains(output, "a1tree/a/b\n") && !contains(output, "axxbyy"), "find directories only");
  findargs[4] = "f";
  check(capture(findargs, "") == 0 && contains(output, "axxbyy") &&
        !contains(output, "a1tree/a\n"), "find files only");
  char *trailing[] = {"find", "a1tree/", "ab", 0};
  check(capture(trailing, "") == 0 && !strcmp(output, "a1tree/ab\n"), "find trailing slash");
  char *root[] = {"find", "/", "a1tree", "-type", "d", 0};
  check(capture(root, "") == 0 && !strcmp(output, "/a1tree\n"), "find root slash");
  char *missing[] = {"find", "a1missing", "*", 0};
  check(capture(missing, "") == 1 && !strcmp(output, "find: cannot open a1missing\n"), "find missing directory");
  check(mknod("a1tree/device", 1, 1) == 0, "find device fixture");
  char *device[] = {"find", "a1tree", "device", "-type", "f", 0};
  check(capture(device, "") == 0 && !strcmp(output, "a1tree/device\n"), "find -type f includes T_DEVICE");
  fd = open("a1tree/abcdefghijklmn", O_CREATE | O_WRONLY);
  check(fd >= 0, "find full DIRSIZ fixture"); close(fd);
  char *full[] = {"find", "a1tree", "abc**mn", 0};
  check(capture(full, "") == 0 && !strcmp(output, "a1tree/abcdefghijklmn\n"), "find full DIRSIZ name and consecutive stars");
  fd = open("a1tree/a?b", O_CREATE | O_WRONLY);
  check(fd >= 0, "find literal punctuation fixture"); close(fd);
  char *literal[] = {"find", "a1tree", "a?b", 0};
  check(capture(literal, "") == 0 && !strcmp(output, "a1tree/a?b\n"), "find only star is special");
  char path[MAXPATH] = "a1tree";
  for (int i = 0; i < 52; i++) {
    strcpy(path + strlen(path), "/d");
    check(mkdir(path) == 0, "deep directory");
  }
  strcpy(path + strlen(path), "/leaf");
  fd = open(path, O_CREATE | O_WRONLY); check(fd >= 0, "deep leaf"); close(fd);
  findargs[2] = "leaf"; findargs[3] = 0;
  check(capture(findargs, "") == 0 && contains(output, path), "find fifty-two levels without fd or stack exhaustion");

  char *xa[] = {"xargs", "echo", "x", 0};
  check(capture(xa, "a b\n\n \t\nc d") == 0 && !strcmp(output, "x a b\nx c d\n"), "xargs per nonempty line and EOF");
  char *xn[] = {"xargs", "-n", "2", "echo", "x", 0};
  check(capture(xn, "a\tb\nc\n\nd e") == 0 && !strcmp(output, "x a b\nx c d\nx e\n"), "xargs -n across lines");
  check(capture(xn, "\n \t\n") == 0 && output[0] == 0, "xargs empty input");
  xn[2] = "0";
  check(capture(xn, "a") == 1 && contains(output, "usage: xargs"), "xargs zero -n");
  xn[2] = "2a";
  check(capture(xn, "a") == 1, "xargs malformed -n");
  char *xfail[] = {"xargs", "-n", "1", "a1test", "ret7", 0};
  check(capture(xfail, "a b c") == 1 && !strcmp(output, "ret7\nret7\nret7\n"), "xargs waits and accumulates failure without stopping");
  char *xexec[] = {"xargs", "a1missing", 0};
  check(capture(xexec, "a") == 1 && contains(output, "cannot exec"), "xargs exec failure");
  char *xmax[] = {"xargs", "-n", "30", "a1test", "args", 0};
  char input[100];
  for (int i = 0; i < 30; i++) {input[2*i]='q'; input[2*i+1]=' ';}
  input[60] = 0;
  check(capture(xmax, input) == 0 && contains(output, "argc=32"), "xargs exact MAXARG actual arguments");
  input[60] = 'q'; input[61] = '\n'; input[62] = 0;
  check(capture(xmax, input) == 0 && contains(output, "argc=32") &&
        contains(output, "\nargc=3 q\n"), "xargs final partial batch after MAXARG");
  char *xover[] = {"xargs", "a1test", "args", 0};
  check(capture(xover, input) == 1 && !strcmp(output, "xargs: too many arguments\n"), "xargs rejects overfull default line");
  char *xone[] = {"xargs", "-n", "1", "echo", 0};
  check(capture(xone, "a\t\nb\n") == 0 && !strcmp(output, "a\nb\n"), "xargs -n 1 and EOF delimiter do not add empty run");
  char *xlong[] = {"xargs", "a1test", "args", 0};
  char *longword = malloc(901);
  check(longword != 0, "long word allocation");
  memset(longword, 'z', 900); longword[900] = 0;
  check(capture(xlong, longword) == 0 && contains(output, "argc=3") &&
        contains(output, longword), "xargs long word without a fixed token buffer");
  free(longword);
}

static void
information(void)
{
  struct sysinfo before, after;
  check(sysinfo(&before) == 0, "sysinfo basic");
  check(sysinfo((void *)-1ULL) == -1, "sysinfo bad pointer");
  check(sysinfo((void *)0) == -1, "sysinfo cannot overwrite read-only text");
  check(before.freemem > 0 && before.freemem % 4096 == 0, "sysinfo free bytes");
  char *memory = sbrk(3 * 4096);
  check(memory != SBRK_ERROR, "allocate memory");
  memset(memory, 1, 3 * 4096);
  check(sysinfo(&after) == 0 && after.freemem + 3 * 4096 <= before.freemem, "sysinfo allocation accounting");
  check(sbrk(-3 * 4096) != SBRK_ERROR, "free memory");
  check(sysinfo(&before) == 0, "sysinfo file baseline");
  int fd = open("README", O_RDONLY);
  check(fd >= 0 && sysinfo(&after) == 0 && after.nopenfile == before.nopenfile + 1, "sysinfo open file entry");
  int duplicate = dup(fd);
  check(duplicate >= 0 && sysinfo(&after) == 0 && after.nopenfile == before.nopenfile + 1, "dup is a reference, not another file entry");
  close(duplicate); close(fd);
  int fds[2];
  check(pipe(fds) == 0 && sysinfo(&after) == 0 && after.nopenfile == before.nopenfile + 2, "sysinfo pipe two entries");
  close(fds[0]); close(fds[1]);
  check(sysinfo(&after) == 0 && after.nopenfile == before.nopenfile, "sysinfo close accounting");

  check(getprocs(rows, -1) == -1, "getprocs negative max");
  check(getprocs((void *)-1ULL, 0) == 0, "getprocs zero max does not dereference");
  check(getprocs((void *)-1ULL, 1) == -1, "getprocs bad pointer");
  check(getprocs((void *)0, 1) == -1, "getprocs cannot overwrite read-only text");
  memset(rows, 0x5a, sizeof(rows));
  check(getprocs(rows, 1) == 1 && rows[0].pid == 1 && rows[0].ppid == 0, "getprocs partial and table order");
  check(rows[1].pid == 0x5a5a5a5a, "getprocs max prevents overwrite of next row");
  int n = getprocs(rows, 2147483647), self = 0;
  check(n >= 3 && n <= NPROC, "getprocs large max");
  for (int i = 0; i < n; i++) {
    check(rows[i].pid > 0 && rows[i].state >= 1 && rows[i].state <= 5, "getprocs valid process fields");
    int terminated = 0;
    for (int j = 0; j < 16; j++) if (!rows[i].name[j]) terminated = 1;
    check(terminated, "getprocs terminated name");
    if (rows[i].pid == getpid()) self = 1;
  }
  check(self, "getprocs contains caller");
  char *lazy = sbrklazy(4096);
  check(lazy != SBRK_ERROR, "allocate lazy copyout destination");
  check(sysinfo((struct sysinfo *)lazy) == 0, "sysinfo copyout can fault without a held lock");
  check(getprocs((struct uproc *)lazy, 1) == 1, "getprocs copyout can fault without a held lock");
  check(sbrk(-4096) != SBRK_ERROR, "release lazy page");

  int control[2];
  check(pipe(control) == 0 && sysinfo(&before) == 0, "process count baseline");
  int child = fork(); check(child >= 0, "count fork");
  if (child == 0) {
    close(control[1]); char c; read(control[0], &c, 1); close(control[0]); exit(0);
  }
  close(control[0]);
  check(sysinfo(&after) == 0 && after.nproc == before.nproc + 1, "sysinfo fork count");
  n = getprocs(rows, NPROC);
  int found = 0;
  for (int i = 0; i < n; i++) if (rows[i].pid == child && rows[i].ppid == getpid()) found = 1;
  check(found, "getprocs correct parent pid");
  close(control[1]); check(wait(0) == child, "reap counted child");
  check(sysinfo(&after) == 0 && after.nproc == before.nproc, "sysinfo after wait");

  child = fork(); check(child >= 0, "zombie count fork");
  if (child == 0) exit(0);
  found = 0;
  for (int attempt = 0; attempt < 100 && !found; attempt++) {
    n = getprocs(rows, NPROC);
    for (int i = 0; i < n; i++)
      if (rows[i].pid == child && rows[i].state == 5) found = 1;
    if (!found) pause(1);
  }
  check(found, "getprocs includes zombie before wait");
  check(sysinfo(&after) == 0 && after.nproc == before.nproc + 1, "sysinfo counts zombie before wait");
  check(wait(0) == child, "reap zombie");

  child = fork(); check(child >= 0, "stress fork");
  if (child == 0) {
    for (int i = 0; i < 80; i++) {
      int pid = fork();
      if (pid < 0) exit(1);
      if (pid == 0) exit(0);
      if (wait(0) != pid) exit(1);
    }
    exit(0);
  }
  for (int i = 0; i < 500; i++) {
    check(getprocs(rows, NPROC) >= 1, "getprocs concurrent fork and exit");
    check(sysinfo(&after) == 0, "sysinfo concurrent fork and exit");
  }
  int status; check(wait(&status) == child && status == 0, "stress completes without lock inversion");
}

static void
tracing(void)
{
  int mask = (1 << SYS_fork) | (1 << SYS_exec) | (1 << SYS_getpid) |
             (1 << SYS_open) | (1 << SYS_trace) | (1 << SYS_kill) | (1 << SYS_exit);
  trace(mask);
  getpid();
  kill(-123);
  open((char *)-1ULL, O_RDONLY);
  int pid = fork();
  if (pid == 0) {getpid(); exit(0);}
  wait(0);
  char *args[] = {"echo", "TRACE_EXEC_OK", 0};
  exec("echo", args);
  exit(1);
}

static void
tracepaths(void)
{
  trace((1 << SYS_open) | (1 << SYS_mkdir) | (1 << SYS_chdir) |
        (1 << SYS_mknod) | (1 << SYS_link) | (1 << SYS_unlink) |
        (1 << SYS_exec) | (1 << SYS_getpid) | (1 << SYS_uptime) |
        (1 << SYS_sync) | (1 << SYS_trace));
  check(mkdir("a1trace") == 0, "trace mkdir");
  check(chdir("a1trace") == 0, "trace chdir");
  check(mknod("node", 1, 1) == 0, "trace mknod");
  int fd = open("file", O_CREATE | O_WRONLY);
  check(fd >= 0, "trace open"); close(fd);
  check(link("file", "linked") == 0, "trace link");
  check(unlink("linked") == 0, "trace unlink");
  char *bad = (char *)-1ULL;
  check(chdir(bad) == -1 && mkdir(bad) == -1 && mknod(bad, 1, 1) == -1 &&
        link(bad, "bad") == -1 && unlink(bad) == -1, "trace bad path pointers");
  char *args[] = {"echo", 0};
  check(exec(bad, args) == -1, "trace invalid exec path");
  getpid(); uptime(); sync();
  trace(0);
  getpid();
  printf("TRACE_OFF_OK\n");
  exit(0);
}

int
main(int argc, char **argv)
{
  if (argc > 1 && !strcmp(argv[1], "args")) {
    printf("argc=%d", argc);
    for (int i = 2; i < argc; i++) printf(" %s", argv[i]);
    printf("\n"); exit(0);
  }
  if (argc > 1 && !strcmp(argv[1], "ret7")) {printf("ret7\n");exit(7);}
  if (argc > 1 && !strcmp(argv[1], "trace")) {tracing(); exit(1);}
  if (argc > 1 && !strcmp(argv[1], "tracepaths")) {tracepaths(); exit(1);}
  utilities();
  information();
  printf("A1 TESTS PASSED (%d checks)\n", checks);
  exit(0);
}
