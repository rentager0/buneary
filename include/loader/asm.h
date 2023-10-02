#ifndef _INCLUDE_LOADER_ASM_H_
#define _INCLUDE_LOADER_ASM_H_


#include <loader/types.h>


#define offsetof(structure, field)      (&(((structure *) 0)->field))


/*
 * This macro is used to generate some assembly header files containing values
 * which should be known after compile time (such as structure offsets).
 * The idea is to define functions with an bad inline assembly instruction
 * and to only process this file until assembly step, then parse the produced
 * assembly code to extract actual values.
 */

#define ASSEMBLY_DECLARE_VALUE(symbol, value)				\
	void __assembly_delcare_value_ ## symbol (void)			\
	{								\
		asm ("ASSEMBLY_DECLARE_VALUE " #symbol " %0"		\
		     : : "n" (value));					\
	}


static inline uint8_t in8(port_t port)
{
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline uint16_t in16(port_t port)
{
	uint16_t ret;
	asm volatile("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline uint32_t in32(port_t port)
{
	uint32_t ret;
	asm volatile("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}


static inline void out8(port_t port, uint8_t val)
{
	asm volatile("outb %0, %1" : : "a" (val), "dN" (port));
}

static inline void out16(port_t port, uint16_t val)
{
	asm volatile("outw %0, %1" : : "a" (val), "dN" (port));
}

static inline void out32(port_t port, uint32_t val)
{
	asm volatile("outl %0, %1" : : "a" (val), "dN" (port));
}


#endif
