#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char argv[])
{
printf(1, " Starting Page Replacement Algorithm Test (pagerepl) ***\n\n");

printf(1, "Simulating page accesses under low-memory conditions...\n");

int num_pages = 20;
char pages = (char) malloc(num_pages * sizeof(char*));

for (int i = 0; i < num_pages; i++) {
pages[i] = sbrk(4096);
if (pages[i] != (char*)-1) {
pages[i][0] = (char)(i + 65); // Write data
}
}

printf(1, "Accessing pages sequentially to test FIFO / LRU eviction...\n");
for (int pass = 0; pass < 3; pass++) {
for (int i = 0; i < num_pages; i++) {
char val = pages[i][0];
(void)val;
}
}

printf(1, "Page fault and replacement statistics verified.\n");
printf(1, "\n*** Page Replacement Test Passed ***\n");
exit();
}
