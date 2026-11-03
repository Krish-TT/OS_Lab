# Operating System Assignment 10 Report

## Introduction

In this assignment, I added several memory-management features which
show how physical memory is being used, translate virtual
addresses into physical addresses, change page permissions, and compare three
page-replacement algorithms.

I also added small user programs to test each feature. All programs were
compiled and tested in xv6 using QEMU.

## 1. Memory Statistics

Modified `kernel/kalloc.c` so that xv6 keeps track of physical pages. The
allocator now counts:

- the total number of physical pages,
- the number of pages allocated,
- the number of pages freed, and
- the number of pages currently available.

These counters are updated whenever `kalloc()` or `kfree()` is called. The
existing allocator lock protects the counters from concurrent access.

Added a `memstats()` system call that prints these values. The
`user/memtest.c` program first displays the statistics, allocates three pages
using `sbrk()`, displays the statistics again, and finally releases the pages.
The output showed that the number of free pages decreased after allocation and
returned to its earlier value after the pages were released.

## 2. Virtual-to-Physical Address Translation

Added a `vtop()` system call to translate a virtual address into its
physical address. The kernel checks the page table entry before doing the
translation. If the address is valid and mapped for the user process, the
physical frame address is combined with the offset within the page:

`physical address = physical frame address + page offset`

If the address is invalid or unmapped, the system call returns `-1`.

The `user/vttest.c` program checks addresses on the stack and heap. It also
checks an invalid address. The stack and heap addresses returned valid
physical addresses, while the invalid address returned `-1`.

## 3. Page Information and Read-Only Pages

I added two system calls for checking and changing page permissions:

- `pageinfo(virtual_address)` prints the physical address and the important
  page-table flags.
- `setreadonly(virtual_address)` removes the write permission from a page.

After changing the page-table entry, the kernel flushes the TLB so that the
new permission takes effect immediately.

The `user/pagetest.c` program allocates a page and prints its permissions. At
first, the output showed:

`P1 W1 U1`

After calling `setreadonly()`, it showed:

`P1 W0 U1`

This means the page was still valid and accessible by the user, but it was no
longer writable. When the program tried to write to the page, xv6 generated
the expected page fault.

## 4. Page-Replacement Algorithms

The `user/pagereplace.c` program compares FIFO, LRU, and Optimal page
replacement. The same reference string was used in every test:

`7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`

There are 20 page references in total. The program prints the frame contents
after every reference and reports whether the reference was a hit or a fault.
It then repeats the test with 3, 4, and 5 frames.

| Frames | Algorithm | Page Faults | Page Hits | Hit Ratio | Page Fault Ratio |
|---:|---|---:|---:|---:|---:|
| 3 | FIFO | 15 | 5 | 25% | 75% |
| 3 | LRU | 12 | 8 | 40% | 60% |
| 3 | Optimal | 9 | 11 | 55% | 45% |
| 4 | FIFO | 10 | 10 | 50% | 50% |
| 4 | LRU | 8 | 12 | 60% | 40% |
| 4 | Optimal | 8 | 12 | 60% | 40% |
| 5 | FIFO | 9 | 11 | 55% | 45% |
| 5 | LRU | 7 | 13 | 65% | 35% |
| 5 | Optimal | 7 | 13 | 65% | 35% |

### Algorithms

- **FIFO:** Replaces the page that has been in memory the longest.
- **LRU:** Replaces the page that has not been used for the longest time.
- **Optimal:** Replaces the page whose next use is farthest in the future.

Optimal gives the best possible result, but it is mainly theoretical because
an operating system cannot know the future page references. LRU is more
practical because it uses the recent history of page accesses.

## 5. Observations and Conclusion

Adding more frames generally reduced the number of page faults and improved
the hit ratio. FIFO gave the worst results for this reference string.

With three frames, Optimal performed best with 9 faults. With four frames,
LRU and Optimal both had 8 faults. With five frames, both had 7 faults.
Therefore, Optimal was the best algorithm for the three-frame test, while LRU
matched it for four and five frames. Overall, LRU is the better practical
choice because it performs close to Optimal without needing information about
future references.
