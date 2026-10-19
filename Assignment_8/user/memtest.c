#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  struct meminfo info;

  if (getmeminfo(&info) < 0) {
    printf("getmeminfo failed\n");
    exit(1);
  }
  printf("total=%d free=%d user=%d page_tables=%d\n",
         info.total_pages, info.free_pages, info.user_pages,
         info.page_table_pages);
  exit(0);
}