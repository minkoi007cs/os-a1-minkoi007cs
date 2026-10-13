// Strict decimal parsing: atoi() silently accepts malformed arguments.
static inline int
a1number(const char *s, int *out)
{
  int n = 0;
  if (*s == 0)
    return -1;
  for (; *s; s++) {
    if (*s < '0' || *s > '9' || n > (2147483647 - (*s - '0')) / 10)
      return -1;
    n = n * 10 + (*s - '0');
  }
  *out = n;
  return 0;
}
