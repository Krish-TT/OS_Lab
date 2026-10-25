#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char argv[])
{
printf(1, " Starting Page Protection Test (protecttest) ***\n\n");

char *page = sbrk(4096);
page[0] = 'A';
printf(1, "Initial write successful: %c\n", page[0]);

// Call system call mprotect(addr, len, prot) -> Read Only (e.g., PROT_READ = 1)
printf(1, "Setting page permission to READ-ONLY...\n");
if (mprotect(page, 4096, 1) < 0) {
printf(1, "mprotect failed or not implemented!\n");
}

int pid = fork();
if (pid == 0) {
// Child attempts illegal write -> should trigger trap and get killed
printf(1, "[Child] Attempting write to Read-Only page...\n");
page[0] = 'B';
printf(1, "ERROR: Read-only protection failed!\n");
exit();
}

wait();
printf(1, "Child process correctly killed by kernel on protection fault.\n");

// Restore Read/Write permissions (PROT_READ | PROT_WRITE = 3)
munprotect(page, 4096);
page[0] = 'C';
printf(1, "Restored WRITE permission successfully: %c\n", page[0]);

printf(1, "\n*** Page Protection Test Passed ***\n");
exit();
}
