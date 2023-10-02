#ifndef _INCLUDE_MISTRAL_MULTIBOOT2_H_
#define _INCLUDE_MISTRAL_MULTIBOOT2_H_


#include <stdint.h>
#include <unistd.h>


#define MULTIBOOT2_SEARCH         32768
#define MULTIBOOT2_ALIGN          8
#define MULTIBOOT2_MAGIC          0xe85250d6

#define MULTIBOOT2_TAG_ADDRESS    2
#define MULTIBOOT2_TAG_ENTRY      3


struct multiboot2_header
{
	uint32_t  magic;
	uint32_t  architecture;
	uint32_t  header_length;
	uint32_t  checksum;
} __attribute__ ((packed));

struct multiboot2_tag
{
	uint16_t  type;
	uint16_t  flags;
	uint32_t  size;
} __attribute__ ((packed));

struct multiboot2_tag_address
{
	uint16_t  type;
	uint16_t  flags;
	uint32_t  size;
	uint32_t  header_addr;
	uint32_t  load_addr;
	uint32_t  load_end_addr;
	uint32_t  bss_end_addr;
} __attribute__ ((packed));;

struct multiboot2_tag_entry
{
	uint16_t  type;
	uint16_t  flags;
	uint32_t  size;
	uint32_t  entry_addr;
} __attribute__ ((packed));;


/*
 * Search a multiboot2 header in the specified file descriptor (from offset 0)
 * and return the offset of the first matching byte of -1 if not found.
 * If not -1, reading from the returned offset can be insterpreted as a struct
 * multiboot_header.
 * If dest is not NULL, it is a buffer of the specified len and is filled with
 * the found header if any.
 */
off_t multiboot2_lseek(int fd, void *dest, size_t len);


#endif
