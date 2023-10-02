#ifndef _INCLUDE_LOADER_CONTROL_H_
#define _INCLUDE_LOADER_CONTROL_H_


#ifndef __ASSEMBLER__
#  include <loader/types.h>
#endif


#define CR0_PE           (1 <<  0)
#define CR0_MP           (1 <<  1)
#define CR0_EM           (1 <<  2)
#define CR0_TS           (1 <<  3)
#define CR0_ET           (1 <<  4)
#define CR0_NE           (1 <<  5)
#define CR0_WP           (1 << 16)
#define CR0_AM           (1 << 18)
#define CR0_NW           (1 << 29)
#define CR0_CD           (1 << 30)
#define CR0_PG           (1 << 31)

#define CR4_VME          (1 <<  0)
#define CR4_PVI          (1 <<  1)
#define CR4_TSD          (1 <<  2)
#define CR4_DE           (1 <<  3)
#define CR4_PSE          (1 <<  4)
#define CR4_PAE          (1 <<  5)
#define CR4_MCE          (1 <<  6)
#define CR4_PGE          (1 <<  7)
#define CR4_PCE          (1 <<  8)
#define CR4_OSFXSR       (1 <<  9)
#define CR4_OSXMMEXCPT   (1 << 10)
#define CR4_FSGSBASE     (1 << 16)
#define CR4_OSXSAVE      (1 << 18)

#define EFER             0xc0000080
#define EFER_SCE         (1 <<  0)
#define EFER_LME         (1 <<  8)
#define EFER_LMA         (1 << 10)
#define EFER_NXE         (1 << 11)
#define EFER_SVME        (1 << 12)
#define EFER_LMSLE       (1 << 13)
#define EFER_FFXSR       (1 << 14)
#define EFER_TCE         (1 << 15)

#define STAR             0xc0000081



#ifndef __ASSEMBLER__

static inline void load_cr0(uint32_t cr0)
{
	asm volatile ("movl %0, %%cr0" : : "r" (cr0) : "memory");
}

static inline uint32_t store_cr0(void)
{
	uint32_t cr0;
	asm volatile ("movl %%cr0, %0" : "=r" (cr0));
	return cr0;
}


static inline void load_cr3(uint32_t cr3)
{
	asm volatile ("movl %0, %%cr3" : : "r" (cr3) : "memory");
}

static inline uint32_t store_cr3(void)
{
	uint32_t cr3;
	asm volatile ("movl %%cr3, %0" : "=r" (cr3));
	return cr3;
}

#endif


#endif
