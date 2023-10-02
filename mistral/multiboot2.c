#include "mistral/multiboot2.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static off_t multiboot2_search(const char *buffer, size_t len)
{
	struct multiboot2_header *header;
	uint32_t checksum;
	size_t i;

	for (i = 0; i <= len - sizeof (*header); i += MULTIBOOT2_ALIGN) {
		header = (struct multiboot2_header *) (buffer + i);

		if (header->magic != MULTIBOOT2_MAGIC)
			continue;
		if (i + header->header_length > len)
			continue;

		checksum = header->magic
			+ header->architecture
			+ header->header_length
			+ header->checksum;
		if (checksum != 0)
			continue;

		return (off_t) i;
	}

	return -1;
}

off_t multiboot2_lseek(int fd, void *dest, size_t len)
{
	struct multiboot2_header *header;
	off_t off, ret = -1;
	char *buffer;
	ssize_t rd;
	size_t rem;

	buffer = malloc(MULTIBOOT2_SEARCH);
	if (buffer == NULL)
		goto out;

	off = 0;
	rem = MULTIBOOT2_SEARCH;
	while (rem > 0) {
		rd = pread(fd, buffer, rem, off);
		if (rd == 0)
			break;
		if (rd == -1)
			goto out_free;
		off += rd;
		rem -= rd;
	}

	ret = multiboot2_search(buffer, MULTIBOOT2_SEARCH - rem);
	if ((ret != -1) && (dest != NULL)) {
		header = (struct multiboot2_header *) (buffer + ret);
		if (header->header_length < len)
			len = header->header_length;
		memcpy(dest, header, len);
	}
	
 out_free:
	free(buffer);
 out:
	return ret;
}
