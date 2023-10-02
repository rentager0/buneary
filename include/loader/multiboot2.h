#ifndef _INCLUDE_LOADER_MULTIBOOT2_H_
#define _INCLUDE_LOADER_MULTIBOOT2_H_


#ifndef __ASSEMBLER__

#  include <loader/gcc.h>
#  include <loader/types.h>

#endif


#define MULTIBOOT2_MAGIC_EAX            0x36d76289

#define MULTIBOOT2_TYPE_END             0
#define MULTIBOOT2_TYPE_CMDLINE         1
#define MULTIBOOT2_TYPE_LOADER          2
#define MULTIBOOT2_TYPE_MODULE          3
#define MULTIBOOT2_TYPE_MEMORY          4
#define MULTIBOOT2_TYPE_BOOTDEV         5
#define MULTIBOOT2_TYPE_MMAP            6
#define MULTIBOOT2_TYPE_ELFSYMBS        9

#define MULTIBOOT2_MMAP_RAM             1
#define MULTIBOOT2_MMAP_ACPI            3
#define MULTIBOOT2_MMAP_RESERVED        4


#ifndef __ASSEMBLER__

struct multiboot2_tag
{
	uint32_t  type;
	uint32_t  size;
} __packed;

struct multiboot2_tag_memory
{
	uint32_t  type;
	uint32_t  size;
	uint32_t  mem_lower;
	uint32_t  mem_upper;
} __packed;

struct multiboot2_tag_bootdev
{
	uint32_t  type;
	uint32_t  size;
	uint32_t  biosdev;
	uint32_t  partition;
	uint32_t  sub_partition;
} __packed;

struct multiboot2_tag_mmap_entry
{
	uint64_t  base_addr;
	uint64_t  length;
	uint32_t  type;
	uint32_t  reserved;
} __packed;

struct multiboot2_tag_mmap
{
	uint32_t  type;
	uint32_t  size;
	uint32_t  entry_size;
	uint32_t  entry_version;
	struct multiboot2_tag_mmap_entry  entries[0];
} __packed;

struct multiboot2_tag_cmdline
{
	uint32_t  type;
	uint32_t  size;
	char      string[];
} __packed;

struct multiboot2_tag_loader
{
	uint32_t  type;
	uint32_t  size;
	char      string[];
} __packed;


struct multiboot2_info
{
	uint32_t   total_size;
	uint32_t   reserved;
	char       tags[];
} __packed;


const struct multiboot2_tag *
multiboot2_first(const struct multiboot2_info *mb2);

const struct multiboot2_tag *
multiboot2_first_type(const struct multiboot2_info *mb2, uint32_t type);

const struct multiboot2_tag *
multiboot2_next(const struct multiboot2_info *mb2,
		const struct multiboot2_tag *tag);

const struct multiboot2_tag *
multiboot2_next_type(const struct multiboot2_info *mb2,
		     const struct multiboot2_tag *tag, uint32_t type);

#endif


#endif
