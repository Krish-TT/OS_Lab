#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  char *page = sbrk(4096);

  if (page == (char *)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  *page = 'A';
  printf("before readonly:\n");
  pageinfo((int)(uint64)page);
  if (setreadonly((int)(uint64)page) < 0) {
    printf("setreadonly failed\n");
    exit(1);
  }
  printf("after readonly:\n");
  pageinfo((int)(uint64)page);
  printf("writing to readonly page (expected page fault)\n");
  *page = 'B';
  printf("unexpected write success\n");
  exit(0);
}
