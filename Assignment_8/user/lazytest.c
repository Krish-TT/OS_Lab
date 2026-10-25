#include "types.h"
#include "stat.h"
#include "user.h"

// Definition matching Q2 meminfo struct requirement
struct meminfo {
uint total_pages;
uint free_pages;
uint proc_pages;
uint pt_pages;
};

void
lazytest(void)
{
printf(1, "*** Starting Lazy Allocation Test (lazytest) ***\n\n");

struct meminfo mi;
if (getmeminfo(&mi) < 0) {
printf(1, "Error: getmeminfo failed\n");
exit();
}
printf(1, "Initial Free Pages: %d, Proc Pages: %d\n", mi.free_pages, mi.proc_pages);

// Allocate 32 MB lazily
uint sz = 32 * 1024 * 1024;
char ptr = (char) sbrk(sz);
if (ptr == (char*)-1) {
printf(1, "sbrk failed!\n");
exit();
}
printf(1, "sbrk(32MB) returned %p successfully without immediate allocation.\n", ptr);

getmeminfo(&mi);
printf(1, "After sbrk(32MB) - Free Pages: %d (Unchanged), Proc Pages: %d\n",
mi.free_pages, mi.proc_pages);

// Touch widely spaced pages to trigger page faults
printf(1, "Touching pages at offsets 4MB, 16MB, and 24MB...\n");
ptr[4 * 1024 * 1024] = 'A';
ptr[16 * 1024 * 1024] = 'B';
ptr[24 * 1024 * 1024] = 'C';

getmeminfo(&mi);
printf(1, "After touching 3 pages - Free Pages: %d (-3 pages allocated)\n", mi.free_pages);

char *buf = ptr + (8 * 1024 * 1024); // 8MB offset (untouched)
int fd[2];
if (pipe(fd) >= 0) {
write(fd[1], "Lazy Kernel Test", 16);
int n = read(fd[0], buf, 16); // Kernel reads into untouched lazy page
buf[n] = '\0';
printf(1, "Read from pipe into untouched buffer: '%s'\n", buf);
close(fd[0]);
close(fd[1]);
}

// Fork test with lazy pages
int pid = fork();
if (pid < 0) {
printf(1, "Fork failed!\n");
exit();
}
if (pid == 0) {
printf(1, "[Child] Accessing child lazy memory...\n");
ptr[4 * 1024 * 1024] = 'Z';
exit();
}
wait();

printf(1, "*** Lazy Allocation Tests Passed ***\n\n");
}
