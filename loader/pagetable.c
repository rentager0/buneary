#include <loader/control.h>
#include <loader/loader.h>
#include <loader/pagetable.h>
#include <loader/string.h>
#include <loader/types.h>


static size_t pte_index_at_level(vaddr_t vaddr, int lvl)
{
	if (lvl > 0) {
		vaddr >>= PAGE_SHIFT;
		lvl--;
	}

	while (lvl > 0) {
		vaddr >>= PGD_SHIFT;
		lvl--;
	}

	return (size_t) (vaddr & (PGD_SIZE - 1));
}

static bool_t setup_vpage(paddr_t table, vaddr_t vaddr, paddr_t paddr)
{
	uint8_t lvl = PAGE_LEVEL;
	pte_t *pgd = (pte_t *) table;
	paddr_t p;
	size_t index;

	while (lvl > 1) {
		index = pte_index_at_level(vaddr, lvl);

		if (pgd[index] == 0) {
			p = (paddr_t) malloc_persistant(PAGE_SIZE, PAGE_SIZE);
			if (p == 0)
				return FALSE;
			memset((void *) p, 0, PAGE_SIZE);
			pgd[index] = p | PTE_PWT | PTE_W | PTE_P;
		}

		pgd = (pte_t *) ((uint32_t) (pgd[index] & PTE_ADDR));
		lvl--;
	}

	index = pte_index_at_level(vaddr, lvl);
	pgd[index] = paddr | PTE_PWT | PTE_W | PTE_P;
	return TRUE;
}

bool_t pagetable_install(void)
{
	paddr_t pgd = (paddr_t) malloc_persistant(PAGE_SIZE, PAGE_SIZE);

	if (pgd == 0)
		return FALSE;
	
	memset((void *) pgd, 0, PAGE_SIZE);
	load_cr3(pgd);
	return TRUE;
}

bool_t pagetable_map(vaddr_t vaddr, paddr_t paddr, size_t size)
{
	size_t i;
	paddr_t table = store_cr3();
	bool_t ret = TRUE;

	if (table == 0)
		return FALSE;
	if (table & (PAGE_SIZE - 1))
		return FALSE;
	if (vaddr & (PAGE_SIZE - 1))
		return FALSE;
	if (paddr & (PAGE_SIZE - 1))
		return FALSE;
	if (size == 0)
		return FALSE;

	size = PAGE_ALIGN(size);

	for (i = 0; i < size; i += PAGE_SIZE) {
		ret = setup_vpage(table, vaddr + i, paddr + i);
		if (!ret)
			break;
	}
	
	return ret;
}
