#ifndef _INCLUDE_LOADER_LOADER_H_
#define _INCLUDE_LOADER_LOADER_H_


#include <loader/allocator.h>
#include <loader/multiboot2.h>
#include <loader/types.h>
#include <loader/vga.h>


struct environment
{
	struct vga                     screen;
	struct allocator               allocator;
	paddr_t                        stack_addr;           /* stack bottom */
	const struct multiboot2_info  *multiboot2_addr;
	paddr_t                        jumper_addr;
	paddr_t                        pagetable_addr;
	size_t                         pagetable_size;
};


extern struct environment this;


extern char __kernel_text_start;
extern char __kernel_jumper_start;
extern char __kernel_text_end;
extern char __kernel_data_start;
extern char __kernel_data_end;
extern char __kernel_bss_end;


static inline paddr_t malloc_transient(size_t size, size_t align)
{
	return allocate_transient(&this.allocator, size, align);
}

static inline paddr_t malloc_persistant(size_t size, size_t align)
{
	return allocate_permanent(&this.allocator, size, align);
}


#endif
