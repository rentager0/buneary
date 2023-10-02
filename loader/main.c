#include <loader/control.h>
#include <loader/cpuid.h>
#include <loader/gcc.h>
#include <loader/jumper.h>
#include <loader/loader.h>
#include <loader/mistral.h>
#include <loader/multiboot2.h>
#include <loader/pagetable.h>
#include <loader/printk.h>
#include <loader/segment.h>
#include <loader/stdarg.h>
#include <loader/string.h>
#include <loader/types.h>
#include <loader/vga.h>


#define GDT32_ZERO   0
#define GDT32_CODE   1
#define GDT32_DATA   2


/*
 * This structure is filled by the userland mistral utility at linkage with
 * the payload. It contains all the necessary information for initialization
 * and loading.
 */

__mistral struct mistral_header  mistral_header = MISTRAL_HEADER;

struct environment     this;


static __noreturn void die(void)
{
	while (1)
		asm volatile ("hlt");
}

static void checkup(const char *format, ...)
{
	va_list ap;

	printk(":: ");
	
	va_start(ap, format);
	vprintk(format, ap);
	va_end(ap);

	printk(" : ");
}

static void success(void)
{
	printk("Success\n");
}

static __noreturn void failure(const char *format, ...)
{
	va_list ap;

	printk("Failure\n\n  -> ");

	va_start(ap, format);
	vprintk(format, ap);
	va_end(ap);

	printk("\n  -> Please report on https://github.com/gauthier-voron/"
	       "mistral\n");
	printk("  -> For more information, see the mistral documentation\n");

	die();
}


static void check_longmode(void)
{
	vendor_t vendor;
	uint32_t cr0;

	checkup("Checking hardware consistency");

	cpuid_vendor(&vendor);
	if (strncmp(vendor, CPUID_VENDOR_INTEL, sizeof (vendor_t)) &&
	    strncmp(vendor, CPUID_VENDOR_AMD, sizeof (vendor_t)))
		failure("unknown CPU vendor: %s", vendor);

	cr0 = store_cr0();
	if ((cr0 & CR0_PG) != 0)
		failure("paging should be disabled: CR0 = %08x", cr0);

	success();
}


static bool_t conflict_payload(paddr_t addr, size_t size)
{
	paddr_t payload_start = mistral_header.load_addr;
	paddr_t payload_end = mistral_header.bss_end_addr;
	paddr_t end_addr = addr + size;

	if (addr >= payload_end)
		return FALSE;
	if (payload_start >= end_addr)
		return FALSE;

	return TRUE;
}


static void relocate_multiboot2(void)
{
	const struct multiboot2_info *mb2 = this.multiboot2_addr;
	struct multiboot2_info *reloc;
	size_t size = mb2->total_size;

	if (!conflict_payload((paddr_t) mb2, size))
		return;

	checkup("Relocating multiboot2 tags   ");

	reloc = (void *) malloc_transient(size, PAGE_SIZE);
	if (reloc == 0)
		failure("not enough memory");
	
	memcpy(reloc, mb2, size);

	this.multiboot2_addr = reloc;
	success();
}

static void relocate_stack(void)
{
	paddr_t stack_addr = mistral_header.stack_addr - LOADER_STACK_SIZE;
	paddr_t reloc_addr;
	
	if (!conflict_payload(stack_addr, LOADER_STACK_SIZE)) {
		this.stack_addr = stack_addr;
		return;
	}

	checkup("Relocating loader stack      ");

	reloc_addr = malloc_transient(LOADER_STACK_SIZE, PAGE_SIZE);
	if (reloc_addr == 0)
		failure("not enough memory");

	memcpy((void *) reloc_addr, (void *) stack_addr, LOADER_STACK_SIZE);
	asm volatile ("addl %0, %%esp\n" : : "r" (reloc_addr - stack_addr));

	this.stack_addr = reloc_addr;
	success();
}

static void setup_jumper(void)
{
	checkup("Relocating jumper code       ");

	this.jumper_addr = malloc_transient(jumper_size(), PAGE_SIZE);
	if (this.jumper_addr == 0)
		failure("not enough memory");

	relocate_jumper(this.jumper_addr);

	success();
}

static void setup_paging(void)
{
	const struct multiboot2_info *mb2 = this.multiboot2_addr;
	const struct multiboot2_tag_mmap *mmap;
	const struct multiboot2_tag_mmap_entry *entry;
	size_t i, len, size, payload_size;
	vaddr_t upper;

	checkup("Building payload page table  ");

	if (!pagetable_install())
		failure("failed to install a page table");

	mmap = (void *) multiboot2_first_type(mb2, MULTIBOOT2_TYPE_MMAP);
	if (mmap == NULL)
		failure("failed to obtain multiboot2 memory map");

	len = (mmap->size - sizeof (*mmap)) / mmap->entry_size;
	size = 0;
	
	for (i = 0; i < len; i++) {
		entry = &mmap->entries[i];
		upper = entry->base_addr + entry->length;

		if (upper > ((paddr_t) -1)) {
			size = -PAGE_SIZE;
			break;
		}

		size = PAGE_ALIGN(upper);
	}
	
	if (!pagetable_map(0, 0, size))
		failure("failed to create identity mapping: 0 -> %x", size);

	payload_size = mistral_header.bss_end_addr - mistral_header.load_addr;
	if (!pagetable_map(mistral_header.payload_vaddr,
			   mistral_header.load_addr, payload_size))
		failure("failed to create payload mapping");

	success();
}


static void malloc_initialize(struct allocator *allocator)
{
	const struct multiboot2_info *mb2 = this.multiboot2_addr;
	const struct multiboot2_tag_mmap *mmap;
	const struct multiboot2_tag_mmap_entry *entry;
	size_t i, len, end;

	mmap = (void *) multiboot2_first_type(mb2, MULTIBOOT2_TYPE_MMAP);
	if (mmap == NULL)
		failure("failed to obtain multiboot2 memory map");

	len = (mmap->size - sizeof (*mmap)) / mmap->entry_size;
	for (i = 0; i < len; i++) {
		entry = &mmap->entries[i];
		end = entry->base_addr + entry->length;

		if (mistral_header.load_addr < entry->base_addr)
			continue;
		if (mistral_header.load_addr >= end)
			continue;

		allocator_initialize(allocator, entry->base_addr, end);

		reserve_zone(allocator, (paddr_t) &__kernel_text_start,
			     (paddr_t) &__kernel_bss_end);

		reserve_zone(allocator, (paddr_t) this.multiboot2_addr,
			     (paddr_t) this.multiboot2_addr + mb2->total_size);
		
		reserve_zone(allocator, mistral_header.load_addr,
			     mistral_header.bss_end_addr);

		return;
	}
	
	failure("failed to build a memory allocator");
}


static __noreturn void main(void)
{
	vga_initialize(&this.screen);
	malloc_initialize(&this.allocator);

	printk("Mistral\n");
	printk("=======\n\n");
	
	check_longmode();

	relocate_multiboot2();
	relocate_stack();
	setup_jumper();
	setup_paging();

	jump_to_payload(this.jumper_addr);
	die();
}

void __noreturn main_multiboot2(const struct multiboot2_info *mb2,
				uint32_t magic)
{
	static descriptor_t gdt32[] = {
		[GDT32_ZERO] = 0,
		[GDT32_CODE] = DESCRIPTOR(0, 0xfffff, 0xc9b),
		[GDT32_DATA] = DESCRIPTOR(0, 0xfffff, 0xc93)
	};

	if (magic != MULTIBOOT2_MAGIC_EAX)
		die();

	lgdt((uint32_t) gdt32, ARRAY_SIZE(gdt32));
	load_ds(GDT32_DATA);
	load_ss(GDT32_DATA);
	load_es(GDT32_DATA);
	load_fs(GDT32_DATA);
	load_gs(GDT32_DATA);
	load_cs(GDT32_CODE);

	this.multiboot2_addr = mb2;
	main();
}
