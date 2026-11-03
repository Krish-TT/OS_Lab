#include "kernel/types.h"
#include "user/user.h"

static void
show(const char *name, void *address)
{
  printf("%s: va=%p pa=0x%x\n", name, address,
      (uint)vtop((int)(uint64)address));
}

int
main(void)
{
  int stack_value = 1;
  void *heap_value = malloc(1);

  show("stack", &stack_value);
  show("heap", heap_value);
  printf("invalid: pa=%d\n", vtop(0x7fffffff));
  free(heap_value);
  exit(0);
}
