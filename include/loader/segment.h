#ifndef _INCLUDE_LOADER_SEGMENT_H_
#define _INCLUDE_LOADER_SEGMENT_H_


#include <loader/types.h>


typedef uint64_t descriptor_t;


#define DESCRIPTOR(base, limit, flags)				 \
	( (             ((base)  & 0x00ffffff)  << 16)		 \
	| ( ((uint64_t) ((base)  & 0xff000000)) << 32)           \
	| (             ((limit) & 0x0000ffff)  <<  0)           \
        | ( ((uint64_t) ((limit) & 0x000f0000)) << 32)           \
        | ( ((uint64_t) ((flags) & 0x000000ff)) << 40)           \
        | ( ((uint64_t) ((flags) & 0x00000f00)) << 44) )


struct gdtr
{
	uint16_t limit;
	uint32_t address;
} __attribute__ ((packed));

static inline void lgdt(uint32_t table, uint16_t len)
{
	static struct gdtr gdtr __attribute__ ((aligned(16)));

	gdtr.address = table;
	gdtr.limit = (len << 3) - 1;

	asm volatile ("lgdt %0" : : "m" (gdtr) : "memory");
}


typedef uint16_t selector_t;

static inline void load_ds(selector_t sel)
{
	asm volatile ("mov %0, %%ds" : : "a" (sel << 3));
}

static inline void load_es(selector_t sel)
{
	asm volatile ("mov %0, %%es" : : "a" (sel << 3));
}

static inline void load_fs(selector_t sel)
{
	asm volatile ("mov %0, %%fs" : : "a" (sel << 3));
}

static inline void load_gs(selector_t sel)
{
	asm volatile ("mov %0, %%gs" : : "a" (sel << 3));
}

static inline void load_ss(selector_t sel)
{
	asm volatile ("mov %0, %%ss" : : "a" (sel << 3));
}

static inline void load_cs(selector_t sel)
{
	asm volatile ("ljmp %0, $entry\n"
		      "entry:\n"
		      : : "i" (sel << 3));
}



#endif
