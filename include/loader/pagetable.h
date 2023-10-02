#ifndef _INCLUDE_LOADER_PAGETABLE_H_
#define _INCLUDE_LOADER_PAGETABLE_H_


#include <loader/types.h>


#define PTE_P     (1 << 0)
#define PTE_W     (1 << 1)
#define PTE_U     (1 << 2)
#define PTE_PWT   (1 << 3)
#define PTE_PCD   (1 << 4)
#define PTE_A     (1 << 5)
#define PTE_D     (1 << 6)
#define PTE_PAT   (1 << 7)
#define PTE_G     (1 << 8)
#define PTE_ADDR  (((1ull << 52) - 1) & ~((1 << 12) - 1))

#define PAGE_SHIFT 12
#define PTE_SHIFT  3
#define PTE_SIZE   (1 << PTE_SHIFT)
#define PAGE_SIZE  (1 << PAGE_SHIFT)
#define PGD_SHIFT  (PAGE_SHIFT - PTE_SHIFT)
#define PGD_SIZE   (PAGE_SIZE / PTE_SIZE)

#define PAGE_LEVEL 4
	

typedef uint64_t    pte_t;


bool_t pagetable_install(void);

bool_t pagetable_map(vaddr_t vaddr, paddr_t paddr, size_t size);




/*
 * Add a virtual address region of size bytes to the dest page table which
 * already has a size of ps bytes.
 * Return the amount of bytes of the updated page table, or 0 if the update
 * has failed.
 * Every argument except size must be a multiple of PAGE_SIZE. If size is not
 * a multiple of PAGE_SIZE, it is rounded up. The given size must be non null.
 */

size_t setup_pagetable(paddr_t table, size_t tsize, vaddr_t vaddr,
		       paddr_t paddr, size_t size);


#define PAGE_ALIGN(addr)				\
	((((addr) & (PAGE_SIZE - 1)) == 0)		\
	 ? (addr)					\
	 : (((addr) & ~(PAGE_SIZE - 1)) + PAGE_SIZE))


#endif
