# Operating Systems Assignment 9 Report

## Question 1: Printing the Page Table of a Process

For this question, I added a new system call `sys_vmprint` (syscall 22) and wrote a helper function `vmprint()` inside `vm.c`. The main idea was to walk through the calling process's page directory for virtual addresses below `KERNBASE`, taking inspiration from how `freevm()` scans memory. Using `walkpgdir()`, the function checks every valid entry and prints its virtual address, physical frame address, and the flags (P, W, U).

At the end of the printout, it prints the total count of valid user pages alongside the page-table page count. I made sure that only pages with the `PTE_U` flag enabled are counted as valid user pages. The stack guard page located at `0x1000` is printed, but because `U=0`, it gets treated as a guard page and excluded from the user page total.

To verify everything works, I ran `pttest`. On startup, it printed 2 valid user pages. After calling `sbrk(4 * 4096)`, 4 new user pages were allocated, bringing the total up to 6 pages.

## Question 2: Lazy Memory Allocation

In `sysproc.c`, I modified `sys_sbrk()` so that when `n > 0`, it simply increases the process size `myproc()->sz` by `n` instead of allocating physical memory right away. When `n < 0`, it deallocates memory like normal.

Because pages are no longer allocated immediately, functions that copy or free process memory had to be tweaked. I updated `copyuvm()` in `vm.c` to skip unmapped pages rather than throwing a kernel panic. I also verified that `deallocuvm()` handles non-present page table entries safely without crashing.

Next, I updated `trap.c` to intercept page faults (`T_PGFLT`). Whenever a fault occurs, the handler reads the faulting address using `rcr2()`. If the address is within bounds (`addr < myproc()->sz`), it allocates a clean frame using `kalloc()` and `memset()`, maps it with `mappages()` (using `PTE_W | PTE_U` flags), increments the process's page fault counter, and returns control to the process. If the faulting address is invalid or above `sz`, the process prints an error and gets killed. I also made `mappages()` non-static so it could be called from `trap.c`, and created a simple system call `getfaults()` to return the fault count.

I tested the implementation with `faulttest`. It called `sbrk(32 * 4096)` and accessed 5 distinct pages, which resulted in exactly 5 page faults as expected. I also tested an invalid access beyond `sz` in a child process, which successfully killed only the child process while leaving the parent alive.

## Question 3: FIFO and LRU Page Replacement

I created `pagerepl.c` to simulate how FIFO and LRU algorithms handle page faults given a page reference string and a fixed number of physical frames. The program prints the frame state and whether each access was a hit or a fault after every reference.

Using the reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` with 3 frames:
* **FIFO** resulted in **15 page faults**
* **LRU** resulted in **12 page faults**

I then ran tests comparing 3, 4, and 5 frame allocations to calculate the total faults and hit ratios:

| Frames | FIFO Faults | FIFO Hit Ratio | LRU Faults | LRU Hit Ratio |
|---|---|---|---|---|
| 3 | 15 | 25.00% | 12 | 40.00% |
| 4 | 10 | 50.00% | 8 | 60.00% |
| 5 | 9 | 55.00% | 7 | 65.00% |

The test output matches the expected behavior, showing that LRU consistently performs better than FIFO by maintaining a higher hit ratio across all tested frame sizes.
