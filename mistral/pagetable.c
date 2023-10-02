#include "mistral/pagetable.h"

#include <string.h>

#define PAGETABLE_MAPS_INITIAL_CAPACITY 32


int initialize_pagetable(struct pagetable *self)
{
	self->capacity = PAGETABLE_MAPS_INITIAL_CAPACITY;
	self->maps = malloc(sizeof (struct mapping) * self->capacity);
	self->size = 0;

	self->buffer = NULL;
	self->bufcap = 0;
	self->bufsize = 0;
	
	return 0;
}

void finalize_pagetable(struct pagetable *self)
{
	free(self->maps);
	free(self->buffer);
}

static int append_mapping(struct pagetable *self, const struct mapping *map)
{
	struct mapping *na;
	size_t nc;

	if (self->size == self->capacity) {
		nc = self->capacity << 1;
		na = realloc(self->maps, sizeof (struct mapping) * nc);

		if (na == NULL)
			return -1;

		self->maps = na;
		self->capacity = nc;
	}

	self->maps[self->size++] = *map;
	
	return 0;
}

int update_pagetable(struct pagetable *self, const struct mapping *map)
{
	vaddr_t istart, nstart = map->vaddr;
	vaddr_t iend, nend = map->vaddr + map->size;
	size_t i;

	if ((map->vaddr & (PAGE_SIZE - 1)) != 0)
		return -1;
	if ((map->paddr & (PAGE_SIZE - 1)) != 0)
		return -1;
	if ((map->size & (PAGE_SIZE - 1)) != 0)
		return -1;
	if (map->size == 0)
		return -1;

	for (i = 0; i < self->size; i++) {
		istart = self->maps[i].vaddr;
		iend = istart + self->maps[i].size;

		if (istart <= nstart && iend >= nstart)
			return -1;
		if (iend >= nend && istart <= nend)
			return -1;
	}
	
	return append_mapping(self, map);
}


static size_t compute_concrete_size(const struct pagetable *self)
{
	size_t upper = 0;
	size_t i, j, tmp;

	for (i = 0; i < self->size; i++) {
		tmp = self->maps[i].size * PTE_SIZE / PAGE_SIZE;
		if ((tmp & (PAGE_SIZE - 1)) != 0) {
			tmp &= ~(PAGE_SIZE - 1);
			tmp +=   PAGE_SIZE;
		}

		upper += tmp;

		for (j = 0; j < 3; j++) {
			tmp /= PGD_SIZE;
			if ((tmp & (PAGE_SIZE - 1)) != 0) {
				tmp &= ~(PAGE_SIZE - 1);
				tmp +=   PAGE_SIZE;
			}

			upper += tmp;
		}

	}

	return upper;
}

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

static void fill_concrete_page(struct pagetable *self, paddr_t at,
			       vaddr_t vaddr, paddr_t paddr)
{
	int lvl = PAGE_LEVEL;
	pte_t *pgd = (pte_t *) self->buffer;
	size_t index;
	paddr_t pgdaddr;

	while (lvl > 1) {
		index = pte_index_at_level(vaddr, lvl);

		if (pgd[index] == 0) {
			pgdaddr = self->bufsize + at;
			pgd[index] = pgdaddr | PTE_W | PTE_PWT | PTE_P;
			self->bufsize += PAGE_SIZE;
		}

		pgd = (pte_t *) ((char *) self->buffer
				 + (pgd[index] & PTE_ADDR)
				 - at);

		lvl--;
	}

	index = pte_index_at_level(vaddr, lvl);
	pgd[index] = paddr | PTE_W | PTE_P;
}

static void fill_concrete_buffer(struct pagetable *self, paddr_t at,
				 const struct mapping *map)
{
	size_t i;

	for (i = 0; i < map->size; i += PAGE_SIZE)
		fill_concrete_page(self, at, map->vaddr + i, map->paddr + i);
}

void *materialize_pagetable(struct pagetable *self, paddr_t at, size_t *len)
{
	void *ret = NULL;
	size_t i, upper;
	char *buf;
	
	if ((at & (PAGE_SIZE - 1)) != 0)
		goto out;

	upper = compute_concrete_size(self);
	
	if (upper > self->bufcap) {
		buf = realloc(self->buffer, upper);
		if (buf == NULL)
			goto out;

		self->buffer = buf;
		self->capacity = upper;
	} else {
		buf = self->buffer;
	}

	memset(buf, 0, upper);
	self->bufsize = PAGE_SIZE;

	for (i = 0; i < self->size; i++)
		fill_concrete_buffer(self, at, &self->maps[i]);

	if (len != NULL)
		*len = self->bufsize;
	ret = self->buffer;
 out:
	return ret;
}
