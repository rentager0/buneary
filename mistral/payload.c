#include "mistral/payload.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mistral/multiboot2.h"


static int parse_multiboot2(struct payload_scheme *dest, const char *buffer)
{
	const struct multiboot2_tag *tag;
	const struct multiboot2_tag_address *hkaddr = NULL;
	const struct multiboot2_tag_entry *hentry = NULL;
	const struct multiboot2_header *header =
		(const struct multiboot2_header *) buffer;
	size_t off, len = header->header_length;

	off = sizeof (*header);
	while (off + sizeof (*tag) <= len) {
		tag = (const struct multiboot2_tag *) (buffer + off);
		if (off + tag->size > len)
			break;

		switch (tag->type) {
		case MULTIBOOT2_TAG_ADDRESS:
			dest->hkaddr_offset = dest->header_offset + off;
			hkaddr = (const struct multiboot2_tag_address *) tag;
			dest->hkaddr = *hkaddr;
			break;
		case MULTIBOOT2_TAG_ENTRY:
			dest->hentry_offset = dest->header_offset + off;
			hentry = (const struct multiboot2_tag_entry *) tag;
			dest->hentry = *hentry;
			break;
		}

		off += tag->size;
		if ((off & 0x7) != 0)
			off = (off & ~0x7) + 8;
	}

	if (hkaddr == NULL)
		return -1;
	if (hentry == NULL)
		return -1;
	return 0;
}

int parse_payload(struct payload_scheme *dest, const char *path)
{
	struct multiboot2_header header;
	char *full_header;
	int ret = -1;
	off_t off;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		goto err;

	dest->image_path = path;

	dest->header_offset = multiboot2_lseek(fd, &header, sizeof (header));
	if (dest->header_offset == -1)
		goto err_close;
	dest->header_length = header.header_length;

	full_header = malloc(header.header_length);
	if (full_header == NULL) {
		perror("");
		exit(EXIT_FAILURE);
	}

	off = multiboot2_lseek(fd, full_header, header.header_length);
	if (off == -1)
		goto err_free;

	ret = parse_multiboot2(dest, full_header);
	if (ret != 0)
		goto err_free;

	dest->load_offset = dest->header_offset
		+ dest->hkaddr.load_addr
		- dest->hkaddr.header_addr;

	dest->load_end_offset = dest->header_offset
		+ dest->hkaddr.load_end_addr
		- dest->hkaddr.header_addr;

 err_free:
	free(full_header);
 err_close:
	close(fd);
 err:
	return ret;
}

void display_payload(const struct payload_scheme *scheme)
{
	printf("image path:                    %s\n", scheme->image_path);
	printf("multiboot2 header offset:      %lx\n", scheme->header_offset);
	
	printf("multiboot2 address tag offset: %lx\n", scheme->hkaddr_offset);
	printf("  header address:              %x\n",
	       scheme->hkaddr.header_addr);
	printf("  load address:                %x\n",
	       scheme->hkaddr.load_addr);
	printf("  load end address:            %x\n",
	       scheme->hkaddr.load_end_addr);
	printf("  bss end address:             %x\n",
	       scheme->hkaddr.bss_end_addr);
	
	printf("multiboot2 entry tag offset:   %lx\n", scheme->hentry_offset);
	printf("  entry address:               %x\n",
	       scheme->hentry.entry_addr);

	printf("multiboot2 header length:      %lx\n", scheme->header_length);
}
