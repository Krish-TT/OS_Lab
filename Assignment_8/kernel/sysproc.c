#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

struct meminfo {
  int total_pages;
  int free_pages;
  int user_pages;
  int page_table_pages;
};

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_va2pa(void)
{
  int va_arg;
  uint64 va;
  uint64 pa;
  pte_t *pte;
  struct proc *p = myproc();

  argint(0, &va_arg);
  va = (uint64)(uint32)va_arg;
  pa = va2pa(p->pagetable, va);
  if (pa == 0)
    return -1;

  pte = walk(p->pagetable, va, 0);
  printk("va2pa: va=%p pa=%p pd=%d pt=%d off=%d flags=P%d W%d U%d\n",
      (void *)va, (void *)pa, (int)PX(2, va), (int)PX(1, va),
      (int)(va % PGSIZE),
         (*pte & PTE_V) != 0, (*pte & PTE_W) != 0, (*pte & PTE_U) != 0);
  return pa;
}

uint64
sys_getmeminfo(void)
{
  uint64 addr;
  struct meminfo info;
  struct proc *p = myproc();

  argaddr(0, &addr);
  kalloc_stats(&info.total_pages, &info.free_pages);
  info.user_pages = PGROUNDUP(p->sz) / PGSIZE;
  info.user_pages = countuserpages(p->pagetable);
  info.page_table_pages = countpagetablepages(p->pagetable);
  if (copyout(p->pagetable, p->sz, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}
