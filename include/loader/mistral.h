#ifndef _INCLUDE_LOADER_MISTRAL_H_
#define _INCLUDE_LOADER_MISTRAL_H_


#include <loader/gcc.h>
#include <loader/pagetable.h>
#include <loader/types.h>


#define LOADER_STACK_ALIGN(addr)  PAGE_ALIGN(addr)
#define LOADER_STACK_SIZE         PAGE_SIZE


#define MISTRAL_SEARCH   32768          /* Search limit in binary loader */
#define MISTRAL_ALIGN    8              /* Expected header alignment */
#define MISTRAL_MAGIC    0x10ad64b5     /* Load 64 Bits System */


struct mistral_header
{
	uint32_t  magic;                /* mistral magic number */
	paddr_t   payload_addr;         /* address of the payload */
	paddr_t   load_addr;            /* address to load payload at */
	paddr_t   load_end_addr;        /* address to stop payload */
	paddr_t   bss_end_addr;         /* last addresss to initialize bss */
	paddr_t   payload_entry_addr;   /* address to branch on */
	vaddr_t   payload_vaddr;        /* virtual address of the payload */
	paddr_t   stack_addr;           /* address to place the loader stack */
} __packed __attribute__ ((aligned(MISTRAL_ALIGN)));


#define MISTRAL_HEADER					\
	((struct mistral_header) {			\
		.magic              = MISTRAL_MAGIC,	\
		.payload_addr       = 0,          	\
		.load_addr          = 0,           	\
		.load_end_addr      = 0,            	\
		.bss_end_addr       = 0,          	\
		.payload_entry_addr = 0,	        \
		.payload_vaddr      = 0,	        \
		.stack_addr         = 0           	\
	})


extern struct mistral_header mistral_header;


#endif
