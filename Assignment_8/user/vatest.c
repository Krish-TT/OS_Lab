#include "kernel/types.h"
#include "user/user.h"

int global_value;

static void
show(const char *name, void *address)
{
    printf("%s: va=%p pa=0x%x\n", name, address,
      va2pa((int)(uint64)address));
}

int
main(void)
{
  int local_value = 0;
  void *heap_value = malloc(1);
  int pid;

  show("global", &global_value);
  show("local", &local_value);
  show("heap", heap_value);
  printf("invalid: pa=%d\n", va2pa(0x7fffffff));

  pid = fork();
  if (pid == 0) {
    show("child global", &global_value);
    exit(0);
  }
  if (pid > 0)
    show("parent global", &global_value);
  wait(0);
  exit(0);
}