#ifndef _INCLUDE_MISTRAL_PAGETABLE_H_
#define _INCLUDE_MISTRAL_PAGETABLE_H_


#include "loader/pagetable.h"


/* A mapping in the pagetable */
struct mapping
{
	vaddr_t  vaddr;       /* map this virtual address */
	paddr_t  paddr;       /*  on this physical address */
	size_t   size;        /* for this amount of bytes */
};

/* An abstract pagetable */
struct pagetable
{
	struct mapping  *maps;      /* mapping to ensure */
	size_t           size;      /* amount of mapping */
	size_t           capacity;  /* size of the maps array */
	char            *buffer;    /* internal buffer for concrete table */
	size_t           bufcap;    /* internal buffer capacity */
	size_t           bufsize;   /* internal buffer usage */
};


/*
 * Initialize a new pagetable structure.
 * Such an initialized structure should be finalized later to avoid memory
 * leak.
 * Return 0 on success.
 */
int initialize_pagetable(struct pagetable *self);

/*
 * Finalize a pagetable structure.
 * Release all the resources acquired by the structure. The structure must has
 * been initialized before.
 */
void finalize_pagetable(struct pagetable *self);

/*
 * Add a mapping to the pagetable.
 * The pagetable must has been initialized before. The mapping structure is
 * deeply copied and can be freed after this function return.
 * The vaddr, paddr and size fields must be multiples of PAGE_SIZE, and size
 * must be greater than 0.
 * Return 0 if the new mapping does not overlap with any previous mapping, -1
 * in case of failure or if it overlaps.
 */
int update_pagetable(struct pagetable *self, const struct mapping *map);

/*
 * Write the page table as it should be dumped in memory to be used.
 * The table is written starting at specified address (must be aligned on
 * PAGE_SIZE).
 * Return a pointer to an internal buffer containing the concrete page table.
 * This buffer is released by finalize_pagetable() and may be released by
 * subsequent calls to materialize_pagetable().
 * If len is not NULL, it is filled with the buffer size.
 * Return NULL in case of failure.
 */
void *materialize_pagetable(struct pagetable *self, paddr_t at, size_t *len);


#endif
