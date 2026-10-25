# Operating Systems Lab Assignment 8 Report

---

## 1. Question 1: Virtual to Physical Address Translation (`va2pa`)

### 1.1 Design Decisions & Implementation
- **System Call `va2pa(int va)`:** Wraps kernel page table walking logic. It extracts `PDX(va)` and `PTX(va)` to traverse two levels of page tables starting from `myproc()->pgdir`.
- **Validation:** Returns `-1` if any intermediate page directory/table entry is absent (`!(*pte & PTE_P)`) or if user permissions are not granted (`!(*pte & PTE_U)`).
- **Kernel Printing:** Added `cprintf` output in the kernel displaying PDE index, PTE index, offset (`va & 0xFFF`), and individual flag bits (`P`, `W`, `U`).

### 1.2 Results & Verification (`vatest.c`)
- Global, stack, and heap variables yielded valid physical addresses.
- Address `0x7FFFFFFF` successfully returned `-1`.
- Calling `fork()` verified that parent and child share identical virtual addresses for global variables but map to distinct physical addresses due to xv6 memory duplication.

---

## 2. Question 2: Physical Memory Statistics (`getmeminfo`)

### 2.1 Design Decisions & Data Structures
- **Free Page Tracker:** Added global variable `free_pages_count` in `kalloc.c`. Incremented inside `kfree()` and decremented inside `kalloc()` while holding `kmem.lock`.
- **System Call `getmeminfo(struct meminfo *mi)`:** Populates a structure containing:
  - `total_pages`: Total frames managed by `kalloc.c` (`PHYSTOP / PGSIZE`).
  - `free_pages`: Current value of `free_pages_count`.
  - `proc_pages`: `myproc()->sz / PGSIZE`.
  - `pt_pages`: Number of allocated page table pages (traversed via process directory).

### 2.2 Memory Verification (`memtest.c`)
- **Initial:** `Free Pages = 5580`.
- **After `sbrk(16384)` (4 pages):** `Free Pages = 5576` (decreased by exactly 4).
- **After `fork()` and child completion:** `Free Pages` returned back to `5576`, confirming no memory leaks in page creation or cleanup.

---

## 3. Question 3: Page Table Inspection (`vmprint`)

### 3.1 Design Decisions & Implementation
- **System Call `vmprint(void)`:** Calls a helper function that walks page directory entries (PDEs) below `KERNBASE`.
- **Formatting:** Prints valid PDEs and nested valid PTEs using indentation. Reports PDE/PTE index, physical addresses, and flag state (`PTE_P`, `PTE_W`, `PTE_U`).
- **Kernel Summary:** Prints a count of kernel-level mappings present above `KERNBASE`.

### 3.2 Stack Guard & Memory Check (`vmprinttest.c`)
- **Guard Page Identification:** Identified index 2 as the stack guard page created by `exec()`. It has `PTE_P` set but `PTE_U` cleared to trigger a fault on stack overflow.
- **User Page Verification:** Verified that user-accessible pages equal `(sz / PGSIZE) - 1`.

---

## 4. Question 4: Lazy Memory Allocation

### 4.1 Design Decisions & Implementation
- **De-allocation in `sys_sbrk()`:** Modified `sys_sbrk()` so that for positive allocation `$n > 0$`, it only increases `myproc()->sz` without calling `growproc()`. Memory shrinking ($n < 0$) continues to deallocate pages immediately via `growproc()`.
- **Page Fault Handler (`trap.c`):** Handles `T_PGFLT` (trap 14). Retrieves faulting address using `rcr2()`.
  - Checks if address lies between the stack guard page and `myproc()->sz`.
  - Allocates frame via `kalloc()`, zeros memory, and maps it using `mappages()` with flags `PTE_P | PTE_W | PTE_U`.
  - Terminates process if access is invalid ($va \ge sz$, access to guard page, or `kalloc()` fails).
- **Edge Cases & Kernel System Calls:** Updated `copyuvm()` during `fork()` to skip unmapped pages rather than triggering a kernel panic. Handled kernel-level access to untouched lazy buffers in system calls like `read()`.

### 4.2 Memory Usage Summary (Before vs. After)

| Test State | Requested Size | Free Pages | Process User Pages (`sz/PGSIZE`) | Allocated Frames |
| :--- | :--- | :--- | :--- | :--- |
| **Initial** | Default | 5580 | 4 | 4 |
| **After `sbrk(32MB)`** | 32 MB | 5580 | 8196 | 4 (Lazy deferred) |
| **After touching 3 pages** | 32 MB | 5577 | 8196 | 7 |
| **Child Exit (Reaped)** | 32 MB | 5577 | 8196 | 7 |

---
