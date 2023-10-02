#ifndef _INCLUDE_LOADER_CPUID_H_
#define _INCLUDE_LOADER_CPUID_H_


#include <loader/types.h>


#define CPUID_FUNCTION_VENDOR   0x00000000
#define CPUID_VENDOR_INTEL      "GenuineIntel"
#define CPUID_VENDOR_AMD        "AuthenticAMD"


static inline void cpuid(uint32_t *eax, uint32_t *ecx, uint32_t *edx,
			 uint32_t *ebx)
{
	asm volatile ("cpuid" : "+a" (*eax), "=c" (*ecx), "=d" (*edx),
		      "=b" (*ebx) : "a" (*eax));
}


typedef char vendor_t[13];

static inline void cpuid_vendor(vendor_t *dest)
{
	uint32_t *__dest = (uint32_t *) dest;
	uint32_t eax = CPUID_FUNCTION_VENDOR;

	cpuid(&eax, &__dest[2], &__dest[1], &__dest[0]);
	(*dest)[12] = 0;
}


#endif
