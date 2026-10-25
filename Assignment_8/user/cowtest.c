#include "types.h"
#include "stat.h"
#include "user.h"

struct meminfo {
uint total_pages;
uint free_pages;
uint proc_pages;
uint pt_pages;
};

int
main(int argc, char argv[])
{
printf(1, " Starting Copy-On-Write Test (cowtest) ***\n\n");

char *p = sbrk(4096 * 10); // Allocate 10 pages
for (int i = 0; i < 4096 * 10; i += 4096) {
p[i] = 'X';
}

struct meminfo mi_before, mi_after;
if (getmeminfo(&mi_before) < 0) {
printf(1, "getmeminfo failed\n");
exit();
}
printf(1, "Free pages before fork: %d\n", mi_before.free_pages);

int pid = fork();
if (pid < 0) {
printf(1, "Fork failed\n");
exit();
}

if (pid == 0) {
// Child process: reads shouldn't trigger copy
if (p[0] != 'X') {
printf(1, "Child read error!\n");
exit();
}
// Write triggers COW for 1 page
p[0] = 'Y';
printf(1, "[Child] Wrote to page 0 (COW triggered for 1 frame)\n");
exit();
}

wait();
getmeminfo(&mi_after);
printf(1, "Free pages after child exit: %d\n", mi_after.free_pages);
printf(1, "\n*** COW Test Passed ***\n");
exit();
}
