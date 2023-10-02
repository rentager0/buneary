#include <loader/multiboot2.h>


static const struct multiboot2_tag *
check_range(const struct multiboot2_info *mb2,
	    const struct multiboot2_tag *tag)
{
	char *start = (char *) mb2;
	char *end = (char *) tag;

	if (((uint32_t) (end - start)) >= mb2->total_size)
		return NULL;
	return tag;
}

const struct multiboot2_tag *
multiboot2_first(const struct multiboot2_info *mb2)
{
	return check_range(mb2, (const struct multiboot2_tag *) mb2->tags);
}

const struct multiboot2_tag *
multiboot2_first_type(const struct multiboot2_info *mb2, uint32_t type)
{
	const struct multiboot2_tag *tag = multiboot2_first(mb2);

	if (tag == NULL)
		return NULL;
	if (tag->type == type)
		return tag;

	return multiboot2_next_type(mb2, tag, type);
}

const struct multiboot2_tag *
multiboot2_next(const struct multiboot2_info *mb2,
		const struct multiboot2_tag *tag)
{
	size_t len = tag->size;
	paddr_t ptr = ((paddr_t) tag) + len;

	if ((ptr & 0x7) != 0)
		ptr = (ptr & ~0x7) + 0x8;

	return check_range(mb2, (const struct multiboot2_tag *) ptr);
}

const struct multiboot2_tag *
multiboot2_next_type(const struct multiboot2_info *mb2,
		     const struct multiboot2_tag *tag, uint32_t type)
{
	do {
		tag = multiboot2_next(mb2, tag);
	} while (tag != NULL && tag->type != type);

	return tag;
}
