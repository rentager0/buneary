#ifndef _INCLUDE_LOADER_ALLOCATOR_H_
#define _INCLUDE_LOADER_ALLOCATOR_H_


#include <loader/types.h>


struct allocator
{
	paddr_t  from;        /* first address of permanent data */
	paddr_t  free;        /* first address of free memory */
	paddr_t  to;          /* first address after the free memory */
};


void allocator_initialize(struct allocator *self, paddr_t from, paddr_t to);

paddr_t allocate_transient(struct allocator *self, size_t size, size_t align);

paddr_t allocate_permanent(struct allocator *self, size_t size, size_t align);

void reserve_zone(struct allocator *self, paddr_t from, paddr_t to);


#endif
