#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char argv[])
{
printf(1, " Starting Shared Memory Test (shmtest) ***\n\n");

int shm_id = 42;
char shm_ptr = (char) shmget(shm_id, 4096);
if (shm_ptr == (char*)-1 || shm_ptr == 0) {
printf(1, "shmget failed\n");
exit();
}

int pid = fork();
if (pid < 0) {
printf(1, "fork failed\n");
exit();
}

if (pid == 0) {
// Attach shared region in child
char child_shm = (char) shmget(shm_id, 4096);
printf(1, "[Child] Reading shared msg: '%s'\n", child_shm);
strcpy(child_shm, "Hello from Child!");
shmdt(child_shm);
exit();
}

strcpy(shm_ptr, "Hello from Parent!");
wait();

printf(1, "[Parent] Read updated shared msg: '%s'\n", shm_ptr);
shmdt(shm_ptr);

printf(1, "\n*** Shared Memory Test Passed ***\n");
exit();
}
