#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  char *pages;

  printf("before allocation:\n");
  memstats();
  pages = sbrk(3 * 4096);
  if (pages == (char *)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  for (int i = 0; i < 3; i++)
    pages[i * 4096] = 1;
  printf("after allocation:\n");
  memstats();
  if (sbrk(-3 * 4096) == (char *)-1) {
    printf("release failed\n");
    exit(1);
  }
  printf("after release:\n");
  memstats();
  exit(0);
}