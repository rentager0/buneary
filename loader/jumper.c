#include <loader/allocator.h>
#include <loader/control.h>
#include <loader/gcc.h>
#include <loader/jumper.h>
#include <loader/loader.h>
#include <loader/mistral.h>
#include <loader/segment.h>
#include <loader/string.h>


/*
 * Jumper code
 *
 * The final step before to jump to the payload in long mode is to relocate it
 * at its definitive place. The jumper code has already been reloacted in a
 * safe zone, but the other parts of the loader could be erased.
 *
 * The jumper code perform the following operations:
 * - save the necessary information 
 * - relocate the payload
 * - clear the payload bss
 * - jump to the payload in long mode
 */


/*
 * Address of original code and data before the relocation occurs.
 * These are LD generated symbols.
 */

extern char __kernel_jumper_start;       /* First byte of the jumper section */
extern char __kernel_text_end;    /* First byte following the jumper section */


/*
 * The jumper code uses the STAR register to store the base address of
 * relocated code (since this loader does not use syscalls).
 * The value of this register is cleared before to jump to payload.
 */

static inline void set_relocation_address(paddr_t addr)
{
	uint32_t shift = addr - ((paddr_t) &__kernel_jumper_start);
	asm volatile ("wrmsr\n" : : "a" (shift), "c" (STAR));
}

static inline paddr_t get_relocation_shift(void)
{
	uint32_t shift;

	asm volatile ("rdmsr\n" : "=a" (shift) : "c" (STAR) : "edx");
	return ((paddr_t) shift);
}

#define RELOCATED(addr)  ((void*) (((char *) addr) + get_relocation_shift()))


/*
 * Saved information.
 * These are structures in the ".jumper" section which can be safely accessed
 * by __jumper_text functions. Actual structures are accessed on their
 * relocated address using the RELOCATED() macro.
 */

static __jumper_data struct mistral_header __jumper_header;

static __jumper_data vaddr_t __jumper_mb2;

static __jumper_data vaddr_t __jumper_mems;

static __jumper_data vaddr_t __jumper_meme;

static __jumper_data descriptor_t __jumper_gdt[] = {
	[JUMPER_GDT_ZERO]   = 0,
	[JUMPER_GDT_CODE64] = DESCRIPTOR(0, 0, 0x29b),
	[JUMPER_GDT_DATA]   = DESCRIPTOR(0, 0xfffff, 0xc93),
	[JUMPER_GDT_CODE32] = DESCRIPTOR(0, 0xfffff, 0xc9b)
};

#define jumper_header  ((struct mistral_header *) RELOCATED(&__jumper_header))
#define jumper_mb2     ((vaddr_t *)               RELOCATED(&__jumper_mb2))
#define jumper_mems    ((vaddr_t *)               RELOCATED(&__jumper_mems))
#define jumper_meme    ((vaddr_t *)               RELOCATED(&__jumper_meme))
#define jumper_gdt     ((descriptor_t *)          RELOCATED(&__jumper_gdt))


static __jumper_text void jumper_relocate_payload(void)
{
	struct mistral_header *header = jumper_header;
	size_t payload_size = header->load_end_addr - header->load_addr;
	size_t bss_size = header->bss_end_addr - header->load_end_addr;
	void *load_addr = (void *) header->load_addr;
	void *bss_start = (void *) header->load_end_addr;
	
	memmove(load_addr, (void *) header->payload_addr, payload_size);
	memset(bss_start, 0, bss_size);
}

extern __noreturn void jump_to_longmode(vaddr_t entry, vaddr_t mb2,
					vaddr_t mems, vaddr_t meme);

static __jumper_text __noreturn void jumper_main(void)
{
	struct mistral_header *header = jumper_header;
	uint64_t payload_vaddr = header->payload_vaddr;
	uint64_t payload_ventry = header->payload_entry_addr
		- header->load_addr + payload_vaddr;

	jumper_relocate_payload();
	jump_to_longmode(payload_ventry, *jumper_mb2, *jumper_mems,
			 *jumper_meme);
}


/*
 * Non relocated code.
 * These are the functions used by regular code to relocate the jumper code and
 * then jump to it.
 */

extern __noreturn void jump_to_relocated(paddr_t addr);

__noreturn paddr_t jump_to_payload(paddr_t addr)
{
	set_relocation_address(addr);

	*jumper_header = mistral_header;
	*jumper_mb2 = (paddr_t) this.multiboot2_addr;
	*jumper_mems = this.allocator.from;
	*jumper_meme = this.allocator.free;

	lgdt((uint32_t) jumper_gdt, ARRAY_SIZE(__jumper_gdt));
	load_ds(JUMPER_GDT_DATA);
	load_ss(JUMPER_GDT_DATA);
	load_es(JUMPER_GDT_DATA);
	load_fs(JUMPER_GDT_DATA);
	load_gs(JUMPER_GDT_DATA);
	load_cs(JUMPER_GDT_CODE32);

	jump_to_relocated((paddr_t) RELOCATED(jumper_main));
}

void relocate_jumper(paddr_t addr)
{
	memcpy((void *) addr, &__kernel_jumper_start, jumper_size());
}

size_t jumper_size(void)
{
	return &__kernel_text_end - &__kernel_jumper_start;
}
